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
runag_ini_LDFLAGS = 					\
	-L../../libycp/src				\
	-L../../liby2/src				\
	-L../../libscr/src				\
	-L$(libdir)					\
	-Xlinker --whole-archive			\
	../../liby2/src/libpy2stdio.la			\
	../../scr/src/libpy2scr.la			\
	../../agent-system/src/libpy2ag_system.la	\
	../src/libpy2ag_ini.la				\
	-Xlinker --no-whole-archive

# was after py2stdio
#	../../liby2/src/libpy2script.la			\
#
