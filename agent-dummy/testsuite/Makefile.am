#
# Makefile.am for core/agent-dummy/testsuite
#

AM_CXXFLAGS = -DY2LOG=\"agent-dummy\"

AUTOMAKE_OPTIONS = dejagnu

INCLUDES =		\
	-I$(includedir)

noinst_PROGRAMS = y2ag_dummy

y2ag_dummy_SOURCES = y2ag_dummy.cc
y2ag_dummy_LDADD = -lscr -ly2 -lycp -ly2util
y2ag_dummy_LDFLAGS =					\
	-L$(srcdir)/../../libycp/src			\
	-L$(srcdir)/../../liby2/src			\
	-L$(srcdir)/../../libscr/src			\
	-L$(libdir)					\
	-Xlinker --whole-archive			\
	$(srcdir)/../../liby2/src/libpy2stdio.la	\
	$(srcdir)/../src/libpy2ag_dummy.la		\
	-Xlinker --no-whole-archive
