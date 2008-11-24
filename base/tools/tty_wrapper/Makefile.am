#
# Makefile.am for tools/ttywrapper
#

INCLUDES = ${AGENT_INCLUDES}

ybin_PROGRAMS = tty_wrapper

tty_wrapper_SOURCES = tty_wrapper.cc

tty_wrapper_CPPFLAGS = $(AM_CPPFLAGS) -I$(top_builddir)/agent-process/src

tty_wrapper_LDADD = $(top_builddir)/agent-process/src/libpy2ag_process.la

