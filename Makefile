#TCF          = ../../../hw/em7d_v1/build/tool_config/arc_no_stack_chk.tcf
OPT = -O3

#SYSCONF_LINK_x86  =              gcc -g -std=c99 -DTARGET_x86 $(OPT)
SYSCONF_LINK_x86  =              gcc -g -std=c99 -I../host -I. -I../libtga-1.0.1/src -I../libbarcg -I../pshaderfw -DTARGET_x86 -DGL_DEBUG_0=0 -DGL_DEBUG_1=0 -DGL_DEBUG_2=0 -DDEBUG_Z=0 -DPTHREAD_DEBUG=0 -DPTHREAD_DEBUG0=0 -DDEBUG_MALLOC=0 $(OPT) -DNDEBUG=0


#SYSCONF_LINK_MIPS = mips-mti-elf-gcc -g -std=c99 -Ilibtga-1.0.1/src -DTARGET_MIPS -DGL_DEBUG_0=0 -DGL_DEBUG_1=0 -DGL_DEBUG_2=0 -DDEBUG_Z=1 -DNDEBUG=1 $(OPT) -EL -msoft-float -march=m14kc
#SYSCONF_LINK_ARC  =             ccac -g          -Ilibtga-1.0.1/src -DTARGET_ARC  -DGL_DEBUG_0=0 -DGL_DEBUG_1=0 -DGL_DEBUG_2=0 -DDEBUG_Z=1 -DNDEBUG=1 $(OPT) -tcf=$(TCF)



CPPFLAGS     =

LDFLAGS_x86  = -lm -lpthread
#LDFLAGS_ARC  = 
##LDFLAGS_MIPS = -lc -lg -lm -EL -msoft-float -march=m14kc -mclib=newlib
##LDFLAGS_MIPS = -lg -lm -lc -EL -msoft-float -march=m14kc -mclib=newlib
#LDFLAGS_MIPS = -lm -lc -EL -msoft-float -march=m14kc -mclib=newlib

LIBS         =

DESTDIR = ./
TARGET_x86  = main.a
#TARGET_ARC  = main_arc.elf
#TARGET_MIPS = main_mips.elf

#VPATH=thirdparty/libtga

#OBJECTS_x86  := $(patsubst %.c, %_x86.o,  $(wildcard *.c) )
OBJECTS_x86  := $(patsubst %.c, %_x86.o,  $(wildcard *.c) $(wildcard ../host/*.c) $(wildcard ../pshaderfw/*.c) $(wildcard ../libbarcg/*.c) $(wildcard ../libtga-1.0.1/src/*.c))
INCLUDES_x86 =                            $(wildcard *.h) $(wildcard ../host/*.h) $(wildcard ../pshaderfw/*.h) $(wildcard ../libbarcg/*.h) $(wildcard ../libtga-1.0.1/src/*.h)

#OBJECTS_ARC  := $(patsubst %.c, %_arc.o,  $(wildcard *.c) $(wildcard libtga-1.0.1/src/*.c)) 
#OBJECTS_MIPS := $(patsubst %.c, %_mips.o, $(wildcard *.c) $(wildcard libtga-1.0.1/src/*.c))

#all: x86 arc mips

#arc: $(DESTDIR)$(TARGET_ARC)

#mips: $(DESTDIR)$(TARGET_MIPS)

x86: $(DESTDIR)$(TARGET_x86)

$(DESTDIR)$(TARGET_x86): $(OBJECTS_x86)
	$(SYSCONF_LINK_x86) -Wall -o $(DESTDIR)$(TARGET_x86) $(OBJECTS_x86) $(LIBS) $(LDFLAGS_x86)

#$(DESTDIR)$(TARGET_ARC): $(OBJECTS_ARC) $(TCF)
#	$(SYSCONF_LINK_ARC) -Wall $(LDFLAGS_ARC) -o $(DESTDIR)$(TARGET_ARC) $(OBJECTS_ARC) $(LIBS)

#$(DESTDIR)$(TARGET_MIPS): $(OBJECTS_MIPS)
#	$(SYSCONF_LINK_MIPS) -Wall -o $(DESTDIR)$(TARGET_MIPS) $(OBJECTS_MIPS) $(LIBS) $(LDFLAGS_MIPS)

$(OBJECTS_x86): %_x86.o: %.c $(INCLUDES_x86)
	$(SYSCONF_LINK_x86) -Wall $(CPPFLAGS) -c $(CFLAGS) $< -o $@

#$(OBJECTS_ARC): %_arc.o: %.c %.h
#	$(SYSCONF_LINK_ARC) -Wall $(CPPFLAGS) -c $(CFLAGS) $< -o $@

#$(OBJECTS_MIPS): %_mips.o: %.c %.h
#	$(SYSCONF_LINK_MIPS) -Wall $(CPPFLAGS) -c $(CFLAGS) $< -o $@


clean:
	-rm -f *.o
	-rm -f ../host/*.o
	-rm -f ../pshaderfw/*.o
	-rm -f ../libbarcg/*.o
	-rm -f ../libtga-1.0.1/src/*.o
	-rm -f *.elf
	-rm -f *.a
	-rm -f *.tga
	-rm -f *.y4m
