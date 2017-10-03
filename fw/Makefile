# TCF is needed for ARC builds only
TCF         ?= ../../hw/em7d_v1/build/tool_config/arc_no_stack_chk.tcf
TCF_HOST    ?= tcf/host.tcf
TCF_USHADER ?= tcf/ushader.tcf


CFLAGS_COMMON      = -std=c99 -g -O3 -Wall -DPSHADER_DEBUG=0 -DPTHREAD_DEBUG=0 -DPTHREAD_DEBUG0=0 -DDEBUG_MALLOC=0 -DNDEBUG
CFLAGS_X86         = $(CFLAGS_COMMON) -DSINGLEPROC_MULTITHREAD -Itop -Ilibbarcg -Ilibtga-1.0.1/src -Ihost -Iushader -Iuser_shaders -pthread 
#CFLAGS_MIPS        = $(CFLAGS_COMMON) -DSINGLEPROC_SINGLETHREAD
CFLAGS_ARC_COMMON  = $(CFLAGS_COMMON) -DMULTIPROC -Hnocopyr -Hpurge -Hccm -Iushader -Ilibbarcg -Idma
CFLAGS_ARC_HOST    = -tcf=$(TCF_HOST) $(CFLAGS_ARC_COMMON) -Ihost -Ilibtga-1.0.1/src -Iuser_shaders
CFLAGS_ARC_COMPACTLIB =  -Hcl -Hcrt_fast_memcpy -Hcrt_fast_memset 
#-Hcrt_reginit
CFLAGS_ARC_USHADER_COMMON = -tcf=$(TCF_USHADER) $(CFLAGS_ARC_COMMON) $(CFLAGS_ARC_COMPACTLIB) -Hall_sdata -Hunroll=30 -Hnoxcheck -Hnoxcheck_obj -Hmemcpy_threshold=16 -Hon=long_enums -Hhostlib= -Hheap=0
CFLAGS_ARC_USER_SHADERS = $(CFLAGS_ARC_USHADER_COMMON) -DARC_APEX -Iuser_shaders -I../../hw/em7d_v2/build/User -Bsymout=symlist_host.txt
CFLAGS_ARC_USHADER      = -DDMA -DARC_APEX $(CFLAGS_ARC_USHADER_COMMON) -I../../hw/em7d_v2/build/User   -Bsymin=symlist_host_ext.txt
#CFLAGS_ARC_USHADER      = -DDMA $(CFLAGS_ARC_USHADER_COMMON) -I../../hw/em7d_v2/build/User   -Bsymin=symlist_host_ext.txt


#SYSCONF_LINK_MIPS = mips-mti-elf-gcc $(CFLAGS_COMMON) (CFLAGS_MIPS) -EL -msoft-float -march=m14kc

LDFLAGS_x86          = -lm -pthread
LDFLAGS_ARC_HOST     = -tcf=$(TCF_HOST)    lcf/host.lcf
LDFLAGS_ARC_USHADER  = -tcf=$(TCF_USHADER) lcf/ushader.lcf
##LDFLAGS_MIPS = -lc -lg -lm -EL -msoft-float -march=m14kc -mclib=newlib
##LDFLAGS_MIPS = -lg -lm -lc -EL -msoft-float -march=m14kc -mclib=newlib
#LDFLAGS_MIPS = -lm -lc -EL -msoft-float -march=m14kc -mclib=newlib

DESTDIR = .
TARGET_x86  = main_x86.a
TARGET_MIPS = main_mips.a
TARGET_ARC_HOST     = arc_host.elf
TARGET_ARC_USHADER  = arc_ushader.elf

