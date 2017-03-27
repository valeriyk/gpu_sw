TCF          = ../hw/em5d_x2_1/build/tool_config/core0_arc.tcf
OPT = -O3

SYSCONF_LINK_x86  =              gcc -g -std=c99 -Ilibtga-1.0.1/src -DTARGET_x86  -DGL_DEBUG_0=0 -DGL_DEBUG_1=0 -DGL_DEBUG_2=0 -DDEBUG_Z=1 -DNDEBUG=1 $(OPT)
SYSCONF_LINK_MIPS = mips-mti-elf-gcc -g -std=c99 -Ilibtga-1.0.1/src -DTARGET_MIPS -DGL_DEBUG_0=0 -DGL_DEBUG_1=0 -DGL_DEBUG_2=0 -DDEBUG_Z=1 -DNDEBUG=1 $(OPT) -EL -msoft-float -march=m14kc
SYSCONF_LINK_ARC  = ccac -g -tcf=$(TCF) ../hw/em5d_x2_1/build/tool_config/core0_link_cmd.txt -Ithirdparty/libtga -DGL_DEBUG_0=0 -DGL_DEBUG_1=0 -DGL_DEBUG_2=0 $(OPT)



CPPFLAGS     =

LDFLAGS_x86  = -lm
LDFLAGS_ARC  =
#LDFLAGS_MIPS = -lc -lg -lm -EL -msoft-float -march=m14kc -mclib=newlib
#LDFLAGS_MIPS = -lg -lm -lc -EL -msoft-float -march=m14kc -mclib=newlib
 LDFLAGS_MIPS = -lm -lc -EL -msoft-float -march=m14kc -mclib=newlib

LIBS         =

DESTDIR = ./
TARGET_x86  = main.a
TARGET_ARC  = main_arc.elf
TARGET_MIPS = main_mips.elf

#VPATH=thirdparty/libtga

OBJECTS_x86  := $(patsubst %.c,%_x86.o,$(wildcard *.c) $(wildcard libtga-1.0.1/src/*.c))
OBJECTS_ARC  := $(patsubst %.c,%_arc.o,$(wildcard *.c)) 
OBJECTS_MIPS := $(patsubst %.c,%_mips.o,$(wildcard *.c) $(wildcard libtga-1.0.1/src/*.c))

all: x86 arc mips

arc: $(DESTDIR)$(TARGET_ARC)

mips: $(DESTDIR)$(TARGET_MIPS)

x86: $(DESTDIR)$(TARGET_x86)

$(DESTDIR)$(TARGET_x86): $(OBJECTS_x86)
	$(SYSCONF_LINK_x86) -Wall -o $(DESTDIR)$(TARGET_x86) $(OBJECTS_x86) $(LIBS) $(LDFLAGS_x86)

$(DESTDIR)$(TARGET_ARC): $(OBJECTS_ARC) $(TCF)
	$(SYSCONF_LINK_ARC) -Wall $(LDFLAGS_ARC) -o $(DESTDIR)$(TARGET_ARC) $(OBJECTS_ARC) $(LIBS)

$(DESTDIR)$(TARGET_MIPS): $(OBJECTS_MIPS)
	$(SYSCONF_LINK_MIPS) -Wall -o $(DESTDIR)$(TARGET_MIPS) $(OBJECTS_MIPS) $(LIBS) $(LDFLAGS_MIPS)



$(OBJECTS_x86): %_x86.o: %.c
	$(SYSCONF_LINK_x86) -Wall $(CPPFLAGS) -c $(CFLAGS) $< -o $@

$(OBJECTS_ARC): %_arc.o: %.c
	$(SYSCONF_LINK_ARC) -Wall $(CPPFLAGS) -c $(CFLAGS) $< -o $@

$(OBJECTS_MIPS): %_mips.o: %.c
	$(SYSCONF_LINK_MIPS) -Wall $(CPPFLAGS) -c $(CFLAGS) $< -o $@


clean:
	-rm -f *.o
	-rm -f thirdparty/libtga/*.o
	-rm -f thirdparty/libfixmath/*.o
	-rm -f *.elf
	-rm -f *.a
	-rm -f *.tga
	-rm -f *.y4m
