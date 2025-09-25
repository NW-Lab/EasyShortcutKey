Import("env")
from pathlib import Path
import shutil

# 出力ディレクトリ
out_dir = Path(env['PROJECT_DIR']) / 'firmware'
out_dir.mkdir(exist_ok=True)

def _write_meta():
    meta = out_dir / 'FLASH_INFO.txt'
    meta.write_text(f"""Flash write example (ESP32-S3 / 4MB flash assumption)\n\n"
    "Erase (必要なら):\n"
    "  esptool.py --chip esp32s3 --port <PORT> erase_flash\n\n"
    "Write (圧縮有効 -z):\n"
    f"  esptool.py --chip esp32s3 --port <PORT> --baud 921600 write_flash -z 0x0 KeyboardGW_{env['PIOENV']}.bin\n\n"
    "Detect chip (動作確認用):\n"
    "  esptool.py --port <PORT> --baud 115200 chip_id\n"""
    )
    print(f"[export_firmware] Wrote meta -> {meta}")

def _copy_firmware(target, source, env):
    # firmware.bin 出力後に呼ばれる (SCons は target, source, env を渡す)
    firmware_bin = Path(env['PROJECT_BUILD_DIR']) / env['PIOENV'] / 'firmware.bin'
    if firmware_bin.exists():
        target_bin = out_dir / f"KeyboardGW_{env['PIOENV']}.bin"
        shutil.copy2(firmware_bin, target_bin)
        print(f"[export_firmware] Copied firmware -> {target_bin}")
    else:
        print("[export_firmware] firmware.bin が見つからない (post action) …")
    _write_meta()

# firmware.bin 生成後にコピーするようにフック
env.AddPostAction("$BUILD_DIR/firmware.bin", _copy_firmware)
