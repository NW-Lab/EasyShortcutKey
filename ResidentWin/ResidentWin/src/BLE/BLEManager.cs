using System;
using System.Text;
using System.Text.Json;
using System.Threading.Tasks;
using System.Linq;
using System.Collections.Generic;
using System.Threading;
using Windows.Devices.Bluetooth;
using Windows.Devices.Bluetooth.GenericAttributeProfile;
using Windows.Storage.Streams;
using ResidentWin.Models;
using ResidentWin.Utils;

namespace ResidentWin.BLE
{
    /// <summary>
    /// BLE GATT サーバーとして動作し、iPhoneからの接続を受け付ける
    /// </summary>
    public class BLEManager
    {
        // KeyboardGWと同じUUID
        private static readonly Guid ServiceUuid = new Guid("12345678-1234-1234-1234-123456789ABC");
        private static readonly Guid ShortcutCharUuid = new Guid("12345678-1234-1234-1234-123456789ABD");
        private static readonly Guid StatusCharUuid = new Guid("12345678-1234-1234-1234-123456789ABE");

        // デバイス名 (KeyboardGWと同じ)
        private const string DeviceName = "EasyShortcutKey-GW";

        private GattServiceProvider? _serviceProvider;
        private GattLocalCharacteristic? _shortcutCharacteristic;
        private GattLocalCharacteristic? _statusCharacteristic;

        private ConnectionState _connectionState = ConnectionState.Disconnected;
        private bool _isRunning = false;
    private DateTime _lastActivityUtc = DateTime.UtcNow;
    private System.Threading.Timer? _idleTimer;
    private readonly TimeSpan _idleTimeout = TimeSpan.FromSeconds(25); // iOS が生きていれば何かしら来る想定
    private readonly TimeSpan _idleCheckInterval = TimeSpan.FromSeconds(5);
    private bool _shuttingDown = false;

        // JSONオプション（大小文字無視 + コメント/末尾カンマ耐性）
        private static readonly JsonSerializerOptions _jsonOptions = new JsonSerializerOptions
        {
            PropertyNameCaseInsensitive = true,
            ReadCommentHandling = JsonCommentHandling.Skip,
            AllowTrailingCommas = true
        };

        /// <summary>
        /// 接続状態が変化したときのイベント
        /// </summary>
        public event EventHandler<ConnectionStateChangedEventArgs>? ConnectionStateChanged;

        /// <summary>
        /// ショートカットコマンドを受信したときのイベント
        /// </summary>
        public event EventHandler<ShortcutCommand>? ShortcutReceived;

        /// <summary>
        /// BLEサーバーを開始
        /// </summary>
        public async Task<bool> StartAsync()
        {
            try
            {
                Logger.Info("Starting BLE GATT Server...");

                // GATTサービスプロバイダーを作成
                var result = await GattServiceProvider.CreateAsync(ServiceUuid);

                if (result.Error != BluetoothError.Success)
                {
                    Logger.Error($"Failed to create GATT service provider: {result.Error}");
                    UpdateConnectionState(ConnectionState.Error, $"Failed to create service: {result.Error}");
                    return false;
                }

                _serviceProvider = result.ServiceProvider;

                // Shortcut Characteristic (WRITE, NOTIFY)
                // AtomS3版と同じく暗号化なし (Plain) で動作
                var shortcutCharParams = new GattLocalCharacteristicParameters
                {
                    // iOS側は payload サイズによって .withResponse / .withoutResponse を切替えるため
                    // WriteWithoutResponse も許可しておく。
                    CharacteristicProperties = GattCharacteristicProperties.Write |
                                              GattCharacteristicProperties.WriteWithoutResponse |
                                              GattCharacteristicProperties.Notify,
                    WriteProtectionLevel = GattProtectionLevel.Plain,
                    UserDescription = "Shortcut Command"
                };

                var shortcutCharResult = await _serviceProvider.Service.CreateCharacteristicAsync(
                    ShortcutCharUuid,
                    shortcutCharParams
                );

                if (shortcutCharResult.Error != BluetoothError.Success)
                {
                    Logger.Error($"Failed to create shortcut characteristic: {shortcutCharResult.Error}");
                    return false;
                }

                _shortcutCharacteristic = shortcutCharResult.Characteristic;
                _shortcutCharacteristic.WriteRequested += OnShortcutWriteRequested;
                _shortcutCharacteristic.SubscribedClientsChanged += OnSubscribedClientsChanged;

                // Status Characteristic (READ, NOTIFY)
                // AtomS3版と同じく暗号化なし (Plain) で動作
                var statusCharParams = new GattLocalCharacteristicParameters
                {
                    CharacteristicProperties = GattCharacteristicProperties.Read | 
                                              GattCharacteristicProperties.Notify,
                    ReadProtectionLevel = GattProtectionLevel.Plain,
                    UserDescription = "Connection Status"
                };

                var statusCharResult = await _serviceProvider.Service.CreateCharacteristicAsync(
                    StatusCharUuid,
                    statusCharParams
                );

                if (statusCharResult.Error != BluetoothError.Success)
                {
                    Logger.Error($"Failed to create status characteristic: {statusCharResult.Error}");
                    return false;
                }

                _statusCharacteristic = statusCharResult.Characteristic;
                _statusCharacteristic.ReadRequested += OnStatusReadRequested;
                _statusCharacteristic.SubscribedClientsChanged += OnSubscribedClientsChanged;

                // アドバタイズパラメータを設定
                var advParameters = new GattServiceProviderAdvertisingParameters
                {
                    IsConnectable = true,
                    IsDiscoverable = true
                };

                // ServiceDataを追加 (これによりiPhoneから発見しやすくなる)
                _serviceProvider.AdvertisementStatusChanged += OnAdvertisementStatusChanged;

                // アドバタイズ開始
                _serviceProvider.StartAdvertising(advParameters);
                
                Logger.Info($"BLE advertising started (device name: {DeviceName})");

                _isRunning = true;
                _lastActivityUtc = DateTime.UtcNow;
                _idleTimer = new System.Threading.Timer(CheckIdle, null, _idleCheckInterval, _idleCheckInterval);
                UpdateConnectionState(ConnectionState.Waiting, "BLE advertising started");

                Logger.Info("BLE GATT Server started successfully");
                Logger.Debug("BLEManager version stamp: v1-fallback-fix");
                return true;
            }
            catch (Exception ex)
            {
                Logger.Error("Failed to start BLE server", ex);
                UpdateConnectionState(ConnectionState.Error, $"Exception: {ex.Message}");
                return false;
            }
        }

