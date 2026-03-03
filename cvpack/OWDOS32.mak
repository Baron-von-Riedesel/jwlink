
# Wmake makefile to create cvpackd.exe
# OW 1.9 is used because OW 2.0 emits serious warnings that haven't been fixed yet.

proj_name = cvpack

WATCOM=\watcom

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
makefile = OWDOS32.mak

object_files =  &
	$(OUTD)\cvpack.obj   $(OUTD)\retrieve.obj  $(OUTD)\cverror.obj   $(OUTD)\packtype.obj &
	$(OUTD)\subsect.obj  $(OUTD)\cssymbol.obj  $(OUTD)\makeexe.obj   $(OUTD)\symdis.obj  &
	$(OUTD)\typemap.obj  $(OUTD)\typearay.obj  $(OUTD)\typercrd.obj  $(OUTD)\common.obj

.cpp{$(OUTD)}.obj: $($(proj_name)_autodepends)
	$(WATCOM)\binnt\wpp386 -zq -w4 -we -oaxt -DNDEBUG -mf -zc -bc -bt=dos $(extra_cpp_flags) $(inc_dirs) -fo$@ $[@

.cpp : $(cpp_dir)
.hpp : $(h_dir)

ALL: $(OUTD) $(OUTD)\cvpackd.exe

$(OUTD):
	@if not exist $(OUTD) mkdir $(OUTD)

$(OUTD)\$(proj_name)d.exe : $(object_files) $(makefile)
	@jwlink.exe $(LOPTD) format win pe hx ru console name $*.exe @<<
FILE { $(object_files) }
libpath $(WATCOM)\lib386\DOS libpath $(WATCOM)\lib386
libfile cstrtdhr.obj
op q, map=$^*, noredefs, stack=0x20000, heapsize=0x1000, stub=loadpero.bin
<<

clean: .SYMBOLIC
	@if exist $(OUTD)\$(proj_name)d.exe erase $(OUTD)\$(proj_name)d.exe
	@if exist $(OUTD)\$(proj_name)d.map erase $(OUTD)\$(proj_name)d.map
	@if exist $(OUTD)\*.obj erase $(OUTD)\*.obj

