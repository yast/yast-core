#
# Makefile.am for core/agents-perl/lib
#

# now using vendorlib instead of /usr/lib/perl5/site_perl
install-data-local:
	VL=`perl -V:vendorlib | sed "s!.*='!!;s!'.*!!"`; \
	$(INSTALL) -d $(DESTDIR)/$$VL; \
	$(INSTALL_DATA) ycp.pm $(DESTDIR)/$$VL

TESTS = ycp.test

EXTRA_DIST = ycp.pm $(TESTS)
