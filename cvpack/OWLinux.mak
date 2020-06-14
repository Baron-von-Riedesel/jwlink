
OUTD=OWLinuxR
WATCOM=\watcom
watcom_dir=..\watcom

proj_name = cvpack

cvpack_autodepends = .autodepend

inc_dirs = -Ihpp -I$(watcom_dir)\h -I$(WATCOM)\lh -I$(WATCOM)\h

extra_cpp_flags = -xs

exe_file = $(proj_name).exe

h_dir = hpp/
cpp_dir = cpp/
makefile = Makefile

object_files =  &
	$(OUTD)\cvpack.obj   $(OUTD)\retrieve.obj  $(OUTD)\cverror.obj   $(OUTD)\packtype.obj &
	$(OUTD)\subsect.obj  $(OUTD)\cssymbol.obj  $(OUTD)\makeexe.obj   $(OUTD)\symdis.obj  &
	$(OUTD)\typemap.obj  $(OUTD)\typearay.obj  $(OUTD)\typercrd.obj  $(OUTD)\common.obj

.cpp{$(OUTD)}.obj: $($(proj_name)_autodepends)
	$(WATCOM)\binnt\wpp386 -zq -w4 -we -oaxt -DNDEBUG -mf -zc -bc -bt=linux $(extra_cpp_flags) $(inc_dirs) -fo$@ $[@

.cpp : $(cpp_dir)
.hpp : $(h_dir)

ALL: $(OUTD)\cvpack

$(OUTD)\cvpack : $(object_files) $(makefile)
	$(WATCOM)\binnt\wlink format elf ru linux name $*. @<<
FILE { $(object_files) }
libpath $(WATCOM)\lib386 libpath $(WATCOM)\lib386\linux
op map=$*, norelocs, noredefs
<<

