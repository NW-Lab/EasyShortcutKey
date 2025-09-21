#!/usr/bin/env python3
"""
validate_ids.py

Checks that all entries across JSON configs have unique, stable `id`s (UUIDs).
Supports validating a directory of per-app JSONs and/or a single canonical file.

Usage:
  python3 scripts/validate_ids.py --dir config/shortcutJsons --file config/shortcuts.json
  python3 scripts/validate_ids.py --dir config/shortcutJsons
  python3 scripts/validate_ids.py --file config/shortcuts.json

Exit codes:
  0: OK
  1: Validation failure (duplicates or missing ids)
  2: Invalid inputs or unexpected error
"""

from __future__ import annotations
import argparse
import json
import sys
from pathlib import Path
from typing import Any, Dict, Iterable, List, Set, Tuple


def iter_json_files(root: Path) -> Iterable[Path]:
    if not root.exists():
        return []
    for p in sorted(root.rglob("*.json")):
        # skip schema files by name heuristic
        if p.name.lower().endswith("schema.json"):
            continue
        yield p


def load_json(path: Path) -> Any:
    with path.open("r", encoding="utf-8") as f:
        return json.load(f)


def collect_ids_from_node(node: Any, ids: List[Tuple[str, str]] , source: str) -> None:
    """
    Collect (id, source) pairs from the config structure. Accepts either array (program list)
    or single program object shape. Traverses program -> groups -> shortcuts.
    """
    def handle_program(prog: Dict[str, Any]):
        _id = prog.get("id")
        if _id:
            ids.append((_id, f"{source}:program:{prog.get('appName','<unknown>')}"))
        for grp in prog.get("groups", []) or []:
            _gid = grp.get("id")
            if _gid:
                ids.append((_gid, f"{source}:group:{grp.get('groupName','<unknown>')}"))
            for it in grp.get("shortcuts", []) or []:
                _sid = it.get("id")
                if _sid:
                    ids.append((_sid, f"{source}:shortcut:{it.get('action','<unknown>')}"))

    if isinstance(node, list):
        for prog in node:
            if isinstance(prog, dict):
                handle_program(prog)
    elif isinstance(node, dict):
        handle_program(node)


def find_missing_ids(node: Any, missing_paths: List[str], source: str) -> None:
    def handle_program(prog: Dict[str, Any]):
        if not prog.get("id"):
            missing_paths.append(f"{source}:program:{prog.get('appName','<unknown>')}")
        for grp in prog.get("groups", []) or []:
            if not grp.get("id"):
                missing_paths.append(f"{source}:group:{grp.get('groupName','<unknown>')}")
            for it in grp.get("shortcuts", []) or []:
                if not it.get("id"):
                    missing_paths.append(f"{source}:shortcut:{it.get('action','<unknown>')}")

    if isinstance(node, list):
        for prog in node:
            if isinstance(prog, dict):
                handle_program(prog)
    elif isinstance(node, dict):
        handle_program(node)


def validate(dir_path: Path | None, file_path: Path | None) -> int:
    all_ids: List[Tuple[str, str]] = []
    missing: List[str] = []

    # Directory mode
    if dir_path and dir_path.exists():
        for jf in iter_json_files(dir_path):
            try:
                data = load_json(jf)
            except Exception as e:
                print(f"ERROR: Failed to parse {jf}: {e}", file=sys.stderr)
                return 2
            collect_ids_from_node(data, all_ids, jf.as_posix())
            find_missing_ids(data, missing, jf.as_posix())

    # Single file mode
    if file_path and file_path.exists():
        try:
            data = load_json(file_path)
        except Exception as e:
            print(f"ERROR: Failed to parse {file_path}: {e}", file=sys.stderr)
            return 2
        collect_ids_from_node(data, all_ids, file_path.as_posix())
        find_missing_ids(data, missing, file_path.as_posix())

    # Check duplicates
    seen: Set[str] = set()
    dups: List[Tuple[str, List[str]]] = []
    locs: Dict[str, List[str]] = {}
    for _id, src in all_ids:
        locs.setdefault(_id, []).append(src)
    for _id, locations in locs.items():
        if len(locations) > 1:
            dups.append((_id, locations))

    ok = True
    if dups:
        ok = False
        print("ERROR: Duplicate ids detected:")
        for _id, locations in dups:
            print(f"  id={_id}")
            for loc in locations:
                print(f"    - {loc}")

    if missing:
        ok = False
        print("ERROR: Missing ids (run assign_ids.py to add UUIDs):")
        for path in missing:
            print(f"  - {path}")

    return 0 if ok else 1


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--dir", type=str, default=None, help="Directory containing per-app JSON files (e.g., config/shortcutJsons)")
    ap.add_argument("--file", type=str, default=None, help="Canonical combined JSON file (e.g., config/shortcuts.json)")
    args = ap.parse_args()

    dir_path = Path(args.dir) if args.dir else None
    file_path = Path(args.file) if args.file else None

    if not dir_path and not file_path:
        print("NOTE: No --dir or --file provided; nothing to validate.")
        return 0

    return validate(dir_path, file_path)


if __name__ == "__main__":
    sys.exit(main())