        /// <summary>
        /// アドバタイズステータス変更時の処理
        /// </summary>
        private void OnAdvertisementStatusChanged(GattServiceProvider sender, GattServiceProviderAdvertisementStatusChangedEventArgs args)
        {
            var status = args.Status;
            Logger.Debug($"Advertisement status changed: {status}, Error: {args.Error}");

            if (status == GattServiceProviderAdvertisementStatus.Started)
            {
                UpdateConnectionState(ConnectionState.Waiting, "BLE advertising active");
            }
            else if (status == GattServiceProviderAdvertisementStatus.Stopped)
            {
                if (args.Error != BluetoothError.Success)
                {
                    Logger.Error($"Advertisement stopped with error: {args.Error}");
                    UpdateConnectionState(ConnectionState.Error, $"Advertisement error: {args.Error}");
                }
            }
        }

        /// <summary>
        /// BLEサーバーを停止
        /// </summary>
        public void Stop()
        {
            try
            {
                Logger.Info("Stopping BLE GATT Server...");

                if (_serviceProvider != null)
                {
                    _serviceProvider.StopAdvertising();
                    _serviceProvider.AdvertisementStatusChanged -= OnAdvertisementStatusChanged;
                    _serviceProvider = null;
                }

                if (_shortcutCharacteristic != null)
                {
                    _shortcutCharacteristic.WriteRequested -= OnShortcutWriteRequested;
                    _shortcutCharacteristic.SubscribedClientsChanged -= OnSubscribedClientsChanged;
                    _shortcutCharacteristic = null;
                }
                if (_statusCharacteristic != null)
                {
                    _statusCharacteristic.ReadRequested -= OnStatusReadRequested;
                    _statusCharacteristic.SubscribedClientsChanged -= OnSubscribedClientsChanged;
                    _statusCharacteristic = null;
                }
                _idleTimer?.Dispose();
                _idleTimer = null;
                _isRunning = false;

                UpdateConnectionState(ConnectionState.Disconnected, "BLE server stopped");
                Logger.Info("BLE GATT Server stopped");
            }
            catch (Exception ex)
            {
                Logger.Error("Failed to stop BLE server", ex);
            }
        }

        /// <summary>
        /// クライアントが Notify を有効/無効にした際に呼ばれる。
        /// (iOS 側が CCCD 書き込み→サブスクライブした瞬間を接続成立扱いにする)
        /// </summary>
        private void OnSubscribedClientsChanged(GattLocalCharacteristic sender, object args)
        {
            try
            {
                var count = sender.SubscribedClients?.Count ?? 0;
                Logger.Debug($"SubscribedClientsChanged: characteristic={sender.Uuid} subscribedCount={count}");

                RecordActivity("subscribe");

                if (count > 0 && _connectionState != ConnectionState.Connected)
                {
                    UpdateConnectionState(ConnectionState.Connected, "Client subscribed notifications");
                }
                else if (count == 0 && _connectionState == ConnectionState.Connected)
                {
                    // すべてのクライアントが退いた場合は切断扱い (軽量ロジック)
                    UpdateConnectionState(ConnectionState.Disconnected, "All clients unsubscribed");
                }
            }
            catch (Exception ex)
            {
                Logger.Warning($"Error in OnSubscribedClientsChanged: {ex.Message}");
            }
        }

