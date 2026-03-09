
proj_name = dwarfw

ifndef DEBUG
DEBUG=0
endif

ifeq ($(DEBUG),0)
extra_c_flags = -DNDEBUG -O2
outd_suffix=R
else
extra_c_flags = -D_INT_DEBUG -g
outd_suffix=D
endif

OUTD=../build/osi386L$(outd_suffix)

objs = $(OUTD)/dwlngen.o $(OUTD)/dwutils.o

watcom_dir=../watcom

.c : c

inc_dirs = -Ih -I$(watcom_dir)/h

c_flags =-D__UNIX__ $(bitopts) -std=gnu99 $(extra_c_flags)

$(OUTD)/%.o: c/%.c 
	gcc -c $(c_flags) $(inc_dirs) -o $(OUTD)/$*.o $<

ALL: $(OUTD) $(OUTD)/dw.lib

$(OUTD):
	mkdir -p $(OUTD)

$(OUTD)/dw.lib : $(objs)
	@ar -r $(OUTD)/dw.lib $(objs)

clean:
	@rm $(OUTD)/dw.lib
	@rm $(OUTD)/dw*.o

