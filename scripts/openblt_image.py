Import("env")

import struct
import subprocess

# Must match the bootloader: first user sector in flashLayout[] and
# BOOT_FLASH_VECTOR_TABLE_CS_OFFSET in
# fenice-evo-steering-wheel-bootloader-sw/Core/Src/flash.c.
APP_BASE_ADDRESS = 0x08020000
CHECKSUM_OFFSET = 0x298

# The bootloader stores the checksum inside the vector table, on the slot of
# this interrupt (the table is 0x2CC long). It is only harmless while the
# firmware never uses that interrupt, i.e. the slot still points to the
# default handler.
CHECKSUM_VECTOR = "OCTOSPI2_IRQHandler"
DEFAULT_HANDLER = "Default_Handler"


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

    addresses = symbol_addresses(elf_path, {CHECKSUM_VECTOR, DEFAULT_HANDLER})
    if addresses.get(CHECKSUM_VECTOR) != addresses.get(DEFAULT_HANDLER):
        raise RuntimeError(
            "%s is implemented, but its vector slot (offset 0x%X) holds the OpenBLT checksum: "
            "move BOOT_FLASH_VECTOR_TABLE_CS_OFFSET to the end of the vector table first"
            % (CHECKSUM_VECTOR, CHECKSUM_OFFSET)
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
