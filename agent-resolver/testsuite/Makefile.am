#
# Makefile.am for core/agent-resolver/testsuite
#

AUTOMAKE_OPTIONS = dejagnu

INCLUDES =					\
	-I$(srcdir)/../../libycp/src/include	\
	-I$(srcdir)/../../liby2/src/include	\
	-I$(srcdir)/../../libscr/src/include	\
	-I$(includedir)

noinst_PROGRAMS = y2ag_resolver
y2ag_resolver_SOURCES = y2ag_resolver.cc

y2ag_resolver_LDADD = -lscr -ly2 -lycp -ly2util
y2ag_resolver_LDFLAGS =					\
	-L$(srcdir)/../../libycp/src			\
	-L$(srcdir)/../../liby2/src			\
	-L$(srcdir)/../../libscr/src			\
 	-L$(libdir)					\
	-Xlinker --whole-archive			\
	$(srcdir)/../../liby2/src/libpy2stdio.la	\
	$(srcdir)/../../liby2/src/libpy2script.la	\
	$(srcdir)/../src/libpy2ag_resolver.la		\
	-Xlinker --no-whole-archive
