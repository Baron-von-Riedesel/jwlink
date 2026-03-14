#pmake/50: build os_osi os_nt os_os2 os_dos os_linux cpu_386

host_os  = osi
host_cpu = 386

!ifndef WATCOM
WATCOM= \watcom
!endif

watcom_dir=..\watcom

proj_name       = orl
orl_autodepends = .autodepend

!ifndef DEBUG
DEBUG=0
!endif

!if $(DEBUG)
c_flags = -od -d2 -w3 -D_INT_DEBUG
OUTD= ..\build\osi386WD
!else
c_flags = -ox -s -DNDEBUG
OUTD= ..\build\osi386WR
!endif

#
#  List of orl library object files
#

orl_objs = &
!include owmod.inc

.c{$(OUTD)}.obj: $($(proj_name)_autodepends)
	$(WATCOM)\binnt\wcc386 -q -zc -bc -bt=nt $(c_flags) $(inc_dirs) -fo$@ $[@

.c: ./c;./elf/c;./coff/c;./omf/c
.h: ./h;./elf/h;./coff/h;./omf/h

inc_dirs = -Ih -Ielf\h -Icoff\h -Iomf\h -I..\h -I$(watcom_dir)\h -I$(WATCOM)\H

ALL : $(OUTD) $(OUTD)/orl.lib

$(OUTD):
	@if not exist $(OUTD) mkdir $(OUTD)

$(OUTD)/orl.lib : $(orl_objs)
	@$(WATCOM)\binnt\wlib -n $(OUTD)/orl.lib $(libflags) @<<
$(orl_objs:$(OUTD)= +$(OUTD))
<<

clean:
	@erase $(OUTD)/orl.lib
