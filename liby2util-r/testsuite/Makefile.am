#
# Makefile.am for liby2util/testsuite
#
SUBDIRS = lib config liby2util.test tests
PACKAGE = liby2util
AUTOMAKE_OPTIONS = dejagnu

INCLUDES= -I$(srcdir)/../src/include

AM_CPPFLAGS = -DY2LOG=\"liby2util-testsuite\"

AM_LDFLAGS = \
	-Xlinker --whole-archive	\
	$(srcdir)/../src/liby2util.la	\
	-Xlinker --no-whole-archive

noinst_PROGRAMS = \
	test_Y2SLog		\
	test_thread_log.prg	\
	test_strutil		\
	test_mkdir.prg		\
	test_chroot.prg

test_Y2SLog_SOURCES = test_Y2SLog.cc

test_thread_log_prg_SOURCES = test_thread_log.cc
test_thread_log_prg_LDFLAGS = $(AM_LDFLAGS) -pthread

test_strutil_SOURCES = test_strutil.cc

test_mkdir_prg_SOURCES = test_mkdir.cc

test_chroot_prg_SOURCES = test_chroot.cc
test_chroot_prg_LDFLAGS = $(AM_LDFLAGS) -static

clean-local:
	rm -f tmp.err.* tmp.out.* y2util.log y2util.sum site.exp site.bak

EXTRA_DIST =
