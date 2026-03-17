
# Makefile to create JWlinkD.exe.
# This makefile creates a DOS binary that uses the OW DOS32 runtime.
# (The DOS variant optionally created by file Makefile uses the Win32 runtime)
#
# Note: the debug version has 2 additional commands
# - XDBG=n - debug log ( usually n is 2 )
# - INTDBG - dump symbol table

# the path of the Open Watcom root directory.
!ifndef WATCOM
WATCOM=\OW20
!endif

!ifndef DEBUG
DEBUG=0
!endif

!if $(DEBUG)
jwlink_trmem = 0
outd_suffix=D
!else
jwlink_trmem = 0
outd_suffix=R
!endif

BOUT=build
OUTD=$(BOUT)\jwlinkD$(outd_suffix)
outd_wres = $(BOUT)\wresD$(outd_suffix)

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
!include owmod.inc

!ifeq jwlink_trmem 1
common_objs += $(OUTD)/$(trmem_objs)
!endif

common_objs += $(OUTD)/demangle.obj
common_objs += $(OUTD)/exerespe.obj  $(OUTD)/sharedio.obj  $(OUTD)/rcstr.obj 

!ifeq use_virtmem 1
common_objs += $(OUTD)/virtmem.obj
!else
common_objs += $(OUTD)/virtpage.obj
!endif

#linkio.c doesn't support LFN
#common_objs += $(OUTD)/linkio.obj
common_objs += $(OUTD)/ntio.obj

comp_objs_exe = $(common_objs)

orl_objs = &
!include orl/owmod.inc

dwarf_objs = $(OUTD)/dwlngen.obj $(OUTD)/dwutils.obj

wres_dir=sdk\rc\wres
wrc_dir=sdk\rc\rc
lib_misc_dir=lib_misc
dwarf_dir=dwarf
watcom_dir=watcom

wres_core_objs = &
!include $(wres_dir)/owmod.inc
wres_more_objs = &
!include $(wres_dir)/owmod2.inc

inc_dirs = -IH -I$(watcom_dir)\H -I$(WATCOM)\H 

!if $(DEBUG)
cflags = -od -d2 -w3 -hc -D_INT_DEBUG -D__WATCOM_LFN__
!else
cflags = -ox -s -DNDEBUG -D__WATCOM_LFN__
!endif

orl_lib  = $(OUTD)\orl.lib
orl_inc_dirs = -Iorl\h -Ih -I$(watcom_dir)\h -I$(WATCOM)\H

dwarf_lib  = $(OUTD)\dwarf.lib

wres_lib = $(outd_wres)\wres.lib

CC = $(WATCOM)\binnt\wcc386 -q -bc -bt=dos

