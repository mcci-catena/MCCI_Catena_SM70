# MCCI Catena SM70

This library implements RS485 communications with an [Aeroqual](https://www.aeroqual.com) SM70 sensor.

[![GitHub release](https://img.shields.io/github/release/mcci-catena/MCCI_Catena_SM70/all.svg)](https://github.com/mcci-catena/MCCI_Catena_SM70/releases/latest) ![GitHub commits](https://img.shields.io/github/commits-since/mcci-catena/MCCI_Catena_SM70/latest.svg)

**Contents:**

<!--
  This TOC uses the VS Code markdown TOC extension AlanWalk.markdown-toc.
  We strongly recommend updating using VS Code, the markdown-toc extension and the
  bierner.markdown-preview-github-styles extension.  Note that if you are using
  VS Code 1.29 and Markdown TOC 1.5.6, https://github.com/AlanWalk/markdown-toc/issues/65
  applies -- you must change your line-ending to some non-auto value in Settings>
  Text Editor>Files.  `\n` works for me.
-->

<!-- markdownlint-disable MD033 MD004 -->
<!-- markdownlint-capture -->
<!-- markdownlint-disable -->
<!-- TOC depthFrom:2 updateOnSave:true -->

- [Terminology](#terminology)
- [Library Contents](#library-contents)
	- [Files](#files)
- [Installation Procedure](#installation-procedure)
- [Known Issues](#known-issues)
- [To-Do List](#to-do-list)
- [The `Sm70Serial<>` Template Class](#the-sm70serial-template-class)
- [Meta](#meta)
	- [License](#license)
	- [Contributors](#contributors)
	- [Support Open Source Hardware and Software](#support-open-source-hardware-and-software)
	- [Trademarks](#trademarks)

<!-- /TOC -->
<!-- markdownlint-restore -->
<!-- Due to a bug in Markdown TOC, the table is formatted incorrectly if tab indentation is set other than 4. Due to another bug, this comment must be *after* the TOC entry. -->

## Terminology

Modbus literature uses "host" and "slave". Time has moved on, so in this library, we use the terms "initiator" and "responder".

## Library Contents

### Files

This list is not exhaustive, but highlights some of the more important files.

File | Description
-----|------------
`LICENSE.md` | GNU License file
`keywords.txt` | Arduino IDE coloring syntax
`documentation/*` | Library documentation generated with Doxygen.
`examples/*` | Sample sketches to implement miscellaneous settings.
`src/MCCI_Catena_SM70.h` | The library header file
`src/lib/MCCI_Catena_SM70.cpp` | The main source file for the library.

## Installation Procedure

Refer to this documentation to install this library:

[`arduino.cc/en/Guide/Libraries`](https://arduino.cc/en/Guide/Libraries)

Starting with IDE version 1.0.5, you can install 3rd party libraries in the IDE.

Do not unzip the downloaded library, leave it as is.

In the Arduino IDE, navigate to Sketch > Import Library. At the top of the drop down list, select the option to "Add Library".

You will be prompted to select this zipped library.

Return to the Sketch > Import Library menu. You should now see the library at the bottom of the drop-down menu. It is ready to be used in your sketch.

The zip file will have been expanded in the libraries folder in your Arduino sketches directory.

Note: the library will be available to use in sketches, but examples for the library will not be exposed in the `File > Examples` until after the IDE has restarted.

## Known Issues

TBD.

## To-Do List

TBD.

## The `Sm70Serial<>` Template Class

We want this library to compile with `SoftwareSerial` and `USBSerial` ports. A polymorphic pointer is needed at the top level to the "port" -- although `Serial`, `UART`, `USBSerial`, and `SoftwareSerial` share a common interface, there is no common abstract class in the standard Arduino library; and their methods aren't virtual.

This library introduces the `cSerialPort` class, which has all the proper abstract semantics, and the `cSerial<T>` template class, which maps the abstract semantics of `cSerialPort` onto the concrete semantics of `T` (whatever `T` happens to be; provided that `T` conforms to the Serial API).

Declaring an SM70 instance takes two steps (which can be done in any order, provided you add suitable declarations).

1. Declare the port connection object:

   ```c++
   // declare sm70Serial as a cSerial<> object
   // and map it onto actual port Serial2:
   cSerial<decltype(Serial2)> sm70Serial(&Serial2);
   ```

   This declares an object named `sm70Serial`, which implements `cSerial` and is a wrapper for serial port `Serial2`.

   Of course, if you know that the type of `Serial2` is `UART`, you can also write:

   ```c++
   // declare modbusSerial as a ModbusSerial<> object
   // and map it onto Serial1:
   ModbusSerial<UART> modbusSerial(&Serial2);
   ```

   We tend to prefer the former form, as it makes the examples more portable.

2. Declare a `cSM70` instance that represents the sensor:

   ```c++
   cSM70 gSm70(&sm70Serial, kTxPin, kRxPin);
   ```

   This declares a variable named `gSm70`, which represents an SM70 device. `kTxPin`, if non-zero, specifies the pin to be used to enable/disable TX. `kRxPin`, if non-zero, specifies the pin to be used to enable/disable RX.

## Meta

### License

This repository is released under the [LGPL 2.1](./LICENSE.md) license.

### Contributors

Terry Moore did the basic architecture. Dhinesh Kumar completed the implementation and debugged the code.

### Support Open Source Hardware and Software

MCCI invests time and resources providing this open source code, please support MCCI and open-source hardware by purchasing products from MCCI, Adafruit and other open-source hardware/software vendors!

For information about MCCI's products, please visit [mcci.com](https://mcci.com/) and [store.mcci.com](https://store.mcci.com/).

### Trademarks

MCCI and MCCI Catena are registered trademarks of MCCI Corporation. LoRaWAN is a registered trademark of the LoRa Alliance. LoRa is a registered trademark of Semtech Corporation. All other marks are the property of their respective owners.
