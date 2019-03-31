# Composite USB Device library

This project implements a platform-independent, highly flexible USB Device software framework,
which allows you to create a full-feature USB 2.0 device firmware 
with multiple independent interfaces.

## Features

* Effective compliance to USB 2.0 specification
* Interfaces are independent of the device and can be added or removed in runtime
* Interface classes support multiple instantiation
* All USB descriptors are created internally (no need for user definition)
* Code size optimized for resource-constrained systems
* Platform-independent stack
* A console interface template provides zero-effort implementation for standard I/O through a CDC serial port

### Supported device classes

* Communications Device Class (**CDC** - ACM) specification version 1.10
* Network Control Model (CDC - **NCM**) specification version 1.0
* Human Interface Device Class (**HID**) specification version 1.11 - with helper macros for report definition
* Mass Storage Class Bulk-Only Transport (**MSC** - BOT) revision 1.0 with transparent SCSI command set
* Device Firmware Upgrade Class (**DFU**) specification version 1.1
  (or DFU STMicroelectronics Extension [(DFUSE)][DFUSE] 1.1A
  using `USBD_DFU_ST_EXTENSION` compile switch)

## Contents

The project consists of the followings:
* The USB 2.0 device framework is located in the **Device** folder.
* Common USB classes are implemented as part of the project, under the **Class** folder.
* The *Templates* folder contains `usbd_config.h` configuration file and various example files.
* The *Doc* folder contains a prepared *doxyfile* for Doxygen documentation generation.

## Platform support

Currently the following hardware platforms are supported:
- STMicroelectronics [STM32][STM32] using the [STM32_XPD][STM32_XPD] peripheral drivers
 or the STM32CubeMX package with [this wrapper project][USBDevice4Cube]

## Basis of operation

The interface implementations are completely separated from the USB device control.
Each of them should use its class-specific API from `usbd_<class>.h`.
There are only two steps to mount an interface to a device:

1. Setting the interface's endpoint addresses;
2. Cross-referencing the interface and the device with a `USBD_<CLASS>_MountInterface()` call.

The interfaces are added to the device configuration in the order of the mount calls.
It is possible to change the active interfaces during runtime by unmounting all and mounting the new ones.
The only requirement is that the device has to be logically disconnected from the host when it is done.

The device control of the library is limited to the global state management
using the public API in *usbd.h*. The bulk of the device operation is servicing
the device peripheral events:
- USB Reset signal on bus -> `USBD_ResetCallback()`
- USB control pipe setup request received -> `USBD_SetupCallback()`
- USB endpoint data transfer completed -> `USBD_EpInCallback()` or `USBD_EpOutCallback()`

The USBD handles are used as a shared management structure for both this stack
and the peripheral driver. Any additional fields that the peripheral driver requires
can be defined in the driver-specific *usbd_pd_def.h* header, while the *usbd_types.h* shall
be included by the driver.

## Example Projects

### [IP over USB][IPoverUSB]

A virtual network with a single lwIP server (DNS, DHCP, HTTP) is presented by the device
(as a network adapter). Composite USB device demonstrating the CDC-NCM function usage
and a reduced DFU interface to enter ROM bootloader.

### [DfuBootloader][DfuBootloader]

A generic USB device bootloader firmware for STM32 controllers.
USB device with a single DFU interface
which is mountable on both the bootloader's and the application's device stack.

### [DebugDongle][DebugDongle]

A debug serial port with selectable output power and battery charging.
Composite USB device with one CDC (serial port),
two HID interfaces (onboard sensors and power management)
and the bootloader's DFU interface.

### [CanDybug][CanDybug]

A CAN bus gateway which uses a custom protocol over a USB serial port emulation.
Composite USB device with CDC-ACM function and the bootloader's DFU interface.

## How to contribute

This project is free to use for everyone as long as the license terms are met. 
Any found defects or suggestions should be reported as a GitHub issue.
Improvements in the form of pull requests are also welcome.

## Authors

* **Benedek Kupper** - [IntergatedCircuits](https://github.com/IntergatedCircuits)

[CanDybug]: https://github.com/IntergatedCircuits/CanDybugFW
[DebugDongle]: https://github.com/IntergatedCircuits/DebugDongleFW
[DfuBootloader]: https://github.com/IntergatedCircuits/DfuBootloader
[DFUSE]: http://www.st.com/resource/en/application_note/cd00264379.pdf
[IPoverUSB]: https://github.com/IntergatedCircuits/IPoverUSB
[STM32]: http://www.st.com/en/microcontrollers/stm32-32-bit-arm-cortex-mcus.html
[STM32_XPD]: https://github.com/IntergatedCircuits/STM32_XPD
[USBDevice4Cube]: https://github.com/IntergatedCircuits/USBDevice4Cube
