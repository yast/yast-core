#
# Makefile.am for y2a_ini/testsuite
#

SUBDIRS = config lib ag_ini.test tests ag_ini.debug debug ag_ini.multi multi

PACKAGE = ag_ini
AUTOMAKE_OPTIONS = dejagnu

INCLUDES = -I$(includedir)

noinst_PROGRAMS = runag_ini
runag_ini_SOURCES = runag_ini.cc

runag_ini_LDADD = -lscr -ly2 -lycp -lcrypt
runag_ini_LDFLAGS = -L$(libdir)	\
	-Xlinker --whole-archive	\
	$(plugindir)/libpy2stdio.la	\
	$(plugindir)/libpy2script.la	\
	../src/libpy2ag_ini.la	\
	-Xlinker --no-whole-archive

