
# creates the Elf binary for Linux/FreeBSD using GCC.
# Untested!

ifndef DEBUG
DEBUG=0
endif

bitopts = -DLONG_IS_64BITS
#bitopts = -m32

ifeq ($(DEBUG),0)
extra_c_flags = -DNDEBUG -O2
outd_suffix=R
else
extra_c_flags = -D_INT_DEBUG -g
outd_suffix=D
endif

OUTD=../build/cvpackL$(outd_suffix)

watcom_dir=../watcom

inc_dirs  = -Ihpp -I$(watcom_dir)/h

#cflags stuff

c_flags =-D__UNIX__ $(bitopts) -DUNALIGNED="" -std=gnu99 $(extra_c_flags)

CC = gcc

.SUFFIXES:
.SUFFIXES: .cpp .o

object_files =  \
	$(OUTD)/cvpack.o   $(OUTD)/retrieve.o  $(OUTD)/cverror.o   $(OUTD)/packtype.o \
	$(OUTD)/subsect.o  $(OUTD)/cssymbol.o  $(OUTD)/makeexe.o   $(OUTD)/symdis.o \
	$(OUTD)/typemap.o  $(OUTD)/typearay.o  $(OUTD)/typercrd.o  $(OUTD)/common.o


$(OUTD)/%.o: cpp/%.cpp
	$(CC) -c $(inc_dirs) $(c_flags) -o $(OUTD)/$*.o $<

all:  $(OUTD) $(OUTD)/cvpack

$(OUTD):
	@mkdir -p $(OUTD)

$(OUTD)/cvpack: $(object_files)
	$(CC) $(object_files) $(bitopts) -s -o $@ -Wl,-Map,$(OUTD)/cvpack.map

######

install:
	@install $(OUTD)/cvpack /usr/local/bin

clean:
	@rm -f $(OUTD)/cvpack
	@rm -f $(OUTD)/*.o
	@rm -f $(OUTD)/*.map

