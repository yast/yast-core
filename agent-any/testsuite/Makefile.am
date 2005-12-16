#
# Makefile.am for core/agent-any/testsuite
#

SUBDIRS = config lib ag_anyagent.test tests

AM_CXXFLAGS = -DY2LOG=\"agent-any\"

PACKAGE = ag_anyagent
AUTOMAKE_OPTIONS = dejagnu

INCLUDES = ${AGENT_INCLUDES}

noinst_PROGRAMS = runag_anyagent

runag_anyagent_SOURCES = runag_anyagent.cc
runag_anyagent_LDADD = ${AGENT_LIBADD}
# why is whole-archive necessary?
runag_anyagent_LDFLAGS = \
	-Xlinker --whole-archive		\
	../src/libpy2ag_anyagent.la		\
	-Xlinker --no-whole-archive

clean-local:
	rm -f tmp.err.* tmp.out.* *.sum *.log site.*
	rm -f runag_anyagent
