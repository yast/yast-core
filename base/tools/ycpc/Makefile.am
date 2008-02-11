#
# Makefile.am for ycpc command line compiler
#

SUBDIRS = doc

AM_CXXFLAGS = -DY2LOG=\"libycp\"

INCLUDES = -I$(top_srcdir)/libycp/src/include \
	-I$(top_srcdir)/liby2/src/include \
	-I$(top_srcdir)/libscr/src/include \
	-I$(top_srcdir)/wfm/src		\
	${Y2UTIL_CFLAGS}

bin_PROGRAMS = ycpc
ycpc_SOURCES = ycpc.cc
ycpc_LDADD = $(top_builddir)/libycp/src/libycp.la \
	$(top_builddir)/wfm/src/libpy2wfm.la \
	$(top_builddir)/liby2/src/liby2.la \
	$(top_builddir)/libscr/src/libscr.la \
	$(top_builddir)/scr/src/libpy2scr.la \
	${Y2UTIL_LIBS}

ybin_SCRIPTS = ybcdump

EXTRA_DIST = $(ybin_SCRIPTS)
