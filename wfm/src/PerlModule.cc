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

   File:	PerlModule.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:  Klaus Kaempf <kkaempf@suse.de>

   Purpose:	Access to packagemanager
		Handles Perl::function (list_of_arguments) calls
		from WFMInterpreter.
/-*/


#include <ycp/y2log.h>
#include <y2/Y2Component.h>
#include <y2/Y2ComponentBroker.h>
#include <ycp/YCPList.h>

#include <PerlModule.h>

using std::string;

//-------------------------------------------------------------------
// PerlModule

PerlModule::PerlModule (YCPInterpreter *wfmInterpreter)
{
    _perlComponent = Y2ComponentBroker::createServer( "perl" );
    
    if ( ! _perlComponent )
	YCPError( "Couldn't create Perl component" );
}

/**
 * Destructor.
 */
PerlModule::~PerlModule ()
{
    if ( _perlComponent )
    {
	_perlComponent->result( YCPVoid() );
	delete _perlComponent;
    }
}

/**
 * evaluate 'function (list-of-arguments)'
 * and return YCPValue
 */
YCPValue
PerlModule::evaluate( YCPValue val )
{
    if ( ! _perlComponent )
	return YCPError( "Couldn't create Perl component", YCPVoid() );
    
    return _perlComponent->evaluate( val );
}
