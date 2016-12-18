TCF          = ../hw/em5d_x2_1/build/tool_config/core0_arc.tcf
OPT = -O3
#SYSCONF_LINK_x86 = gcc -ggdb -g3 -pg -O0 -I.
SYSCONF_LINK_x86 = gcc -g -pg -std=c99 -Ilibtga-1.0.1/src -DGL_DEBUG_0=0 -DGL_DEBUG_1=0 -DGL_DEBUG_2=0 -DDEBUG_Z=1 $(OPT)

#SYSCONF_LINK_ARC = ccac -g -tcf=$(TCF)
#SYSCONF_LINK_ARC = ccac -g -tcf=$(TCF) ../../hw/em5d_x2_1/build/tool_config/core0_link_cmd.txt -I.
SYSCONF_LINK_ARC = ccac -g -tcf=$(TCF) ../hw/em5d_x2_1/build/tool_config/core0_link_cmd.txt -Ithirdparty/libtga -DGL_DEBUG_0=0 -DGL_DEBUG_1=0 -DGL_DEBUG_2=0 $(OPT)

CPPFLAGS     =
LDFLAGS_x86  = -lm
LDFLAGS_ARC  =
LIBS         =

DESTDIR = ./
TARGET_x86 = main.a
TARGET_ARC = main.elf

#VPATH=thirdparty/libtga

OBJECTS_x86 := $(patsubst %.c,%_x86.o,$(wildcard *.c) $(wildcard libtga-1.0.1/src/*.c))
OBJECTS_ARC := $(patsubst %.c,%_arc.o,$(wildcard *.c)) 

all: x86 arc

arc: $(DESTDIR)$(TARGET_ARC)

x86: $(DESTDIR)$(TARGET_x86)

$(DESTDIR)$(TARGET_x86): $(OBJECTS_x86)
	$(SYSCONF_LINK_x86) -Wall -o $(DESTDIR)$(TARGET_x86) $(OBJECTS_x86) $(LIBS) $(LDFLAGS_x86)

$(DESTDIR)$(TARGET_ARC): $(OBJECTS_ARC) $(TCF)
	$(SYSCONF_LINK_ARC) -Wall $(LDFLAGS_ARC) -o $(DESTDIR)$(TARGET_ARC) $(OBJECTS_ARC) $(LIBS)


$(OBJECTS_x86): %_x86.o: %.c
	$(SYSCONF_LINK_x86) -Wall $(CPPFLAGS) -c $(CFLAGS) $< -o $@

$(OBJECTS_ARC): %_arc.o: %.c
	$(SYSCONF_LINK_ARC) -Wall $(CPPFLAGS) -c $(CFLAGS) $< -o $@


clean:
	-rm -f *.o
	-rm -f thirdparty/libtga/*.o
	-rm -f thirdparty/libfixmath/*.o
	-rm -f *.elf
	-rm -f *.a
	-rm -f *.tga
	-rm -f *.y4m
