
# Wmake makefile to create Win32 wlib.exe

proj_name = jwlib

WATCOM=\watcom

!ifndef wlib_autodepends
wlib_autodepends = .AUTODEPEND
!endif

wlib_trmem = 0

!ifndef DEBUG
DEBUG=0
!endif

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
    $(OUTD)/inlib.obj

comp_objs_exe = $(common_objs)

xlibs = $(orl_lib) 
external_dependencies = $(xlibs)

depends_exe = $(comp_objs_exe) $(external_dependencies)

#########
# cflags

!if $(DEBUG)
OUTD=OWLinuxD
extra_c_flags += -D__DEBUG__
cflags = -od -d2 -w3
!else
OUTD=OWLinuxR
cflags = -ox -s -DNDEBUG
!endif

extra_c_flags += -DIDE_PGM

.c: c;$(lib_misc_dir)/c
.h: h;$(watcom_dir)/h

inc_dirs = -Ih -I"$(orl_dir)/h" -I"$(lib_misc_dir)/h" -I"$(watcom_dir)/H" -I$(WATCOM)\LH 

.c{$(OUTD)}.obj: $($(proj_name)_autodepends)
	$(WATCOM)\binnt\wcc386 -q -zc -bc -bt=linux $(cflags) $(extra_c_flags) $(inc_dirs) -fo$@ $[@

ALL: $(OUTD) $(OUTD)/jwlib.

$(OUTD):
	@if not exist $(OUTD) mkdir $(OUTD)

$(OUTD)/jwlib.: $(depends_exe)
	jwlink format elf ru linux name $*. @<<
f {$(comp_objs_exe)}
libpath $(WATCOM)\lib386
libpath $(WATCOM)\lib386\linux
lib {$(xlibs) }
op q, map=$*
<<

