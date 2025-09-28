#!/usr/bin/env python3
"""
iOS BLE検出問題の詳細診断ツール
LEDが青で点灯、USBでキーボード認識済みだが、iOSアプリで検出できない場合の診断
"""

import subprocess
import sys
import time

def print_status(status, message):
    if status == "OK":
        print(f"✅ {message}")
    elif status == "WARNING":
        print(f"⚠️  {message}")
    elif status == "ERROR":
        print(f"❌ {message}")
    else:
        print(f"ℹ️  {message}")

def check_ble_interference():
    """BLE干渉をチェック"""
    print("🔍 Bluetooth環境とBLE干渉のチェック")
    print("=" * 50)
    
    try:
        # macOSのBluetooth接続済みデバイス一覧
        result = subprocess.run(['system_profiler', 'SPBluetoothDataType'], 
                              capture_output=True, text=True, timeout=10)
        
        if result.returncode == 0:
            bluetooth_info = result.stdout
            
            # 接続済みデバイス数をカウント
            connected_count = bluetooth_info.count("Connected: Yes")
            paired_count = bluetooth_info.count("Paired: Yes")
            
            print_status("INFO", f"Bluetooth接続済みデバイス数: {connected_count}")
            print_status("INFO", f"Bluetoothペアリング済みデバイス数: {paired_count}")
            
            if connected_count > 5:
                print_status("WARNING", "多数のBluetoothデバイスが接続されています（BLE干渉の可能性）")
                
            # EasyShortcutKey-GWがペアリング済みリストにあるかチェック
            if "EasyShortcutKey-GW" in bluetooth_info:
                print_status("WARNING", "EasyShortcutKey-GWが既にペアリング済みの可能性があります")
                print_status("INFO", "システム設定 > Bluetooth で一度デバイスを削除してみてください")
            else:
                print_status("OK", "EasyShortcutKey-GWはペアリングリストにありません")
                
        else:
            print_status("ERROR", "Bluetooth情報を取得できませんでした")
            
    except Exception as e:
        print_status("ERROR", f"Bluetoothチェック中にエラー: {e}")

def analyze_ble_issues():
    """BLE関連の問題を分析"""
    print("\n🔍 BLE検出問題の可能性分析")
    print("=" * 50)
    
    print_status("INFO", "現在の症状:")
    print("  - ✅ USBでキーボードとして認識")
    print("  - ✅ LEDが青色で点灯（BLE待機中）")
    print("  - ❌ iOSアプリでデバイスが検出されない")
    
    print("\n" + "="*30 + " 考えられる原因 " + "="*30)
    
    causes = [
        {
            "title": "🎯 原因1: BLE広告パケットの問題",
            "description": "ファームウェアがBLE広告を正常に送信していない",
            "solutions": [
                "デバイスを一度リセット（USB抜き差し）",
                    "PlatformIO でファームウェアを再ビルド・書き込み",
                    "pio device monitor でシリアルログ(BLE初期化メッセージ)を確認"
            ]
        },
        {
            "title": "🎯 原因2: macOSのBluetoothキャッシュ問題", 
            "description": "macOS側のBLEスキャンが正常に動作していない",
            "solutions": [
                "Bluetoothを一度オフ→オンにする",
                "macOSの再起動",
                "Bluetooth設定をリセット（Option+クリック）"
            ]
        },
        {
            "title": "🎯 原因3: iOS側のBLE権限問題",
            "description": "iOSアプリがBluetooth使用許可を得られていない",
            "solutions": [
                "設定 > プライバシー > Bluetooth でアプリの許可を確認",
                "アプリを完全に終了して再起動",
                "iPhoneの再起動"
            ]
        },
        {
            "title": "🎯 原因4: BLE UUIDフィルタリング問題",
            "description": "iOSアプリのスキャンフィルターが適切に動作していない",
            "solutions": [
                "アプリで「すべてのBLEデバイスを表示」オプションがあるか確認",
                "UUID設定の再確認",
                "フィルターなしスキャンの実施"
            ]
        },
        {
            "title": "🎯 原因5: デバイス名の問題",
            "description": "BLE広告でのデバイス名が正しく設定されていない",
            "solutions": [
                "Config.hのBLE_DEVICE_NAMEを確認",
                "ファームウェア再コンパイル・書き込み"
            ]
        }
    ]
    
    for i, cause in enumerate(causes, 1):
        print(f"\n{cause['title']}")
        print(f"説明: {cause['description']}")
        print("解決方法:")
        for j, solution in enumerate(cause['solutions'], 1):
            print(f"  {j}. {solution}")

def provide_step_by_step_solution():
    """段階的な解決手順"""
    print("\n🔧 段階的トラブルシューティング手順")
    print("=" * 50)
    
    steps = [
        {
            "step": 1,
            "title": "デバイスリセット（簡単）",
            "actions": [
                "KeyboardGWのUSBケーブルを抜く",
                "10秒待機",
                "再度接続し、LEDが青色で点灯することを確認",
                "iOSアプリで「デバイスを検索」を実行"
            ],
            "expected": "これで解決する可能性: 30%"
        },
        {
            "step": 2, 
            "title": "iOS側のリセット（中程度）",
            "actions": [
                "EasyShortcutKeyアプリを完全に終了",
                "設定 > プライバシーとセキュリティ > Bluetooth でアプリ許可を確認",
                "iPhoneを再起動",
                "アプリを起動してデバイス検索"
            ],
            "expected": "これで解決する可能性: 40%"
        },
        {
            "step": 3,
            "title": "macOS Bluetoothリセット（中程度）", 
            "actions": [
                "システム設定 > Bluetooth を開く",
                "Bluetoothをオフにする",
                "10秒待機後、再度オンにする",
                "iOSアプリでデバイス検索を実行"
            ],
            "expected": "これで解決する可能性: 25%"
        },
        {
            "step": 4,
            "title": "ファームウェア確認・再書き込み（上級）",
            "actions": [
                    "PlatformIOでファームウェア再書き込み",
                "シリアルモニターでBLE初期化ログを確認",
                "「BLE initialization completed」メッセージを確認",
                "再度iOSアプリでデバイス検索"
            ],
            "expected": "これで解決する可能性: 90%"
        }
    ]
    
    for step in steps:
        print(f"\n📋 ステップ{step['step']}: {step['title']}")
        print(f"期待値: {step['expected']}")
        print("手順:")
        for i, action in enumerate(step['actions'], 1):
            print(f"  {i}. {action}")

def main():
    print("🔍 iOS BLE検出問題 - 詳細診断ツール")
    print("=" * 60)
    print("対象: LEDが青色点灯、USB接続済み、だがiOSで検出されない")
    print("=" * 60)
    
    check_ble_interference()
    analyze_ble_issues()
    provide_step_by_step_solution()
    
    print("\n" + "=" * 60)
    print("🎯 推奨する最初のアクション:")
    print("1. デバイスのUSB抜き差しリセット")
    print("2. iPhoneのBluetooth設定確認")
    print("3. ファームウェアログの確認（可能であれば）")
    print("=" * 60)

if __name__ == "__main__":
    main()