#!/usr/bin/env python3
"""
EasyShortcutKey デバイス検出デバッグツール

このスクリプトは以下をチェックします：
1. Bluetoothの状態確認
2. 周辺デバイスのスキャン
3. KeyboardGWデバイスの検出
4. 設定ファイルの整合性確認
"""

import sys
import os
import json
import subprocess
import platform
from pathlib import Path

def print_header(text):
    """セクションヘッダーを表示"""
    print(f"\n{'='*50}")
    print(f" {text}")
    print(f"{'='*50}")

def print_status(status, message):
    """ステータスメッセージを表示"""
    if status == "OK":
        print(f"✅ {message}")
    elif status == "WARNING":
        print(f"⚠️  {message}")
    elif status == "ERROR":
        print(f"❌ {message}")
    else:
        print(f"ℹ️  {message}")

def check_system_bluetooth():
    """システムのBluetooth状態をチェック"""
    print_header("システムBluetooth状態")
    
    system = platform.system()
    
    if system == "Darwin":  # macOS
        try:
            # system_profilerを使ってBluetooth情報を取得
            result = subprocess.run(['system_profiler', 'SPBluetoothDataType'], 
                                  capture_output=True, text=True, timeout=10)
            if result.returncode == 0:
                print_status("OK", "macOS Bluetoothシステムが利用可能です")
                if "State: On" in result.stdout or "状態: オン" in result.stdout:
                    print_status("OK", "Bluetoothがオンになっています")
                else:
                    print_status("WARNING", "Bluetoothがオフになっている可能性があります")
            else:
                print_status("WARNING", "Bluetooth情報を取得できませんでした")
                
            # bleutilidを使ってスキャンを試行（可能な場合）
            print_status("INFO", "BLEスキャンを試行中...")
            
        except subprocess.TimeoutExpired:
            print_status("ERROR", "Bluetoothステータス取得がタイムアウトしました")
        except Exception as e:
            print_status("ERROR", f"Bluetoothチェック中にエラー: {e}")
    
    elif system == "Linux":
        # Linuxでのhcitoolチェック
        try:
            result = subprocess.run(['hciconfig'], capture_output=True, text=True)
            if result.returncode == 0:
                print_status("OK", "Linux Bluetoothシステムが利用可能です")
            else:
                print_status("ERROR", "Bluetoothアダプターが見つからないか、権限が不足しています")
        except FileNotFoundError:
            print_status("WARNING", "hciconfigコマンドが見つかりません（bluez-utilsをインストールしてください）")
        except Exception as e:
            print_status("ERROR", f"Bluetoothチェック中にエラー: {e}")
    
    else:
        print_status("INFO", f"現在のOS（{system}）でのBluetoothチェックは未サポートです")

def check_config_files():
    """設定ファイルの整合性をチェック"""
    print_header("設定ファイル整合性チェック")
    
    base_dir = Path(__file__).parent.parent
    
    # KeyboardGW Config.h
    keyboardgw_config = base_dir / "KeyboardGW" / "Config.h"
    ios_manager = base_dir / "iOS" / "EasyShortcutKey" / "KeyboardGWManager.swift"
    
    if keyboardgw_config.exists():
        print_status("OK", f"KeyboardGW設定ファイルが存在: {keyboardgw_config}")
        try:
            with open(keyboardgw_config, 'r', encoding='utf-8') as f:
                content = f.read()
                
            # UUIDの抽出
            service_uuid = None
            device_name = None
            
            for line in content.split('\n'):
                if 'BLE_SERVICE_UUID' in line and '"' in line:
                    service_uuid = line.split('"')[1]
                elif 'BLE_DEVICE_NAME' in line and '"' in line:
                    device_name = line.split('"')[1]
            
            if service_uuid and device_name:
                print_status("OK", f"デバイス名: {device_name}")
                print_status("OK", f"サービスUUID: {service_uuid}")
            else:
                print_status("WARNING", "設定ファイルからUUIDまたはデバイス名を読み取れませんでした")
                
        except Exception as e:
            print_status("ERROR", f"KeyboardGW設定ファイル読み取りエラー: {e}")
    else:
        print_status("ERROR", f"KeyboardGW設定ファイルが見つかりません: {keyboardgw_config}")
    
    # iOS Manager.swift
    if ios_manager.exists():
        print_status("OK", f"iOS設定ファイルが存在: {ios_manager}")
        try:
            with open(ios_manager, 'r', encoding='utf-8') as f:
                content = f.read()
            
            # UUIDの確認
            if "12345678-1234-1234-1234-123456789ABC" in content:
                print_status("OK", "iOSアプリのUUID設定が正しいです")
            else:
                print_status("WARNING", "iOSアプリのUUID設定に問題がある可能性があります")
                
        except Exception as e:
            print_status("ERROR", f"iOS設定ファイル読み取りエラー: {e}")
    else:
        print_status("WARNING", f"iOS設定ファイルが見つかりません: {ios_manager}")

