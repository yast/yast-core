#
# Makefile.am for y2scr/ag_dummy/testsuite
#

AUTOMAKE_OPTIONS = dejagnu

INCLUDES = -I$(includedir)

noinst_PROGRAMS = y2ag_dummy
y2ag_dummy_SOURCES = y2ag_dummy.cc

y2ag_dummy_LDADD = -lscr -ly2 -lycp
y2ag_dummy_LDFLAGS = -L$(libdir)	\
	-Xlinker --whole-archive	\
	$(plugindir)/libpy2stdio.la	\
	$(plugindir)/libpy2script.la	\
	../src/libpy2ag_dummy.la	\
	-Xlinker --no-whole-archive
