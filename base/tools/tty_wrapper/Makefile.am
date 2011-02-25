#
# Makefile.am for tools/ttywrapper
#

INCLUDES = ${AGENT_INCLUDES}

ybin_PROGRAMS = tty_wrapper

tty_wrapper_SOURCES = tty_wrapper.cc

tty_wrapper_CPPFLAGS = $(AM_CPPFLAGS) 

tty_wrapper_LDADD = -ly2util

tty_wrapper_LDFLAGS = -L$(srcdir)/../../../liby2util-r/src -L$(libdir)
