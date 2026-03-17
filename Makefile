
# Makefile to create JWlink.exe (Win32) and JWLinkd.exe (DOS)
#
# Note: the debug version has 2 additional commands
# - XDBG=n - debug log ( usually n is 2 )
# - INTDBG - dump symbol table

# the path of the Open Watcom root directory
!ifndef WATCOM
WATCOM=\watcom
!endif

# set to 1 if the DOS version is to be built!
DOS=0
# set path of HX if DOS=1.
# The HXDEV package must be installed.
HXDIR=\hx

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
OUTD=$(BOUT)\jwlinkW$(outd_suffix)
outd_wres = $(BOUT)\wresW$(outd_suffix)

proj_name = jwlink
host_os  = nt

!if $(DOS)
!if !$(DEBUG)
dos_target = $(OUTD)/JWlinkd.exe
!endif
!endif

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
cflags = -od -d2 -w3 -hc -D_INT_DEBUG
!else
cflags = -ox -s -DNDEBUG
!endif

orl_lib = $(OUTD)\orl.lib
orl_inc_dirs = -Iorl\h -Ih -I$(watcom_dir)\h -I$(WATCOM)\H

dwarf_lib = $(OUTD)\dwarf.lib

wres_lib = $(outd_wres)\wres.lib

CC = $(WATCOM)\binnt\wcc386 -q -bc -bt=nt

{orl/c}.c{$(OUTD)}.obj: 
	@$(CC) $(cflags) -Iorl\coff\h -Iorl\elf\h -Iorl\omf\h $(orl_inc_dirs) -fo$@ $[@

{orl/coff/c}.c{$(OUTD)}.obj: 
	@$(CC) $(cflags) -Iorl\coff\h $(orl_inc_dirs) -fo$@ $[@

{orl/elf/c}.c{$(OUTD)}.obj: 
	@$(CC) $(cflags) -Iorl\elf\h $(orl_inc_dirs) -fo$@ $[@

{orl/omf/c}.c{$(OUTD)}.obj: 
	@$(CC) $(cflags) -Iorl\omf\h $(orl_inc_dirs) -fo$@ $[@

{dwarf/c}.c{$(OUTD)}.obj: 
	@$(CC) $(cflags) -I$(dwarf_dir)\h $(inc_dirs) -fo$@ $[@

wres_extra_cflags = -DMICROSOFT -DUNALIGNED=_WCUNALIGNED
wres_inc_dirs = -I$(wres_dir)\h -I$(watcom_dir)\h -Ih -I$(WATCOM)\h

{sdk/rc/wres/c}.c{$(outd_wres)}.obj: 
	@$(CC) $(cflags) $(wres_extra_cflags) $(wres_inc_dirs) -fo$@ $[@

.c{$(OUTD)}.obj: $($(proj_name)_autodepends)
	@$(CC) $(cflags) $(extra_c_flags_$[&) $(inc_dirs) -fo$@ $[@

.c: c;$(wrc_dir)/c;$(lib_misc_dir)/c;$(trmem_dir)

################
# c flags stuff

!ifeq bootstrap 1
extra_c_flags = -I"$(lib_misc_dir)/h"
!else
extra_c_flags = -zp4
!ifeq use_virtmem 1
extra_c_flags += -DUSE_VIRTMEM
!endif
!endif

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

ALL: $(BOUT) $(OUTD) $(outd_wres) $(OUTD)/JWlink.exe $(dos_target) $(xlibs)

$(BOUT):
	@if not exist $(BOUT) mkdir $(BOUT)

$(OUTD):
	@if not exist $(OUTD) mkdir $(OUTD)

$(outd_wres):
	@if not exist $(outd_wres) mkdir $(outd_wres)

$(OUTD)/JWlink.exe : $(comp_objs_exe) $(xlibs)
	@jwlink $(lflagsd) format windows pe runtime console $(extra_l_flags) @<<
libpath $(WATCOM)\lib386\nt;$(WATCOM)\lib386
file { $(common_objs) }
name $@
lib { $(xlibs) }
lib kernel32.lib, user32.lib
op q, norelocs, map=$^*, noredefs, stack=0x100000, heapsize=0x100000 com stack=0x1000
!ifndef WLINK
segment CONST readonly
segment CONST2 readonly
!endif
<<

$(OUTD)/JWlinkd.exe : $(comp_objs_exe) $(xlibs)
	@jwlink $(lflagsd) format windows pe hx runtime console $(extra_l_flags) @<<
Libpath $(WATCOM)\lib386\nt;$(WATCOM)\lib386
Libpath $(HXDIR)\Lib
file { $(common_objs) }
name $@
Library imphlp.lib, dkrnl32s.lib, duser32s.lib
Libfile $(HXDIR)\Lib\InitW3ow.obj
disable 1030
lib { $(xlibs) }
op stub=$(HXDIR)\Bin\loadpero.bin
op q, map=$^*, noredefs, stack=0x20000, heapsize=0x100000 com stack=0x1000
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

