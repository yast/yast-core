#
# Makefile.am for core/libycp/testsuite
#

SUBDIRS = config lib libycp.test tests

AM_CXXFLAGS = -DY2LOG=\"libycp\"

INCLUDES = -I$(srcdir)/../src/include

bin_PROGRAMS = runycp parseycp testscope
runycp_SOURCES = runycp.cc
runycp_LDADD = ../src/libycp.la
parseycp_SOURCES = parseycp.cc
parseycp_LDADD = ../src/libycp.la
testscope_SOURCES = testscope.cc
testscope_LDADD = ../src/libycp.la

PACKAGE = libycp
AUTOMAKE_OPTIONS = dejagnu

clean-local:
	rm -f tmp.err.* tmp.out.* ycp.log ycp.sum site.exp	\
	libycp.log libycp.sum site.bak
	rm -f runycp parseycp testscope

EXTRA_DIST = README
