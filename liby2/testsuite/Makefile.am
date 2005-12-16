#
# Makefile.am for core/liby2/testsuite
#

AUTOMAKE_OPTIONS = dejagnu

INCLUDES =							\
	-I$(srcdir)/../../libycp/src/include			\
	-I$(srcdir)/../src/include				\
	-I$(includedir)

noinst_PROGRAMS = rungeneric runplugin runprogram runremote \
	runserial runstdio

rungeneric_SOURCES = runy2.cc
rungeneric_LDADD = -lycp -ly2util
rungeneric_LDFLAGS = -L$(srcdir)/../../libycp/src -L$(libdir)	\
	-Xlinker --whole-archive	\
	../src/liby2.la	\
	-Xlinker --no-whole-archive

runplugin_SOURCES = $(rungeneric_SOURCES)
runplugin_LDADD = -lycp
runplugin_LDFLAGS = -L$(srcdir)/../../libycp/src -L$(libdir)		\
	-Xlinker --whole-archive	\
	../src/liby2.la	\
	-Xlinker --no-whole-archive

runprogram_SOURCES = $(rungeneric_SOURCES)
runprogram_LDADD = -lycp
runprogram_LDFLAGS = -L$(srcdir)/../../libycp/src -L$(libdir)	\
	-Xlinker --whole-archive	\
	../src/liby2.la	\
	-Xlinker --no-whole-archive

runserial_SOURCES = $(rungeneric_SOURCES)
runserial_LDADD = -lycp
runserial_LDFLAGS = -L$(srcdir)/../../libycp/src -L$(libdir)		\
	-Xlinker --whole-archive	\
	../src/liby2.la	\
	../src/libpy2serial.la	\
	-Xlinker --no-whole-archive

runstdio_SOURCES = $(rungeneric_SOURCES)
runstdio_LDADD = -lycp
runstdio_LDFLAGS = -L$(srcdir)/../../libycp/src -L$(libdir)		\
	-Xlinker --whole-archive	\
	../src/liby2.la	\
	-Xlinker --no-whole-archive

clean-local:
	rm -f tmp.err.* tmp.out.* ycp.log ycp.sum site.exp libycp.log \
	libycp.sum site.bak

EXTRA_DIST = README
