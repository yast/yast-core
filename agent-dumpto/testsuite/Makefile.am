#
# Makefile.am for y2scr/ag_dumpto/testsuite
#

AUTOMAKE_OPTIONS = dejagnu

INCLUDES = -I$(includedir)

noinst_PROGRAMS = y2ag_dumpto
y2ag_dumpto_SOURCES = y2ag_dumpto.cc

y2ag_dumpto_LDADD = -lscr -ly2 -lycp
y2ag_dumpto_LDFLAGS = -L$(libdir)	\
	-Xlinker --whole-archive	\
	$(plugindir)/libpy2stdio.la	\
	$(plugindir)/libpy2script.la	\
	../src/libpy2ag_dumpto.la	\
	-Xlinker --no-whole-archive
