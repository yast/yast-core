#
# Makefile.am for core/libpkg/testsuite
#

AM_CXXFLAGS = -DY2LOG=\"libpkg\"

INCLUDES =						\
	-I$(srcdir)/../../libycp/src/include		\
	-I$(srcdir)/../../liby2/src/include		\
	-I$(srcdir)/../src/include

bin_PROGRAMS = runpkg
runpkg_SOURCES = runpkg.cc
runpkg_LDADD = $(srcdir)/../src/libpkg.la -lycp
runpkg_LDFLAGS = -L$(YAST2DIR)/lib -L$(srcdir)/../../libycp/src

AUTOMAKE_OPTIONS = dejagnu

clean-local:
	rm -f tmp.err.* tmp.out.* ycp.log ycp.sum site.exp	\
	libycp.log libycp.sum site.bak

EXTRA_DIST = README
