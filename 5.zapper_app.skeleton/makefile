CROSS_COMPILE=$(TOOLCHAIN_CROSS_COMPILE)

CC_PREFIX=$(CROSS_COMPILE)-
CC=$(CC_PREFIX)gcc
CXX=$(CC_PREFIX)g++
LD=$(CC_PREFIX)ld
ROOTFS_PATH=$(SDK_ROOTFS)

GALOIS_INCLUDE=$(SDK_GALOIS)

INCS += -I./include/ 							\
		-I./tdp_api								\
		-I$(ROOTFS_PATH)/usr/include/         	\
		-I$(ROOTFS_PATH)/usr/include/directfb/	\
		-I$(GALOIS_INCLUDE)/Common/include/     \
		-I$(GALOIS_INCLUDE)/OSAL/include/		\
		-I$(GALOIS_INCLUDE)/OSAL/include/CPU1/	\
		-I$(GALOIS_INCLUDE)/PE/Common/include/	\
		
SRCS =  ./vezba_5.c				\
		./tables_parser.c		\
		./remote_controller.c	\
		./stream_controller.c	\
		./grafika.c
		
CFLAGS += -D__LINUX__ -O0 -Wno-psabi --sysroot=$(ROOTFS_PATH)

CXXFLAGS = $(CFLAGS)
		
LIBS_PATH = -L./tdp_api

LIBS_PATH += -L$(ROOTFS_PATH)/home/galois/lib/

LIBS_PATH += -L$(ROOTFS_PATH)/home/galois/lib/directfb-1.4-6-libs

LIBS := $(LIBS_PATH) -ltdp -ldirectfb -ldirect -lfusion -lrt -lpthread

LIBS += $(LIBS_PATH) -lOSAL	-lshm -lPEAgent 

all: parser_playback_sample

parser_playback_sample:
	$(CC) -o vezba_5 $(INCS) $(SRCS) $(CFLAGS) $(LIBS)
    
clean:
	rm -f vezba_5
