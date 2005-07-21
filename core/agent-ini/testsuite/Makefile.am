#
# Makefile.am for core/agent-ini/testsuite
#

SUBDIRS = config lib ag_ini.test tests ag_ini.debug debug ag_ini.multi multi

AM_CXXFLAGS = -DY2LOG=\"agent-ini\"

PACKAGE = ag_ini
AUTOMAKE_OPTIONS = dejagnu

INCLUDES = ${AGENT_INCLUDES}

noinst_PROGRAMS = runag_ini

runag_ini_SOURCES = runag_ini.cc
runag_ini_LDADD = ${AGENT_LIBADD}
runag_ini_LDFLAGS = 					\
	-Xlinker --whole-archive			\
	$(top_builddir)/liby2/src/libpy2stdio.la	\
	$(top_builddir)/scr/src/libpy2scr.la		\
	$(top_builddir)/agent-system/src/libpy2ag_system.la	\
	../src/libpy2ag_ini.la		\
	-Xlinker --no-whole-archive

