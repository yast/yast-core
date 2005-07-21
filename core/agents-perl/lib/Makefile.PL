#! /usr/bin/perl -w
use ExtUtils::MakeMaker;

WriteMakefile
    (
     # must not be YaST::SCRAgent because that would put everything under YaST
     NAME	=> 'ycp',
     # Normally, MakeMaker takes everything under ./YaST ($(BASEEXT)) and ./lib
     # and that would mean leaving ycp.pm out
     # But because of the CVS file layout, which I don't want to change,
     # let's enumerate the modules:
     PM		=>
     {
      'ycp.pm'			=> '$(INST_LIBDIR)/ycp.pm',
      'YaST/SCRAgent.pm'	=> '$(INST_LIBDIR)/YaST/SCRAgent.pm',
     }
    );
