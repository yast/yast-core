#
# Makefile.am for core/libyui/testsuite
#

AM_CXXFLAGS = -DY2LOG=\"libyui\"

INCLUDES =						\
	-I$(srcdir)/../../libycp/src/include		\
	-I$(srcdir)/../../liby2/src/include		\
	-I$(srcdir)/../src/include			\
	-I$(includedir)

bin_PROGRAMS = runyui
runyui_SOURCES = runyui.cc
runyui_LDADD = ../src/libyui.la -lycp -lpthread -ly2util
runyui_LDFLAGS = -L$(libdir) -L$(srcdir)/../../libycp/src

AUTOMAKE_OPTIONS = dejagnu

clean-local:
	rm -f tmp.err.* tmp.out.* ycp.log ycp.sum site.exp \
	libycp.log libycp.sum site.bak

EXTRA_DIST = README
