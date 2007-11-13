#
# Makefile.am for liby2util/src
#

SUBDIRS = include

INCLUDES = -I$(srcdir)/include #-I$(oldincludedir)/libxml2 

AM_LDFLAGS = -L$(libdir)

AM_CPPFLAGS = -DY2LOG=\"liby2util\" $(DEBUG_FLAGS) -D_GNU_SOURCE -DLOCALEDIR=\"${localedir}\"

lib_LTLIBRARIES = liby2util.la

liby2util_la_SOURCES =		\
	ExternalDataSource.cc \
	ExternalProgram.cc \
	MemUsage.cc \
	PathInfo.cc \
	Pathname.cc \
	Rep.cc \
	Y2SLog.cc \
	miniini.cc \
	stringutil.cc \
	y2log.cc


liby2util_la_LDFLAGS = -version-info 4:0:0

liby2util_la_LIBADD = #-lutil -lz $(OPENSSL_LIBS) -lxml2
