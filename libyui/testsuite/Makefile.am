#
# Makefile.am for core/libyui/testsuite
#

AM_CXXFLAGS = -DY2LOG=\"libyui\"

INCLUDES =						\
	-I$(srcdir)/../../libycp/src/include		\
	-I$(srcdir)/../../liby2/src/include		\
	-I$(srcdir)/../src/include			\
	-I$(includedir)

noinst_PROGRAMS = runyui
runyui_SOURCES = runyui.cc
runyui_LDADD = ../src/libyui.la ../../liby2/src/liby2.la ../../libycp/src/libycp.la -lpthread -ly2util
runyui_LDFLAGS = -L$(srcdir)/../../libycp/src -L$(libdir)

AUTOMAKE_OPTIONS = dejagnu

clean-local:
	rm -f tmp.err.* tmp.out.* ycp.log ycp.sum site.exp \
	libycp.log libycp.sum site.bak

EXTRA_DIST = README
