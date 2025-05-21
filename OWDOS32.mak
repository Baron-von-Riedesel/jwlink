
# Makefile to create JWlinkD.exe.
# This makefile creates a DOS binary that uses the OW DOS32 runtime.
# (The DOS variant optionally created by file Makefile uses the Win32 runtime)
#
# Note: the debug version has 2 additional commands
# - XDBG=n - debug log ( usually n is 2 )
# - INTDBG - dump symbol table

# the path of the Open Watcom root directory
WATCOM=\ow20

DOS=1

!ifndef DEBUG
DEBUG=0
!endif

!if $(DEBUG)
wlink_trmem = 0
outd_suffix=D
!else
wlink_trmem = 0
outd_suffix=R
!endif

BOUT=build
OUTD=$(BOUT)\jwlD32$(outd_suffix)

proj_name = jwlink
host_os  = dos

!ifndef wlink_autodepends
wlink_autodepends = .AUTODEPEND
!endif

#!include trmem.mif

#
# common files
#
common_objs = &
    $(OUTD)/autogrp.obj   $(OUTD)/carve.obj     $(OUTD)/wcomdef.obj   $(OUTD)/cmd16m.obj   &
    $(OUTD)/cmdall.obj    $(OUTD)/cmddos.obj    $(OUTD)/cmdelf.obj    $(OUTD)/cmdline.obj  &
    $(OUTD)/cmdnov.obj    $(OUTD)/cmdos2.obj    $(OUTD)/cmdphar.obj   $(OUTD)/cmdtable.obj &
    $(OUTD)/cmdutils.obj  $(OUTD)/cmdqnx.obj    $(OUTD)/dbgall.obj    $(OUTD)/dbgcv.obj    &
    $(OUTD)/dbgdwarf.obj  $(OUTD)/dbginfo.obj   $(OUTD)/debug.obj     $(OUTD)/distrib.obj  &
    $(OUTD)/global.obj    $(OUTD)/hash.obj      $(OUTD)/impexp.obj    $(OUTD)/libr.obj     &
    $(OUTD)/libsupp.obj   $(OUTD)/linkutil.obj  $(OUTD)/load16m.obj   $(OUTD)/loaddos.obj  &
    $(OUTD)/loadelf.obj   $(OUTD)/loadelf2.obj  $(OUTD)/loadfile.obj  $(OUTD)/loadflat.obj &
    $(OUTD)/loadnov.obj   $(OUTD)/loados2.obj   $(OUTD)/loadpe.obj    $(OUTD)/loadphar.obj &
    $(OUTD)/loadqnx.obj   $(OUTD)/loadraw.obj   $(OUTD)/lsymtab.obj   $(OUTD)/mapio.obj    &
    $(OUTD)/mem.obj       $(OUTD)/mixcache.obj  $(OUTD)/msg.obj       $(OUTD)/objio.obj    &
    $(OUTD)/obj2supp.obj  $(OUTD)/objcalc.obj   $(OUTD)/objfree.obj   $(OUTD)/objnode.obj  &
    $(OUTD)/objomf.obj    $(OUTD)/objorl.obj    $(OUTD)/objpass1.obj  $(OUTD)/objpass2.obj &
    $(OUTD)/objstrip.obj  $(OUTD)/omfreloc.obj  $(OUTD)/overlays.obj  $(OUTD)/ovlsupp.obj  &
    $(OUTD)/permdata.obj  $(OUTD)/procfile.obj  $(OUTD)/reloc.obj     $(OUTD)/ring.obj     &
    $(OUTD)/ring2.obj     $(OUTD)/salloc.obj    $(OUTD)/spillio.obj   $(OUTD)/strtab.obj   &
    $(OUTD)/symmem.obj    $(OUTD)/symtrace.obj  $(OUTD)/toc.obj       $(OUTD)/wlink.obj    &
    $(OUTD)/wlnkmsg.obj   $(OUTD)/cmdraw.obj    &
    $(OUTD)/demangle.obj  &
!ifeq wlink_trmem 1
    $(OUTD)/$(trmem_objs) &
!endif
    $(OUTD)/exerespe.obj  $(OUTD)/sharedio.obj  $(OUTD)/rcstr.obj 

!ifeq use_virtmem 1
common_objs += $(OUTD)/virtmem.obj
!else
common_objs += $(OUTD)/virtpage.obj
!endif

#
# target OS dependent files
#
wlink_objs_dos   = $(OUTD)/linkio.obj
wlink_objs_linux = $(OUTD)/posixio.obj
wlink_objs_osx   = $(OUTD)/posixio.obj
wlink_objs_bsd   = $(OUTD)/posixio.obj
wlink_objs_nt    = $(OUTD)/ntio.obj
wlink_objs_os2   = $(OUTD)/ntio.obj

