#
# Makefile.am for core/agent-modules/testsuite
#

SUBDIRS = config lib ag_modules.test tests

AM_CXXFLAGS = -DY2LOG=\"agent-modules\"

PACKAGE = ag_modules
AUTOMAKE_OPTIONS = dejagnu

INCLUDES = ${AGENT_INCLUDES}

noinst_PROGRAMS = runmodules

runmodules_SOURCES = runmodules.cc
runmodules_LDADD = ${AGENT_LIBADD}
runmodules_LDFLAGS = 				\
	-Xlinker --whole-archive		\
	../src/libpy2ag_modules.la	\
	-Xlinker --no-whole-archive

clean-local:
	rm -f tmp.err.* tmp.out.* site.exp *.log *.sum *.bak

