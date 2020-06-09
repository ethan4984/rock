CXX = ~/opt/cross/bin/x86_64-elf-g++

NORMAL_CXXFLAGS = -ffreestanding -Wall -O2 -Wextra -fno-stack-protector -fno-exceptions -I../ -I.
KERNEL_CRAP = -fno-pic -mno-sse -mno-sse2 -mno-mmx -mno-80387 -mno-red-zone -gdwarf -mcmodel=kernel -fno-omit-frame-pointer -fno-threadsafe-statics
CXXFLAGS = $(NORMAL_CXXFLAGS) $(KERNEL_CRAP)

SCRCXX = $(shell find . -type f -name '*.cpp')

all:
	$(CXX) $(CXXFLAGS) -c $(SCRCXX) 
	mv *.o ../Bin

