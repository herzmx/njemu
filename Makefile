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
#KERNEL_MODE = 1
RELEASE = 1


#------------------------------------------------------------------------------
# Defines
#------------------------------------------------------------------------------

VERSION_MAJOR = 1
VERSION_MINOR = 6
VERSION_BUILD = 6

ifdef BUILD_CPS1PSP
BUILD_CPS2PSP=
BUILD_MVSPSP=
TARGET = CPS1PSP
endif

ifdef BUILD_CPS2PSP
BUILD_MVSPSP=
TARGET = CPS2PSP
endif

ifdef BUILD_MVSPSP
VERSION_MAJOR = 1
VERSION_MINOR = 6
VERSION_BUILD = 3
TARGET = MVSPSP
endif

PBPNAME_STR = $(TARGET)
VERSION_STR = $(VERSION_MAJOR).$(VERSION_MINOR)$(VERSION_BUILD)

EXTRA_TARGETS = maketree EBOOT.PBP delelf


#------------------------------------------------------------------------------
# Utilities
#------------------------------------------------------------------------------

ifeq ($(PSPDEV),)
MD = -mkdir
RM = -rm
else
MD = -mkdir.exe
RM = -rm.exe
endif


#------------------------------------------------------------------------------
# File include path
#------------------------------------------------------------------------------

INCDIR = \
	src \
	src/zip \
	src/zlib


#------------------------------------------------------------------------------
# Object Directory
#------------------------------------------------------------------------------

OBJDIRS = \
	$(OBJ) \
	$(OBJ)/cpu \
	$(OBJ)/common \
	$(OBJ)/sound \
	$(OBJ)/zip \
	$(OBJ)/zlib \
	$(OBJ)/psp \
	$(OBJ)/psp/font \
	$(OBJ)/psp/icon


#------------------------------------------------------------------------------
# Object Files (common)
#------------------------------------------------------------------------------

MAINOBJS = \
	$(OBJ)/emumain.o \
	$(OBJ)/zip/zfile.o \
	$(OBJ)/zip/unzip.o \
	$(OBJ)/sound/sndintrf.o \
	$(OBJ)/common/cache.o \
	$(OBJ)/common/loadrom.o

ifdef SAVE_STATE
MAINOBJS += $(OBJ)/common/state.o
endif


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

EXTOBJS += $(FONTOBJS) $(ICONOBJS)

ZLIB = $(OBJ)/zlib.a


#------------------------------------------------------------------------------
# Include makefiles
#------------------------------------------------------------------------------

include src/makefiles/$(TARGET).mak


#------------------------------------------------------------------------------
# Compiler Flags
#------------------------------------------------------------------------------

CFLAGS = -O2 \
	-fomit-frame-pointer \
	-fstrict-aliasing \
	-Wall \
	-Wundef \
	-Wpointer-arith  \
	-Wbad-function-cast \
	-Wwrite-strings \
	-Wsign-compare \
	-Wmissing-prototypes \
	-Werror

#------------------------------------------------------------------------------
# Compiler Defines
#------------------------------------------------------------------------------

CDEFS = -DINLINE='static __inline' \
	-Dinline=__inline \
	-D__inline__=__inline \
	-DBUILD_$(TARGET)=1 \
	-DPBPNAME_STR='"$(PBPNAME_STR)"' \
	-DVERSION_STR='"$(VERSION_STR)"' \
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

ifdef RELEASE
CDEFS += -DRELEASE=1
else
CDEFS += -DRELEASE=0
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
# Rules to make libraries
#------------------------------------------------------------------------------

OBJS = $(MAINOBJS) $(COREOBJS) $(EXTOBJS) $(ZLIB)

include src/makefiles/build.mak

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

delelf:
	@$(RM) -f $(PSP_EBOOT_SFO)
	@$(RM) -f $(TARGET).elf

maketree:
	@$(MD) -p $(subst //,\,$(sort $(OBJDIRS)))
