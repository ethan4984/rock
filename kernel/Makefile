CXX = g++

CXX_FLAGS = -I. \
			-Ilib \
			-Wall \
			-Wextra \
			-ffreestanding \
			-fno-stack-protector \
			-fno-exceptions \
			-mno-sse \
			-mno-sse2 \
			-mno-80387 \
			-mno-red-zone \
			-gdwarf \
			-fno-omit-frame-pointer \
			-fno-pic \
			-std=c++20 \
			-fno-use-cxa-atexit \
			-fno-rtti \
			-mcmodel=kernel \
			-fno-builtin \

LINK_FLAGS = -nostartfiles \
			 -nodefaultlibs \
			 -ffreestanding \
			 -fno-builtin \
			 -nostdlib \
			 -no-pie

CXX_SRC = $(shell find . -type f -name '*.cpp')
ASM_SRC = $(shell find . -type f -name '*.asm')
REAL_SRC = $(shell find . -type f -name '*.real')

OBJ = $(CXX_SRC:.cpp=.o) $(ASM_SRC:.asm=.o)
BINS = $(REAL_SRC:.real=.bin)

KERNEL = $(shell pwd)/rock.elf

all: $(KERNEL)

$(KERNEL): $(BINS) $(OBJ)
	$(CXX) $(LINK_FLAGS) $(OBJ) -T linker.ld -o $@

%.bin: %.real
	nasm -fbin $< -o $@

%.o: %.cpp
	$(CXX) $(CXX_FLAGS) -c $< -o $@

%.o: %.asm
	nasm -felf64 -g $< -o $@

clean:
	rm -rf $(OBJ) $(KERNEL) $(BINS)
