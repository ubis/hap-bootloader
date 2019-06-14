# HAP Bootloader

![alt text](https://api.travis-ci.org/ubis/hap-bootloader.svg?branch=develop)

HAP Bootloader is essentially the open source OpenBLT bootloader with a few modifications
for my home automation node network.

My home automation nodes use STM32F1xx series microcontrollers so all the changes are
done for STM32F1xx architecture only.

HAP Bootloader contains the following changes:
* some sort of console support, mostly printing messages and other information, input is not supported yet;
* using ee_printf implementation;
* initial "u-boot style like" boot-up information and messages;
* availability to set custom CAN RX/TX FD "on-the-fly";
* availability to hook bootloader state to determine state in main project;
* slightly different device identification: generate crc16 from unique STM32's 96bit number;
* notifying bootloader state due possible hook;
* increased boot timeout to 5s;
* switched to libopencm3 in main project(not OpenBLT core) to reduce flash/sram usage.

A few example messages, when no firmware is available:
```
tty enabled 115200,8N1
OpenBLT 2018.07-git#a24b9e5 (May 11 2019 - 19:12:08)
CPU:    STM32F10xxx ARMv7-M Cortex-M3
CLK:    72 MHz
Flash:  128 KiB
UID:    0676FF505057717067144019

CAN at 125000 bit/s initialized, ready!

Device address: 2634
Sending boot init command...

Autoboot in 5 seconds...
Preparing to boot...

Failed to verify checksum!
```

and once firmware is available:

```
tty enabled 115200,8N1
OpenBLT 2018.07-git#a24b9e5 (May 11 2019 - 19:13:06)
CPU:    STM32F10xxx ARMv7-M Cortex-M3
CLK:    72 MHz
Flash:  64 KiB
UID:    0670FF515257675187044545

CAN at 125000 bit/s initialized, ready!

Address: 973C
Sending boot init command...

Autoboot in 5 seconds...
Preparing to boot...

## Booting kernel from flash...
        Image Name:     Embedded ARM Image
        Image Type:     ARM Cortex-M3 Kernel Image
        Load Address:   08002000
        Entry Point:    08002000
        Loading Kernel Image ... OK
OK

Starting kernel ...

===> tty0,115200n8 enabled
===> running on dhp `TempSensor` hardware...
```
