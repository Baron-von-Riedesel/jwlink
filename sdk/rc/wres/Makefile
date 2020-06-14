#
#  Makefile for   WRes library
#
# path names used in the script

proj_name = wres

wres_autodepends = .AUTODEPEND
wres_distrib_lib = 1

!ifndef WATCOM
WATCOM = \watcom
!endif

watcom_dir=..\..\..\watcom
wres_dir = .

!ifndef DEBUG
DEBUG=0
!endif

!if $(DEBUG)
OUTD=flat386d
!else
OUTD=flat386
!endif

#
#  List of WRes library object files
#

# Objects needed for a stripped down wres lib that can only read string resources
wres_core_objs = &
    $(OUTD)/loadfind.obj     $(OUTD)/loadstr.obj     $(OUTD)/mrrdrh.obj   &
    $(OUTD)/ralliae.obj      $(OUTD)/rclosef.obj     $(OUTD)/rduint16.obj &
    $(OUTD)/rduint32.obj     $(OUTD)/rduint8.obj     $(OUTD)/resseek.obj  &
    $(OUTD)/ropenfro.obj     $(OUTD)/rrdstr.obj      $(OUTD)/rrnamoor.obj &
    $(OUTD)/varstr.obj       $(OUTD)/wraddres.obj    $(OUTD)/wrerror.obj  &
    $(OUTD)/wrfindrs.obj     $(OUTD)/wrfirres.obj    $(OUTD)/wrfree.obj   &
    $(OUTD)/wrfreedi.obj     $(OUTD)/wrgetli.obj     $(OUTD)/wridcmp.obj  &
    $(OUTD)/wridexby.obj     $(OUTD)/wridfnoo.obj    $(OUTD)/wridfrnu.obj &
    $(OUTD)/wridfrst.obj     $(OUTD)/wridnacm.obj    $(OUTD)/wriidfn.obj  &
    $(OUTD)/wrinitdi.obj     $(OUTD)/wriswrf.obj     $(OUTD)/wrreaddi.obj &
    $(OUTD)/wrrewrid.obj     $(OUTD)/wrrfrr.obj      $(OUTD)/wrrftr.obj &
    $(OUTD)/wrrhr.obj 

# Additional objects for the full wres library
wres_more_objs = &
    $(OUTD)/loadres.obj      $(OUTD)/mropnewf.obj    $(OUTD)/rdlli.obj    &
    $(OUTD)/resaccel.obj     $(OUTD)/resbitmp.obj    $(OUTD)/resdiag.obj  &
    $(OUTD)/resfont.obj      $(OUTD)/resiccu.obj     $(OUTD)/resmenu.obj  &
    $(OUTD)/resnamor.obj     $(OUTD)/resraw.obj      $(OUTD)/resstr.obj   &
    $(OUTD)/restbar.obj      $(OUTD)/restell.obj     $(OUTD)/resver.obj   &
    $(OUTD)/rillia.obj       $(OUTD)/rillib.obj      $(OUTD)/ropenfrw.obj &
    $(OUTD)/rrlli.obj        $(OUTD)/wrauto.obj      $(OUTD)/wrchkwrf.obj &
    $(OUTD)/wrdelrs.obj      $(OUTD)/wresrtns.obj    $(OUTD)/wrfilein.obj &
    $(OUTD)/wrgetri.obj      $(OUTD)/wrgetti.obj     $(OUTD)/wrhidcmp.obj &
    $(OUTD)/wrhidfno.obj     $(OUTD)/wrhidfre.obj    $(OUTD)/wrhidfrn.obj &
    $(OUTD)/wrhidfrs.obj     $(OUTD)/wridfree.obj    $(OUTD)/wridnfst.obj &
    $(OUTD)/wridtnoo.obj     $(OUTD)/wridtonu.obj    $(OUTD)/wridtost.obj &
    $(OUTD)/wrihidf.obj      $(OUTD)/wrisempt.obj    $(OUTD)/wrislr.obj   &
    $(OUTD)/write.obj        $(OUTD)/wrmergdi.obj    $(OUTD)/wrnextrs.obj &
    $(OUTD)/wropnewf.obj     $(OUTD)/wrrfwrid.obj    $(OUTD)/wrrwrid.obj  &
    $(OUTD)/wrwritdi.obj
objs = $(wres_core_objs) $(wres_more_objs)

# options to use
#!ifdef IS_GUI
#extra_c_flags = -DMICROSOFT -s -DWIN_GUI
#!else
extra_c_flags = -DMICROSOFT -s
#!endif

#!ifeq convention stack
#extra_c_flags_386 = -3s
#!else
extra_c_flags_386 = -3r
#!endif

#!ifeq host_os os2
#extra_c_flags_i86 = -zdp
#!else ifeq host_os win
#extra_c_flags_i86 = -zw-zu
#!endif

!if $(DEBUG)
cflags = -od -d2 -w3 -D_INT_DEBUG
!else
cflags = -ox -s -DNDEBUG
!endif

# where to look for various files

.c : $(wres_dir)/c

inc_dirs = -I"$(wres_dir)/h" -I$(watcom_dir)\H -I$(WATCOM)\H  -I..\..\..\H

.c{$(OUTD)}.obj: $($(proj_name)_autodepends)
	$(WATCOM)\binnt\wcc386 -q -zc -bc -bt=nt $(cflags) $(extra_c_flags_$[&) $(inc_dirs) -fo$@ $[@

# explicit rules

ALL: $(OUTD) $(OUTD)\wres.lib

$(OUTD):
	@if not exist $(OUTD) mkdir $(OUTD)

$(OUTD)\wres.lib : $(objs) $(__MAKEFILES__) .PRECIOUS
#!ifdef bootstrap
#    ar crs $@ $(objs)
#!else
    %create $^&.lbc
    @for %i in ($(objs)) do @%append $^&.lbc +%i
    wlib -n $(OUTD)\wres.lib @$^&.lbc
#!endif
