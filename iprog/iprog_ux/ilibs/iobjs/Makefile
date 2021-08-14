# iobjs Makefile

# VPATH=.

INCLUDE=.

CC_ECHO=@
CC=$(CC_ECHO) g++
AR                      = ar
SHLIB_LD		= $(CC) -pipe -shared
SHLIB_CFLAGS		= -fPIC

#CC_DEFS=-DDEBUG_MIN
CC_DEFS=$(CC_BASIC)
CC_DEBUG=-g $(CC_DEFS)

#CFLAGS=-O2 -Wall
#CFLAGS=-O2 -Wcast-qual
#CFLAGS_GCC_3=-Wnormalized=none
#CFLAGS=-O2 -Wno-deprecated -Wno-write-strings $(SHLIB_CFLAGS)
CFLAGS=-O2 -Wall -Werror -Wdeprecated -Wwrite-strings -Wno-register $(SHLIB_CFLAGS)
CC_OPT=$(CC_DEBUG)

LIB_ME=iobjs
LIB_OWN=lib$(LIB_ME).so

LINK_LIB=-l$(LIB_ME) -L. -liobjs -L$(LIB_PATH)
LINK_CC=-pipe -rdynamic
LINK_OPT=$(LINK_CC) -Wl,-rpath,.:$(INCLUDE) $(LINK_LIB)


OBJ_PATH=.

OBJS_ALL=\
	iarg.o         \
	ibase.o        \
	icalendar.o    \
	iconfig.o      \
	icontrol.o     \
	ifile.o        \
	ifilestat.o    \
	ilist.o        \
	istorage.o     \
	istring.o      \
	idir.o         \
	imath.o        \
	inet.o         \
	itime.o        \
	iunicode.o


.PHONY: goal target clean clearall


goal: $(LIB_OWN)

$(LIB_OWN): $(OBJS_ALL)
	$(SHLIB_LD) $(SHLIB_CFLAGS) -o $(LIB_OWN) $(OBJS_ALL)

debug: $(LIB_OWN) debug.o
	$(CC) $(CC_OPT) $(CFLAGS) $(LINK_OPT) $(OBJS_ALL) debug.o -o debug


# Below, the implicit rule for all C++ files!
.cpp.o:
	@echo Compiling $< : obj is $@
	$(CC) $(CC_OPT) $(CFLAGS) -o $(OBJ_PATH)/$@ -c $<

target: goal
	@echo "g++ -pipe -shared $(SHLIB_CFLAGS) -o $(LIB_OWN) $(OBJS_ALL)"
	strip $(LIB_OWN)

clean:
	rm -f core *.o *~ *%

clearall: clean
	rm -f *.a *.so

