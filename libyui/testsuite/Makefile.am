#
# Makefile.am for libyui/testsuite
#

INCLUDES = -I../src/include -I$(includedir)

bin_PROGRAMS = runyui
runyui_SOURCES = runyui.cc
runyui_LDADD = ../src/libyui.la -lycp -lpthread
runyui_LDFLAGS = -L$(libdir)

AUTOMAKE_OPTIONS = dejagnu

clean-local:
	rm -f tmp.err.* tmp.out.* ycp.log ycp.sum site.exp \
	libycp.log libycp.sum site.bak

EXTRA_DIST = README
