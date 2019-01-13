CONFIG_UCLIBC_BUILD=y
#your cross compiler path
CROSS_COMPILE ?= /root/T30/mips-gcc472-glibc216-32bit/bin/mips-linux-gnu-

CC = $(CROSS_COMPILE)gcc
CPLUSPLUS = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar cr
STRIP = $(CROSS_COMPILE)strip

CFLAGS = $(INCLUDES) -O2 -Wall -march=mips32r2

ifeq ($(CONFIG_UCLIBC_BUILD), y)
CFLAGS += -muclibc
LDFLAG += -muclibc
endif

ifeq ($(CONFIG_UCLIBC_BUILD), y)
SDK_LIB_DIR = ../../lib/uclibc
else
SDK_LIB_DIR = ../../lib/glibc
endif

SDK_INC_DIR = ../../include

INCLUDES = -I$(SDK_INC_DIR)

LIBS = $(SDK_LIB_DIR)/libimp.a $(SDK_LIB_DIR)/libalog.a

LDFLAG += -Wl,-gc-sections

SAMPLES = sample-Snap-Raw \
#	sample-Snap \


all: 	$(SAMPLES)

sample-Snap-Raw: $(SDK_LIB_DIR)/libimp.a $(SDK_LIB_DIR)/libalog.a sample-common.o sample-Snap-Raw.o
	$(CPLUSPLUS) $(LDFLAG) -o $@ $^ $(LIBS) -lpthread -lm -lrt
	$(STRIP) $@

%.o:%.c sample-common.h
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o *~

distclean: clean
	rm -f $(SAMPLES)
