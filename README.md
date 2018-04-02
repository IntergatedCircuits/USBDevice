# Composite USB Device library

This project implements a platform-independent, highly flexible USB Device software framework,
which allows you to create a full-feature full or high speed USB device firmware 
with multiple independent interfaces,
compliant to the USB 2.0 specification.
It is designed for resource-constrained systems, but it is a completely hardware-independent stack.

## Contents

The project consists of the followings:
1. The USB 2.0 device framework is located in the **Device** folder.
2. Common USB classes are implemented as part of the project, under the **Class** folder.
3. The *Templates* folder contains example files which are mandatory for the library's functionality.
4. The *Doc* folder contains a prepared *doxyfile* for Doxygen documentation generation.

## Class support

Currently the following USB device classes are supported:
- Communications Device Class (CDC) specification version 1.10
- Human Interface Device Class (HID) specification version 1.11
- Mass Storage Class (MSC) Bulk-Only Transport revision 1.0 with transparent SCSI command set

## Platform support

Currently the following hardware platforms are supported:
- STMicroelectronics [STM32](http://www.st.com/en/microcontrollers/stm32-32-bit-arm-cortex-mcus.html)
using the [STM32_XPD](https://github.com/IntergatedCircuits/STM32_XPD) peripheral drivers
(a [standalone](https://github.com/IntergatedCircuits/USBDevice/wiki/Integration-for-STM32-without-XPD) 
solution is also possible)

## Basis of operation

This stack on one side is called by the user application to start or stop the device
using the public API in *usbd.h*. On the other side the stack shall be notified when any of 
these device peripheral events occur:
- USB Reset signal on bus -> `USBD_ResetCallback()`
- USB control pipe setup request received -> `USBD_SetupCallback()`
- USB endpoint data transfer completed -> `USBD_EpInCallback()` or `USBD_EpOutCallback()`

The goal of the USBD structures is to be a common management structure for both this stack 
and the peripheral driver. Any additional fields that the peripheral driver necessitates
can be defined in the driver-specific *usbd_pd_def.h* header, while the *usbd_types.h* shall
be included by the driver.

## Projects using USBDevice

### [DebugDongle](https://github.com/IntergatedCircuits/DebugDongleFW)

A debug serial port with selectable output power and battery charging. Composite USB device with one CDC (serial port) and two HID interfaces (onboard sensors and power management).

## How to contribute

This project is free to use for everyone as long as the license terms are met. 
Any found defects or suggestions should be reported as a GitHub issue.
Improvements in the form of pull requests are also welcome.

## Authors

* **Benedek Kupper** - [IntergatedCircuits](https://github.com/IntergatedCircuits)
