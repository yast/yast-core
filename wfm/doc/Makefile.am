# Makefile.am for core/wfm/doc

SUBDIRS =

htmldir = $(docdir)/wfm
htmlfiles = $(wildcard html/*.html)
builtin = $(srcdir)/../src/Y2WFMComponent.cc 
builtinxml := $(builtin:%.cc=%.xml)
xmlfiles := $(foreach xml,$(builtinxml),$(notdir $(xml)))


html_DATA =			\
	html/yast2docs.css	\
	html/index.html		\
	$(htmlfiles)

html/index.html: $(xmlfiles) wfm.xml
	XML_CATALOG_FILES=@XML_CATALOG@\
	@XSLTPROC@ --xinclude \
	@STYLESHEET_HTML@ wfm.xml



$(xmlfiles) : $(builtin)
	@ybindir@/ydoc2 -o $@.tmp ../src/$(subst xml,cc,$@)
	@XSLTPROC@ @STYLESHEET_YDOC@ $@.tmp > $@
	@ybindir@/replace_entities $@
	rm $@.tmp

html/yast2docs.css: html/index.html
	cp @STYLESHEET_CSS@ html
	cp -a $(ydatadir)/docbook/images html

EXTRA_DIST = wfm.xml

CLEANFILES =  $(xmlfiles)

clean-local:
	rm -rf html

install-data-local:
	mkdir -p $(DESTDIR)$(htmldir)
	cp -a $(srcdir)/html/images $(DESTDIR)$(htmldir)