        private void RecordActivity(string source)
        {
            _lastActivityUtc = DateTime.UtcNow;
            Logger.Debug($"Activity: {source}");
        }

        private void CheckIdle(object? state)
        {
            if (_shuttingDown) return;
            try
            {
                // サブスクライバ監視: イベント飛ばない環境がある場合にポーリングで補正
                if (_connectionState != ConnectionState.Connected)
                {
                    var scCount = _shortcutCharacteristic?.SubscribedClients?.Count ?? 0;
                    var stCount = _statusCharacteristic?.SubscribedClients?.Count ?? 0;
                    if (scCount + stCount > 0)
                    {
                        Logger.Info($"Subscribed client detected by polling (shortcut={scCount}, status={stCount}) -> mark Connected");
                        UpdateConnectionState(ConnectionState.Connected, "Client subscription detected by poll");
                        RecordActivity("poll-detected-subscribe");
                    }
                }

                var idle = DateTime.UtcNow - _lastActivityUtc;
                if (_connectionState == ConnectionState.Connected && idle > _idleTimeout)
                {
                    Logger.Info($"Idle timeout ({idle.TotalSeconds:F0}s) -> marking disconnected");
                    UpdateConnectionState(ConnectionState.Disconnected, "Idle timeout");
                }
            }
            catch (Exception ex)
            {
                Logger.Warning($"Idle check error: {ex.Message}");
            }
        }

        /// <summary>
        /// アプリ終了前にステータスを通知（iOS側が受け取れば即座に『切断』扱いできる）
        /// </summary>
        public async Task SendShutdownNoticeAsync()
        {
            try
            {
                if (!_isRunning || _statusCharacteristic == null) return;
                _shuttingDown = true;
                var payload = new
                {
                    state = "ShuttingDown",
                    message = "server_exiting",
                    timestamp = DateTime.UtcNow.ToString("o")
                };
                var json = JsonSerializer.Serialize(payload);
                var writer = new DataWriter();
                writer.WriteString(json);
                await _statusCharacteristic.NotifyValueAsync(writer.DetachBuffer());
                Logger.Info("Sent shutdown notice via status notify");
            }
            catch (Exception ex)
            {
                Logger.Warning($"Failed to send shutdown notice: {ex.Message}");
            }
        }

        /// <summary>
        /// ショートカット受信時の処理
        /// </summary>
        private async void OnShortcutWriteRequested(
            GattLocalCharacteristic sender,
            GattWriteRequestedEventArgs args)
        {
            try
            {
                Logger.Debug("[BLE] Shortcut write handler enter (v1-fallback-fix+diagnostic)");
                var deferral = args.GetDeferral();

                var request = await args.GetRequestAsync();
                if (request == null)
                {
                    Logger.Warning("Write request is null");
                    deferral.Complete();
                    return;
                }

                // データを読み取る
                var reader = DataReader.FromBuffer(request.Value);
                var bytes = new byte[reader.UnconsumedBufferLength];
                reader.ReadBytes(bytes);

                var json = Encoding.UTF8.GetString(bytes);
                var hex = string.Join("", bytes.Select(b => b.ToString("X2")));
                Logger.Debug($"[BLE] Payload len={bytes.Length} HEX={hex}");
                Logger.Debug($"[BLE] Raw='{json}'");

                // JSONをデシリアライズ（大小文字無視）
                ShortcutCommand? command = null;
                try
                {
                    command = JsonSerializer.Deserialize<ShortcutCommand>(json, _jsonOptions);
                }
                catch (Exception jex)
                {
                    Logger.Error("JSON primary deserialization failed", jex);
                }

                // クライアントが実際にCharacteristicへアクセス=論理的に接続状態とみなす
                if (_connectionState != ConnectionState.Connected)
                {
                    UpdateConnectionState(ConnectionState.Connected, "Client activity (write)");
                }

                // Fallback: commandがnull もしくは Keys未取得の場合は手動抽出
                if (command == null || (command.Keys == null || command.Keys.Count == 0))
                {
                    try
                    {
                        using var doc = JsonDocument.Parse(json);
                        List<string>? found = TryExtractKeysRecursive(doc.RootElement);
                        if (found != null && found.Count > 0)
                        {
                            if (command == null) command = new ShortcutCommand();
                            command.Keys = found;
                            Logger.Debug($"[BLE] Fallback extracted keys (recursive): [{string.Join(",", found)}]");
                        }
                        else
                        {
                            Logger.Debug("[BLE] Recursive fallback could not find a non-empty 'keys' string array");
                        }
                    }
                    catch (Exception fallbackEx)
                    {
                        Logger.Warning($"Fallback key extraction failed: {fallbackEx.Message}");
                    }
                }

                if (command != null)
                {
                    if (command.Keys == null || command.Keys.Count == 0)
                    {
                        Logger.Warning("ShortcutCommand parsed but Keys is null/empty"); // keep warning (real issue)
                    }
                    // イベントを発火
                    ShortcutReceived?.Invoke(this, command);
                    
                    // 成功レスポンス
                    try
                    {
                        request.Respond();
                    }
                    catch (Exception respEx)
                    {
                        Logger.Warning($"Failed to respond to write request: {respEx.Message}");
                    }
                }
                else
                {
                    Logger.Warning("Failed to deserialize shortcut command");
                    request.RespondWithProtocolError(GattProtocolError.InvalidPdu);
                }

                deferral.Complete();
            }
            catch (Exception ex)
            {
                Logger.Error("Error handling shortcut write request", ex);
            }
        }

