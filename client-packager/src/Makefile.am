#
# Makefile.am for core/client-packager/src
#

AM_CXXFLAGS = -DY2LOG=\"client-packages\"

plugin_LTLIBRARIES = libpy2packager.la

libpy2packager_la_SOURCES = 				\
	Y2CCPackager.cc Y2CCPackager.h 			\
	Y2PackagerComponent.cc Y2PackagerComponent.h	\
	Rpm_Installer.cc Rpm_Installer.h

libpy2packager_la_LDFLAGS = -version-info 2:0

INCLUDES =						\
	-I$(srcdir)/../../libycp/src/include		\
	-I$(srcdir)/../../liby2/src/include
