# imaudio Makefile

INCLUDE=../iobjs
INCLUDE_2=../ilog

SHLIB_LD		= gcc -pipe -shared
SHLIB_CFLAGS		= -fPIC

### usual defs
#CC_DEBUG_DEF=-DDEBUG
#CC_DEBUG_DEF=-DDEBUG_MIN
CC_DEBUG_DEF=
CC_DEFS=$(CC_DEF) $(CC_BASIC)
CC_DEBUG=-g $(CC_DEFS) $(CC_DEBUG_DEF)
CCINCLUDE=-I$(INCLUDE)
CCINCLUDE_2=-I$(INCLUDE_2)
#CFLAGS=-O2 -Wall $(SHLIB_CFLAGS)
CFLAGS=-O2 -Wall -Werror -Wdeprecated -Wwrite-strings $(SHLIB_CFLAGS)
CC=g++
CC_OPT=$(CC_DEBUG) \
	$(CCINCLUDE) \
	$(CCINCLUDE_2)
LIB_PATH=$(INCLUDE)
LIB_PATH_2=$(INCLUDE_2)

LIB_ME=imaudio
LIB_OWN=lib$(LIB_ME).so

LINK_LIB=-l$(LIB_ME) \
		-liobjs \
		-lilog \
		-L$(LIB_PATH) \
		-L$(LIB_PATH_2) \
		-L.
LINK_CC=-pipe -rdynamic
LINK_OPT=$(LINK_CC) \
		-Wl,-rpath,.:$(INCLUDE):$(INCLUDE_2) \
		$(LINK_LIB)

OBJ_PATH=.

OBJS_ALL=\
	audiostored.o \
	id3v1genres.o \
	imaudio.o \
	listed.o \
	mmdb.o


.PHONY: goal target clean clearall


goal: $(LIB_OWN)
	cd $(LIB_PATH) && $(MAKE)
	cd $(LIB_PATH_2) && $(MAKE)

$(LIB_OWN): $(OBJS_ALL)
	$(SHLIB_LD) $(SHLIB_CFLAGS) -o $(LIB_OWN) $(OBJS_ALL)

debug: $(LIB_OWN) debug.o
	cd $(LIB_PATH) && $(MAKE)
	$(CC) $(CC_OPT) $(CFLAGS) $(LINK_OPT) $(OBJS_ALL) debug.o -o debug


# Below, the implicit rule for all C++ files!
.cpp.o:
	@echo Compiling $< : obj is $@
	$(CC) $(CC_OPT) $(CFLAGS) -o $(OBJ_PATH)/$@ -c $<

target: goal
	cd $(INCLUDE) && $(MAKE) $@
	strip $(LIB_OWN)

clean:
	rm -f core *.o *~ *%

clearall: clean
	rm -f *.a *.so

