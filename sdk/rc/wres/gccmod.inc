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

proj_obj = $(wres_core_objs) $(wres_more_objs)
