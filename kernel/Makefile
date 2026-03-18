ifndef TARGET_COMPILE
$(error TARGET_COMPILE not set)
endif

TARGET=kpimg

CC = $(TARGET_COMPILE)gcc
LD = $(TARGET_COMPILE)ld
AS = $(TARGET_COMPILE)as
OBJCOPY = $(TARGET_COMPILE)objcopy

CFLAGS += -Wall -fno-builtin -std=gnu11 -nostdinc
CFLAGS += -g

# Detect architecture
ARCH := $(shell $(CC) -dumpmachine | grep -o 'arm[^-]*' | head -1)
ifeq ($(findstring aarch64,$(ARCH)),aarch64)
    CFLAGS += -mgeneral-regs-only
    ARCH_ARM64 := 1
else ifeq ($(findstring arm,$(ARCH)),arm)
    CFLAGS += -mthumb -march=armv7-a -mfloat-abi=soft
ARCH_ARMv7 := 1
endif

ifdef DEBUG
	CFLAGS += -DDEBUG -DMAP_DEBUG -g
endif

ifdef ANDROID
	CFLAGS += -DANDROID
endif

ifdef ARCH_ARMv7
INCLUDE := -I. -Iinclude -Ipatch/include -Ilinux -Ilinux/include -Ilinux/arch/arm/include
else
INCLUDE := -I. -Iinclude -Ipatch/include -Ilinux -Ilinux/include -Ilinux/arch/arm64/include -Ilinux/tools/arch/arm64/include
endif

BASE_SRCS += base/setup.c
ifdef ARCH_ARMv7
BASE_SRCS += base/arm/setup1_arm.S
BASE_SRCS += base/arm/cache_arm.S
BASE_SRCS += base/arm/map1_arm.S
else
BASE_SRCS += base/setup1.S
BASE_SRCS += base/cache.S
BASE_SRCS += base/map1.S
endif
BASE_SRCS += base/tlsf.c
BASE_SRCS += base/start.c
BASE_SRCS += base/map.c
BASE_SRCS += base/hook.c
BASE_SRCS += base/fphook.c
BASE_SRCS += base/hotpatch.c
BASE_SRCS += base/hmem.c
BASE_SRCS += base/predata.c
BASE_SRCS += base/symbol.c
BASE_SRCS += base/baselib.c
BASE_SRCS += base/sha256.c

BASE_SRCS += $(wildcard patch/*.c)
BASE_SRCS += $(wildcard patch/common/*.c)
BASE_SRCS += $(wildcard patch/module/*.c)
BASE_SRCS += $(wildcard patch/ksyms/*.c)

ifdef ANDROID
	BASE_SRCS += $(wildcard patch/android/*.c)
endif

SRCS += $(BASE_SRCS)
SRCS += $(LINUX_SRCS)

OBJS := $(SRCS:.c=.o)
OBJS := $(OBJS:.S=.o)

all: hdr ${TARGET}

${TARGET}: ${TARGET}.elf
	${OBJCOPY} -O binary -S $^ $@

LDSCRIPT := $(if $(ARCH_ARMv7),kpimg_arm.lds,kpimg.lds)

${TARGET}.elf: ${OBJS}
	${LD} -nostdlib -static -no-pie -T$(LDSCRIPT) -o $@ $^

%.o: %.c
	${CC} $(CFLAGS) $(INCLUDE) -c -O2 -o $@ $<

base/sha256.o: base/sha256.c
	${CC} $(CFLAGS) $(INCLUDE) -c -O0 -o $@ $<

%.o: %.S
	${CC} $(CFLAGS) $(INCLUDE) -c -o $@ $<

.PHONY: hdr
hdr:
	cp -Rf patch/include/uapi ../user/
	cp -f ../version ../user/
	cp -f include/preset.h ../tools/

.PHONY: clean
clean:
	rm -rf *.elf
	rm -rf kpimg
	find . -name *.o | xargs rm -f

