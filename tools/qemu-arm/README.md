# Test "arm" version of cryptodb

## Prerequirements

Buildroot system requirements: https://buildroot.org/downloads/manual/manual.html#requirement

## Obtain qemu machines

### Common

$ ./install.sh

### ARM32

1. cd buildroot-arm/output/images/
2. ./start-qemu.sh (user: root, without password)
3. scp needed files from **host-username**@10.0.2.2:**path-to-files**/build
4. test what you need
5. CTRL+C to exit

### ARM32 (Hard Float)

1. cd buildroot-armhf/output/images/
2. ./start-qemu.sh (user: root, without password)
3. scp needed files from **host-username**@10.0.2.2:**path-to-files**/build
4. test what you need
5. CTRL+C to exit

### ARM64

1. cd buildroot-arm64/output/images/
2. ./start-qemu.sh (user: root, without password)
3. scp needed files from **host-username**@10.0.2.2:**path-to-files**/build
4. test what you need
5. CTRL+A and then X to exit

## Manual buildroot building

### Architectures defconfigs

For ARM32:

BUILDROOT_DEFCONFIG = qemu_arm_versatile_defconfig

For ARM32 (Hard Float):

BUILDROOT_DEFCONFIG = qemu_arm_vexpress_defconfig

For ARM64:

BUILDROOT_DEFCONFIG = qemu_aarch64_virt_defconfig

### Steps to test in QEMU

1. wget https://buildroot.org/downloads/buildroot-2023.11.1.tar.gz
2. tar xf buildroot-2023.11.1.tar.gz
3. cd buildroot-2023.11.1
4. unset LD_LIBRARY_PATH
5. make BUILDROOT_DEFCONFIG
6. make menuconfig
    1. Target packages -> Networking applications -> openssh
    2. Save -> OK -> Exit -> Exit -> Exit -> Exit
7. make -j$(nproc)
8. cd output/images/
9. ./start-qemu.sh (user: root, without password)
