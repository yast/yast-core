#
# Makefile.am for core/agent-ypserv/src
#

AM_CXXFLAGS = -DY2LOG=\"agent-ypserv\"

plugin_LTLIBRARIES = libpy2ag_ypserv.la

libpy2ag_ypserv_la_SOURCES =			\
	Y2CCYpservAgent.cc			\
	FindYpserv.cc FindYpserv.h		\
	YpservAgent.cc YpservAgent.h

libpy2ag_ypserv_la_LDFLAGS = -version-info 2:0
libpy2ag_ypserv_la_LIBADD = -lnsl

INCLUDES =					\
	-I$(srcdir)/../../libycp/src/include	\
	-I$(srcdir)/../../liby2/src/include	\
	-I$(srcdir)/../../libscr/src/include	\
	-I$(includedir)
