#
# Makefile.am for core/agents-perl/lib
#

# google: automake makemaker
# http://sources.redhat.com/ml/automake/2001-03/msg00395.html
PERL=perl
Makefile.perl: Makefile.PL
	$(PERL) $< MAKEFILE=$@ PREFIX=$(prefix)

noinst_DATA = .made_mod
# a flag file meaning that perl's make was run
.made_mod: Makefile.perl
	$(MAKE) -f Makefile.perl
	touch $@

install-data-local:
	$(MAKE) -f Makefile.perl install_vendor

CLEANFILES = .made_mod
clean-local:
	-$(MAKE) -f Makefile.perl realclean
	rm -f Makefile.perl Makefile.perl.old

TESTS = ycp.test

EXTRA_DIST = Makefile.PL MANIFEST ycp.pm YaST/SCRAgent.pm $(TESTS)
