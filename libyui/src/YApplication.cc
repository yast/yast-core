/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|								       |
|					  (c) SuSE Linux Products GmbH |
\----------------------------------------------------------------------/

  File:		YApplication.cc

  Author:	Stefan Hundhammer <sh@suse.de>

/-*/

#include <locale.h> 	// setlocale()
#include <map>

#define YUILogComponent "ui"
#include "YUILog.h"

#include "YApplication.h"
#include "YDialog.h"
#include "YUIException.h"
#include "YShortcut.h"
#include "YUI.h"

using std::map;

typedef map<string, int> YFunctionKeyMap;


struct YApplicationPrivate
{
    string		iconBasePath;
    YFunctionKeyMap	defaultFunctionKey;
};


YApplication::YApplication()
    : priv( new YApplicationPrivate() )
{
    YUI_CHECK_NEW( priv );
}


YApplication::~YApplication()
{
    // NOP
}


YWidget *
YApplication::findWidget( YWidgetID * id, bool doThrow ) const
{
    YDialog * dialog = YDialog::currentDialog( doThrow );

    if ( ! dialog ) // has already thrown if doThrow == true
	return 0;

    return dialog->findWidget( id, doThrow );
}


string
YApplication::iconBasePath() const
{
    return priv->iconBasePath;
}


void
YApplication::setIconBasePath( const string & newIconBasePath )
{
    priv->iconBasePath = newIconBasePath;
}


int
YApplication::defaultFunctionKey( const string & label ) const
{
    YFunctionKeyMap::const_iterator result =
	priv->defaultFunctionKey.find( YShortcut::cleanShortcutString( label  ) );

    if ( result == priv->defaultFunctionKey.end() )
	return 0;
    else
	return result->second;
}


void
YApplication::setDefaultFunctionKey( const string & label, int fkey )
{
    if ( fkey > 0 )
	priv->defaultFunctionKey[ YShortcut::cleanShortcutString( label ) ] = fkey;
    else
	YUI_THROW( YUIException( "Bad function key number" ) );
}


void
YApplication::clearDefaultFunctionKeys()
{
    priv->defaultFunctionKey.clear();
}


void
YApplication::setLanguage( const string & language, const string & encoding )
{
    string lang = language;
    
    if ( ! encoding.empty() )
    {
	lang += ".";
	lang += encoding;
    }

    setenv( "LANG", lang.c_str(), 1 );  // 1 : replace
    setlocale( LC_NUMERIC, "C" );	// but always format numbers with "."
    
    yuiMilestone() <<  "Setting language to " << lang << endl;
}


string
YApplication::language( bool stripEncoding ) const
{
    const char *lang_env = getenv( "LANG" );

    if ( ! lang_env )
	return "";
    
    string lang( lang_env );

    if ( stripEncoding )
    {
	string::size_type pos = lang.find_first_of( ".@" );

	if ( pos != string::npos )		// if encoding etc. specified
	{
	    lang = lang.substr( 0, pos );	// remove it
	}
    }

    return lang;
}
