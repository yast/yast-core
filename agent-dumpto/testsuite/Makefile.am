#
# Makefile.am for core/agent-dumpto/testsuite
#

AM_CXXFLAGS = -DY2LOG=\"agent-dumpto\"

AUTOMAKE_OPTIONS = dejagnu

INCLUDES =		\
	-I$(includedir)

noinst_PROGRAMS = y2ag_dumpto

y2ag_dumpto_SOURCES = y2ag_dumpto.cc
y2ag_dumpto_LDADD = -lscr -ly2 -lycp -ly2util
y2ag_dumpto_LDFLAGS = -L$(libdir)			\
	-L$(srcdir)/../../libycp/src			\
	-L$(srcdir)/../../liby2/src			\
	-L$(srcdir)/../../libscr/src			\
	-Xlinker --whole-archive			\
	$(srcdir)/../../liby2/src/libpy2stdio.la	\
	$(srcdir)/../../liby2/src/libpy2script.la	\
	$(srcdir)/../src/libpy2ag_dumpto.la		\
	-Xlinker --no-whole-archive