def check_device_visibility():
    """BLEデバイスの可視性をチェック"""
    print_header("BLEデバイス検出テスト")
    
    print_status("INFO", "BLEスキャンを15秒間実行します...")
    print_status("INFO", "KeyboardGWデバイスがUSBに接続され、青いLEDが点灯していることを確認してください")
    
    # Pythonでのbleコマンド実行を試行
    system = platform.system()
    
    if system == "Darwin":  # macOS
        print_status("INFO", "macOSでのBLEスキャンを試行...")
        try:
            # bleutilidライブラリが利用可能かチェック
            result = subprocess.run([sys.executable, '-c', 'import bleak; print("bleak available")'], 
                                  capture_output=True, text=True)
            if result.returncode == 0:
                print_status("OK", "bleakライブラリが利用可能です")
                run_ble_scan()
            else:
                print_status("WARNING", "bleakライブラリが利用不可（pip install bleakで インストール可能）")
                suggest_manual_check()
        except Exception as e:
            print_status("ERROR", f"BLEスキャン中にエラー: {e}")
            suggest_manual_check()
    else:
        suggest_manual_check()

def run_ble_scan():
    """BLEスキャンを実行"""
    try:
        # bleakを使用したスキャン
        scan_code = '''
import asyncio
from bleak import BleakScanner

async def scan():
    print("BLEデバイスをスキャン中...")
    devices = await BleakScanner.discover(timeout=15.0)
    
    easyshortucut_devices = []
    all_devices = []
    
    for device in devices:
        all_devices.append({
            "name": device.name or "Unknown",
            "address": device.address,
            "rssi": device.rssi
        })
        
        if device.name and ("EasyShortcutKey" in device.name or "shortcut" in device.name.lower()):
            easyshortucut_devices.append(device)
    
    print(f"\\n発見されたBLEデバイス総数: {len(all_devices)}")
    
    if easyshortucut_devices:
        print("\\n✅ EasyShortcutKey関連デバイスが見つかりました:")
        for device in easyshortucut_devices:
            print(f"  - 名前: {device.name}")
            print(f"    アドレス: {device.address}")
            print(f"    信号強度: {device.rssi}dBm")
    else:
        print("\\n❌ EasyShortcutKey関連デバイスは見つかりませんでした")
        print("\\n検出されたその他のBLEデバイス（参考）:")
        for device in sorted(all_devices, key=lambda x: x["rssi"], reverse=True)[:10]:
            print(f"  - {device['name']} ({device['rssi']}dBm)")

if __name__ == "__main__":
    asyncio.run(scan())
'''
        
        result = subprocess.run([sys.executable, '-c', scan_code], 
                              capture_output=True, text=True, timeout=20)
        print(result.stdout)
        if result.stderr:
            print(f"警告: {result.stderr}")
            
    except subprocess.TimeoutExpired:
        print_status("ERROR", "BLEスキャンがタイムアウトしました")
    except Exception as e:
        print_status("ERROR", f"BLEスキャン実行エラー: {e}")

def suggest_manual_check():
    """手動確認方法を提案"""
    print_status("INFO", "手動確認方法:")
    print("1. KeyboardGWデバイスがPCのUSBポートに正しく接続されているか確認")
    print("2. デバイスのLEDが青色で点灯していることを確認")
    print("3. iPhoneの設定 > Bluetooth でデバイス一覧を確認")
    print("4. 'EasyShortcutKey-GW' という名前のデバイスが表示されるか確認")

def provide_troubleshooting():
    """トラブルシューティングガイドを表示"""
    print_header("トラブルシューティングガイド")
    
    print_status("INFO", "デバイスが見つからない場合:")
    print("1. 🔌 KeyboardGWの電源状態を確認")
    print("   - USBケーブルで正しく接続されているか")
    print("   - LEDが青色で点灯しているか（広告中）")
    print("   - LEDが緑色の場合は他のデバイスと接続済み")
    
    print_status("INFO", "2. 📱 iPhone/iPad側の確認")
    print("   - Bluetoothがオンになっているか")
    print("   - アプリにBluetooth使用許可が与えられているか")
    print("   - 他のBLEデバイスが多数接続されていないか")
    
    print_status("INFO", "3. 🔧 KeyboardGWのリセット方法")
    print("   - USBケーブルを抜いて10秒待つ")
    print("   - 再度接続してLEDが青色で点灯することを確認")
    
    print_status("INFO", "4. 🍎 iOSアプリ側のリセット")
    print("   - アプリを完全に終了して再起動")
    print("   - iPhone自体を再起動")
    print("   - 必要に応じてアプリを再インストール")

def main():
    """メイン実行関数"""
    print("🔍 EasyShortcutKey デバイス検出デバッグツール")
    print("=" * 50)
    
    # システムチェック
    check_system_bluetooth()
    
    # 設定ファイルチェック
    check_config_files()
    
    # デバイス検出テスト
    check_device_visibility()
    
    # トラブルシューティング
    provide_troubleshooting()
    
    print_header("診断完了")
    print_status("INFO", "問題が解決しない場合は、上記のトラブルシューティングを順番に試してください")

if __name__ == "__main__":
    main()