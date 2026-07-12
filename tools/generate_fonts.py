import hashlib
import logging
import os
import re
import sys
from pathlib import Path

from SCons.Script import Import

Import("env")

JSON_PATH = Path("tools/fonts.json")
HASH_PATH = Path("tools/.fonts.json.sha256")
OUTPUT_DIR = Path("tools/generated")


logging.basicConfig(level=logging.INFO, format="[libraster] %(message)s")
logger = logging.getLogger("libraster")


def hash_file(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main() -> int:
    if not JSON_PATH.exists():
        logger.warning("fonts.json not found at %s", JSON_PATH)
        return 1

    try:
        generator = next(Path(".pio").rglob("generator.py"))
    except StopIteration:
        logger.warning("libraster generator.py not found under .pio/")
        return 1

    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    current_hash = hash_file(JSON_PATH)
    output_files_present = (
        OUTPUT_DIR / "raster-fonts.c").exists() and (OUTPUT_DIR / "raster-fonts.h").exists()
    cache_valid = HASH_PATH.exists() and HASH_PATH.read_text() == current_hash

    if cache_valid and output_files_present:
        logger.info("fonts.json unchanged, skipping")
        return 0

    logger.info("regenerating fonts into %s", OUTPUT_DIR)
    cmd = f"{sys.executable} {generator} --json {JSON_PATH} --output {OUTPUT_DIR}"
    if os.system(cmd) != 0:
        logger.error("font generator failed")
        return 1

    HASH_PATH.write_text(current_hash)
    return 0


if main() != 0:
    sys.exit(1)
