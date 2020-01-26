#!/bin/bash
if [ ! -d toolchain ]; then
    echo "Decompressing Toolchain..."
    mv toolchain.tar toolchain.tar.xz
    tar xf toolchain.tar.xz
    if [ $? != 0 ]; then
        echo Please download toolchain.tar from https://www.lanzous.com/i8tqw3i
        exit 1
    fi
fi

export PATH=$PATH:/`pwd`/toolchain/Sourcery_G++_Lite/bin 
make clean
make m8b_m201_1G_config
make -j`nproc` firmware
echo "********************"
echo "tools/convert --soc m8b -s build/firmware.bin -i build/u-boot-comp.bin -o build/u-boot.bin"
echo "***Firmware Built***"
