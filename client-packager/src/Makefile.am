#
# Makefile.am for packager/src
#

plugin_LTLIBRARIES = libpy2packager.la
libpy2packager_la_SOURCES = 				\
	Y2CCPackager.cc Y2CCPackager.h 			\
	Y2PackagerComponent.cc Y2PackagerComponent.h	\
	Rpm_Installer.cc Rpm_Installer.h
libpy2packager_la_LDFLAGS = -version-info 2:0

INCLUDES = -I$(includedir)
