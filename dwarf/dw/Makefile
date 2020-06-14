
proj_name = dwarfw

OUTD=osi386

!ifndef WATCOM
WATCOM=\watcom
!endif

!ifndef dwarfw_autodepends
dwarfw_autodepends = .AUTODEPEND
!endif

objs = $(OUTD)/dwlngen.obj $(OUTD)/dwutils.obj

watcom_dir=..\..\watcom

.c : c

inc_dirs = -IH -I$(watcom_dir)\H -I$(WATCOM)\H 

cflags = -ox -s -DNDEBUG

extra_c_flags = 

.c{$(OUTD)}.obj: $($(proj_name)_autodepends)
	$(WATCOM)\binnt\wcc386 -q -zc -bc -bt=nt $(cflags) $(extra_c_flags_$[&) $(inc_dirs) -fo$@ $[@

ALL: $(OUTD) $(OUTD)/dw.lib

$(OUTD):
	@if not exist $(OUTD) mkdir $(OUTD)

$(OUTD)/dw.lib : $(objs)
        @%create tmp.lbc
        @for %i in ($(objs)) do @%append tmp.lbc +%i
        wlib -n $(OUTD)/dw.lib @tmp.lbc

