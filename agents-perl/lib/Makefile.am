#
# Makefile.am for y2scr/agents_non_y2
#

install-data-local:
	$(INSTALL) -d $(DESTDIR)/usr/lib/perl5/site_perl
	$(INSTALL_DATA) ycp.pm $(DESTDIR)/usr/lib/perl5/site_perl

TESTS = ycp.test

EXTRA_DIST = ycp.pm $(TESTS)
