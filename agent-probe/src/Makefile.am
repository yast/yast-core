#
# Makefile.am for y2a_hwpb/src
#

plugin_LTLIBRARIES = libpy2ag_hwprobe.la
noinst_LTLIBRARIES = liby2ag_hwprobe.la

liby2ag_hwprobe_la_SOURCES =	\
	HwProbe.cc HwProbe.h	\
	HwParse.cc

liby2ag_hwprobe_la_LDFLAGS = -version-info 2:0
liby2ag_hwprobe_la_LIBADD = -lhd

libpy2ag_hwprobe_la_SOURCES =				\
	$(liby2ag_hwprobe_la_SOURCES)			\
	Y2CCHwProbe.cc
libpy2ag_hwprobe_la_LDFLAGS = -version-info 2:0
libpy2ag_hwprobe_la_LIBADD = -lhd

INCLUDES = -I$(includedir)
