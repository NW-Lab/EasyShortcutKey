#!/usr/bin/env python3
"""
assign_ids.py

Scan JSON files under config/shortcutJsons and assign stable UUIDs to missing
`id` fields at program, group, and shortcut levels.

Usage:
  python3 scripts/assign_ids.py --dry-run
  python3 scripts/assign_ids.py --apply

Dry-run prints proposed changes; --apply writes files in-place.
"""
import argparse
import json
import uuid
from pathlib import Path


def gen_id():
    return str(uuid.uuid4())


def ensure_ids(obj):
    changed = False
    if 'id' not in obj:
        obj['id'] = gen_id()
        changed = True
    # groups
    if 'groups' in obj and isinstance(obj['groups'], list):
        for g in obj['groups']:
            if 'id' not in g:
                g['id'] = gen_id()
                changed = True
            if 'shortcuts' in g and isinstance(g['shortcuts'], list):
                for s in g['shortcuts']:
                    if 'id' not in s:
                        s['id'] = gen_id()
                        changed = True
    return changed


def process_file(p: Path, apply: bool):
    data = json.loads(p.read_text(encoding='utf-8'))
    # Expect either an array (legacy) or an object representing a single app
    changed = False
    if isinstance(data, list):
        for item in data:
            if ensure_ids(item):
                changed = True
    elif isinstance(data, dict):
        if ensure_ids(data):
            changed = True
    else:
        print(f"Skipping {p}: unexpected JSON root type: {type(data)}")
        return False

    if changed:
        print(f"Proposed changes for {p}:")
        print(json.dumps(data, ensure_ascii=False, indent=2))
        if apply:
            p.write_text(json.dumps(data, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
            print(f"Applied changes to {p}")
    else:
        print(f"No changes for {p}")
    return changed


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--dry-run', action='store_true')
    parser.add_argument('--apply', action='store_true')
    parser.add_argument('--dir', default='config/shortcutJsons')
    args = parser.parse_args()

    p = Path(args.dir)
    if not p.exists():
        print(f"Directory not found: {p}")
        return

    files = sorted(p.glob('*.json'))
    if not files:
        print(f"No JSON files found in {p}")
        return

    any_changed = False
    for f in files:
        changed = process_file(f, apply=args.apply)
        any_changed = any_changed or changed

    if any_changed and not args.apply:
        print('\nRun with --apply to write the suggested ids into files.')


if __name__ == '__main__':
    main()
