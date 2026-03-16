
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

OUTD=build/jwlinkL$(outd_suffix)

orl_lib = $(OUTD)/orl.lib
outd_wres = build/wresL$(outd_suffix)
wres_lib = $(outd_wres)/wres.lib

wres_dir=sdk/rc/wres
wrc_dir=sdk/rc/rc
lib_misc_dir=lib_misc
dwarf_dir=dwarf
watcom_dir=watcom

inc_dirs  = -Ih -I$(watcom_dir)/h -I$(dwarf_dir)/h -Iorl/h -I$(wrc_dir)/h -I$(wres_dir)/h -I$(lib_misc_dir)/h
orl_inc_dirs = -Iorl/h -I$(watcom_dir)/h -Ih

#cflags stuff

c_flags =-D__UNIX__ $(bitopts) -DUNALIGNED="" -std=gnu99 $(extra_c_flags)

CC = gcc

.SUFFIXES:
.SUFFIXES: .c .o

include gccmod.inc

proj_obj += $(OUTD)/demangle.o
proj_obj += $(OUTD)/rcstr.o $(OUTD)/exerespe.o $(OUTD)/sharedio.o

include orl/gccmod.inc

dwarf_obj = $(OUTD)/dwlngen.o $(OUTD)/dwutils.o

include $(wres_dir)/gccmod.inc
include $(wres_dir)/gccmod2.inc

$(OUTD)/%.o: c/%.c
	@$(CC) -c $(inc_dirs) $(c_flags) -o $(OUTD)/$*.o $<

$(OUTD)/%.o: $(lib_misc_dir)/c/%.c
	@$(CC) -c $(inc_dirs) $(c_flags) -o $(OUTD)/$*.o $<

$(OUTD)/%.o: orl/c/%.c
	@$(CC) -c -Iorl/coff/h -Iorl/elf/h -Iorl/omf/h $(orl_inc_dirs) $(c_flags) -o $(OUTD)/$*.o $<

$(OUTD)/%.o: orl/coff/c/%.c
	@$(CC) -c -Iorl/coff/h $(orl_inc_dirs) $(c_flags) -o $(OUTD)/$*.o $<

$(OUTD)/%.o: orl/elf/c/%.c
	@$(CC) -c -Iorl/elf/h $(orl_inc_dirs) $(c_flags) -o $(OUTD)/$*.o $<

$(OUTD)/%.o: orl/omf/c/%.c
	@$(CC) -c -Iorl/omf/h $(orl_inc_dirs) $(c_flags) -o $(OUTD)/$*.o $<

$(OUTD)/%.o: $(dwarf_dir)/c/%.c
	@$(CC) -c $(inc_dirs) $(c_flags) -o $(OUTD)/$*.o $<

$(OUTD)/%.o: $(wrc_dir)/c/%.c
	@$(CC) -c -DINSIDE_WLINK $(inc_dirs) $(c_flags) -o $(OUTD)/$*.o $<

wres_inc_dirs = -I$(wres_dir)/h -I$(watcom_dir)/h -Ih

$(outd_wres)/%.o: $(wres_dir)/c/%.c
	@$(CC) -c $(wres_inc_dirs) $(c_flags) -DMICROSOFT -o $(outd_wres)/$*.o $<

all:  $(OUTD) $(outd_wres) $(OUTD)/$(TARGET1)

$(OUTD):
	mkdir -p $(OUTD)

$(outd_wres):
	mkdir -p $(outd_wres)

$(OUTD)/$(TARGET1) : $(OUTD)/$(TARGET1).lib $(orl_lib) $(wres_lib)
ifeq ($(DEBUG),0)
	@$(CC) $(OUTD)/$(TARGET1).lib $(orl_lib) $(wres_lib) $(bitopts) -s -o $@ -Wl,-Map,$(OUTD)/$(TARGET1).map
else
	@$(CC) $(OUTD)/$(TARGET1).lib $(orl_lib) $(wres_lib) $(bitopts) -o $@ -Wl,-Map,$(OUTD)/$(TARGET1).map
endif

$(OUTD)/$(TARGET1).lib: $(proj_obj) $(dwarf_obj)
	@ar -r $@ $(proj_obj) $(dwarf_obj)

$(orl_lib): $(orl_obj)
	@ar -r $@ $(orl_obj)

$(wres_lib): $(wres_core_objs) $(wres_more_objs)
	@ar -r $@ $(wres_core_objs) $(wres_more_objs)

######

install:
	@install $(OUTD)/$(TARGET1) /usr/local/bin

clean:
	@rm -f $(OUTD)/$(TARGET1)
	@rm -f $(OUTD)/*.o
	@rm -f $(OUTD)/*.map
	@rm $(wres_lib)
	@rm $(outd_wres)/*.o

