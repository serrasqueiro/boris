# isubs Makefile

INCLUDE=../iobjs
INCLUDE_U=../langed
INCLUDE_M=../imedia

SHLIB_LD		= gcc -pipe -shared
SHLIB_CFLAGS		= -fPIC

### usual defs
#CC_DEBUG_DEF=-DDEBUG
#CC_DEBUG_DEF=-DDEBUG_MIN
CC_DEBUG_DEF=
CC_DEFS=$(CC_DEF) $(CC_BASIC)
CC_DEBUG=-g $(CC_DEFS) $(CC_DEBUG_DEF)
CCINCLUDE=-I$(INCLUDE) \
		-I$(INCLUDE_U) \
		-I$(INCLUDE_M)
#CFLAGS=-O2 -Wall $(SHLIB_CFLAGS)
CFLAGS=-O2 -Wall -Werror -Wdeprecated -Wwrite-strings $(SHLIB_CFLAGS)
CC=g++
CC_OPT=$(CC_DEBUG) $(CCINCLUDE)
LIB_PATH=$(INCLUDE)

LIB_ME=isubs
LIB_OWN=lib$(LIB_ME).so

LIB_U=langed
LIB_M=imedia

LIBS_OTHER=-l$(LIB_U) -l$(LIB_M)

LINK_LIB=-l$(LIB_ME) $(LIBS_OTHER) \
		-L. -L$(LIB_PATH) \
		-L$(INCLUDE_U) \
		-L$(INCLUDE_M) \
		-liobjs \
		-l$(LIB_U)
LINK_CC=-pipe -rdynamic
LINK_OPT=$(LINK_CC) -Wl,-rpath,.:$(INCLUDE) $(LINK_LIB)


OBJ_PATH=.

OBJS_ALL=\
	isubs.o


.PHONY: goal target clean clearall


goal: $(LIB_OWN)
	cd $(LIB_PATH) && $(MAKE)

$(LIB_OWN): $(OBJS_ALL)
	$(SHLIB_LD) $(SHLIB_CFLAGS) -o $(LIB_OWN) $(OBJS_ALL)

debug: $(LIB_OWN) $(OBJS_ALL) debug.o
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

