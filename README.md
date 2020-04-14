# crepOS

This is a simple x86_64 operating system made for fun and learning

![Basic demo](Demo/osdemo8.gif)

## About

  This project strives for simplicity. This project exists as a means for fun, "software from scracth" is really fun. This OS is amateur, however, here are some of the features we do include:

  - interrupts including irqs and exceptions

  - Basic mm with identity paging and a pmm

  - A basic shell with simple commands

  - Malloc & Free and panic

  - Text based graphics

  - Process allocation

  - Long mode

  This OS is only around a month old and all of the features that we currently support will be 100% improved upon and more

  Here are some project aspirations:

  - Multi-processing and multi-threading

  - apic instead of pic

  - Process scheduler

  - Virtual address spaces for individual programs

  - GUI

## Build

  This project uses a cross compiler, for obvious reasons this is required. Since this OS is 64 bit, we use the `x86_64-elf`
  variant of cross compilers. To setup this cross compiler, simply just run the `Tools/cross.sh` script. However, if you
  are feeling dangerous and dont want to wait the compile time, just edit the Makefile and change the compiler options
  and flags.

  To setup the cross compiler on Linux, you will need a few dependencies, some but not all include: mtools, xorriso, grub,
  qemu, etc

  During the build, if you find yourself getting a `Command not found` error, just put the error into google, and install
  the dependency. Its fairly easy to find missing dependencies.

  Then After everything is setup just simply run `make qemu` to build and run the OS

 (If you have already sorted out a compiler and dont need to run the `Tools/cross.sh` script then create the Bin directory
 If its not already created)
