#
# Makefile.am for core/agent-resolver/testsuite
#

AUTOMAKE_OPTIONS = dejagnu

INCLUDES =					\
	-I$(srcdir)/../../libycp/src/include	\
	-I$(srcdir)/../../liby2/src/include	\
	-I$(srcdir)/../../libscr/src/include

noinst_PROGRAMS = y2ag_resolver
y2ag_resolver_SOURCES = y2ag_resolver.cc

y2ag_resolver_LDADD = -lscr -ly2 -lycp
y2ag_resolver_LDFLAGS = -L$(libdir)	\
	-L$(srcdir)/../../libycp/src	\
	-L$(srcdir)/../../liby2/src	\
	-L$(srcdir)/../../libscr/src	\
	-Xlinker --whole-archive	\
	$(plugindir)/libpy2stdio.la	\
	$(plugindir)/libpy2script.la	\
	../src/libpy2ag_resolver.la	\
	-Xlinker --no-whole-archive
