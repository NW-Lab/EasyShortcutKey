using System;
using System.Threading.Tasks;
using Windows.Devices.Bluetooth;
using Windows.Devices.Bluetooth.GenericAttributeProfile;

namespace BLETestConsole
{
    class Program
    {
        // KeyboardGWと同じUUID
        private static readonly Guid SERVICE_UUID = new Guid("12345678-1234-1234-1234-123456789ABC");
        private static readonly Guid SHORTCUT_CHARACTERISTIC_UUID = new Guid("12345678-1234-1234-1234-123456789ABD");
        private static readonly Guid STATUS_CHARACTERISTIC_UUID = new Guid("12345678-1234-1234-1234-123456789ABE");

        static async Task Main(string[] args)
        {
            Console.WriteLine("=== Windows BLE GATT Server テスト ===\n");
            
            await TestBluetoothCapabilities();
            
            Console.WriteLine("\nEnterキーで終了...");
            Console.ReadLine();
        }

        static async Task TestBluetoothCapabilities()
        {
            Console.WriteLine("■ Step 1: Bluetoothアダプタの確認");
            
            try
            {
                var adapter = await BluetoothAdapter.GetDefaultAsync();
                if (adapter == null)
                {
                    Console.ForegroundColor = ConsoleColor.Red;
                    Console.WriteLine("❌ Bluetoothアダプタが見つかりません");
                    Console.ResetColor();
                    Console.WriteLine("   → 外付けBluetoothアダプタを接続してください");
                    return;
                }

                Console.ForegroundColor = ConsoleColor.Green;
                Console.WriteLine($"✅ Bluetoothアダプタ検出: {adapter.DeviceId}");
                Console.ResetColor();
                Console.WriteLine($"   BluetoothAddress: 0x{adapter.BluetoothAddress:X}");
                Console.WriteLine($"   IsClassicSupported: {adapter.IsClassicSupported}");
                Console.WriteLine($"   IsLowEnergySupported: {adapter.IsLowEnergySupported}");
                Console.WriteLine($"   IsCentralRoleSupported: {adapter.IsCentralRoleSupported}");
                Console.WriteLine($"   IsPeripheralRoleSupported: {adapter.IsPeripheralRoleSupported}");
                Console.WriteLine($"   IsAdvertisementOffloadSupported: {adapter.IsAdvertisementOffloadSupported}");

                Console.WriteLine();
                
                if (!adapter.IsPeripheralRoleSupported)
                {
                    Console.ForegroundColor = ConsoleColor.Red;
                    Console.WriteLine("❌ このBluetoothアダプタはBLE Peripheral Roleをサポートしていません!");
                    Console.ResetColor();
                    Console.WriteLine("   → iOS端末から検出できません");
                    Console.WriteLine("   → BLE Peripheral対応のアダプタが必要です");
                    Console.WriteLine("      (Intel Wireless Bluetooth、Realtek など一部のチップのみ対応)");
                    return;
                }

                Console.ForegroundColor = ConsoleColor.Green;
                Console.WriteLine("✅ Peripheral Roleがサポートされています!");
                Console.ResetColor();
                Console.WriteLine();

                // Step 2: GATT Service Provider作成テスト
                await TestGattServiceProvider();
            }
            catch (Exception ex)
            {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine($"❌ エラーが発生しました: {ex.Message}");
                Console.ResetColor();
                Console.WriteLine($"   {ex.GetType().Name}");
            }
        }

        static async Task TestGattServiceProvider()
        {
            Console.WriteLine("■ Step 2: GATT Service Providerの作成");

            try
            {
                var serviceProviderResult = await GattServiceProvider.CreateAsync(SERVICE_UUID);

                if (serviceProviderResult.Error != BluetoothError.Success)
                {
                    Console.ForegroundColor = ConsoleColor.Red;
                    Console.WriteLine($"❌ GATT Service Providerの作成に失敗: {serviceProviderResult.Error}");
                    Console.ResetColor();
                    
                    switch (serviceProviderResult.Error)
                    {
                        case BluetoothError.RadioNotAvailable:
                            Console.WriteLine("   → Bluetoothがオフになっています");
                            break;
                        case BluetoothError.ResourceInUse:
                            Console.WriteLine("   → 既に同じサービスが起動しています");
                            break;
                        case BluetoothError.NotSupported:
                            Console.WriteLine("   → このデバイスではBLE GATT Serverがサポートされていません");
                            break;
                        case BluetoothError.DisabledByPolicy:
                            Console.WriteLine("   → ポリシーによりBluetoothが無効化されています");
                            break;
                        case BluetoothError.DisabledByUser:
                            Console.WriteLine("   → ユーザー設定でBluetoothが無効になっています");
                            break;
                    }
                    return;
                }

                var serviceProvider = serviceProviderResult.ServiceProvider;
                Console.ForegroundColor = ConsoleColor.Green;
                Console.WriteLine("✅ GATT Service Providerを作成しました");
                Console.ResetColor();
                Console.WriteLine();

                // Step 3: Characteristicの作成
                await TestCharacteristics(serviceProvider);
            }
            catch (Exception ex)
            {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine($"❌ エラーが発生しました: {ex.Message}");
                Console.ResetColor();
            }
        }

