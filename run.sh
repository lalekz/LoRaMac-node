#!/bin/bash
cd build
rm -rf *
cmake .. -DCMAKE_MODULE_PATH=/home/vladimir/Work/LoRaMac-node/cmake -DAPPLICATION=$1 -DCMAKE_TOOLCHAIN_FILE=/home/vladimir/Work/LoRaMac-node/cmake/toolchain-arm-none-eabi.cmake
make
python3 /home/vladimir/asr_lora_6601/build/scripts/tremo_loader.py -p /dev/ttyCH341USB0 -b 921600 flash 0x08000000 src/apps/$1/$1.bin
