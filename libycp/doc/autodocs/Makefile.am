#
# Makefile.am for core/libycp/doc/autodocs
#

htmldir = $(docdir)/libycp/autodocs

html_DATA = index.html $(wildcard *.html *.css *.png)

CLEANFILES = $(html_DATA) doxygen.log

header_dir = $(srcdir)/../../src/include
index.html: $(header_dir)/*.h $(header_dir)/ycp/*.h
	@ybindir@/ydoxygen PROJECT_NAME=libycp
