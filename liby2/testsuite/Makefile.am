#
# Makefile.am for liby2/testsuite
#

AUTOMAKE_OPTIONS = dejagnu

INCLUDES= -I../src/include -I$(includedir)

noinst_PROGRAMS = rungeneric runplugin runprogram runremote \
	runscript runserial runstdio

rungeneric_SOURCES = runy2.cc
rungeneric_LDADD = -lycp
rungeneric_LDFLAGS = -L$(libdir)	\
	-Xlinker --whole-archive	\
	../src/liby2.la	\
	-Xlinker --no-whole-archive

runplugin_SOURCES = $(rungeneric_SOURCES)
runplugin_LDADD = -lycp
runplugin_LDFLAGS = -L$(libdir)		\
	-Xlinker --whole-archive	\
	../src/liby2.la	\
	../src/libpy2plugin.la\
	../src/libpy2stdio.la	\
	-Xlinker --no-whole-archive

runprogram_SOURCES = $(rungeneric_SOURCES)
runprogram_LDADD = -lycp
runprogram_LDFLAGS = -L$(libdir)	\
	-Xlinker --whole-archive	\
	../src/liby2.la	\
	../src/libpy2program.la	\
	../src/libpy2stdio.la	\
	-Xlinker --no-whole-archive

runremote_SOURCES = $(rungeneric_SOURCES)
runremote_LDADD = -lycp
runremote_LDFLAGS = -L$(libdir)		\
	-Xlinker --whole-archive	\
	../src/liby2.la	\
	../src/libpy2remote.la	\
	../src/libpy2stdio.la	\
	-Xlinker --no-whole-archive

runscript_SOURCES = $(rungeneric_SOURCES)
runscript_LDADD = -lycp
runscript_LDFLAGS = -L$(libdir)		\
	-Xlinker --whole-archive	\
	../src/liby2.la	\
	../src/libpy2script.la	\
	../src/libpy2stdio.la	\
	-Xlinker --no-whole-archive

runserial_SOURCES = $(rungeneric_SOURCES)
runserial_LDADD = -lycp
runserial_LDFLAGS = -L$(libdir)		\
	-Xlinker --whole-archive	\
	../src/liby2.la	\
	../src/libpy2serial.la	\
	../src/libpy2stdio.la	\
	-Xlinker --no-whole-archive

runstdio_SOURCES = $(rungeneric_SOURCES)
runstdio_LDADD = -lycp
runstdio_LDFLAGS = -L$(libdir)		\
	-Xlinker --whole-archive	\
	../src/liby2.la	\
	../src/libpy2stdio.la	\
	-Xlinker --no-whole-archive

clean-local:
	rm -f tmp.err.* tmp.out.* ycp.log ycp.sum site.exp libycp.log \
	libycp.sum site.bak

EXTRA_DIST = README
