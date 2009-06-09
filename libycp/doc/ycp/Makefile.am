#
# Makefile.am for core/libycp/doc/ycp
#
htmldir = $(docdir)/libycp

validate:
	xmllint --valid --noout ycp.xml

ycp.fo: ycp.xml customize-fo.xsl
	$(XSLTPROC) $(XSLTPROC_FLAGS) -o ycp.fo customize-fo.xsl ycp.xml

ycp.pdf: ycp.fo
	fop -q ycp.fo ycp.pdf

html/index.html: ycp.xml customize-html.xsl
	$(XSLTPROC) $(XSLTPROC_FLAGS) customize-html.xsl ycp.xml

html_DATA = $(wildcard html/*.*)

EXTRA_DIST = *.xml $(html_DATA)

CLEANFILES = CATALOG.local *.out *.aux *.dvi *.tex *.ps *.pdf *.log *.dsl *.fo

clean-local:
	rm -rf html/*.html
