#
# Makefile.am for core/client-packager/testsuite
#

AUTOMAKE_OPTIONS = dejagnu

INCLUDES =

noinst_PROGRAMS = y2packager
y2packager_SOURCES = y2packager.cc

y2packager_LDADD = -ly2 -lycp
y2packager_LDFLAGS = -L$(libdir)			\
	-L$(srcdir)/../../libycp/src			\
	-Xlinker --whole-archive			\
	$(srcdir)/../../liby2/src/libpy2stdio.la	\
	$(srcdir)/../../liby2/src/libpy2script.la	\
	$(srcdir)/../src/libpy2packager.la		\
	-Xlinker --no-whole-archive

clean-local:
	rm -f tmp.err.* tmp.out.* *.log *.sum site.exp
