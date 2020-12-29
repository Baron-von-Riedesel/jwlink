
# This makefile creates the jwlink Elf binary for Linux/FreeBSD.
# not finished yet!!!

TARGET1=jwlink

ifndef DEBUG
DEBUG=0
endif

inc_dirs  = -Ih -Iwatcom/h

#cflags stuff

ifeq ($(DEBUG),0)
extra_c_flags = -DNDEBUG -O2
OUTD=build/GccUnixR
else
extra_c_flags = -DDEBUG_OUT -g
OUTD=build/GccUnixD
endif

c_flags =-D__UNIX__ $(extra_c_flags)

CC = gcc

.SUFFIXES:
.SUFFIXES: .c .o

include gccmod.inc

#.c.o:
#	$(CC) -c $(inc_dirs) $(c_flags) -o $(OUTD)/$*.o $<
$(OUTD)/%.o: src/%.c
	$(CC) -c $(inc_dirs) $(c_flags) -o $(OUTD)/$*.o $<

all:  $(OUTD) $(OUTD)/$(TARGET1)

$(OUTD):
	mkdir -p $(OUTD)

$(OUTD)/$(TARGET1) : $(proj_obj)
ifeq ($(DEBUG),0)
	$(CC) $(proj_obj) -s -o $@ -Wl,-Map,$(OUTD)/$(TARGET1).map
else
	$(CC) $(proj_obj) -o $@ -Wl,-Map,$(OUTD)/$(TARGET1).map
endif

######

install:
	@install $(OUTD)/$(TARGET1) /usr/local/bin

clean:
	@rm -f $(OUTD)/$(TARGET1)
	@rm -f $(OUTD)/*.o
	@rm -f $(OUTD)/*.map

