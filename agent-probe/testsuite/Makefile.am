#
# Makefile.am for core/agent-probe/testsuite
#

SUBDIRS = config lib ag_hwprobe.test tests

AM_CXXFLAGS = -DY2LOG=\"agent-probe\"

PACKAGE = ag_hwprobe
AUTOMAKE_OPTIONS = dejagnu

INCLUDES = ${AGENT_INCLUDES}

noinst_PROGRAMS = runag_hwprobe

runag_hwprobe_SOURCES = runag_hwprobe.cc
runag_hwprobe_LDADD = ${AGENT_LIBADD}
runag_hwprobe_LDFLAGS = 			\
	-Xlinker --whole-archive		\
	$(top_builddir)/liby2/src/libpy2stdio.la	\
	../src/libpy2ag_hwprobe.la		\
	-Xlinker --no-whole-archive
