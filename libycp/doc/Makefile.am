#
# Makefile.am for core/libycp/doc
#
SUBDIRS = ycp

htmldir = $(docdir)/libycp
builtin = $(wildcard $(srcdir)/../src/YCPBuiltin*.cc)
builtinxml := $(builtin:%.cc=%.xml)
xmlfiles := $(foreach xml,$(builtinxml),$(notdir $(xml)))
htmlfiles = $(wildcard html/.html)


html_DATA =			\
	html/yast2docs.css	\
	html/index.html		\
	$(htmlfiles)

html/index.html: $(xmlfiles) builtins.xml
	XML_CATALOG_FILES=@XML_CATALOG@\
	@XSLTPROC@ --xinclude \
	@STYLESHEET_HTML@ builtins.xml


$(xmlfiles) : $(builtin)
	@ybindir@/ydoc2 -o $@.tmp ../src/$(subst xml,cc,$@)
	@XSLTPROC@ @STYLESHEET_YDOC@ $@.tmp > $@
	@ybindir@/replace_entities $@
	rm $@.tmp

html/yast2docs.css: html/index.html
	cp @STYLESHEET_CSS@ html
	cp -a $(ydatadir)/docbook/images html

EXTRA_DIST = 

CLEANFILES = $(xmlfiles)

clean-local:
	rm -rf html

install-data-local:
	mkdir -p $(DESTDIR)$(htmldir)
	cp -a $(srcdir)/html/images $(DESTDIR)$(htmldir)
