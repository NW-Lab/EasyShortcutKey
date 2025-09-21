#!/usr/bin/env python3
"""
compare_ids.py

Compares the set of IDs between an old and a new JSON config and reports
added and removed IDs. This is helpful to treat "missing in latest" as deleted.

Usage:
  python3 scripts/compare_ids.py --old-file /tmp/base.json --new-file config/shortcuts.json

Exit codes:
  0: Ran comparison (purely informational)
  2: Invalid inputs or unexpected error
"""
from __future__ import annotations
import argparse
import json
import sys
from pathlib import Path
from typing import Any, Dict, Set


def collect_ids(node: Any) -> Set[str]:
    ids: Set[str] = set()

    def handle_program(prog: Dict[str, Any]):
        _id = prog.get("id")
        if _id:
            ids.add(_id)
        for grp in prog.get("groups", []) or []:
            _gid = grp.get("id")
            if _gid:
                ids.add(_gid)
            for it in grp.get("shortcuts", []) or []:
                _sid = it.get("id")
                if _sid:
                    ids.add(_sid)

    if isinstance(node, list):
        for prog in node:
            if isinstance(prog, dict):
                handle_program(prog)
    elif isinstance(node, dict):
        handle_program(node)

    return ids


def load_json(path: Path) -> Any:
    with path.open("r", encoding="utf-8") as f:
        return json.load(f)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--old-file", required=True, help="Old (base) JSON file path")
    ap.add_argument("--new-file", required=True, help="New (head) JSON file path")
    args = ap.parse_args()

    oldp = Path(args.old_file)
    newp = Path(args.new_file)
    if not oldp.exists() or not newp.exists():
        print("compare_ids: file not found", file=sys.stderr)
        return 2

    try:
        old = load_json(oldp)
        new = load_json(newp)
    except Exception as e:
        print(f"compare_ids: failed to parse json: {e}", file=sys.stderr)
        return 2

    old_ids = collect_ids(old)
    new_ids = collect_ids(new)

    removed = sorted(old_ids - new_ids)
    added = sorted(new_ids - old_ids)

    print("ID changes compared to base:")
    print(f"  removed: {len(removed)}")
    for i in removed:
        print(f"    - {i}")
    print(f"  added:   {len(added)}")
    for i in added:
        print(f"    + {i}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