OBJECTS_x86  := $(patsubst %.c, %_x86.o,  $(wildcard top/*.c) $(wildcard host/*.c) $(wildcard ushader/*.c) $(wildcard user_shaders/*.c) $(wildcard libbarcg/*.c) $(wildcard libtga-1.0.1/src/*.c))
INCLUDES_x86 =                            $(wildcard top/*.h) $(wildcard host/*.h) $(wildcard ushader/*.h) $(wildcard user_shaders/*.h) $(wildcard libbarcg/*.h) $(wildcard libtga-1.0.1/src/*.h)

OBJECTS_ARC_HOST  := $(patsubst %.c, %_host_arc.o,  $(wildcard host/*.c) $(wildcard libbarcg/*.c) $(wildcard libtga-1.0.1/src/*.c))
INCLUDES_ARC_HOST =                                 $(wildcard host/*.h) $(wildcard libbarcg/*.h) $(wildcard libtga-1.0.1/src/*.h) $(wildcard ushader/*.h) $(wildcard user_shaders/*.h)

OBJECTS_ARC_USER_SHADERS  := $(patsubst %.c, %_user_shaders_arc.o,  ushader/ushader_public.c $(wildcard user_shaders/*.c))
INCLUDES_ARC_USER_SHADERS =                                         ushader/ushader_public.h $(wildcard user_shaders/*.h)  ../../hw/em7d_v2/build/User/apexextensions.h

OBJECTS_ARC_USHADER  := $(patsubst %.c, %_ushader_arc.o,  $(wildcard ushader/*.c) $(wildcard dma/*.c) libbarcg/geometry.c)
INCLUDES_ARC_USHADER =                                    $(wildcard ushader/*.h) $(wildcard dma/*.h) libbarcg/geometry.h ../../hw/em7d_v2/build/User/apexextensions.h

#OBJECTS_MIPS := $(patsubst %.c, %_mips.o, $(wildcard *.c) $(wildcard libtga-1.0.1/src/*.c))

all: x86 arc
arc: arc_host arc_ushader
arc_host:    $(DESTDIR)/$(TARGET_ARC_HOST)
arc_ushader: $(DESTDIR)/$(TARGET_ARC_USHADER)
#mips: $(DESTDIR)/$(TARGET_MIPS)
x86: $(DESTDIR)/$(TARGET_x86)



$(DESTDIR)/$(TARGET_x86): $(OBJECTS_x86)
	gcc -o $(DESTDIR)/$(TARGET_x86) $(OBJECTS_x86) $(LIBS) $(LDFLAGS_x86)

$(DESTDIR)/$(TARGET_ARC_HOST): $(OBJECTS_ARC_HOST) $(TCF_HOST) $(OBJECTS_ARC_USER_SHADERS) $(TCF_USHADER)
	ccac $(LDFLAGS_ARC_HOST) -o $(DESTDIR)/$(TARGET_ARC_HOST) $(OBJECTS_ARC_HOST) $(OBJECTS_ARC_USER_SHADERS) $(LIBS)
	elfdump -S $(TARGET_ARC_HOST) > $(TARGET_ARC_HOST).dump
	echo "#version=1" > symlist_host_ext.txt 
	grep shader symlist_host.txt >> symlist_host_ext.txt

$(DESTDIR)/$(TARGET_ARC_USHADER): $(OBJECTS_ARC_USHADER) $(TCF_USHADER)
	ccac $(LDFLAGS_ARC_USHADER) -o $(DESTDIR)/$(TARGET_ARC_USHADER) $(OBJECTS_ARC_USHADER) $(LIBS)
	elfdump -S $(TARGET_ARC_USHADER) > $(TARGET_ARC_USHADER).dump

#$(DESTDIR)/$(TARGET_MIPS): $(OBJECTS_MIPS)
#	$(SYSCONF_LINK_MIPS) -o $(DESTDIR)/$(TARGET_MIPS) $(OBJECTS_MIPS) $(LIBS) $(LDFLAGS_MIPS)

$(OBJECTS_x86): %_x86.o: %.c $(INCLUDES_x86)
	gcc -c $(CFLAGS_X86) $< -o $@

$(OBJECTS_ARC_HOST): %_host_arc.o: %.c $(INCLUDES_ARC_HOST)
	ccac -c $(CFLAGS_ARC_HOST) $< -o $@

$(OBJECTS_ARC_USER_SHADERS): %_user_shaders_arc.o: %.c $(INCLUDES_ARC_USER_SHADERS)
	ccac -c $(CFLAGS_ARC_USER_SHADERS) $< -o $@
	
$(OBJECTS_ARC_USHADER): %_ushader_arc.o: %.c $(INCLUDES_ARC_USHADER)
	ccac -c $(CFLAGS_ARC_USHADER) $< -o $@
	
#$(OBJECTS_MIPS): %_mips.o: %.c %.h
#	$(SYSCONF_LINK_MIPS) $(CPPFLAGS) -c $(CFLAGS) $< -o $@


clean:
	-rm -f top/*.o
	-rm -f host/*.o
	-rm -f ushader/*.o
	-rm -f user_shaders/*.o
	-rm -f libbarcg/*.o
	-rm -f libtga-1.0.1/src/*.o
	-rm -f *.elf
	-rm -f *.a
	-rm -f *.tga
	-rm -f *.y4m
