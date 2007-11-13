#############################################################
#
# Makefile
#
#############################################################

PROGAM = 	tt
LIBS =
OBJ =

INCLUDE =	.
LINCLUDE =
LLOAD =		y2util

DEFINE =        Y2LOG=\"tt\"

#############################################################
#
# Compiler and linker configuration
#
#############################################################

.EXPORT_ALL_VARIABLES:

SHELL 	=	/bin/bash
CMD   	=	$(SHELL) -ec

TOPDIR	=	$(shell pwd)

#############################################################
#
# Compiler and linker options
#
#############################################################

CC  =		g++
CXX =		g++

CPPFLAGS =	$(addprefix -D, $(DEFINE)) $(addprefix -I, $(INCLUDE))

DEPOPTS  =	-MMD
CXXOPTS  =	-Wall -g -Wformat -Woverloaded-virtual
CXXFLAGS =      $(DEPOPTS) $(CXXOPTS)

LDOPTS  =
LDFLAGS =	$(LDOPTS) $(addprefix -L, $(LINCLUDE))

LOADLIBES =	$(addprefix -l, $(LLOAD))

#############################################################

.SUFFIXES:
.SUFFIXES: .o .cc
.PHONY: all clean run testenv

#############################################################

all:	$(PROGAM)

run:	$(PROGAM)
	./$(PROGAM) 2>/Local/ma/yast2/current/source/packagemanager/testsuite/devel.ma/Y2LOGFILE
gen:	$(PROGAM)
	./$(PROGAM) gen 2>/Local/ma/yast2/current/source/packagemanager/testsuite/devel.ma/Y2LOGFILE
check:	$(PROGAM)
	./$(PROGAM) check 2>/Local/ma/yast2/current/source/packagemanager/testsuite/devel.ma/Y2LOGFILE

#############################################################

testenv: test test/Makefile
	$(MAKE) -C test sub_makefiles
test:
	@mkdir test
test/Makefile:
	@ln Makefile.test ./test/Makefile

#############################################################

con:	testenv
	$(MAKE) -C test/construct
	$(MAKE) check

conass:	testenv
	$(MAKE) -C test/construct_via_assign
	$(MAKE) check

ref:	testenv
	$(MAKE) -C test/construct_reference
	$(MAKE) check

refass:	testenv
	$(MAKE) -C test/construct_reference_via_assign
	$(MAKE) check

ass:	testenv
	$(MAKE) -C test/assign
	$(MAKE) check

testall:	testenv
	$(MAKE) -C test
	$(MAKE) check

#############################################################

$(PROGAM):	$(PROGAM).o $(OBJ) $(LIBS)

clean:
	rm -f `find . -type f -name '*.[oad]'`

#############################################################
#
# Common rules
#
#############################################################
#
# Don't use them in NT-match-anything rules.
# (i.e. don't try to make them from a .c .y .o....)
#

#Make% :

#############################################################
#
# Never try to remake them
#

Make% ::	;
%.d   ::	;

#############################################################
# include dependency files they exist
#

ifneq "$(strip $(wildcard *.d))" ""
  include $(wildcard *.d)
endif

#############################################################