        static async Task TestCharacteristics(GattServiceProvider serviceProvider)
        {
            Console.WriteLine("■ Step 3: Characteristicの作成");

            try
            {
                // Shortcut Characteristic (Write)
                var shortcutParams = new GattLocalCharacteristicParameters
                {
                    CharacteristicProperties = GattCharacteristicProperties.Write,
                    WriteProtectionLevel = GattProtectionLevel.Plain
                };

                var shortcutResult = await serviceProvider.Service.CreateCharacteristicAsync(
                    SHORTCUT_CHARACTERISTIC_UUID,
                    shortcutParams
                );

                if (shortcutResult.Error != BluetoothError.Success)
                {
                    Console.ForegroundColor = ConsoleColor.Red;
                    Console.WriteLine($"❌ Shortcut Characteristicの作成に失敗: {shortcutResult.Error}");
                    Console.ResetColor();
                    return;
                }

                Console.ForegroundColor = ConsoleColor.Green;
                Console.WriteLine("✅ Shortcut Characteristic作成成功");
                Console.ResetColor();

                // Status Characteristic (Read + Notify)
                var statusParams = new GattLocalCharacteristicParameters
                {
                    CharacteristicProperties = GattCharacteristicProperties.Read | GattCharacteristicProperties.Notify,
                    ReadProtectionLevel = GattProtectionLevel.Plain
                };

                var statusResult = await serviceProvider.Service.CreateCharacteristicAsync(
                    STATUS_CHARACTERISTIC_UUID,
                    statusParams
                );

                if (statusResult.Error != BluetoothError.Success)
                {
                    Console.ForegroundColor = ConsoleColor.Red;
                    Console.WriteLine($"❌ Status Characteristicの作成に失敗: {statusResult.Error}");
                    Console.ResetColor();
                    return;
                }

                Console.ForegroundColor = ConsoleColor.Green;
                Console.WriteLine("✅ Status Characteristic作成成功");
                Console.ResetColor();
                Console.WriteLine();

                // Step 4: Advertisementの開始
                await TestAdvertisement(serviceProvider);
            }
            catch (Exception ex)
            {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine($"❌ エラーが発生しました: {ex.Message}");
                Console.ResetColor();
            }
        }

        static async Task TestAdvertisement(GattServiceProvider serviceProvider)
        {
            Console.WriteLine("■ Step 4: BLE Advertisementの開始");

            try
            {
                var advertisingParameters = new GattServiceProviderAdvertisingParameters
                {
                    IsConnectable = true,
                    IsDiscoverable = true,
                    // ★重要: Service UUIDをAdvertisementに含める
                    // iOSアプリがこのUUIDでフィルタリングしてスキャンしている
                    ServiceData = null  // ServiceDataは使わない
                };

                serviceProvider.StartAdvertising(advertisingParameters);

                Console.ForegroundColor = ConsoleColor.Green;
                Console.WriteLine("✅ BLE Advertisement開始!");
                Console.ResetColor();
                Console.WriteLine();
                Console.WriteLine("==============================================");
                Console.ForegroundColor = ConsoleColor.Yellow;
                Console.WriteLine("📡 BLE GATT Serverが動作しています!");
                Console.ResetColor();
                Console.WriteLine("==============================================");
                Console.WriteLine();
                Console.WriteLine($"デバイス名: {Environment.MachineName}");
                Console.WriteLine($"Service UUID: {SERVICE_UUID}");
                Console.WriteLine();
                Console.WriteLine("iPhone/iPadのBLEスキャナーアプリで検索してみてください:");
                Console.WriteLine("  - LightBlue (推奨)");
                Console.WriteLine("  - nRF Connect");
                Console.WriteLine();
                Console.ForegroundColor = ConsoleColor.Cyan;
                Console.WriteLine("💡 ヒント:");
                Console.ResetColor();
                Console.WriteLine($"  デバイス名は「{Environment.MachineName}」として表示されます");
                Console.WriteLine("  または、Service UUID で検索してください");
                Console.WriteLine();
                Console.WriteLine("このウィンドウを開いたまま、iOSアプリで検索してください。");
                Console.WriteLine("検出できたら成功です!");

                // 60秒間アドバタイズを続ける
                Console.WriteLine();
                Console.WriteLine("60秒間アドバタイズします...");
                await Task.Delay(60000);

                serviceProvider.StopAdvertising();
                Console.WriteLine();
                Console.ForegroundColor = ConsoleColor.Yellow;
                Console.WriteLine("⏹ Advertisementを停止しました");
                Console.ResetColor();
            }
            catch (Exception ex)
            {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine($"❌ エラーが発生しました: {ex.Message}");
                Console.ResetColor();
            }
        }
    }
}
