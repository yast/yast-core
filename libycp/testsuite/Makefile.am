#
# Makefile.am for libycp/testsuite
#

SUBDIRS = config lib libycp.test tests

AM_CXXFLAGS = -DY2LOG=\"libycp\"

INCLUDES = -I$(srcdir)/../src/include -I$(top_srcdir)/liby2/src/include ${Y2UTIL_CFLAGS}

bindir = $(prefix)/bin
libdir = ../src/.libs

noinst_PROGRAMS = testSignature runc runycp

runc_SOURCES = runc.cc
runc_LDADD = ../src/libycp.la ../../liby2/src/liby2.la ${Y2UTIL_LIBS}

runycp_SOURCES = runycp.cc
runycp_LDADD = ../src/libycp.la ../../liby2/src/liby2.la ${Y2UTIL_LIBS} 

testSignature_SOURCES = testSignature.cc
testSignature_LDADD = ../src/libycp.la ../../liby2/src/liby2.la ${Y2UTIL_LIBS}

PACKAGE=libycp

AUTOMAKE_OPTIONS = dejagnu

clean-local:
	rm -f tmp.err.* tmp.out.* ycp.log ycp.sum site.exp libycp.log libycp.sum site.bak log.tmp
	rm -f $(bin_PROGRAMS)

EXTRA_DIST = README runtest.sh xfail