common_objs += $(wlink_objs_$(host_os))

comp_objs_exe = $(common_objs)

wres_dir=sdk/rc/wres
wrc_dir=sdk/rc/rc
lib_misc_dir=lib_misc
dwarf_dir=dwarf
watcom_dir=watcom

inc_dirs = -IH -I$(watcom_dir)\H -I$(WATCOM)\H 

!if $(DEBUG)
cflags = -od -d2 -w3 -hc -D_INT_DEBUG
!else
cflags = -ox -s -DNDEBUG
!endif

outd_orl_lib   = build\osi386W$(outd_suffix)
outd_dwarf_lib = build\osi386W$(outd_suffix)
outd_wres_lib  = build\wresW$(outd_suffix)
orl_lib  = $(outd_orl_lib)\orl.lib
dwarf_lib= $(outd_dwarf_lib)\dw.lib
wres_lib = $(outd_wres_lib)\wres.lib

.c{$(OUTD)}.obj: $($(proj_name)_autodepends)
	$(WATCOM)\binnt\wcc386 -q -bc -bt=dos $(cflags) $(extra_c_flags_$[&) $(inc_dirs) -fo$@ $[@

.c: c;$(wrc_dir)/c;$(lib_misc_dir)/c;$(trmem_dir)

################
# c flags stuff

!ifeq bootstrap 1
extra_c_flags = -I"$(lib_misc_dir)/h"
!else
extra_c_flags = -zp4
!ifeq use_virtmem 1
extra_c_flags += -DUSE_VIRTMEM -D__WATCOM_LFN__
!endif
!endif

extra_c_flags_ntio       = -I"$(wres_dir)/h"
extra_c_flags_posixio    = -I"$(wres_dir)/h"
extra_c_flags_linkio     = -I"$(wres_dir)/h"
extra_c_flags_objorl     = -I"orl/h"
extra_c_flags_orlstubs   = -I"orl/h"
extra_c_flags_dbgdwarf   = -I"$(dwarf_dir)/h"
!ifeq wlink_trmem 1
extra_c_flags_debug      = -DTRMEM
!endif
extra_c_flags_loadpe     = -I"$(wrc_dir)/h" -I"$(wres_dir)/h"
extra_c_flags_loados2    = -I"$(wrc_dir)/h" -I"$(wres_dir)/h"
extra_c_flags_demangle   = -I"$(lib_misc_dir)/h"
extra_c_flags_msg        = -I"$(lib_misc_dir)/h"
extra_c_flags_rcstr      = -DINSIDE_WLINK -I"$(wrc_dir)/h" -I"$(wres_dir)/h" -I"$(watcom_dir)/h"
extra_c_flags_exerespe   = -DINSIDE_WLINK -I"$(wrc_dir)/h" -I"$(wres_dir)/h" -I"$(watcom_dir)/h"
extra_c_flags_sharedio   = -I"$(wrc_dir)/h" -I"$(wres_dir)/h" -I"$(watcom_dir)/h"
extra_c_flags_trmem      = $(trmem_cflags)
extra_c_flags_mem        = $(trmem_cover_cflags)

#####################
# linker .EXE options

extra_l_flags =

xlibs = $(wres_lib) $(dwarf_lib) $(orl_lib)

!if $(DEBUG)
lflagsd = debug c op cvp, symfile
!else
lflagsd = 
!endif

#################
# explicit rules

ALL: $(BOUT) $(OUTD) $(OUTD)/JWlinkD.exe $(xlibs)

$(BOUT):
	@if not exist $(BOUT) mkdir $(BOUT)

$(OUTD):
	@if not exist $(OUTD) mkdir $(OUTD)

$(OUTD)/JWlinkD.exe : $(comp_objs_exe) $(xlibs)
	jwlink @<<
$(lflagsd) format windows pe hx runtime console
$(extra_l_flags)
file { $(common_objs) }
name $@
libpath $(WATCOM)\lib386\dos
libpath $(WATCOM)\lib386
Libfile cstrtdhr.obj
lib { $(xlibs) }
op quiet, stack=0x10000, heapsize=0x1000, map=$^*, stub=loadpero.bin
disable 171
op statics
!ifndef WLINK
segment CONST readonly
segment CONST2 readonly
!endif
<<

clean: .SYMBOLIC
	@if exist $(OUTD)\$(proj_name).exe erase $(OUTD)\$(proj_name).exe
	@if exist $(OUTD)\$(proj_name).map erase $(OUTD)\$(proj_name).map
	@if exist $(OUTD)\*.obj erase $(OUTD)\*.obj

