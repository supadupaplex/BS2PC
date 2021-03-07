# Tested on Windows (MSYS + Mingw32 4.8.2)
# Use "make DEBUG=1 all" to have gdb enabled executable

# dirs
OBJDIR=./obj
BINDIR=./bin
RELDIR=./release

# bs2pc stuff
OBJS=$(OBJDIR)/bs2pc.o $(OBJDIR)/bs2pc_map.o $(OBJDIR)/bs2pc_nodraw.o \
	$(OBJDIR)/bs2pc_subdiv.o $(OBJDIR)/bs2pc_util.o $(OBJDIR)/bs2pc_wad.o \
	$(OBJDIR)/buildinfo.o
LIBS=-L$(OBJDIR) -lz

# zlib stuff
ZLIB_VER=1.2.11
ZLIB_DIR=./zlib
ZLIB_WDIR=$(ZLIB_DIR)/zlib-$(ZLIB_VER)

# toolchain
CC=gcc
LD=gcc
CFLAGS=-c -std=c99 -Wall -m32 -D ZLIB_STATIC -D __USE_MINGW_ANSI_STDIO \
	-Wno-unknown-pragmas
LDFLAGS=-m32 $(LIBS)

# debug
ifdef DEBUG
CFLAGS:=-g $(CFLAGS) -O0
LDFLAGS:=-g $(LDFLAGS)
else
CFLAGS:=$(CFLAGS) -O2
endif

# generic targets
all: bs2pc
clean: bs2pc-clean
clean-all: bs2pc-clean zlib-clean


# prepare release zip
release: chzip clean-all all
	cp README.md $(BINDIR)/readme.txt
	cp gpl-2.0.md $(BINDIR)/license.txt
	7z a -tzip -mx=9 $(RELDIR)/bs2pc-`date -u +%y%m%d%H%M%S`.zip $(BINDIR)/*


# BS2PC
$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

buildinfo.c: ALWAYS
	echo "const char *build_info = \"Build info: \\" > $@
	echo "[head] `git log --pretty=format:'%h' -n 1 || :`, \\" >> $@
	(LANG=en ; date -u +"[date] %b %d, %Y %H:%M:%S \\") >> $@
	echo "\\n\";" >> $@

bs2pc: cpu-chk dirs zlib-build buildinfo.c $(OBJS)
	$(LD) $(OBJS) -o $(BINDIR)/bs2pc $(LDFLAGS)

bs2pc-clean:
	rm -rf $(OBJDIR)
	rm -rf $(BINDIR)
	rm -rf $(RELDIR)
	rm -f ./buildinfo.c

# ZLIB
zlib-build: dirs
	cd $(ZLIB_DIR)	&& tar -xzf zlib-$(ZLIB_VER).tar.gz
ifeq ($(OS),Windows_NT)
	# windows
	cd $(ZLIB_WDIR)	&& "$(MAKE)" -fwin32/Makefile.gcc CFLAGS='-m32 -O3 -Wall'
else
	# other os
	cd $(ZLIB_WDIR)	&& CFLAGS='-m32 -O3 -Wall' ./configure && "$(MAKE)"
endif
	cp $(ZLIB_WDIR)/zlib.h ./
	cp $(ZLIB_WDIR)/zconf.h ./
	cp $(ZLIB_WDIR)/libz.a $(OBJDIR)/

zlib-clean:
	-rm -rf $(ZLIB_WDIR)
	-rm -f ./zlib.h
	-rm -f ./zconf.h
	-rm -f $(OBJDIR)/libz.a

# Misc
cpu-chk:
	# limit to x86 only
	uname -a | grep 'x86'

dirs:
	mkdir -p $(OBJDIR)
	mkdir -p $(BINDIR)
	mkdir -p $(RELDIR)

chzip:
	echo "> Checking 7z location ..." && which 7z

ALWAYS:
	# Enforce target
