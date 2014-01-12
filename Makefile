#------------------------------------------------------------------------------
#
#               CPS1/CPS2/NEOGEO Emulator for PSP Makefile
#
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Configration
#------------------------------------------------------------------------------

#BUILD_CPS1PSP = 1
BUILD_CPS2PSP = 1
#BUILD_MVSPSP = 1

SAVE_STATE = 1
KERNEL_MODE = 1
#SOUND_TEST = 1
RELEASE = 1


#------------------------------------------------------------------------------
# Defines
#------------------------------------------------------------------------------

VERSION_MAJOR = 1
VERSION_MINOR = 6
VERSION_BUILD = 2

ifdef BUILD_CPS1PSP
BUILD_CPS2PSP=
BUILD_MVSPSP=
SOUND_TEST=
TARGET = CPS1PSP
PSP_EBOOT_ICON = data/cps1.png
endif

ifdef BUILD_CPS2PSP
BUILD_MVSPSP=
SOUND_TEST=
TARGET = CPS2PSP
PSP_EBOOT_ICON = data/cps2.png
endif

ifdef BUILD_MVSPSP
TARGET = MVSPSP
PSP_EBOOT_ICON = data/mvs.png
endif

PBPNAME_STR = $(TARGET)
VERSION_STR = $(VERSION_MAJOR).$(VERSION_MINOR)$(VERSION_BUILD)

PSP_EBOOT_TITLE = $(PBPNAME_STR) $(VERSION_STR)

EXTRA_TARGETS = mkdir EBOOT.PBP delelf
EXTRA_CLEAN = pspclean


#------------------------------------------------------------------------------
# Compiler Flags
#------------------------------------------------------------------------------

CFLAGS = -O3 -Wundef -Wunused


#------------------------------------------------------------------------------
# Compiler Defines
#------------------------------------------------------------------------------

CDEFS = -DINLINE='static __inline' \
	-Dinline=__inline \
	-D__inline__=__inline \
	-DBUILD_$(TARGET)=1 \
	-DPBPNAME_STR=\"$(PBPNAME_STR)\" \
	-DVERSION_STR=\"$(VERSION_STR)\" \
	-DVERSION_MAJOR=$(VERSION_MAJOR) \
	-DVERSION_MINOR=$(VERSION_MINOR) \
	-DVERSION_BUILD=$(VERSION_BUILD) \
	-DPSP

ifdef KERNEL_MODE
CDEFS += -DKERNEL_MODE=1
endif

ifdef SAVE_STATE
CDEFS += -DSAVE_STATE=1
endif

ifdef SOUND_TEST
CDEFS += -DSOUND_TEST=1
endif

ifdef RELEASE
CDEFS += -DRELEASE=1
else
CDEFS += -DRELEASE=0
endif

#------------------------------------------------------------------------------
# Object File Output Directtory
#------------------------------------------------------------------------------

ifdef BUILD_CPS1PSP
OBJ = obj_cps1
endif

ifdef BUILD_CPS2PSP
OBJ = obj_cps2
endif

ifdef BUILD_MVSPSP
OBJ = obj_mvs
endif


#------------------------------------------------------------------------------
# File include path
#------------------------------------------------------------------------------

INCDIR = \
	src \
	src/cpu/m68000 \
	src/cpu/z80 \
	src/zip \
	src/zlib

ifdef BUILD_CPS1PSP
INCDIR += src/cps1
endif

ifdef BUILD_CPS2PSP
INCDIR += src/cps2
endif

ifdef BUILD_MVSPSP
INCDIR += src/mvs
endif


#------------------------------------------------------------------------------
# Linker Flags
#------------------------------------------------------------------------------

LIBDIR =
LDFLAGS =


#------------------------------------------------------------------------------
# Library
#------------------------------------------------------------------------------

USE_PSPSDK_LIBC = 1

LIBS = -lm -lc -lpspaudio -lpspgu -lpsppower -lpsprtc


#------------------------------------------------------------------------------
# Object Directory
#------------------------------------------------------------------------------

