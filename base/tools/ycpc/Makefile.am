#
# Makefile.am for ycpc command line compiler
#

AM_CXXFLAGS = -DY2LOG=\"libycp\"
AM_LDFLAGS = -L$(libdir)

INCLUDES = -I$(srcdir)/../../../libycp/src/include \
	-I$(srcdir)/../../../liby2/src/include \
	-I$(srcdir)/../../../libyui/src/include \
	-I$(srcdir)/../../../libscr/src/include \
	-I$(srcdir)/../../../wfm/src		\
	-I$(includedir)				\
	-I/usr/include/YaST2

bin_PROGRAMS = ycpc
ycpc_SOURCES = ycpc.cc
ycpc_LDADD = ../../../libycp/src/libycp.la \
	../../../libyui/src/libyui.la \
	../../../wfm/src/libpy2wfm.la \
	../../../liby2/src/liby2.la \
	../../../libscr/src/libscr.la \
	../../../scr/src/libpy2scr.la \
	-ly2util
	
ycpc_LDFLAGS = \
       -Xlinker --whole-archive        \
        ../../../liby2/src/libpy2plugin.la         \
       -Xlinker --no-whole-archive 


PACKAGE=ycpc

clean-local:
	rm -f $(bin_PROGRAMS)
