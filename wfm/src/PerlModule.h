/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|							 (C) SuSE GmbH |
\----------------------------------------------------------------------/

   File:	PerlModule.h

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:  Klaus Kaempf <kkaempf@suse.de>

   Purpose:	Access to packagemanager
		Handles Perl::function (list_of_arguments) calls
		from WFMInterpreter.
/-*/

#ifndef PerlModule_h
#define PerlModule_h

#include <string>
using std::string;

#include <ycp/YCPValue.h>
#include <ycp/YCPList.h>
#include <ycp/YCPInterpreter.h>

class Y2Component;


/**
 * A simple class for perl access
 */
class PerlModule
{
  public:

    /**
     * Constructor.
     */
    PerlModule( YCPInterpreter *wfmInterpreter );

    /**
     * Destructor.
     */
    ~PerlModule ();

    /**
     * evaluate 'val' and return YCPValue
     */
    YCPValue evaluate( YCPValue val );

protected:

    Y2Component * _perlComponent;
};

#endif // PerlModule_h
