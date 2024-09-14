SERVER := scheduler

CXX ?= g++
LD ?= ld

# Compiler and linker flags
CXXFLAGS ?= -Wall -Wextra -O3 -pipe
LDFLAGS ?=

INTERNALLDFLAGS := \
	-nostdlib \
	-zmax-page-size=0x1000 \
	-static \
	-ztext

INTERNALCXXFLAGS := \
	-I.	\
	-Ifayt/cxxshim/stage2/include \
	-std=c++23 \
	-ffreestanding \
	-fstack-protector \
	-mno-80387 \
	-mno-mmx \
	-mno-3dnow \
	-mno-sse \
	-mno-sse2

CPPFILES := $(shell find ./ -type f -name '*.cpp')
OBJ := $(CPPFILES:.cpp=.o)
HEADER_DEPS := $(CPPFILES:.cpp=.d)

.PHONY: all
all: $(SERVER)

$(SERVER): $(OBJ)
	$(LD) $(LDFLAGS) $(INTERNALLDFLAGS) $(OBJ) -n -T linker.ld -o $@

-include $(HEADER_DEPS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INTERNALCXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(SERVER) $(OBJ) $(HEADER_DEPS)

