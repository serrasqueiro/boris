# ssed Makefile

INCLUDE=../../ilibs/iobjs
INCLUDE2=../../ilibs/imedia
INCLUDE3=../../ilibs/ilog

NORM_DECK=-DHAS_NORM

#CC_DEFS=-DDEBUG_MIN
CC_DEFS=$(CC_BASIC) $(NORM_DECK)
CC_DEBUG=-g $(CC_DEFS)
CCINCLUDE=-I$(INCLUDE) \
		-I$(INCLUDE2) \
		-I$(INCLUDE3)

#CEXTRA=-Wall -Wno-write-strings
CEXTRA=-Wall -Werror -Wno-write-strings -Wno-stringop-truncation
CFLAGS=-O2 $(CEXTRA)
LINK_OPT=-Wl,-rpath,$(INCLUDE):$(INCLUDE2):$(INCLUDE3)
CC=g++
CC_OPT=$(CC_DEBUG) $(CCINCLUDE) -pipe -rdynamic $(LINK_OPT)

LIB=iobjs
LIB_PATH=$(INCLUDE)
LIB_NAME=$(LIB_PATH)/lib$(LIB).so

LIB2=imedia
LIB2_PATH=$(INCLUDE2)
LIB2_NAME=$(LIB2_PATH)/lib$(LIB2).so

LIB3=ilog
LIB3_PATH=$(INCLUDE3)
LIB3_NAME=$(LIB3_PATH)/lib$(LIB3).so

SH_LIBS=-l$(LIB) \
	-l$(LIB2) \
	-l$(LIB3)

LIBS=-L$(LIB_PATH) \
	-L$(LIB2_PATH) \
	-L$(LIB3_PATH) \
	$(SH_LIBS)

OBJ_PATH=.
OBJ_OWN=ssed.o

OBJS_ALL=\
	sBatch.o \
	sDates.o \
	sHistogram.o \
	sIO.o \
	sNorm.o \
	sUrlNames.o \
	$(OBJ_OWN)

EXECNAME=ssed
EXECNAME_static=ssed_static

.PHONY: goal target clean clearall \
	half_static iobjall


goal: $(EXECNAME) $(LIB_NAME) $(LIB2_NAME) $(LIB3_NAME)

$(EXECNAME): $(OBJS_ALL) $(LIB_NAME) $(LIB2_NAME) $(LIB3_NAME)
	$(CC) $(CC_OPT) $(OBJS_ALL) $(CFLAGS) $(LIBS) -o $(EXECNAME)


$(LIB_NAME):
	cd $(LIB_PATH) && $(MAKE)

$(LIB2_NAME):
	cd $(LIB2_PATH) && $(MAKE)

$(LIB3_NAME):
	cd $(LIB3_PATH) && $(MAKE)

target: goal
	strip $(EXECNAME)

half_static: $(LIB_NAME) $(LIB2_NAME) $(LIB3_NAME)
	rm -f ../../ilibs/imedia/debug.o ../../ilibs/ilog/debug.o
	$(CC) -pipe -rdynamic -Wl,-rpath,../../ilibs/iobjs -L../../ilibs/iobjs -liobjs -o $(EXECNAME_static) ../../ilibs/imedia/*.o ../../ilibs/ilog/*.o *.o

iobjall: $(EXECNAME) $(LIB_NAME) $(LIB2_NAME) $(LIB3_NAME)
	g++ -pipe -rdynamic *.o -L../../ilibs -liobjall -s -o ssed

# Below, the implicit rule for all C++ files!
.cpp.o:
	@echo Compiling $< : obj is $@
	$(CC) $(CC_OPT) $(CFLAGS) -o $(OBJ_PATH)/$@ -c $<

clean:
	cd $(LIB_PATH) && $(MAKE) clean
	rm -f core $(OBJS_ALL) *~ *%

clearall: clean
	rm -f $(EXECNAME)

