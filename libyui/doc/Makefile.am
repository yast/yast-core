#
# Makefile.am for core/libyui/doc
#

SUBDIRS = events examples FAQs

htmldir = $(docdir)/libyui
builtin	= $(srcdir)/../src/YUI_builtins.cc $(srcdir)/../src/YUI_widgets.cc \
	  $(srcdir)/../src/YUI_special_widgets.cc
builtinxml := $(builtin:%.cc=%.xml)
xmlfiles := $(foreach xml,$(builtinxml),$(notdir $(xml)))
htmlfiles = $(wildcard html/*.html)

html_DATA =			\
	ui-arch.jpg		\
	README.newapi		\
	html/index.html		\
	$(htmlfiles)		\
	html/yast2docs.css

EXTRA_DIST = $(html_DATA)



$(xmlfiles) : $(builtin)
	@ybindir@/ydoc2 ../src/$(subst xml,cc,$@)
	@XSLTPROC@ @STYLESHEET_YDOC@ output.xml > $@
	@ybindir@/replace_entities $@

html/index.html: $(xmlfiles) ui.xml
	XML_CATALOG_FILES=@XML_CATALOG@\
	@XSLTPROC@ --xinclude \
	@STYLESHEET_HTML@ ui.xml

html/yast2docs.css: html/index.html
	cp @STYLESHEET_CSS@ html
	cp -a $(ydatadir)/docbook/images html
	cp -a $(srcdir)/examples html 

CLEANFILES = $(xmlfiles) output.xml

clean-local:
	rm -rf html

install-data-local:
	mkdir -p $(DESTDIR)$(htmldir)
	cp -a $(srcdir)/html/images $(DESTDIR)$(htmldir)
	cp -a $(srcdir)/examples $(DESTDIR)$(htmldir)
