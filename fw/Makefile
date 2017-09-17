# TCF is needed for ARC builds only
TCF         ?= ../../hw/em7d_v1/build/tool_config/arc_no_stack_chk.tcf
TCF_HOST    ?= tcf/host.tcf
TCF_USHADER ?= tcf/ushader.tcf
TCF_VSHADER ?= tcf/vshader.tcf
TCF_PSHADER ?= tcf/pshader.tcf


#OPT = -O3
OPT = -g -O3

CFLAGS_COMMON      = -std=c99 $(OPT) -Wall -Itop -Ilibbarcg -Ilibtga-1.0.1/src -DPSHADER_DEBUG=0 -DGL_DEBUG_0=0 -DGL_DEBUG_1=0 -DGL_DEBUG_2=0 -DDEBUG_Z=0 -DPTHREAD_DEBUG=0 -DPTHREAD_DEBUG0=0 -DDEBUG_MALLOC=0
#-DDEBUG_FIXPT_VARYING
#-DNDEBUG 
CFLAGS_X86         = -DTARGET_x86  -DSINGLEPROC_MULTITHREAD
CFLAGS_MIPS        = -DTARGET_MIPS -DSINGLEPROC_SINGLETHREAD
CFLAGS_ARC_COMMON  = -DTARGET_ARC
CFLAGS_ARC         = -tcf=$(TCF)         -DSINGLEPROC_SINGLETHREAD
CFLAGS_ARC_HOST    = -tcf=$(TCF_HOST)    -DMULTIPROC
CFLAGS_ARC_USHADER = -tcf=$(TCF_USHADER) -DMULTIPROC
CFLAGS_ARC_VSHADER = -tcf=$(TCF_VSHADER) -DMULTIPROC
CFLAGS_ARC_PSHADER = -tcf=$(TCF_PSHADER) -DMULTIPROC

SYSCONF_LINK_x86  =          gcc $(CFLAGS_COMMON) $(CFLAGS_X86) -pthread -Ihost -Ivshader -Ipshader -Iushader
#SYSCONF_LINK_MIPS = mips-mti-elf-gcc $(CFLAGS_COMMON) (CFLAGS_MIPS) -EL -msoft-float -march=m14kc
SYSCONF_LINK_ARC          =  ccac $(CFLAGS_COMMON) $(CFLAGS_ARC_COMMON) $(CFLAGS_ARC)         -Ihost -Ivshader -Ipshader -Iushader
SYSCONF_LINK_ARC_HOST     =  ccac $(CFLAGS_COMMON) $(CFLAGS_ARC_COMMON) $(CFLAGS_ARC_HOST)    -Ihost     
SYSCONF_LINK_ARC_USHADER  =  ccac $(CFLAGS_COMMON) $(CFLAGS_ARC_COMMON) $(CFLAGS_ARC_USHADER) -Iushader -Ivshader -Ipshader
SYSCONF_LINK_ARC_VSHADER  =  ccac $(CFLAGS_COMMON) $(CFLAGS_ARC_COMMON) $(CFLAGS_ARC_VSHADER) -Ivshader
SYSCONF_LINK_ARC_PSHADER  =  ccac $(CFLAGS_COMMON) $(CFLAGS_ARC_COMMON) $(CFLAGS_ARC_PSHADER) -Ipshader

LDFLAGS_x86  = -lm -pthread
LDFLAGS_ARC  = 
##LDFLAGS_MIPS = -lc -lg -lm -EL -msoft-float -march=m14kc -mclib=newlib
##LDFLAGS_MIPS = -lg -lm -lc -EL -msoft-float -march=m14kc -mclib=newlib
#LDFLAGS_MIPS = -lm -lc -EL -msoft-float -march=m14kc -mclib=newlib

DESTDIR = .
TARGET_x86  = main_x86.a
TARGET_MIPS = main_mips.a
TARGET_ARC  = main_arc.a
TARGET_ARC_HOST     = arc_host.elf
TARGET_ARC_USHADER  = arc_ushader.elf
TARGET_ARC_VSHADER  = arc_vshader.elf
TARGET_ARC_PSHADER  = arc_pshader.elf


#VPATH=thirdparty/libtga

