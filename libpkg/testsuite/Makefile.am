#
# Makefile.am for lib2/testsuite
#

INCLUDES= -I$(srcdir)/../src/include -I$(includedir)

bin_PROGRAMS = runpkg
runpkg_SOURCES = runpkg.cc
runpkg_LDADD = $(srcdir)/../src/libpkg.la -lycp
runpkg_LDFLAGS = -L$(YAST2DIR)/lib

AUTOMAKE_OPTIONS = dejagnu

clean-local:
	rm -f tmp.err.* tmp.out.* ycp.log ycp.sum site.exp libycp.log libycp.sum site.bak

EXTRA_DIST = README
