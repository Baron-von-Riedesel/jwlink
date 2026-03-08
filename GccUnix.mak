
# This makefile creates the jwlink Elf binary for Linux/FreeBSD using GCC.

# The 64-bit variant still is somewhat "experimental"; in case of
# problems, create the 32-bit variant ( see bitopts below ).

TARGET1=jwlink

ifndef DEBUG
DEBUG=0
endif

bitopts = -DLONG_IS_64BITS
#bitopts = -m32

ifeq ($(DEBUG),0)
extra_c_flags = -DNDEBUG -O2
outd_suffix=R
else
extra_c_flags = -D_INT_DEBUG -g
outd_suffix=D
endif

OUTD=build/GccUnix$(outd_suffix)

outd_orl_lib   = build/osi386L$(outd_suffix)
outd_dwarf_lib = build/osi386L$(outd_suffix)
outd_wres_lib  = build/wresL$(outd_suffix)
orl_lib  = $(outd_orl_lib)/orl.lib
dwarf_lib= $(outd_dwarf_lib)/dw.lib
wres_lib = $(outd_wres_lib)/wres.lib

wres_dir=sdk/rc/wres
wrc_dir=sdk/rc/rc
lib_misc_dir=lib_misc
dwarf_dir=dwarf
orl_dir=orl
watcom_dir=watcom

inc_dirs  = -Ih -I$(watcom_dir)/h -I$(dwarf_dir)/h -Iorl/h -I$(wrc_dir)/h -I$(wres_dir)/h -I$(lib_misc_dir)/h

#cflags stuff

c_flags =-D__UNIX__ $(bitopts) -DUNALIGNED="" -std=gnu99 $(extra_c_flags)

CC = gcc

xlibs = $(dwarf_lib) $(orl_lib) $(wres_lib)

.SUFFIXES:
.SUFFIXES: .c .o

include gccmod.inc

proj_obj += $(OUTD)/rcstr.o $(OUTD)/exerespe.o $(OUTD)/sharedio.o

#.c.o:
#	$(CC) -c $(inc_dirs) $(c_flags) -o $(OUTD)/$*.o $<
$(OUTD)/%.o: c/%.c
	$(CC) -c $(inc_dirs) $(c_flags) -o $(OUTD)/$*.o $<

all:  $(OUTD) $(OUTD)/$(TARGET1)

$(OUTD):
	mkdir -p $(OUTD)

$(OUTD)/$(TARGET1) : $(proj_obj) $(xlibs)
ifeq ($(DEBUG),0)
	$(CC) $(proj_obj) $(xlibs) $(bitopts) -s -o $@ -Wl,-Map,$(OUTD)/$(TARGET1).map
else
	$(CC) $(proj_obj) $(xlibs) $(bitopts) -o $@ -Wl,-Map,$(OUTD)/$(TARGET1).map
endif

$(OUTD)/rcstr.o: sdk/rc/rc/c/rcstr.c
	$(CC) -c -DINSIDE_WLINK $(inc_dirs) $(c_flags) -o $*.o $<

$(OUTD)/exerespe.o: sdk/rc/rc/c/exerespe.c
	$(CC) -c -DINSIDE_WLINK $(inc_dirs) $(c_flags) -o $*.o $<

$(OUTD)/sharedio.o: sdk/rc/rc/c/sharedio.c
	$(CC) -c $(inc_dirs) $(c_flags) -o $*.o $<


######

$(orl_lib):
	make -C $(orl_dir) -f GccUnix.mak bitopts=$(bitopts) DEBUG=$(DEBUG)

$(dwarf_lib):
	make -C $(dwarf_dir) -f GccUnix.mak bitopts=$(bitopts) DEBUG=$(DEBUG)

$(wres_lib):
	make -C $(wres_dir) -f GccUnix.mak bitopts=$(bitopts) DEBUG=$(DEBUG)

install:
	@install $(OUTD)/$(TARGET1) /usr/local/bin

clean:
	@rm -f $(OUTD)/$(TARGET1)
	@rm -f $(OUTD)/*.o
	@rm -f $(OUTD)/*.map
	@make -C $(orl_dir)   -f GccUnix.mak DEBUG=$(DEBUG) clean
	@make -C $(dwarf_dir) -f GccUnix.mak DEBUG=$(DEBUG) clean
	@make -C $(wres_dir)  -f GccUnix.mak DEBUG=$(DEBUG) clean

