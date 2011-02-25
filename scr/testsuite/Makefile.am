#
# Makefile.am for core/scr/testsuite
#

SUBDIRS = config lib scr.test tests

AM_CXXFLAGS = -DY2LOG=\"scr\"

PACKAGE = scr
AUTOMAKE_OPTIONS = dejagnu

INCLUDES = ${AGENT_INCLUDES}

noinst_PROGRAMS = runscr

runscr_SOURCES = runscr.cc
runscr_LDADD = ${AGENT_LIBADD}
runscr_LDFLAGS = 				\
	-Xlinker --whole-archive		\
	$(top_builddir)/agent-dummy/src/libpy2ag_dummy.la	\
	../src/libpy2scr.la				\
	-Xlinker --no-whole-archive

