
WATCOM=\watcom

host_os  = nt
proj_name = cvpack

!ifndef DEBUG
DEBUG=0
!endif

!if $(DEBUG)
OUTD=..\build\cvpackDD
LOPTD=debug c op cvp
COPTD=-d2 -D_DEBUG
!else
OUTD=..\build\cvpackDR
LOPTD=
COPTD=
!endif

watcom_dir=..\watcom

cvpack_autodepends = .autodepend

inc_dirs = -Ihpp -I$(watcom_dir)\h -I$(WATCOM)\h

extra_cpp_flags = -xs $(COPTD)

exe_file = $(proj_name).exe

h_dir = hpp/
cpp_dir = cpp/
makefile = Makefile

HXDIR=\hx
dos_target=$(OUTD)\cvpackd.exe

object_files =  &
	$(OUTD)\cvpack.obj   $(OUTD)\retrieve.obj  $(OUTD)\cverror.obj   $(OUTD)\packtype.obj &
	$(OUTD)\subsect.obj  $(OUTD)\cssymbol.obj  $(OUTD)\makeexe.obj   $(OUTD)\symdis.obj  &
	$(OUTD)\typemap.obj  $(OUTD)\typearay.obj  $(OUTD)\typercrd.obj  $(OUTD)\common.obj

.cpp{$(OUTD)}.obj: $($(proj_name)_autodepends)
	$(WATCOM)\binnt\wpp386 -zq -w4 -we -oaxt -DNDEBUG -mf -zc -bc -bt=dos $(extra_cpp_flags) $(inc_dirs) -fo$@ $[@

.cpp : $(cpp_dir)
.hpp : $(h_dir)

ALL: $(OUTD) $(OUTD)\cvpackd.exe $(dos_target)

$(OUTD):
	@if not exist $(OUTD) mkdir $(OUTD)

$(OUTD)\cvpackd.exe : $(object_files) $(makefile)
	$(WATCOM)\binnt\wlink $(LOPTD) format win pe ru console name $*.exe @<<
FILE { $(object_files) }
libpath $(WATCOM)\lib386\DOS libpath $(WATCOM)\lib386
libfile cstrtdhr.obj
op q, map=$*, noredefs, stack=0x20000, heapsize=0x1000
<<