OBJDIRS = \
	$(OBJ) \
	$(OBJ)/cpu \
	$(OBJ)/cpu/m68000 \
	$(OBJ)/cpu/z80 \
	$(OBJ)/common \
	$(OBJ)/sound \
	$(OBJ)/zip \
	$(OBJ)/zlib \
	$(OBJ)/psp \
	$(OBJ)/psp/font \
	$(OBJ)/psp/icon

ifdef BUILD_CPS1PSP
OBJDIRS += $(OBJ)/cps1
endif

ifdef BUILD_CPS2PSP
OBJDIRS += $(OBJ)/cps2
endif

ifdef BUILD_MVSPSP
OBJDIRS += $(OBJ)/mvs
endif

#------------------------------------------------------------------------------
# Object Files (common)
#------------------------------------------------------------------------------

MAINOBJS = \
	$(OBJ)/emumain.o \
	$(OBJ)/zip/zfile.o \
	$(OBJ)/zip/unzip.o \
	$(OBJ)/cpu/m68000/m68000.o \
	$(OBJ)/cpu/m68000/c68k.o \
	$(OBJ)/cpu/z80/z80.o \
	$(OBJ)/cpu/z80/cz80.o \
	$(OBJ)/sound/sndintrf.o \
	$(OBJ)/common/cache.o \
	$(OBJ)/common/loadrom.o

ifndef BUILD_MVSPSP
MAINOBJS  += $(OBJ)/common/coin.o
endif

ifdef SAVE_STATE
MAINOBJS += $(OBJ)/common/state.o
endif

ifdef SOUND_TEST
MAINOBJS += $(OBJ)/common/sndtest.o
endif


#------------------------------------------------------------------------------
# Object Files (CPS1PSP)
#------------------------------------------------------------------------------

ifdef BUILD_CPS1PSP

COREOBJS = \
	$(OBJ)/cps1/cps1.o \
	$(OBJ)/cps1/driver.o \
	$(OBJ)/cps1/memintrf.o \
	$(OBJ)/cps1/inptport.o \
	$(OBJ)/cps1/dipsw.o \
	$(OBJ)/cps1/timer.o \
	$(OBJ)/cps1/vidhrdw.o \
	$(OBJ)/cps1/sprite.o \
	$(OBJ)/cps1/eeprom.o \
	$(OBJ)/cps1/kabuki.o \
	$(OBJ)/sound/2151intf.o \
	$(OBJ)/sound/ym2151.o \
	$(OBJ)/sound/qsound.o

ICONOBJS = \
	$(OBJ)/psp/icon/cps_s.o \
	$(OBJ)/psp/icon/cps_l.o

endif


#------------------------------------------------------------------------------
# Object Files (CPS2PSP)
#------------------------------------------------------------------------------

ifdef BUILD_CPS2PSP

COREOBJS = \
	$(OBJ)/cps2/cps2.o \
	$(OBJ)/cps2/driver.o \
	$(OBJ)/cps2/memintrf.o \
	$(OBJ)/cps2/inptport.o \
	$(OBJ)/cps2/timer.o \
	$(OBJ)/cps2/vidhrdw.o \
	$(OBJ)/cps2/sprite.o \
	$(OBJ)/cps2/eeprom.o \
	$(OBJ)/sound/qsound.o

ICONOBJS = \
	$(OBJ)/psp/icon/cps_s.o \
	$(OBJ)/psp/icon/cps_l.o

endif


#------------------------------------------------------------------------------
# Object Files (MVSPSP)
#------------------------------------------------------------------------------

ifdef BUILD_MVSPSP

COREOBJS = \
	$(OBJ)/mvs/mvs.o \
	$(OBJ)/mvs/driver.o \
	$(OBJ)/mvs/memintrf.o \
	$(OBJ)/mvs/inptport.o \
	$(OBJ)/mvs/dipsw.o \
	$(OBJ)/mvs/timer.o \
	$(OBJ)/mvs/vidhrdw.o \
	$(OBJ)/mvs/sprite.o \
	$(OBJ)/mvs/pd4990a.o \
	$(OBJ)/mvs/biosmenu.o \
	$(OBJ)/sound/2610intf.o \
	$(OBJ)/sound/ym2610.o

