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

  File:	      YShortcut.cc

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/



#define y2log_component "ui-shortcuts"
#include <ycp/y2log.h>
#include <ycp/YCPString.h>
#include <ycp/YCPSymbol.h>
#include "YShortcut.h"
#include <ctype.h>	// toupper(), tolower()


// Return the number of elements of an array of any type
#define DIM( ARRAY )	( (int) ( sizeof( ARRAY)/( sizeof( ARRAY[0] ) ) ) )


YShortcut::YShortcut( YWidget *shortcut_widget )
    : _widget( shortcut_widget )
{
    _preferred				= -1;
    _shortcut				= -1;
    _distinctShortcutChars		= -1;
    _conflict				= false;
    _shortcut_string_chached		= false;
    _clean_shortcut_string_chached	= false;
}


YShortcut::~YShortcut()
{
}


string
YShortcut::shortcutString()
{
    if ( ! _shortcut_string_chached )
    {
	_shortcut_string = getShortcutString();
	_shortcut_string_chached = true;

	// Note: We really need a separate variable here - an empty string
	// might be a valid value!
    }

    return _shortcut_string;
}


string
YShortcut::cleanShortcutString()
{
    if ( ! _clean_shortcut_string_chached )
    {
	_clean_shortcut_string = cleanShortcutString( shortcutString() );
    }

    return _clean_shortcut_string;
}


string
YShortcut::cleanShortcutString( string shortcut_string )
{
    string::size_type pos = 0;

    while ( ( pos = findShortcutPos( shortcut_string, pos ) ) != string::npos )
    {
	shortcut_string.erase( pos, ( string::size_type ) 1 );
    }

    return shortcut_string;
}


char
YShortcut::preferred()
{
    if ( _preferred < 0 )
    {
	_preferred = normalized( findShortcut( shortcutString() ) );
    }

    return ( char ) _preferred;
}


char
YShortcut::shortcut()
{
    if ( _shortcut < 0 )
    {
	_shortcut = preferred();
    }

    return ( char ) _shortcut;
}


void
YShortcut::setShortcut( char new_shortcut )
{
    string str = cleanShortcutString();
    char findme[] = { tolower( new_shortcut ), toupper( new_shortcut ), 0 };
    string::size_type pos = str.find_first_of( findme );

    if ( pos == string::npos )
    {
	y2error( "Can't find '%c' in %s \"%s\"",
		 new_shortcut, widgetClass(), cleanShortcutString().c_str() );

	return;
    }

    str.insert( pos,
		string( 1, shortcutMarker() ) );	// equivalent to 'string( "& " )'

    YCPSymbol propertyName( widget()->shortcutProperty(), true );
    YCPValue propertyValue = YCPString( str );
    widget()->changeWidget( propertyName, propertyValue );

    _shortcut_string_chached		= false;
    _clean_shortcut_string_chached	= false;
    _shortcut = new_shortcut;
}


int
YShortcut::distinctShortcutChars()
{
    if ( _distinctShortcutChars < 0 )	// chache this value - it's expensive!
    {
	// Create and initiazlize "contained" array - what possible shortcut
	// characters are contained in that string?

	bool contained[ sizeof( char ) << 8 ];

	for ( int i=0; i < DIM( contained ); i++ )
	    contained[i] = false;


	// Mark characters as contained

	string clean = cleanShortcutString();

	for ( string::size_type pos=0; pos < clean.length(); pos++ )
	{
	    if ( YShortcut::isValid( clean[ pos ] ) )
		contained[ ( int ) clean[ pos ] ] = true;
	}


	// Count number of contained characters

	_distinctShortcutChars=0;

	for ( int i=0; i < DIM( contained ); i++ )
	{
	    if ( contained[i] )
	    {
		_distinctShortcutChars++;
	    }
	}
    }

    return _distinctShortcutChars;
}


string
YShortcut::getShortcutString()
{
    if ( ! widget()->shortcutProperty() )
    {
	y2error( "ERROR: %s widgets can't handle shortcuts!", widgetClass() );
	return string( "" );
    }

    return getShortcutString( widget() );
}


string
YShortcut::getShortcutString( YWidget * widget )
{
    if ( ! widget || ! widget->shortcutProperty() )
	return string( "" );

    YCPSymbol propertyName( widget->shortcutProperty(), true );
    YCPValue  propertyValue = widget->queryWidget( propertyName );

    return propertyValue.isNull() || ! propertyValue->isString() ?
	string( "" ) : propertyValue->asString()->value();
}


string::size_type
YShortcut::findShortcutPos( const string & str, string::size_type pos )
{
    while ( ( pos = str.find( shortcutMarker(), pos ) ) != string::npos )
    {
	if ( pos+1 < str.length() )
	{
	    if ( str[ pos+1 ] == shortcutMarker() )	// escaped marker? ( "&&" )
	    {
		pos += 2;				// skip this and search for more
	    }
	    else
		return pos;
	}
	else
	{
	    // A pathological case: The string ends with '& '.
	    // This is invalid anyway, but prevent endless loop even in this case.
	    return string::npos;
	}
    }

    return string::npos;	// not found
}


char
YShortcut::findShortcut( const string & str, string::size_type pos )
{
    pos = findShortcutPos( str, pos );

    return pos == string::npos ? ( char ) 0 : str[ pos+1 ];
}


bool
YShortcut::isValid( char c )
{
    if ( c >= 'a' && c <= 'z' )	return true;
    if ( c >= 'A' && c <= 'Z' )	return true;
    if ( c >= '0' && c <= '9' )	return true;
    return false;
}


char
YShortcut::normalized( char c )
{
    if ( c >= 'a' && c <= 'z' )	return c - 'a' + 'A';
    if ( c >= 'A' && c <= 'Z' )	return c;
    if ( c >= '0' && c <= '9' )	return c;
    return ( char ) 0;
}

