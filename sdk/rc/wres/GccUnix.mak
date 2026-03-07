#
#  Makefile for   WRes library - GCC used.
#

proj_name = wres

wres_autodepends = .AUTODEPEND
wres_distrib_lib = 1

watcom_dir=../../../watcom
wres_dir = .

ifndef DEBUG
DEBUG=0
endif

ifeq ($(DEBUG),0)
OUTD=../../../build/wresLR
else
OUTD=../../../build/wresLD
endif

#
#  List of WRes library object files
#

# Objects needed for a stripped down wres lib that can only read string resources
wres_core_objs = \
    $(OUTD)/loadfind.o     $(OUTD)/loadstr.o     $(OUTD)/mrrdrh.o   \
    $(OUTD)/ralliae.o      $(OUTD)/rclosef.o     $(OUTD)/rduint16.o \
    $(OUTD)/rduint32.o     $(OUTD)/rduint8.o     $(OUTD)/resseek.o  \
    $(OUTD)/ropenfro.o     $(OUTD)/rrdstr.o      $(OUTD)/rrnamoor.o \
    $(OUTD)/varstr.o       $(OUTD)/wraddres.o    $(OUTD)/wrerror.o  \
    $(OUTD)/wrfindrs.o     $(OUTD)/wrfirres.o    $(OUTD)/wrfree.o   \
    $(OUTD)/wrfreedi.o     $(OUTD)/wrgetli.o     $(OUTD)/wridcmp.o  \
    $(OUTD)/wridexby.o     $(OUTD)/wridfnoo.o    $(OUTD)/wridfrnu.o \
    $(OUTD)/wridfrst.o     $(OUTD)/wridnacm.o    $(OUTD)/wriidfn.o  \
    $(OUTD)/wrinitdi.o     $(OUTD)/wriswrf.o     $(OUTD)/wrreaddi.o \
    $(OUTD)/wrrewrid.o     $(OUTD)/wrrfrr.o      $(OUTD)/wrrftr.o \
    $(OUTD)/wrrhr.o 

# Additional objects for the full wres library
wres_more_objs = \
    $(OUTD)/loadres.o      $(OUTD)/mropnewf.o    $(OUTD)/rdlli.o    \
    $(OUTD)/resaccel.o     $(OUTD)/resbitmp.o    $(OUTD)/resdiag.o  \
    $(OUTD)/resfont.o      $(OUTD)/resiccu.o     $(OUTD)/resmenu.o  \
    $(OUTD)/resnamor.o     $(OUTD)/resraw.o      $(OUTD)/resstr.o   \
    $(OUTD)/restbar.o      $(OUTD)/restell.o     $(OUTD)/resver.o   \
    $(OUTD)/rillia.o       $(OUTD)/rillib.o      $(OUTD)/ropenfrw.o \
    $(OUTD)/rrlli.o        $(OUTD)/wrauto.o      $(OUTD)/wrchkwrf.o \
    $(OUTD)/wrdelrs.o      $(OUTD)/wresrtns.o    $(OUTD)/wrfilein.o \
    $(OUTD)/wrgetri.o      $(OUTD)/wrgetti.o     $(OUTD)/wrhidcmp.o \
    $(OUTD)/wrhidfno.o     $(OUTD)/wrhidfre.o    $(OUTD)/wrhidfrn.o \
    $(OUTD)/wrhidfrs.o     $(OUTD)/wridfree.o    $(OUTD)/wridnfst.o \
    $(OUTD)/wridtnoo.o     $(OUTD)/wridtonu.o    $(OUTD)/wridtost.o \
    $(OUTD)/wrihidf.o      $(OUTD)/wrisempt.o    $(OUTD)/wrislr.o   \
    $(OUTD)/write.o        $(OUTD)/wrmergdi.o    $(OUTD)/wrnextrs.o \
    $(OUTD)/wropnewf.o     $(OUTD)/wrrfwrid.o    $(OUTD)/wrrwrid.o  \
    $(OUTD)/wrwritdi.o
objs = $(wres_core_objs) $(wres_more_objs)

# options to use
#!ifdef IS_GUI
#extra_c_flags = -DMICROSOFT -s -DWIN_GUI
#!else
extra_c_flags = -DMICROSOFT -s
#!endif

ifeq ($(DEBUG),0)
extra_c_flags += -DNDEBUG
else
extra_c_flags += -D_INT_DEBUG
endif

#c_flags = -D__UNIX__ -std=gnu99 -DUNALIGNED="" -DLONG_IS_64BITS $(extra_c_flags)
c_flags = -D__UNIX__ $(bitopts) -std=gnu99 -DUNALIGNED="" $(extra_c_flags)

# where to look for various files

.c : $(wres_dir)/c

inc_dirs = -I$(wres_dir)/h -I$(watcom_dir)/h -I../../../h

$(OUTD)/%.o: c/%.c
	gcc -c $(c_flags) $(extra_c_flags_$[&) $(inc_dirs) -o $(OUTD)/$*.o $<

# explicit rules

ALL: $(OUTD) $(OUTD)/wres.lib

$(OUTD):
	@mkdir -p $(OUTD)

$(OUTD)/wres.lib : $(objs)
	@ar -r $(OUTD)/wres.lib $(objs)

clean:
	@rm $(OUTD)/wres.lib
	@rm $(OUTD)/*.o
