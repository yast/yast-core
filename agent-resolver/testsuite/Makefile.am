#
# Makefile.am for y2scr/ag_resolver/testsuite
#

AUTOMAKE_OPTIONS = dejagnu

INCLUDES = -I$(includedir)

noinst_PROGRAMS = y2ag_resolver
y2ag_resolver_SOURCES = y2ag_resolver.cc

y2ag_resolver_LDADD = -lscr -ly2 -lycp
y2ag_resolver_LDFLAGS = -L$(libdir)	\
	-Xlinker --whole-archive	\
	$(plugindir)/libpy2stdio.la	\
	$(plugindir)/libpy2script.la	\
	../src/libpy2ag_resolver.la	\
	-Xlinker --no-whole-archive
