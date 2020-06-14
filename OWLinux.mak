
# Makefile to create jwlink, the Linux binary
#
# This depends on the following projects:
# orl, plusplus, dwarf, watcom, and trmem

# the path of the Open Watcom root directory
WATCOM=\watcom

!ifndef DEBUG
DEBUG=0
!endif

!if $(DEBUG)
OUTD=OWLinuxD
!else
OUTD=OWLinuxR
!endif

wlink_trmem = 0

host_os  = linux
host_cpu = 386

proj_name = jwlink

!ifndef wlink_autodepends
wlink_autodepends = .AUTODEPEND
!endif

# get rid of the -zc
suppress_zc = 1

#!include trmem.mif

#
# stand-alone executable
#
exe_objs =

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
    $(OUTD)/wlnkmsg.obj   $(OUTD)/demangle.obj  $(OUTD)/exerespe.obj  $(OUTD)/sharedio.obj &
    $(OUTD)/cmdraw.obj    &
!ifeq wlink_trmem
    $(OUTD)/$(trmem_objs) &
!endif
    $(OUTD)/rcstr.obj 

!ifeq use_virtmem 1
common_objs += $(OUTD)/virtmem.obj
!else
common_objs += $(OUTD)/virtpage.obj
!endif

#
# target OS dependent files
#
wlink_objs_dos   = $(OUTD)/linkio.obj
wlink_objs_qnx   = $(OUTD)/posixio.obj
wlink_objs_linux = $(OUTD)/posixio.obj
wlink_objs_osx   = $(OUTD)/posixio.obj
wlink_objs_bsd   = $(OUTD)/posixio.obj
wlink_objs_nt    = $(OUTD)/ntio.obj
wlink_objs_os2   = $(OUTD)/ntio.obj

common_objs += $(wlink_objs_$(host_os))

!ifdef no_orl
# do not link against ORL
common_objs += $(OUTD)/orlstubs.obj
orl_lib = 
!endif

comp_objs_exe = $(common_objs) $(exe_objs)

wres_dir=sdk/rc/wres
wrc_dir=sdk/rc/rc
lib_misc_dir=lib_misc
dwarf_dir=dwarf
watcom_dir=watcom

orl_lib = orl/osi386/orl.lib
dwarf_dw_lib= $(dwarf_dir)/dw/osi386/dw.lib
wres_lib= $(wres_dir)/flat386/wres.lib

inc_dirs = -IH -I$(watcom_dir)\H -I$(WATCOM)\LH -I$(WATCOM)\H 

!if $(DEBUG)
cflags = -od -d2 -w3 -D_INT_DEBUG 
!else
cflags = -ox -s -DNDEBUG
!endif

.c{$(OUTD)}.obj: $($(proj_name)_autodepends)
	$(WATCOM)\binnt\wcc386 -q -zc -bc -bt=linux $(cflags) $(extra_c_flags_$[&) $(inc_dirs) -fo$@ $[@

.c: c;$(wrc_dir)/c;$(lib_misc_dir)/c;$(trmem_dir)

################
# c flags stuff

!ifeq bootstrap 1
comp_objs_exe += clibext.obj
!endif

!ifeq bootstrap 1
extra_c_flags = -I"$(lib_misc_dir)/h"
!else
extra_c_flags = -zp4
!ifdef wlink_dll
extra_c_flags += -D_DLLHOST
!else ifdef wlink_rtdll
extra_c_flags += -D_DLLHOST
!endif
!ifeq use_virtmem 1
extra_c_flags += -DUSE_VIRTMEM
!endif
!endif

extra_c_flags_ntio       = -I"$(wres_dir)/h"
extra_c_flags_posixio    = -I"$(wres_dir)/h"
extra_c_flags_linkio     = -I"$(wres_dir)/h"
extra_c_flags_objorl     = -I"orl/h"
extra_c_flags_orlstubs   = -I"orl/h"
extra_c_flags_dbgdwarf   = -I"$(dwarf_dir)/dw/h"
!ifdef wlink_trmem
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

extra_l_flags     = op map=$^*, noredefs, norelocs

xlibs = $(wres_lib) $(dwarf_dw_lib) $(orl_lib)

!if $(DEBUG)
lflags = debug dwarf op symfile $(extra_l_flags)
!else
lflags = $(extra_l_flags)
!endif

#################
# explicit rules

ALL: $(OUTD) $(OUTD)/jwlink.

$(OUTD):
	@if not exist $(OUTD) mkdir $(OUTD)

$(OUTD)/jwlink. : $(comp_objs_exe) $(xlibs)
	wlink format elf runtime linux $(lflags) name $@ @<<
file { $(common_objs) }
libpath $(WATCOM)/lib386
libpath $(WATCOM)/lib386/linux
lib { $(xlibs) }
<<

$(wres_lib):
	@cd $(wres_dir)
	@wmake debug=$(DEBUG) watcom=$(WATCOM)
	@cd ../../..

$(orl_lib):
	@cd orl
	@wmake debug=$(DEBUG) watcom=$(WATCOM)
	@cd ..
