#
# Makefile.am for core/agent-ini/testsuite
#

SUBDIRS = config lib ag_ini.test tests ag_ini.debug debug ag_ini.multi multi

AM_CXXFLAGS = -DY2LOG=\"agent-ini\"

PACKAGE = ag_ini
AUTOMAKE_OPTIONS = dejagnu

INCLUDES =					\
	-I$(srcdir)/../../libycp/src/include	\
	-I$(srcdir)/../../liby2/src/include	\
	-I$(srcdir)/../../libscr/src/include	\
	-I$(includedir)

noinst_PROGRAMS = runag_ini

runag_ini_SOURCES = runag_ini.cc
runag_ini_LDADD = -lscr -ly2 -lycp -ly2util
runag_ini_LDFLAGS = -L$(libdir)				\
	-L$(srcdir)/../../libycp/src			\
	-L$(srcdir)/../../liby2/src			\
	-L$(srcdir)/../../libscr/src			\
	-Xlinker --whole-archive			\
	$(srcdir)/../../liby2/src/libpy2stdio.la	\
	$(srcdir)/../../liby2/src/libpy2script.la	\
	$(srcdir)/../src/libpy2ag_ini.la		\
	-Xlinker --no-whole-archive
