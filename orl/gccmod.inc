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

proj_obj = $(orl_objs)
