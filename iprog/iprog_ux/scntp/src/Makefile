# scntp - Makefile

# One backdir more...
INCLUDE=../../ilibs
INCLUDE1=$(INCLUDE)/iobjs
INCLUDE2=$(INCLUDE)/ilog
# CNTP-lib
INCLUDE3=$(INCLUDE)/icntp

#CC_DEFS=-DDEBUG
#CC_DEFS=-DDEBUG -DDEBUG_MIN
CC_DEBUG=-g $(CC_DEFS) $(CC_BASIC)

CCINCLUDE1=-I$(INCLUDE1)
CCINCLUDE2=-I$(INCLUDE2)
CCINCLUDE3=-I$(INCLUDE3)

#CEXTRA=-Wall -Wno-write-strings
CEXTRA=-Wall -Werror -Wno-format-truncation
CFLAGS=-O2 $(CEXTRA)
LINK_OPT=-Wl,-rpath,$(INCLUDE1):$(INCLUDE2):$(INCLUDE3)
CC=g++
CC_OPT=$(CC_DEBUG) $(CCINCLUDE1) $(CCINCLUDE2) $(CCINCLUDE3) -pipe -rdynamic $(LINK_OPT)

LIB=iobjs
LIB_PATH=$(INCLUDE1)
LIB_OBJ=$(LIB_PATH)/lib$(LIB).so

LIB2=ilog
LIB_PATH2=$(INCLUDE2)
LIB_OBJ2=$(LIB_PATH2)/lib$(LIB2).so

LIB3=icntp
LIB_PATH3=$(INCLUDE3)
LIB_OBJ3=$(LIB_PATH3)/lib$(LIB3).so

SH_LIBS=-l$(LIB) -l$(LIB2) -l$(LIB3)
LIBS=-L$(LIB_PATH) -L$(LIB_PATH2) -L$(LIB_PATH3) $(SH_LIBS)

OBJ_PATH=.
OBJ_OWN=iCntpBServer.o

OBJS_BASE=\
	iCIO.o \
	iCntpConfig.o \
	iCntpDates.o \
	iCntpResponse.o \
	iCBindPart.o \
	iCProxy.o \
	iXmeasrv.o

OBJS_ALL=\
	iCntpHelper.o \
	rfc822date.o \
	$(OBJS_BASE) \
	$(OBJ_OWN)

EXECNAME=scntp

.PHONY: goal target clean clearall \
	all \
	soft-link \
	ignore


goal: $(EXECNAME)
	cd $(LIB_PATH3) && $(MAKE)

all: soft-link goal show_peerstats

$(EXECNAME): $(OBJS_ALL) $(LIB_OBJ2) $(LIB_OBJ3)
	@echo "Main prog : $(LIB_OBJ3)"
	$(CC) $(CC_OPT) $(OBJS_ALL) $(CFLAGS) $(LIBS) -o $(EXECNAME)

show_peerstats: $(OBJS_BASE) iNtpStats.o
	$(CC) $(CC_OPT) $(OBJS_BASE) iNtpStats.o $(CFLAGS) $(LIBS) -o show_peerstats

$(LIB_OBJ2):
	@echo "Need to rebuild lib$(LIB2)"
	cd $(LIB_PATH2) && $(MAKE)

$(LIB_OBJ3):
	@echo "Need to rebuild lib$(LIB3)"
	cd $(LIB_PATH3) && $(MAKE)

target: all
	cd $(LIB_PATH3) && $(MAKE) $@
	strip $(EXECNAME)
	@# Best-effort: strip if exists (in principle it exists, as this target depends on 'all')
	-strip show_peerstats


# Below, the implicit rule for all C++ files!
iNtpStats.o: iNtpStats.cpp
	$(CC) $(CC_OPT) $(CFLAGS) -o $(OBJ_PATH)/$@ -c $<

.cpp.o:
	$(CC) $(CC_OPT) $(CFLAGS) -o $(OBJ_PATH)/$@ -c $<

ignore:
	@echo "Compiling $< : obj is $@"

soft-link:
	@-ln -s iCntpBServer.cpp main.cpp

clean:
	rm -f core *.o *~ *%

clearall: clean
	cd $(LIB_PATH) && $(MAKE) clean
	rm -f $(EXECNAME)
	rm -f show_peerstats

