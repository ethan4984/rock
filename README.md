# crepOS

This is a simple x86_64 operating system made for fun and learning

![Basic demo](Demo/osdemo8.gif)

## About

  This project strives for simplicity. This project exists as a means for fun, "software from scratch" is really fun. This OS is amateur, however, here are some of the features we do include:

  - interrupts including irqs and exceptions

  - Basic mm with identity paging and a pmm
  
  - Multitasking

  - A basic shell with simple commands

  - Malloc & Free and panic

  - Text based graphics

  - Process allocation

  - Long mode

  This OS is only around a month old and all of the features that we currently support will be 100% improved upon and more

  Here are some project aspirations:

  - apic and smp

  - Virtual address spaces for individual programs

  - File system and disk driver

  - GUI

## Build

  This project uses a cross compiler, for obvious reasons this is required. Since this OS is 64 bit, we use the `x86_64-elf`
  variant of cross compilers. To setup this cross compiler, simply just run the `Tools/setup.sh` script. Within `setup.sh`, it also
  installs and sets up qloader2 and echfs

  To setup the cross compiler on Linux, you will need a few dependencies, some but not all include: mtools, xorriso, grub,
  qemu, etc

  During the build, if you find yourself getting a `Command not found` error as a result of a missing dependency,
  just put the error into google, and install the dependency. Its fairly easy to find missing dependencies.

  Then After everything is setup just simply run `make qemu` to build and run the OS
