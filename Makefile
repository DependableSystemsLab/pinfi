##
## PIN tools
##
PIN_HOME ?= /ubc/ece/home/kp/grads/jwei/pin
OBJDIR = testobj
##############################################################
#
# Here are some things you might want to configure
#
##############################################################
PIN_KIT=$(PIN_HOME)
TARGET_COMPILER?=gnu
ifdef OS
    ifeq (${OS},Windows_NT)
        TARGET_COMPILER=ms
    endif
endif

##############################################################
#
# include *.config files
#
##############################################################

ifeq ($(TARGET_COMPILER),gnu)
    include $(PIN_HOME)/source/tools/makefile.gnu.config
    OPT=-O2
    #CXXFLAGS = -I$(PIN_HOME)/source/tools/InstLib -Wall -Werror -Wno-unknown-pragmas $(DBG) $(OPT) -MMD
    CXXFLAGS ?= -Wall -Wno-unknown-pragmas $(DBG) $(OPT)
    PIN=$(PIN_HOME)/pin
endif

ifeq ($(TARGET_COMPILER),ms)
    include ../makefile.ms.config
#    DBG?=
endif


 
TOOL_ROOTS = instcount faultinjection instcategory


TOOLS = $(TOOL_ROOTS:%=$(OBJDIR)%$(PINTOOL_SUFFIX))

all: tools
tools: $(OBJDIR) $(TOOLS)
test: $(OBJDIR) faultinjection.test instcount.test instcategory.test

UTIL_OBJ =$(OBJDIR)fi_util.o

## build rules

$(OBJDIR):
	mkdir -p $(OBJDIR)



$(OBJDIR)faultinjection.o: faultinjection.cpp utils.cpp instselector.cpp
	$(CXX) -c $(CXXFLAGS) $(PIN_CXXFLAGS) ${OUTOPT} $@ $<

$(OBJDIR)instcount.o: instcount.cpp utils.cpp instselector.cpp
	$(CXX) -c $(CXXFLAGS) $(PIN_CXXFLAGS) $(OUTOPT) $@ $<

$(OBJDIR)instcategory.o: instcategory.cpp utils.cpp
	$(CXX) -c $(CXXFLAGS) $(PIN_CXXFLAGS) $(OUTOPT) $@ $<

# $(UTIL_OBJ) : fi_util.cpp fi_util.h
# 	$(CXX) -c $(CXXFLAGS) $(PIN_CXXFLAGS) ${OUTOPT} $@ $<
	

$(TOOLS): $(PIN_LIBNAMES)
$(TOOLS): %$(PINTOOL_SUFFIX) : %.o 
	$(PIN_LD) $(PIN_LDFLAGS) ${LINK_OUT}$@ $< $(PIN_LIBS) $(DBG)

## cleaning
clean:
	-rm -rf $(OBJDIR) *.out *.log *.tested *.failed

-include *.d
