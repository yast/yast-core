#
# Makefile.am for libycp/testsuite
#

SUBDIRS = config lib libycp.test tests

AM_CXXFLAGS = -DY2LOG=\"libycp\"

INCLUDES = -I$(srcdir)/../src/include -I$(srcdir)/../../liby2/src/include -I$(includedir) -I/usr/include/YaST2

bindir = $(prefix)/bin
libdir = ../src/.libs

bin_PROGRAMS = runc runycp
noinst_PROGRAMS = testSignature

runc_SOURCES = runc.cc
runc_LDFLAGS = -L$(libdir)
runc_LDADD = ../src/libycp.la ../../liby2/src/liby2.la -ly2util

runycp_SOURCES = runycp.cc
runycp_LDFLAGS = -L$(libdir)
runycp_LDADD = ../src/libycp.la ../../liby2/src/liby2.la -ly2util

testSignature_SOURCES = testSignature.cc
testSignature_LDFLAGS = -L$(libdir)
testSignature_LDADD = ../src/libycp.la ../../liby2/src/liby2.la -ly2util

PACKAGE=libycp

AUTOMAKE_OPTIONS = dejagnu

clean-local:
	rm -f tmp.err.* tmp.out.* ycp.log ycp.sum site.exp libycp.log libycp.sum site.bak log.tmp
	rm -f $(bin_PROGRAMS)

EXTRA_DIST = README runtest.sh