ICONOBJS = \
	$(OBJ)/psp/icon/mvs_s.o \
	$(OBJ)/psp/icon/mvs_l.o

endif


#------------------------------------------------------------------------------
# Object Files (common)
#------------------------------------------------------------------------------

FONTOBJS = \
	$(OBJ)/psp/font/jpnfont.o \
	$(OBJ)/psp/font/graphic.o \
	$(OBJ)/psp/font/ascii_14p.o \
	$(OBJ)/psp/font/font_s.o \
	$(OBJ)/psp/font/bshadow.o

EXTOBJS = \
	$(OBJ)/psp/psp.o \
	$(OBJ)/psp/blend.o \
	$(OBJ)/psp/config.o \
	$(OBJ)/psp/filer.o \
	$(OBJ)/psp/help.o \
	$(OBJ)/psp/input.o \
	$(OBJ)/psp/menu.o \
	$(OBJ)/psp/mesbox.o \
	$(OBJ)/psp/misc.o \
	$(OBJ)/psp/ticker.o \
	$(OBJ)/psp/usrintrf.o \
	$(OBJ)/psp/video.o \
	$(OBJ)/psp/sound.o

ifdef BUILD_CPS1PSP
EXTOBJS += $(OBJ)/psp/png.o
else
EXTOBJS += $(OBJ)/psp/bmp.o
endif

EXTOBJS += $(FONTOBJS) $(ICONOBJS) $(ADHOCOBJS)

ZLIB = $(OBJ)/zlib.a

OBJS= $(MAINOBJS) $(COREOBJS) $(EXTOBJS) $(ZLIB)


#------------------------------------------------------------------------------
# Rules to make libraries
#------------------------------------------------------------------------------

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

$(OBJ)/zlib.a:  \
	$(OBJ)/zlib/adler32.o \
	$(OBJ)/zlib/compress.o \
	$(OBJ)/zlib/crc32.o \
	$(OBJ)/zlib/deflate.o \
	$(OBJ)/zlib/inflate.o \
	$(OBJ)/zlib/inftrees.o \
	$(OBJ)/zlib/inffast.o \
	$(OBJ)/zlib/trees.o \
	$(OBJ)/zlib/zutil.o

#---------------------------------------------------------------------
# Rules to manage files (CZ80)
#---------------------------------------------------------------------

Z80_SRC = \
	src/cpu/z80/cz80.c \
	src/cpu/z80/cz80_op.c \
	src/cpu/z80/cz80_opCB.c \
	src/cpu/z80/cz80_opED.c \
	src/cpu/z80/cz80_opXY.c  \
	src/cpu/z80/cz80_opXYCB.c

$(OBJ)/cpu/z80/cz80.o: $(Z80_SRC)
	@echo Compiling $<...
	@$(CC) $(CDEFS) $(CFLAGS) -c $< -o$@

#------------------------------------------------------------------------------
# Rules to manage files
#------------------------------------------------------------------------------

$(OBJ)/%.o: src/%.c
	@echo Compiling $<...
	@$(CC) $(CDEFS) $(CFLAGS) -c $< -o$@

$(OBJ)/%.o: src/%.S
	@echo Assembling $<...
	@$(CC) $(CDEFS) $(CFLAGS) -c $< -o$@

$(OBJ)/%.o: src/%.s
	@echo Assembling $<...
	@$(AS) $(ASDEFS) $(ASFLAGS) -c $< -o$@

$(OBJ)/%.a:
	@echo Archiving $@...
	@$(AR) -r $@ $^

$(sort $(OBJDIRS)):
	@mkdir -p $(subst //,\,$@)

pspclean:
	@echo Remove all object files and directories.
	@rm -rd $(OBJ)
	@rm -rf $(PSP_EBOOT_SFO)
	@rm -rf $(TARGET).elf

delelf:
	@rm -rf $(PSP_EBOOT_SFO)
	@rm -rf $(TARGET).elf

maketree:
	@echo Making object tree...

mkdir:	maketree $(sort $(OBJDIRS))
