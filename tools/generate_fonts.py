import os
import hashlib
import sys
from pathlib import Path
from SCons.Script import Import

Import("env")

json_path = Path("tools/fonts.json")
hash_path = Path("tools/.fonts.json.sha256")
libgen = next(Path(".pio").rglob("generator.py"))


def hash_file(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()


if not json_path.exists():
    print("[libraster-sw] fonts.json not found")
    exit(1)
elif not hash_path.exists() or hash_file(json_path) != hash_path.read_text():
    print("[libraster] generating")
    os.system(f"{sys.executable} {libgen} --json {json_path}")
    hash_path.write_text(hash_file(json_path))
else:
    print("[libraster] fonts.json unchanged, skipping")
