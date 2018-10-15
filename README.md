# UWB Firmware

## Source code

* [UAV](UWB_Firmware/source/UAV/) - main logic of UAV UWB-transceiver;
* [decadriver](UWB_Firmware/source/decadriver/) - wrappers for deca registers;
* [mLibs](UWB_Firmware/source/mLibs/) - wrappers for common stm-periphery;
* [platform](UWB_Firmware/source/platform/) - wrappers for deca stm-periphery.

## Compilator

1) Download: https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads/7-2017-q4-major-1-1 ;
2) Extract;
3) Link (ln -sf) to the following files (or step 4):
* arm-none-eabi-g++
* arm-none-eabi-gcc
* arm-none-eabi-objcopy
* arm-none-eabi-size
4) Change the contents of makefiles:
* comp_path="compilator_dir/arm-none-eabi"
* sed -i -e "s/\(arm-none-eabi\).*/\1${comp_path}/" makefiles (with *.mk)

## Stlink Tools

1) Download: https://github.com/texane/stlink ;
2) Extract;
3) Run the following commands under sudo:
* apt-get install libusb-dev libusb-1.0-0-dev libusb-1.0-0 pkg-config libsgutils2-dev
* cp stlink_dir/etc/udev/rules.d/* /etc/udev/rules.d
* udevadm control --reload-rules
4) Build project:
* cd stlink
* make

## Compiling and Flashing

1) Run the following commands:
* cd UWB_Firmware/Release
* make
* stlink_dir/build/Release/st-flash erase
* stlink_dir/build/Release/st-flash write UWB_Firmware.bin 0x8000000
2) Reset target.



