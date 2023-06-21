FROM ubuntu:20.04@sha256:ca5534a51dd04bbcebe9b23ba05f389466cf0c190f1f8f182d7eea92a9671d00

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update -y
RUN apt-get install -y qemu-system-aarch64 fdisk wget mtools xz-utils

WORKDIR /qemu

# Download the image
RUN wget https://downloads.raspberrypi.org/raspios_arm64/images/raspios_arm64-2023-05-03/2023-05-03-raspios-bullseye-arm64.img.xz
ENV IMAGE_FILE=2023-05-03-raspios-bullseye-arm64.img

# Uncompress the image
RUN xz -d ${IMAGE_FILE}.xz

# Resize the image to next power of two
RUN CURRENT_SIZE=$(stat -c%s "${IMAGE_FILE}") && \
    NEXT_POWER_OF_TWO=$(python3 -c "import math; \
                                    print(2**(math.ceil(math.log(${CURRENT_SIZE}, 2))))") && \
    qemu-img resize "${IMAGE_FILE}" "${NEXT_POWER_OF_TWO}"

# Extract files from the image
# First, find the offset and size of the FAT32 partition
RUN OFFSET=$(fdisk -lu ${IMAGE_FILE} | awk '/^Sector size/ {sector_size=$4} /FAT32 \(LBA\)/ {print $2 * sector_size}') && \
    # Check that the offset is not empty
    if [ -z "$OFFSET" ]; then \
        echo "Error: FAT32 not found in disk image" && \
        exit 1; \
    fi && \
    # Setup mtools config to extract files from the partition
    echo "drive x: file=\"${IMAGE_FILE}\" offset=${OFFSET}" > ~/.mtoolsrc

# Copy out the kernel and device tree
RUN mcopy x:/bcm2710-rpi-3-b-plus.dtb . && \
    mcopy x:/kernel8.img .

# Set up SSH
# RPI changed default password policy, there is no longer default password
RUN mkdir -p /tmp && \
    # First create ssh file to enable ssh
    touch /tmp/ssh && \
    # Then create userconf file to set default password (raspberry)
    echo 'pi:$6$rBoByrWRKMY1EHFy$ho.LISnfm83CLBWBE/yqJ6Lq1TinRlxw/ImMTPcvvMuUfhQYcMmFnpFXUPowjy2br1NA0IACwF9JKugSNuHoe0' | tee /tmp/userconf

# Copy the files onto the image
RUN mcopy /tmp/ssh x:/ && \
    mcopy /tmp/userconf x:/

EXPOSE 2222

# Start qemu with SSH port forwarding
ENTRYPOINT qemu-system-aarch64 -machine raspi3b -cpu cortex-a72 -nographic -dtb bcm2710-rpi-3-b-plus.dtb -m 1G -smp 4 -kernel kernel8.img -sd ${IMAGE_FILE} -append "rw earlyprintk loglevel=8 console=ttyAMA0,115200 dwc_otg.lpm_enable=0 root=/dev/mmcblk0p2 rootdelay=1" -device usb-net,netdev=net0 -netdev user,id=net0,hostfwd=tcp::2222-:22
