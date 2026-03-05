#pmake/50: build os_osi os_nt os_os2 os_dos os_linux cpu_386

host_os  = osi
host_cpu = 386

watcom_dir=../watcom

proj_name       = orl
orl_autodepends = .autodepend

ifndef DEBUG
DEBUG=0
endif

ifeq ($(DEBUG),0)
extra_c_flags = -DNDEBUG -O2
OUTD= ../build/osi386LR
else
extra_c_flags = -D_INT_DEBUG -g
OUTD= ../build/osi386LD
endif

#c_flags = -D__UNIX__ -std=gnu99 -DLONG_IS_64BITS $(extra_c_flags)
c_flags = -D__UNIX__ -m32 -std=gnu99 $(extra_c_flags)

#
#  List of orl library object files
#

omf_objs =  $(OUTD)/omfload.o \
            $(OUTD)/omfmunge.o \
            $(OUTD)/omfentr.o \
            $(OUTD)/omfflhn.o \
            $(OUTD)/omfdrctv.o

elf_objs  = $(OUTD)/elfentr.o \
            $(OUTD)/elflwlv.o \
            $(OUTD)/elfflhn.o \
            $(OUTD)/elfload.o

coff_objs = $(OUTD)/coffentr.o \
            $(OUTD)/cofflwlv.o \
            $(OUTD)/coffflhn.o \
            $(OUTD)/coffload.o \
            $(OUTD)/coffimpl.o

orl_objs  = $(OUTD)/orlentry.o \
            $(OUTD)/orlflhnd.o \
            $(OUTD)/orlhash.o \
            $(omf_objs) \
            $(elf_objs) \
            $(coff_objs)

.c: ./c;./elf/c;./coff/c;./omf/c
.h: ./h;./elf/h;./coff/h;./omf/h

inc_dirs = -Ih -Ielf/h -Icoff/h -Iomf/h -I../h -I$(watcom_dir)/h

$(OUTD)/%.o: c/%.c
	gcc -c $(c_flags) $(inc_dirs) -o $(OUTD)/$*.o $<

$(OUTD)/%.o: omf/c/%.c
	gcc -c $(c_flags) $(inc_dirs) -o $(OUTD)/$*.o $<

$(OUTD)/%.o: coff/c/%.c
	gcc -c $(c_flags) $(inc_dirs) -o $(OUTD)/$*.o $<

$(OUTD)/%.o: elf/c/%.c
	gcc -c $(c_flags) $(inc_dirs) -o $(OUTD)/$*.o $<

ALL : $(OUTD) $(OUTD)/orl.lib

$(OUTD):
	mkdir -p $(OUTD)

$(OUTD)/orl.lib : $(orl_objs)
	@ar -r $(OUTD)/orl.lib $(orl_objs)

clean:
	@rm $(OUTD)/orl.lib
	@rm $(OUTD)/coff*.o
	@rm $(OUTD)/elf*.o
	@rm $(OUTD)/omf*.o
	@rm $(OUTD)/orl*.o

