import hashlib
import json
import logging
import os
import re
import sys
from pathlib import Path

from SCons.Script import Import

Import("env")

# An environment can generate its own glyph set (e.g. the bootloader, which
# can't afford the firmware's font) with, in platformio.ini:
#   custom_fonts_json = path/to/fonts.json
#   custom_fonts_output = path/to/generated
JSON_PATH = Path(env.GetProjectOption("custom_fonts_json", "tools/fonts.json"))
OUTPUT_DIR = Path(env.GetProjectOption("custom_fonts_output", "tools/generated"))
HASH_PATH = JSON_PATH.parent / ("." + JSON_PATH.name + ".sha256")


logging.basicConfig(level=logging.INFO, format="[libraster] %(message)s")
logger = logging.getLogger("libraster")


def find_generator() -> Path | None:
    """Locate libraster's tools/generator.py.

    This is needed to test local changes to the libraster without needing to push on github each time
    """
    found = next(Path(".pio").rglob("generator.py"), None)
    if found is not None:
        return found
    for link in Path(".pio").rglob("*.pio-link"):
        try:
            data = json.loads(link.read_text())
        except (OSError, ValueError):
            continue
        uri = data.get("spec", {}).get("uri") or ""
        if not uri.startswith("symlink://"):
            continue
        candidate = Path(data.get("cwd", ".")) / uri[len("symlink://"):] / "tools" / "generator.py"
        if candidate.exists():
            return candidate
    return None


def hash_file(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main() -> int:
    if not JSON_PATH.exists():
        logger.warning("fonts.json not found at %s", JSON_PATH)
        return 1

    generator = find_generator()
    if generator is None:
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
