
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

OUTD=../build/jwlibL$(outd_suffix)

outd_jwlink = ../build/jwlinkL$(outd_suffix)
orl_dir = ../build/jwlinkL$(outd_suffix)
orl_lib = $(orl_dir)/orl.lib

watcom_dir   = ../watcom
lib_misc_dir = ../lib_misc

#cflags stuff

c_flags =-D__UNIX__ $(bitopts) -DUNALIGNED="" -D_WCUNALIGNED="" -DIDE_PGM -std=gnu99 $(extra_c_flags)

CC = gcc

xlibs = $(orl_lib)

.SUFFIXES:
.SUFFIXES: .c .o

proj_obj = \
    $(OUTD)/wlib.o     $(OUTD)/libio.o    $(OUTD)/symtable.o \
    $(OUTD)/omfproc.o  $(OUTD)/writelib.o $(OUTD)/convert.o  \
    $(OUTD)/wlibutil.o $(OUTD)/libwalk.o  $(OUTD)/symlist.o  \
    $(OUTD)/proclib.o  $(OUTD)/cmdline.o  $(OUTD)/error.o    \
    $(OUTD)/implib.o   $(OUTD)/elfobjs.o  $(OUTD)/orlrtns.o  \
    $(OUTD)/memfuncs.o $(OUTD)/ideentry.o $(OUTD)/idedrv.o   \
    $(OUTD)/idemsgfm.o $(OUTD)/idemsgpr.o $(OUTD)/maindrv.o  \
    $(OUTD)/omfutil.o  $(OUTD)/coffwrt.o  $(OUTD)/apiemu.o   \
    $(OUTD)/inlib.o    $(OUTD)/debug.o

proj_obj += $(outd_jwlink)/demangle.o

inc_dirs  = -Ih -I../orl/h -I$(lib_misc_dir)/h -I$(watcom_dir)/h 

#.c.o:
#	$(CC) -c $(inc_dirs) $(c_flags) -o $(OUTD)/$*.o $<
$(OUTD)/%.o: c/%.c
	$(CC) -c $(inc_dirs) $(c_flags) -o $(OUTD)/$*.o $<

all:  $(OUTD) $(OUTD)/$(TARGET1)

$(OUTD):
	@mkdir -p $(OUTD)

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

