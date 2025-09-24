#!/usr/bin/env python3
"""
簡単なUSBデバイス検出ツール
KeyboardGWデバイス（M5Stack AtomS3）がUSBで認識されているかチェック
"""

import subprocess
import sys
import re

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

def check_usb_devices():
    """USBデバイスをチェック"""
    print("🔍 USB接続デバイスをチェック中...")
    print("=" * 50)
    
    try:
        # system_profilerでUSBデバイス一覧を取得
        result = subprocess.run(['system_profiler', 'SPUSBDataType'], 
                              capture_output=True, text=True, timeout=10)
        
        if result.returncode != 0:
            print_status("ERROR", "USBデバイス情報を取得できませんでした")
            return
        
        usb_info = result.stdout
        
        # M5Stack、AtomS3、ESP32、FTDIなど関連するキーワードで検索
        keywords = [
            'M5Stack', 'AtomS3', 'ESP32', 'FTDI', 'CP210x', 'CH340', 
            'Silicon Labs', 'Espressif', 'USB Serial'
        ]
        
        found_devices = []
        lines = usb_info.split('\n')
        
        for i, line in enumerate(lines):
            for keyword in keywords:
                if keyword.lower() in line.lower():
                    # 関連する情報を収集
                    device_info = {'keyword': keyword, 'line': line.strip()}
                    
                    # 前後数行で詳細情報を探す
                    context_start = max(0, i-5)
                    context_end = min(len(lines), i+5)
                    context = lines[context_start:context_end]
                    
                    for ctx_line in context:
                        if 'Product ID' in ctx_line or 'Vendor ID' in ctx_line:
                            device_info['id'] = ctx_line.strip()
                        elif 'Manufacturer' in ctx_line:
                            device_info['manufacturer'] = ctx_line.strip()
                        elif 'Location ID' in ctx_line:
                            device_info['location'] = ctx_line.strip()
                    
                    found_devices.append(device_info)
                    break
        
        if found_devices:
            print_status("OK", f"関連デバイスが {len(found_devices)} 個見つかりました:")
            for i, device in enumerate(found_devices):
                print(f"\n📱 デバイス {i+1}:")
                print(f"   キーワード: {device['keyword']}")
                print(f"   情報: {device['line']}")
                if 'id' in device:
                    print(f"   ID: {device['id']}")
                if 'manufacturer' in device:
                    print(f"   メーカー: {device['manufacturer']}")
                if 'location' in device:
                    print(f"   場所: {device['location']}")
        else:
            print_status("WARNING", "M5Stack/ESP32関連デバイスが見つかりませんでした")
        
        # シリアルポート（/dev/tty.*）もチェック
        check_serial_ports()
        
    except subprocess.TimeoutExpired:
        print_status("ERROR", "USBデバイスチェックがタイムアウトしました")
    except Exception as e:
        print_status("ERROR", f"USBデバイスチェック中にエラー: {e}")

def check_serial_ports():
    """シリアルポートをチェック"""
    print("\n" + "=" * 50)
    print("🔌 シリアルポートをチェック中...")
    
    try:
        result = subprocess.run(['ls', '/dev/tty.*'], 
                              capture_output=True, text=True)
        
        if result.returncode == 0:
            ports = result.stdout.strip().split('\n')
            
            # ESP32/M5Stack関連のポートを探す
            relevant_ports = []
            keywords = ['usbserial', 'SLAB_USBtoUART', 'wchusbserial', 'usbmodem']
            
            for port in ports:
                for keyword in keywords:
                    if keyword in port:
                        relevant_ports.append(port)
                        break
            
            if relevant_ports:
                print_status("OK", f"関連シリアルポートが見つかりました:")
                for port in relevant_ports:
                    print(f"   📡 {port}")
            else:
                print_status("INFO", f"シリアルポート総数: {len(ports)}")
                print_status("WARNING", "ESP32/M5Stack関連のシリアルポートが見つかりませんでした")
                print("   先頭5個のポート（参考）:")
                for port in ports[:5]:
                    print(f"   📡 {port}")
        else:
            print_status("WARNING", "シリアルポートが見つかりませんでした")
            
    except Exception as e:
        print_status("ERROR", f"シリアルポートチェック中にエラー: {e}")

def provide_hardware_troubleshooting():
    """ハードウェアトラブルシューティング"""
    print("\n" + "=" * 50)
    print("🔧 ハードウェアトラブルシューティング")
    print("=" * 50)
    
    print_status("INFO", "KeyboardGWデバイスのチェックリスト:")
    print("1. 💡 LEDの状態確認:")
    print("   - 青色で点滅: 起動中（正常）")
    print("   - 青色で点灯: Bluetooth広告中（正常・検出可能）")
    print("   - 緑色で点灯: デバイス接続済み")
    print("   - 赤色: エラー状態")
    print("   - 消灯: 電源未供給またはファームウェア異常")
    
    print_status("INFO", "2. 🔌 USB接続の確認:")
    print("   - USBケーブルがしっかり接続されている")
    print("   - USBポートが正常に動作している（他のデバイスで確認）")
    print("   - ケーブルが断線していない（データ通信可能なケーブル）")
    
    print_status("INFO", "3. 🔄 リセット手順:")
    print("   - USBケーブルを抜く")
    print("   - 10秒待機")
    print("   - 再度接続")
    print("   - LEDが青色で点滅→点灯することを確認")
    
    print_status("INFO", "4. 🛠 ファームウェアの確認:")
    print("   - Arduino IDEまたはPlatform IOでファームウェアが正しく書き込まれているか")
    print("   - シリアルモニターでデバッグメッセージを確認")

def main():
    print("🖥 EasyShortcutKey USBデバイスチェックツール")
    check_usb_devices()
    provide_hardware_troubleshooting()

if __name__ == "__main__":
    main()