#OBJECTS_x86  := $(patsubst %.c, %_x86.o,  $(wildcard *.c) )
OBJECTS_x86  := $(patsubst %.c, %_x86.o,  $(wildcard top/*.c) $(wildcard host/*.c) $(wildcard ushader/*.c) $(wildcard vshader/*.c) $(wildcard pshader/*.c) $(wildcard libbarcg/*.c) $(wildcard libtga-1.0.1/src/*.c))
INCLUDES_x86 =                            $(wildcard top/*.h) $(wildcard host/*.h) $(wildcard ushader/*.h) $(wildcard vshader/*.h) $(wildcard pshader/*.h) $(wildcard libbarcg/*.h) $(wildcard libtga-1.0.1/src/*.h)

OBJECTS_ARC  := $(patsubst %.c, %_arc.o,  $(wildcard top/*.c) $(wildcard host/*.c) $(wildcard ushader/*.c) $(wildcard vshader/*.c) $(wildcard pshader/*.c) $(wildcard libbarcg/*.c) $(wildcard libtga-1.0.1/src/*.c))
INCLUDES_ARC =                            $(wildcard top/*.h) $(wildcard host/*.h) $(wildcard ushader/*.h) $(wildcard vshader/*.h) $(wildcard pshader/*.h) $(wildcard libbarcg/*.h) $(wildcard libtga-1.0.1/src/*.h)

OBJECTS_ARC_HOST  := $(patsubst %.c, %_host_arc.o,  $(wildcard host/*.c) $(wildcard libbarcg/*.c) $(wildcard libtga-1.0.1/src/*.c))
INCLUDES_ARC_HOST =                                 $(wildcard host/*.h) $(wildcard libbarcg/*.h) $(wildcard libtga-1.0.1/src/*.h)
OBJECTS_ARC_USHADER  := $(patsubst %.c, %_ushader_arc.o,  vshader/vshader_loop.c pshader/pshader_loop.c $(wildcard ushader/*.c) $(wildcard libbarcg/*.c) $(wildcard libtga-1.0.1/src/*.c))
INCLUDES_ARC_USHADER =                                    vshader/vshader_loop.h pshader/pshader_loop.h $(wildcard ushader/*.h) $(wildcard libbarcg/*.h) $(wildcard libtga-1.0.1/src/*.h)
OBJECTS_ARC_VSHADER  := $(patsubst %.c, %_vshader_arc.o,  $(wildcard vshader/*.c) $(wildcard libbarcg/*.c) $(wildcard libtga-1.0.1/src/*.c))
INCLUDES_ARC_VSHADER =                                    $(wildcard vshader/*.h) $(wildcard libbarcg/*.h) $(wildcard libtga-1.0.1/src/*.h)
OBJECTS_ARC_PSHADER  := $(patsubst %.c, %_vshader_arc.o,  $(wildcard pshader/*.c) $(wildcard libbarcg/*.c) $(wildcard libtga-1.0.1/src/*.c))
INCLUDES_ARC_PSHADER =                                    $(wildcard pshader/*.h) $(wildcard libbarcg/*.h) $(wildcard libtga-1.0.1/src/*.h)
#OBJECTS_MIPS := $(patsubst %.c, %_mips.o, $(wildcard *.c) $(wildcard libtga-1.0.1/src/*.c))

#all: x86 arc_single arc mips

arc: arc_host arc_ushader
arc_nonunishader: arc_host arc_vshader arc_pshader
arc_singlecore: $(DESTDIR)/$(TARGET_ARC)


arc_host:    $(DESTDIR)/$(TARGET_ARC_HOST)
arc_ushader: $(DESTDIR)/$(TARGET_ARC_USHADER)
arc_vshader: $(DESTDIR)/$(TARGET_ARC_VSHADER)
arc_pshader: $(DESTDIR)/$(TARGET_ARC_PSHADER)



#mips: $(DESTDIR)/$(TARGET_MIPS)

x86: $(DESTDIR)/$(TARGET_x86)

$(DESTDIR)/$(TARGET_x86): $(OBJECTS_x86)
	$(SYSCONF_LINK_x86) -o $(DESTDIR)/$(TARGET_x86) $(OBJECTS_x86) $(LIBS) $(LDFLAGS_x86)

$(DESTDIR)/$(TARGET_ARC): $(OBJECTS_ARC) $(TCF)
	$(SYSCONF_LINK_ARC) $(LDFLAGS_ARC) -o $(DESTDIR)/$(TARGET_ARC) $(OBJECTS_ARC) $(LIBS)

$(DESTDIR)/$(TARGET_ARC_HOST): $(OBJECTS_ARC_HOST) $(TCF_HOST)
	$(SYSCONF_LINK_ARC_HOST) $(LDFLAGS_ARC) -o $(DESTDIR)/$(TARGET_ARC_HOST) $(OBJECTS_ARC_HOST) $(LIBS)

$(DESTDIR)/$(TARGET_ARC_USHADER): $(OBJECTS_ARC_USHADER) $(TCF_USHADER)
	$(SYSCONF_LINK_ARC_USHADER) $(LDFLAGS_ARC) -o $(DESTDIR)/$(TARGET_ARC_USHADER) $(OBJECTS_ARC_USHADER) $(LIBS)

$(DESTDIR)/$(TARGET_ARC_VSHADER): $(OBJECTS_ARC_VSHADER) $(TCF_VSHADER)
	$(SYSCONF_LINK_ARC_VSHADER) $(LDFLAGS_ARC) -o $(DESTDIR)/$(TARGET_ARC_VSHADER) $(OBJECTS_ARC_VSHADER) $(LIBS)

$(DESTDIR)/$(TARGET_ARC_PSHADER): $(OBJECTS_ARC_PSHADER) $(TCF_PSHADER)
	$(SYSCONF_LINK_ARC_PSHADER) $(LDFLAGS_ARC) -o $(DESTDIR)/$(TARGET_ARC_PSHADER) $(OBJECTS_ARC_PSHADER) $(LIBS)

#$(DESTDIR)/$(TARGET_MIPS): $(OBJECTS_MIPS)
#	$(SYSCONF_LINK_MIPS) -o $(DESTDIR)/$(TARGET_MIPS) $(OBJECTS_MIPS) $(LIBS) $(LDFLAGS_MIPS)

$(OBJECTS_x86): %_x86.o: %.c $(INCLUDES_x86)
	$(SYSCONF_LINK_x86) $(CPPFLAGS) -c $(CFLAGS) $< -o $@

$(OBJECTS_ARC): %_arc.o: %.c $(INCLUDES_ARC)
	$(SYSCONF_LINK_ARC) $(CPPFLAGS) -c $(CFLAGS) $< -o $@

$(OBJECTS_ARC_HOST): %_host_arc.o: %.c $(INCLUDES_ARC_HOST)
	$(SYSCONF_LINK_ARC_HOST) $(CPPFLAGS) -c $(CFLAGS) $< -o $@

$(OBJECTS_ARC_USHADER): %_ushader_arc.o: %.c $(INCLUDES_ARC_USHADER)
	$(SYSCONF_LINK_ARC_USHADER) $(CPPFLAGS) -c $(CFLAGS) $< -o $@
	
$(OBJECTS_ARC_VSHADER): %_vshader_arc.o: %.c $(INCLUDES_ARC_VSHADER)
	$(SYSCONF_LINK_ARC_VSHADER) $(CPPFLAGS) -c $(CFLAGS) $< -o $@

$(OBJECTS_ARC_PSHADER): %_vshader_arc.o: %.c $(INCLUDES_ARC_PSHADER)
	$(SYSCONF_LINK_ARC_PSHADER) $(CPPFLAGS) -c $(CFLAGS) $< -o $@

#$(OBJECTS_MIPS): %_mips.o: %.c %.h
#	$(SYSCONF_LINK_MIPS) $(CPPFLAGS) -c $(CFLAGS) $< -o $@


clean:
	-rm -f top/*.o
	-rm -f host/*.o
	-rm -f pshader/*.o
	-rm -f libbarcg/*.o
	-rm -f libtga-1.0.1/src/*.o
	-rm -f *.elf
	-rm -f *.a
	-rm -f *.tga
	-rm -f *.y4m