{orl/c}.c{$(OUTD)}.obj: 
	@$(CC) $(cflags) -Iorl\coff\h -Iorl\elf\h -Iorl\omf\h $(orl_inc_dirs) -fo$@ $[@

{orl/coff/c}.c{$(OUTD)}.obj: 
	@$(CC) $(cflags) -Iorl\coff\h $(orl_inc_dirs) -fo$@ $[@

{orl/elf/c}.c{$(OUTD)}.obj: 
	@$(CC) $(cflags) -Iorl\elf\h $(orl_inc_dirs) -fo$@ $[@

{orl/omf/c}.c{$(OUTD)}.obj: 
	@$(CC) $(cflags) -Iorl\omf\h $(orl_inc_dirs) -fo$@ $[@

{dwarf/c}.c{$(OUTD)}.obj: 
	@$(CC) $(cflags) -Idwarf\h $(inc_dirs) -fo$@ $[@

wres_extra_cflags = -DMICROSOFT -DUNALIGNED=_WCUNALIGNED
wres_inc_dirs = -I$(wres_dir)\h -I$(watcom_dir)\h -Ih -I$(WATCOM)\h

{sdk/rc/wres/c}.c{$(outd_wres)}.obj: 
	@$(CC) $(cflags) $(wres_extra_cflags) $(wres_inc_dirs) -fo$@ $[@

.c{$(OUTD)}.obj: $($(proj_name)_autodepends)
	@$(CC) $(cflags) $(extra_c_flags_$[&) $(inc_dirs) -fo$@ $[@

.c: c;$(wrc_dir)/c;$(lib_misc_dir)/c;$(trmem_dir)

################
# c flags stuff

extra_c_flags = -zp4
extra_c_flags += -D__WATCOM_LFN__

extra_c_flags_ntio       = -I"$(wres_dir)/h"
extra_c_flags_posixio    = -I"$(wres_dir)/h"
extra_c_flags_linkio     = -I"$(wres_dir)/h"
extra_c_flags_objorl     = -I"orl/h"
extra_c_flags_orlstubs   = -I"orl/h"
extra_c_flags_dbgdwarf   = -I"$(dwarf_dir)/h"
!ifeq jwlink_trmem 1
extra_c_flags_debug      = -DTRMEM
!endif
extra_c_flags_loadpe     = -DUNALIGNED=_WCUNALIGNED -I"$(wrc_dir)/h" -I"$(wres_dir)/h"
extra_c_flags_loados2    = -I"$(wrc_dir)/h" -I"$(wres_dir)/h"
extra_c_flags_demangle   = -I"$(lib_misc_dir)/h"
extra_c_flags_msg        = -I"$(lib_misc_dir)/h"
extra_c_flags_rcstr      = -DUNALIGNED=_WCUNALIGNED -DINSIDE_WLINK -I"$(wrc_dir)/h" -I"$(wres_dir)/h" -I"$(watcom_dir)/h"
extra_c_flags_exerespe   = -DUNALIGNED=_WCUNALIGNED -DINSIDE_WLINK -I"$(wrc_dir)/h" -I"$(wres_dir)/h" -I"$(watcom_dir)/h"
extra_c_flags_sharedio   = -DUNALIGNED=_WCUNALIGNED -I"$(wrc_dir)/h" -I"$(wres_dir)/h" -I"$(watcom_dir)/h"
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

ALL: $(BOUT) $(OUTD) $(outd_wres) $(OUTD)/JWlinkD.exe $(xlibs)

$(BOUT):
	@if not exist $(BOUT) mkdir $(BOUT)

$(OUTD):
	@if not exist $(OUTD) mkdir $(OUTD)

$(outd_wres):
	@if not exist $(outd_wres) mkdir $(outd_wres)

$(OUTD)/JWlinkD.exe : $(comp_objs_exe) $(xlibs)
	@jwlink.exe @<<
$(lflagsd) format windows pe hx runtime console
$(extra_l_flags)
file { $(common_objs) }
name $@
libpath $(WATCOM)\lib386\dos
libpath $(WATCOM)\lib386
Libfile cstrtdhr.obj, inirmlfn.obj
lib { $(xlibs) }
op quiet, stack=0x10000, heapsize=0x1000, map=$^*, stub=loadpero.bin
disable 171
op statics
!ifndef WLINK
segment CONST readonly
segment CONST2 readonly
!endif
<<

$(orl_lib): $(orl_objs)
	@$(WATCOM)\binnt\wlib -q -n $(orl_lib) @<<
$(orl_objs:$(OUTD)= +$(OUTD))
<<

$(dwarf_lib): $(dwarf_objs)
	@$(WATCOM)\binnt\wlib -q -n $(dwarf_lib) @<<
$(dwarf_objs:$(OUTD)= +$(OUTD))
<<

$(wres_lib): $(wres_core_objs) $(wres_more_objs)
	@$(WATCOM)\binnt\wlib -q -n $(wres_lib) @<<
$(wres_core_objs:$(outd_wres)= +$(outd_wres)) $(wres_more_objs:$(outd_wres)= +$(outd_wres))
<<

clean: .SYMBOLIC
	@if exist $(OUTD)\$(proj_name).exe erase $(OUTD)\$(proj_name).exe
	@if exist $(OUTD)\$(proj_name).map erase $(OUTD)\$(proj_name).map
	@if exist $(OUTD)\*.obj erase $(OUTD)\*.obj
	@if exist $(orl_lib)   erase $(orl_lib)
	@if exist $(dwarf_lib) erase $(dwarf_lib)
	@if exist $(wres_lib)  erase $(wres_lib)
	@if exist $(outd_wres)\*.obj  erase $(outd_wres)\*.obj

