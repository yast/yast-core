#
# Makefile.am for libycp/testsuite
#

SUBDIRS = config lib libycp.test tests

INCLUDES = -I$(srcdir)/../src/include

bindir = /usr/bin
libdir = ../src/.libs

bin_PROGRAMS = ycpc runc runycp
ycpc_SOURCES = ycpc.cc
ycpc_LDADD = ../src/libycp.la
runc_SOURCES = runc.cc
runc_LDADD = ../src/libycp.la
runycp_SOURCES = runycp.cc
runycp_LDADD = ../src/libycp.la

AUTOMAKE_OPTIONS = dejagnu

clean-local:
	rm -f tmp.err.* tmp.out.* ycp.log ycp.sum site.exp libycp.log libycp.sum site.bak log.tmp
	rm -f $(bin_PROGRAMS)

EXTRA_DIST = README
