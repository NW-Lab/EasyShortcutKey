using System;
using System.Threading.Tasks;
using Windows.Devices.Bluetooth;
using Windows.Devices.Bluetooth.GenericAttributeProfile;

namespace ResidentWin.Test
{
    /// <summary>
    /// BLE GATT Serverの動作確認用テストクラス
    /// </summary>
    public class BLETest
    {
        public static async Task RunTest()
        {
            Console.WriteLine("=== BLE GATT Server Test ===");
            Console.WriteLine();

            // 1. Bluetooth アダプタの確認
            Console.WriteLine("1. Checking Bluetooth adapter...");
            try
            {
                var adapter = await BluetoothAdapter.GetDefaultAsync();
                if (adapter == null)
                {
                    Console.WriteLine("   ❌ No Bluetooth adapter found!");
                    Console.WriteLine("   → このPCにはBluetoothアダプタがありません");
                    return;
                }

                Console.WriteLine($"   ✅ Adapter found: {adapter.DeviceId}");
                Console.WriteLine($"   - Is Central Role Supported: {adapter.IsCentralRoleSupported}");
                Console.WriteLine($"   - Is Peripheral Role Supported: {adapter.IsPeripheralRoleSupported}");
                Console.WriteLine($"   - Is LE Supported: {adapter.IsLowEnergySupported}");
                Console.WriteLine();

                if (!adapter.IsPeripheralRoleSupported)
                {
                    Console.WriteLine("   ⚠️  Peripheral Role is NOT supported!");
                    Console.WriteLine("   → このBluetoothアダプタはPeripheral(GATT Server)として動作できません");
                    Console.WriteLine("   → BLE GATT Serverは動作しない可能性が高いです");
                    Console.WriteLine();
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"   ❌ Error: {ex.Message}");
                Console.WriteLine();
            }

            // 2. GATT Service Provider の作成テスト
            Console.WriteLine("2. Testing GATT Service Provider creation...");
            try
            {
                var serviceUuid = new Guid("12345678-1234-1234-1234-123456789ABC");
                var result = await GattServiceProvider.CreateAsync(serviceUuid);

                if (result.Error != BluetoothError.Success)
                {
                    Console.WriteLine($"   ❌ Failed to create service: {result.Error}");
                    
                    switch (result.Error)
                    {
                        case BluetoothError.RadioNotAvailable:
                            Console.WriteLine("   → Bluetoothがオフになっているか、利用できません");
                            break;
                        case BluetoothError.NotSupported:
                            Console.WriteLine("   → この機能はサポートされていません");
                            break;
                        case BluetoothError.DisabledByPolicy:
                            Console.WriteLine("   → グループポリシーで無効化されています");
                            break;
                        case BluetoothError.DisabledByUser:
                            Console.WriteLine("   → ユーザーによって無効化されています");
                            break;
                        default:
                            Console.WriteLine($"   → エラーコード: {result.Error}");
                            break;
                    }
                    return;
                }

                Console.WriteLine("   ✅ Service Provider created successfully!");
                Console.WriteLine($"   - Service UUID: {serviceUuid}");
                Console.WriteLine();

                // 3. Characteristic の作成テスト
                Console.WriteLine("3. Testing Characteristic creation...");
                var charUuid = new Guid("12345678-1234-1234-1234-123456789ABD");
                var charParams = new GattLocalCharacteristicParameters
                {
                    CharacteristicProperties = GattCharacteristicProperties.Write,
                    WriteProtectionLevel = GattProtectionLevel.Plain,
                    UserDescription = "Test Characteristic"
                };

                var charResult = await result.ServiceProvider.Service.CreateCharacteristicAsync(
                    charUuid,
                    charParams
                );

                if (charResult.Error != BluetoothError.Success)
                {
                    Console.WriteLine($"   ❌ Failed to create characteristic: {charResult.Error}");
                    return;
                }

                Console.WriteLine("   ✅ Characteristic created successfully!");
                Console.WriteLine();

                // 4. アドバタイズのテスト
                Console.WriteLine("4. Testing Advertisement...");
                var advParams = new GattServiceProviderAdvertisingParameters
                {
                    IsConnectable = true,
                    IsDiscoverable = true
                };

                result.ServiceProvider.StartAdvertising(advParams);
                Console.WriteLine("   ✅ Advertisement started!");
                Console.WriteLine();

                Console.WriteLine("=== Test Result ===");
                Console.WriteLine("✅ BLE GATT Server is working!");
                Console.WriteLine();
                Console.WriteLine("📱 Try scanning from iPhone now...");
                Console.WriteLine($"   Device name: {Environment.MachineName}");
                Console.WriteLine($"   Service UUID: {serviceUuid}");
                Console.WriteLine();
                Console.WriteLine("Press any key to stop...");
                Console.ReadKey();

                result.ServiceProvider.StopAdvertising();
                Console.WriteLine("Stopped.");
            }
            catch (UnauthorizedAccessException)
            {
                Console.WriteLine("   ❌ Access Denied!");
                Console.WriteLine("   → アプリに必要な権限がありません");
                Console.WriteLine("   → app.manifestでbluetooth権限を設定してください");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"   ❌ Error: {ex.Message}");
                Console.WriteLine($"   Type: {ex.GetType().Name}");
                if (ex.InnerException != null)
                {
                    Console.WriteLine($"   Inner: {ex.InnerException.Message}");
                }
            }
        }
    }
}
