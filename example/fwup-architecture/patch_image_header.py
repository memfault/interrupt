import argparse
import binascii
import struct


def patch_binary_payload(bin_filename):
    """
    Patch crc & data_size fields of image_hdr_t in place in binary

    Raise exception if binary is not a supported type
    """
    IMAGE_HDR_SIZE_BYTES = 32
    IMAGE_HDR_MAGIC = 0xCAFE
    IMAGE_HDR_VERSION = 1

    with open(bin_filename, "rb") as f:
        image_hdr = f.read(IMAGE_HDR_SIZE_BYTES)
        data = f.read()

    image_magic, image_hdr_version = struct.unpack("<HH", image_hdr[0:4])

    if image_magic != IMAGE_HDR_MAGIC:
        raise Exception(
            "Unsupported Binary Type. Expected 0x{:02x} Got 0x{:02x}".format(
                IMAGE_HDR_MAGIC, image_magic
            )
        )

    if image_hdr_version != IMAGE_HDR_VERSION:
        raise Exception(
            "Unsupported Image Header Version. Expected 0x{:02x} Got 0x{:02x}".format(
                IMAGE_HDR_VERSION, image_hdr_version
            )
        )

    data_size = len(data)
    crc32 = binascii.crc32(data) & 0xffffffff

    image_hdr_crc_data_size = struct.pack("<LL", crc32, data_size)
    print(
        "Adding crc:0x{:08x} data_size:{} to '{}'".format(
            crc32, data_size, bin_filename
        )
    )
    with open(bin_filename, "r+b") as f:
        # Seek to beginning of "uint32_t crc"
        f.seek(4)
        # Write correct values into crc & data_size
        f.write(image_hdr_crc_data_size)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument("bin", action="store")
    args = parser.parse_args()

    patch_binary_payload(args.bin)
