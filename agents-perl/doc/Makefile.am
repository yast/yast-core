#
# Makefile.am for core/agents-perl/doc
#

htmldir = $(docdir)/agents-perl

html_DATA =	ag_yp_makefile.txt \
	  	ycp-pm.html \
	  	ycp-pm-pod.html \
		docu0perl.htm  \
		docu1perl.htm  \
		docu2perl.htm  \
		docu3perl.htm  \
		docu4perl.htm  \
		docu5perl.htm  \
		docu6perl.htm  \
		docu7perl.htm  \
		ag_background.html \
		ag_background_example.ycp

ycp-pm-pod.html: ../lib/ycp.pm
	pod2html < $^ > $@

EXTRA_DIST = $(html_DATA)
