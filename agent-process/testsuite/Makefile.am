#
# Makefile.am for core/agent-process/testsuite
#

SUBDIRS = config lib ag_process.test tests

CLEANFILES = $(wildcard tmp.out.* tmp.err.* tmp.write.* *.sum *.log)

AM_CXXFLAGS = -DY2LOG=\"agent-process\"

PACKAGE = ag_process
AUTOMAKE_OPTIONS = dejagnu

INCLUDES = ${AGENT_INCLUDES}

noinst_PROGRAMS = run_ag_process

run_ag_process_SOURCES = run_ag_process.cc
run_ag_process_LDADD = ${AGENT_LIBADD}
run_ag_process_LDFLAGS = 			\
	-Xlinker --whole-archive		\
	../src/libpy2ag_process.la		\
	-Xlinker --no-whole-archive
