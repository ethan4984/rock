# Rock

rockOS is a simple Unix-Like operating system using mlibc

# Features

- x86_64 kernel
  - MM
    - vmm
    - pmm
    - slab allocator
  - FS
    - ext2
    - ramfs
    - devfs
    - vfs
  - SCHED
    - smp
    - preemptive multicore scheduler
  - DRIVERS
    - pci
    - ahci
    - nvme (coming soon)
    - e1000 (work in progrss0

# Build Instructions

Here are some programs you will need to build rock:
  - `nasm`
  - `make`
  - `qemu`
  - `tar`
  - `sed`
  - `wget`
  - `git`
  - `xbstrap`
  
Once you have those run the script `build_tools.sh` in `/tools` and then do `make build_toolchain` in `/user`

To compile and run rock, there are three default build targets available:
  - `make qemu`
    - run with regular qemu (live serial debugger)
  - `make info`
    -  run with the qemu console 
  - `make debug`
    - run with the qemu interrupt monitor

# Contributing

Contributors are very welcome, just make sure to use the same code style as me :^)