        /// <summary>
        /// 再帰的に 'keys' (大小文字無視) 配列(string要素) を探す
        /// </summary>
        private static List<string>? TryExtractKeysRecursive(JsonElement element)
        {
            switch (element.ValueKind)
            {
                case JsonValueKind.Object:
                    foreach (var prop in element.EnumerateObject())
                    {
                        if (string.Equals(prop.Name, "keys", StringComparison.OrdinalIgnoreCase) && prop.Value.ValueKind == JsonValueKind.Array)
                        {
                            var list = prop.Value.EnumerateArray()
                                .Where(e => e.ValueKind == JsonValueKind.String)
                                .Select(e => e.GetString())
                                .Where(s => !string.IsNullOrWhiteSpace(s))
                                .ToList();
                            if (list.Count > 0) return list!;
                        }
                        // 再帰探索
                        var deeper = TryExtractKeysRecursive(prop.Value);
                        if (deeper != null && deeper.Count > 0) return deeper;
                    }
                    break;
                case JsonValueKind.Array:
                    foreach (var item in element.EnumerateArray())
                    {
                        var deeper = TryExtractKeysRecursive(item);
                        if (deeper != null && deeper.Count > 0) return deeper;
                    }
                    break;
            }
            return null;
        }

        /// <summary>
        /// ステータス読み取り時の処理
        /// </summary>
        private async void OnStatusReadRequested(
            GattLocalCharacteristic sender,
            GattReadRequestedEventArgs args)
        {
            try
            {
                var deferral = args.GetDeferral();

                var request = await args.GetRequestAsync();
                if (request == null)
                {
                    deferral.Complete();
                    return;
                }

                // 現在の接続状態をJSON形式で返す
                var status = new
                {
                    state = _connectionState.ToString(),
                    timestamp = DateTime.Now.ToString("o")
                };

                // ステータス読み取りが来た段階でも接続成立と見なす (iOS側がReadしてくるケース用)
                if (_connectionState != ConnectionState.Connected)
                {
                    UpdateConnectionState(ConnectionState.Connected, "Client activity (read)");
                }

                var json = JsonSerializer.Serialize(status);
                var writer = new DataWriter();
                writer.WriteString(json);

                request.RespondWithValue(writer.DetachBuffer());

                deferral.Complete();
            }
            catch (Exception ex)
            {
                Logger.Error("Error handling status read request", ex);
            }
        }

        /// <summary>
        /// ステータスを通知
        /// </summary>
        public async Task NotifyStatusAsync(string message)
        {
            if (_statusCharacteristic == null || !_isRunning)
                return;

            try
            {
                var status = new
                {
                    state = _connectionState.ToString(),
                    message = message,
                    timestamp = DateTime.Now.ToString("o")
                };

                var json = JsonSerializer.Serialize(status);
                var writer = new DataWriter();
                writer.WriteString(json);

                await _statusCharacteristic.NotifyValueAsync(writer.DetachBuffer());
                Logger.Debug($"Status notified: {message}");
            }
            catch (Exception ex)
            {
                Logger.Error("Failed to notify status", ex);
            }
        }

        /// <summary>
        /// 接続状態を更新
        /// </summary>
        private void UpdateConnectionState(ConnectionState newState, string? message = null)
        {
            if (_connectionState != newState)
            {
                _connectionState = newState;
                Logger.Debug($"Connection state changed: {newState} - {message}");
                ConnectionStateChanged?.Invoke(this, new ConnectionStateChangedEventArgs(newState, message));
            }
        }

        /// <summary>
        /// 現在の接続状態を取得
        /// </summary>
        public ConnectionState GetConnectionState()
        {
            return _connectionState;
        }

        /// <summary>
        /// 実行中かどうか
        /// </summary>
        public bool IsRunning => _isRunning;
    }
}
