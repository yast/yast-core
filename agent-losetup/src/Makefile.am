#
# Makefile.am for y2scr/y2losetup/src
#

plugin_LTLIBRARIES = libpy2ag_losetup.la

libpy2ag_losetup_la_SOURCES =		\
	LosetupAgent.cc	LosetupAgent.h LosetupIO.c LosetupIO.h rmd160.h	\
	rmd160.c y2loop.h loop.h y2lomount.h

libpy2ag_losetup_la_LDFLAGS = -version-info 2:0

INCLUDES = -I$(top_srcdir) -I$(includedir)

##################################
# 
# bin_PROGRAMS = y2losetup
# y2losetup_SOURCES = \
# 	  y2losetup.c	 \
# 	  y2loop.h	\
# 	  loop.h		\
# 	  y2lomount.h	\
# 	  rmd160.h	\
# 	  rmd160.c
# 
# serversdir = $(YAST2DIR)/servers
# 
# servers_PROGRAMS = ag_losetup
# 
# ag_losetup_SOURCES = \
# 	  LosetupAgent.cc	LosetupAgent.h LosetupIO.c LosetupIO.h rmd160.h	\
# 	  rmd160.c y2loop.h loop.h y2lomount.h
# 
# INCLUDES =  -I$(includedir)
# 
# ag_losetup_LDADD = -L$(libdir) -lscr -ly2 -lycp -lcrypt
# 
# noinst_LTLIBRARIES = libag_losetup.la
# libag_losetup_la_SOURCES = $(ag_losetup_SOURCES)
# 

