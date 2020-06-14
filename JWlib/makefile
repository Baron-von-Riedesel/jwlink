
# Wmake makefile to create Win32 jwlib.exe

proj_name = JWlib

WATCOM=\watcom
# set to 0 if the DOS version is NOT to be built!
DOS=1

HXDIR=\hx

!ifndef debug
debug=0
!endif

!ifndef wlib_autodepends
wlib_autodepends = .AUTODEPEND
!endif

!if $(DOS)
dos_target = $(OUTD)/jwlibd.exe
!endif

!if $(debug)
extra_c_flags += -D__DEBUG__
OUTD=Debug
cflags = -od -d2 -w3
!else
OUTD=Release
cflags = -ox -s -DNDEBUG
!endif


wlib_trmem = 0

orl_dir      = ../orl
watcom_dir   = ../watcom
lib_misc_dir = ../lib_misc

orl_lib  = $(orl_dir)/osi386/orl.lib

##########
# objects

common_objs = &
    $(OUTD)/wlib.obj     $(OUTD)/libio.obj    $(OUTD)/symtable.obj &
    $(OUTD)/omfproc.obj  $(OUTD)/writelib.obj $(OUTD)/convert.obj  &
    $(OUTD)/wlibutil.obj $(OUTD)/libwalk.obj  $(OUTD)/symlist.obj  &
    $(OUTD)/proclib.obj  $(OUTD)/cmdline.obj  $(OUTD)/error.obj    &
    $(OUTD)/implib.obj   $(OUTD)/elfobjs.obj  $(OUTD)/orlrtns.obj  &
    $(OUTD)/memfuncs.obj $(OUTD)/ideentry.obj $(OUTD)/idedrv.obj   &
    $(OUTD)/idemsgfm.obj $(OUTD)/idemsgpr.obj $(OUTD)/maindrv.obj  &
!if $(wlib_trmem)
    $(OUTD)/trmemcvr.obj &
!endif
    $(OUTD)/demangle.obj $(OUTD)/omfutil.obj  $(OUTD)/coffwrt.obj &
    $(OUTD)/inlib.obj    $(OUTD)/debug.obj

comp_objs_exe = $(common_objs)

xlibs = $(orl_lib) 
external_dependencies = $(xlibs)

depends_exe = $(comp_objs_exe) $(external_dependencies)

#########
# cflags

extra_c_flags += -DIDE_PGM

.c: c;$(lib_misc_dir)/c
.h: h;$(watcom_dir)/h

inc_dirs = -Ih -I"$(orl_dir)/h" -I"$(lib_misc_dir)/h" -I"$(watcom_dir)/H" -I$(WATCOM)\H

.c{$(OUTD)}.obj: $($(proj_name)_autodepends)
	$(WATCOM)\binnt\wcc386 -q -zc -bc -bt=nt $(cflags) $(extra_c_flags) $(inc_dirs) -fo$@ $[@

ALL: $(OUTD) $(OUTD)/jwlib.exe $(dos_target)

$(OUTD):
	@if not exist $(OUTD) mkdir $(OUTD)

$(OUTD)/jwlib.exe: $(depends_exe)
	jwlink format win pe ru console name $*.exe @<<
f {$(comp_objs_exe)}
libpath $(WATCOM)\lib386
libpath $(WATCOM)\lib386\nt
lib {$(xlibs) kernel32 user32 clib3r }
op q, norelocs, map=$*, stack=0x100000, heapsize=0x100000 com stack=0x1000
<<

$(OUTD)/jwlibd.exe : $(depends_exe)
	jwlink format win pe hx ru console name $*.exe @<<
f {$(comp_objs_exe)}
Libfile $(HXDIR)\lib\initw3ow.obj
libpath $(WATCOM)\lib386\nt
libpath $(WATCOM)\lib386
libpath $(HXDIR)\lib
Lib imphlp, dkrnl32s, duser32s
lib { $(xlibs) }
op stub=$(HXDIR)\Bin\loadpex.bin
op q, map=$^*, noredefs, stack=0x20000, heapsize=0x100000 com stack=0x1000
<<
