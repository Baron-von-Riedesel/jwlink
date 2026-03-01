host_os = linux

common_objs = \
    $(OUTD)/autogrp.o   $(OUTD)/carve.o     $(OUTD)/wcomdef.o   $(OUTD)/cmd16m.o   \
    $(OUTD)/cmdall.o    $(OUTD)/cmddos.o    $(OUTD)/cmdelf.o    $(OUTD)/cmdline.o  \
    $(OUTD)/cmdnov.o    $(OUTD)/cmdos2.o    $(OUTD)/cmdphar.o   $(OUTD)/cmdtable.o \
    $(OUTD)/cmdutils.o  $(OUTD)/cmdqnx.o    $(OUTD)/dbgall.o    $(OUTD)/dbgcv.o    \
    $(OUTD)/dbgdwarf.o  $(OUTD)/dbginfo.o   $(OUTD)/debug.o     $(OUTD)/distrib.o  \
    $(OUTD)/global.o    $(OUTD)/hash.o      $(OUTD)/impexp.o    $(OUTD)/libr.o     \
    $(OUTD)/libsupp.o   $(OUTD)/linkutil.o  $(OUTD)/load16m.o   $(OUTD)/loaddos.o  \
    $(OUTD)/loadelf.o   $(OUTD)/loadelf2.o  $(OUTD)/loadfile.o  $(OUTD)/loadflat.o \
    $(OUTD)/loadnov.o   $(OUTD)/loados2.o   $(OUTD)/loadpe.o    $(OUTD)/loadphar.o \
    $(OUTD)/loadqnx.o   $(OUTD)/loadraw.o   $(OUTD)/lsymtab.o   $(OUTD)/mapio.o    \
    $(OUTD)/mem.o       $(OUTD)/mixcache.o  $(OUTD)/msg.o       $(OUTD)/objio.o    \
    $(OUTD)/obj2supp.o  $(OUTD)/objcalc.o   $(OUTD)/objfree.o   $(OUTD)/objnode.o  \
    $(OUTD)/objomf.o    $(OUTD)/objorl.o    $(OUTD)/objpass1.o  $(OUTD)/objpass2.o \
    $(OUTD)/objstrip.o  $(OUTD)/omfreloc.o  $(OUTD)/overlays.o  $(OUTD)/ovlsupp.o  \
    $(OUTD)/permdata.o  $(OUTD)/procfile.o  $(OUTD)/reloc.o     $(OUTD)/ring.o     \
    $(OUTD)/ring2.o     $(OUTD)/salloc.o    $(OUTD)/spillio.o   $(OUTD)/strtab.o   \
    $(OUTD)/symmem.o    $(OUTD)/symtrace.o  $(OUTD)/toc.o       $(OUTD)/wlink.o    \
    $(OUTD)/wlnkmsg.o   $(OUTD)/demangle.o  $(OUTD)/exerespe.o  $(OUTD)/sharedio.o \
    $(OUTD)/cmdraw.o    \
    $(OUTD)/rcstr.o

ifeq ($(use_virtmem),1)
common_objs += $(OUTD)/virtmem.o
else
common_objs += $(OUTD)/virtpage.o
endif

common_objs += $(OUTD)/clibext.o

#
# target OS dependent files
#
wlink_objs_dos   = $(OUTD)/linkio.o
wlink_objs_qnx   = $(OUTD)/posixio.o
wlink_objs_linux = $(OUTD)/posixio.o
wlink_objs_osx   = $(OUTD)/posixio.o
wlink_objs_bsd   = $(OUTD)/posixio.o
wlink_objs_nt    = $(OUTD)/ntio.o
wlink_objs_os2   = $(OUTD)/ntio.o

common_objs += $(wlink_objs_$(host_os))

ifdef no_orl
# do not link against ORL
common_objs += $(OUTD)/orlstubs.o
orl_lib =
endif

comp_objs_exe = $(common_objs)

proj_obj = $(comp_objs_exe)
