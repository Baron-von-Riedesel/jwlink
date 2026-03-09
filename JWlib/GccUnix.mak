
# This makefile creates the jwlib Elf binary for Linux/FreeBSD using GCC.

TARGET1=jwlib

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

outd_orl_lib   = ../build/osi386L$(outd_suffix)
orl_lib  = $(outd_orl_lib)/orl.lib

orl_dir      = ../build/osi386WR
watcom_dir   = ../watcom
lib_misc_dir = ../lib_misc

inc_dirs  = -Ih -I$(watcom_dir)/h -I$(dwarf_dir)/h -Iorl/h -I$(wrc_dir)/h -I$(wres_dir)/h -I$(lib_misc_dir)/h

#cflags stuff

c_flags =-D__UNIX__ $(bitopts) -DUNALIGNED="" -std=gnu99 $(extra_c_flags)

CC = gcc

xlibs = $(orl_lib)

.SUFFIXES:
.SUFFIXES: .c .o

common_objs = &
    $(OUTD)/wlib.obj     $(OUTD)/libio.obj    $(OUTD)/symtable.obj &
    $(OUTD)/omfproc.obj  $(OUTD)/writelib.obj $(OUTD)/convert.obj  &
    $(OUTD)/wlibutil.obj $(OUTD)/libwalk.obj  $(OUTD)/symlist.obj  &
    $(OUTD)/proclib.obj  $(OUTD)/cmdline.obj  $(OUTD)/error.obj    &
    $(OUTD)/implib.obj   $(OUTD)/elfobjs.obj  $(OUTD)/orlrtns.obj  &
    $(OUTD)/memfuncs.obj $(OUTD)/ideentry.obj $(OUTD)/idedrv.obj   &
    $(OUTD)/idemsgfm.obj $(OUTD)/idemsgpr.obj $(OUTD)/maindrv.obj  &
if $(wlib_trmem)
    $(OUTD)/trmemcvr.obj &
endif
    $(OUTD)/demangle.obj $(OUTD)/omfutil.obj  $(OUTD)/coffwrt.obj &
    $(OUTD)/inlib.obj    $(OUTD)/debug.obj

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

######

install:
	@install $(OUTD)/$(TARGET1) /usr/local/bin

clean:
	@rm -f $(OUTD)/$(TARGET1)
	@rm -f $(OUTD)/*.o
	@rm -f $(OUTD)/*.map

