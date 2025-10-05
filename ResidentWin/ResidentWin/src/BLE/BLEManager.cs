using System;
using System.Text;
using System.Text.Json;
using System.Threading.Tasks;
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
                    CharacteristicProperties = GattCharacteristicProperties.Write | 
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
                
                Logger.Info($"BLE advertising started with device name: {DeviceName}");

                _isRunning = true;
                UpdateConnectionState(ConnectionState.Waiting, "BLE advertising started");

                Logger.Info("BLE GATT Server started successfully");
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
            Logger.Info($"Advertisement status changed: {status}, Error: {args.Error}");

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

                _shortcutCharacteristic = null;
                _statusCharacteristic = null;
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
        /// ショートカット受信時の処理
        /// </summary>
        private async void OnShortcutWriteRequested(
            GattLocalCharacteristic sender,
            GattWriteRequestedEventArgs args)
        {
            try
            {
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
                Logger.Debug($"Received shortcut JSON: {json}");

                // JSONをデシリアライズ
                var command = JsonSerializer.Deserialize<ShortcutCommand>(json);

                if (command != null)
                {
                    // イベントを発火
                    ShortcutReceived?.Invoke(this, command);
                    
                    // 成功レスポンス
                    request.Respond();
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
                Logger.Info($"Connection state changed: {newState} - {message}");
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
