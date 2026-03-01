
# This makefile creates the jwlink ELF binary for Linux/FreeBSD.

TARGET1=jwlink

ifndef DEBUG
DEBUG=0
endif

inc_dirs = -I. -Ih -Iwatcom/h -Ilib_misc/h -Iorl/h -Idwarf/h -Isdk/rc/rc/h -Isdk/rc/wres/h

#cflags stuff

ifeq ($(DEBUG),0)
extra_c_flags = -DNDEBUG -O2
OUTD=build/GccUnixR
else
extra_c_flags = -DDEBUG_OUT -g
OUTD=build/GccUnixD
endif

# CC=clang allowed
CC ?= gcc
AR ?= ar

# Define LONG_IS_64BITS only on LP64 platforms (e.g. x86-64 Linux) where
# sizeof(long)==8, so that uint_32/unsigned_32 are kept at exactly 32 bits.
ifeq ($(shell $(CC) -x c /dev/null -E -dM 2>/dev/null | grep -c __LP64__),1)
long_flag = -DLONG_IS_64BITS
endif

c_flags = -D__UNIX__ -D_BSD_SOURCE $(long_flag) -DINSIDE_WLINK $(extra_c_flags) -D_WCUNALIGNED= -DO_BINARY=0 -Wno-incompatible-pointer-types

.SUFFIXES:
.SUFFIXES: .c .o

include gccmod.inc

ifeq ($(DEBUG),0)
orl_lib   = orl/GccUnixR/orl.a
dwarf_lib = dwarf/GccUnixR/dwarf.a
wres_lib  = sdk/rc/wres/GccUnixR/wres.a
else
orl_lib   = orl/GccUnixD/orl.a
dwarf_lib = dwarf/GccUnixD/dwarf.a
wres_lib  = sdk/rc/wres/GccUnixD/wres.a
endif

xlibs = $(wres_lib) $(dwarf_lib) $(orl_lib)

$(OUTD)/%.o: c/%.c
	$(CC) -c $(inc_dirs) $(c_flags) -o $@ $<
$(OUTD)/%.o: lib_misc/c/%.c
	$(CC) -c $(inc_dirs) $(c_flags) -o $@ $<
$(OUTD)/%.o: sdk/rc/rc/c/%.c
	$(CC) -c $(inc_dirs) $(c_flags) -o $@ $<

all: $(OUTD) $(xlibs) $(OUTD)/$(TARGET1)

$(OUTD):
	mkdir -p $(OUTD)

$(orl_lib):
	$(MAKE) -C orl -f GccUnix.mak DEBUG=$(DEBUG)

$(dwarf_lib):
	$(MAKE) -C dwarf -f GccUnix.mak DEBUG=$(DEBUG)

$(wres_lib):
	$(MAKE) -C sdk/rc/wres -f GccUnix.mak DEBUG=$(DEBUG)

$(OUTD)/$(TARGET1): $(proj_obj) $(xlibs)
ifeq ($(DEBUG),0)
	$(CC) -s -o $@ $(proj_obj) $(xlibs) -Wl,-Map,$(OUTD)/$(TARGET1).map
else
	$(CC) -o $@ $(proj_obj) $(xlibs) -Wl,-Map,$(OUTD)/$(TARGET1).map
endif

######

install:
	@install $(OUTD)/$(TARGET1) /usr/local/bin

clean:
	rm -f $(OUTD)/$(TARGET1)
	rm -f $(OUTD)/*.o
	rm -f $(OUTD)/*.map
	$(MAKE) -C orl -f GccUnix.mak clean DEBUG=$(DEBUG)
	$(MAKE) -C dwarf -f GccUnix.mak clean DEBUG=$(DEBUG)
	$(MAKE) -C sdk/rc/wres -f GccUnix.mak clean DEBUG=$(DEBUG)
