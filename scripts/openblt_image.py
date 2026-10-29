Import("env")

import struct
import subprocess

# Must match the bootloader: first user sector in flashLayout[] (flash.c) and
# BOOT_FLASH_VECTOR_TABLE_CS_OFFSET in
# fenice-evo-steering-wheel-bootloader-sw/Core/Src/blt_conf.h.
APP_BASE_ADDRESS = 0x08020000
CHECKSUM_OFFSET = 0x2CC

# Word reserved for the checksum by STM32H723XG_FLASH_SHIFTED.ld, right after
# the 0x2CC-long vector table.
CHECKSUM_SYMBOL = "_openblt_checksum"


def symbol_addresses(elf_path, names):
    nm = env.subst("$OBJCOPY").replace("objcopy", "nm")
    output = subprocess.run([nm, elf_path], check=True, capture_output=True, text=True).stdout
    addresses = {}
    for line in output.splitlines():
        parts = line.split()
        if len(parts) == 3 and parts[2] in names:
            addresses[parts[2]] = int(parts[0], 16)
    return addresses


def write_openblt_checksum(source, target, env):
    """
    Writes the OpenBLT signature checksum into firmware.bin and generates firmware.srec.

    The bootloader only starts the main firmware if the two's complement of the sum of the
    first 7 vector table entries is stored at CHECKSUM_OFFSET. It writes this value itself
    when flashing over CAN, but not when the image is flashed through SWD, so it is
    precomputed here. The .srec is the file format BootCommander/MicroBoot expect.
    """
    bin_path = target[0].get_abspath()
    elf_path = env.subst("$BUILD_DIR/${PROGNAME}.elf")

    address = symbol_addresses(elf_path, {CHECKSUM_SYMBOL}).get(CHECKSUM_SYMBOL)
    if address != APP_BASE_ADDRESS + CHECKSUM_OFFSET:
        raise RuntimeError(
            "the OpenBLT checksum word (%s) is at %s, the bootloader reads it at 0x%08X: "
            "the vector table changed size, or the linker script lost the reserved word"
            % (CHECKSUM_SYMBOL, hex(address) if address is not None else "nowhere", APP_BASE_ADDRESS + CHECKSUM_OFFSET)
        )

    with open(bin_path, "rb") as f:
        image = bytearray(f.read())

    checksum = (-sum(struct.unpack_from("<7I", image, 0))) & 0xFFFFFFFF
    struct.pack_into("<I", image, CHECKSUM_OFFSET, checksum)
    with open(bin_path, "wb") as f:
        f.write(image)
    print("OpenBLT checksum 0x%08X written to %s" % (checksum, bin_path))

    srec_path = bin_path[: -len(".bin")] + ".srec"
    env.Execute(
        env.VerboseAction(
            '"$OBJCOPY" -I binary -O srec --srec-forceS3 --change-addresses 0x%08X "%s" "%s"'
            % (APP_BASE_ADDRESS, bin_path, srec_path),
            "Generating %s" % srec_path,
        )
    )


env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", write_openblt_checksum)
