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

  File:	      YShortcutManager.cc

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/


#define y2log_component "ui-shortcuts"
#include <ycp/y2log.h>
#include <ycp/YCPString.h>
#include "YShortcutManager.h"

// Return the number of elements of an array of any type
#define DIM( ARRAY )	( ( int ) ( sizeof( ARRAY)/( sizeof( ARRAY[0] ) ) ) )


YShortcutManager::YShortcutManager( YDialog *dialog )
    : _dialog( dialog )
{
    _conflictCount	= 0;
    _did_check		= false;
}


YShortcutManager::~YShortcutManager()
{
    clearShortcutList();
}


void
YShortcutManager::checkShortcuts( bool autoResolve )
{
    y2debug( "Checking keyboard shortcuts" );

    clearShortcutList();
    findShortcutWidgets();


    // Initialize wanted character counters
    for ( int i=0; i < DIM( _wanted ); i++ )
	_wanted[i] = 0;

    // Initialize used character flags
    for ( int i=0; i < DIM( _wanted ); i++ )
	_used[i] = false;

    // Count wanted shortcuts
    for ( unsigned i=0; i < _shortcutList.size(); i++ )
	_wanted[ ( int ) _shortcutList[i]->preferred() ]++;


    // Report errors

    _conflictCount = 0;

    for ( unsigned i=0; i < _shortcutList.size(); i++ )
    {
	YShortcut *shortcut = _shortcutList[i];

	if ( YShortcut::isValid( shortcut->preferred() ) )
	{
	    if ( _wanted[ ( int ) shortcut->preferred() ] > 1 )	// shortcut char used more than once
	    {
		shortcut->setConflict();
		_conflictCount++;
		y2milestone( "Shortcut conflict: '%c' used for %s \"%s\"",
			     shortcut->preferred(),
			     shortcut->widgetClass(),
			     shortcut->shortcutString().c_str() );
	    }
	}
	else	// No or invalid shortcut
	{
	    if ( shortcut->cleanShortcutString().length() > 0 )
	    {
		shortcut->setConflict();
		_conflictCount++;

		if ( ! shortcut->widget()->autoShortcut() )
		{
		    y2milestone( "No valid shortcut for %s \"%s\"",
				 shortcut->widgetClass(),
				 shortcut->cleanShortcutString().c_str() );
		}
	    }
	}

	if ( ! shortcut->conflict() )
	{
	    _used[ ( int ) shortcut->preferred() ] = true;
	}
    }

    _did_check = true;

    if ( _conflictCount > 0 )
    {
	if ( autoResolve )
	{
	    resolveAllConflicts();
	}
    }
    else
    {
	y2debug( "No shortcut conflicts" );
    }

#if 0
    for ( unsigned i=0; i < _shortcutList.size(); i++ )
    {
	YShortcut *shortcut = _shortcutList[i];

	y2debug( "Shortcut '%c' for %s \"%s\" ( original: \"%s\" )",
		 shortcut->shortcut() ? shortcut->shortcut() : ' ',
		 shortcut->widgetClass(),
		 shortcut->cleanShortcutString().c_str(),
		 shortcut->shortcutString().c_str() );
    }
#endif
}


void
YShortcutManager::resolveAllConflicts()
{
    y2debug( "Resolving shortcut conflicts" );

    if ( ! _did_check )
    {
	y2milestone( "Call checkShortcuts() first!" );
	return;
    }


    // Make a list of all shortcuts with conflicts

    YShortcutList 		conflictList;
    YShortcutListIterator	it( _shortcutList.begin() );
    _conflictCount = 0;

    while ( it != _shortcutList.end() )
    {
	if ( ( *it )->conflict() )
	{
	    conflictList.push_back( *it );
	    _conflictCount++;
	}
	++it;
    }


    // Resolve each conflict

    while ( conflictList.size() > 0 )
    {
	/**
	 * Pick a conflict widget to resolve. Use the widget with the least
	 * number of eligible shortcut characters first to maximize chances for
	 * success.
	 **/

	int shortest_index = 0;
	int shortest_len = conflictList[0]->distinctShortcutChars();

	for ( unsigned i=1; i < conflictList.size(); i++ )
	{
	    if ( conflictList[i]->distinctShortcutChars() < shortest_len )
	    {
		// Found an even shorter one

		shortest_index	= i;
		shortest_len	= conflictList[i]->distinctShortcutChars();
	    }
	}


	// Pick a new shortcut for this widget.

	YShortcut * shortcut = conflictList[ shortest_index ];
	resolveConflict( shortcut );

	if ( shortcut->conflict() )
	{
	    y2milestone( "Couldn't resolve shortcut conflict for %s \"%s\"",
			 shortcut->widgetClass(), shortcut->cleanShortcutString().c_str() );

	}


	// Mark this particular conflict as resolved.

	conflictList.erase( conflictList.begin() + shortest_index );
    }

    if ( _conflictCount > 0 )
    {
	y2debug( "%d shortcut conflict(s) left", _conflictCount );
    }
}


void
YShortcutManager::resolveConflict( YShortcut * shortcut )
{
    // y2debug( "Picking shortcut for %s \"%s\"", shortcut->widgetClass(), shortcut->cleanShortcutString().c_str() );

    char candidate = shortcut->preferred();			// This is always normalized, no need to normalize again.

    if ( ! YShortcut::isValid( candidate )			// Can't use this character - pick another one.
	 || _used[ (int) candidate ] )				
    {
	candidate = 0;						// Restart from scratch - forget the preferred character.
	string str = shortcut->cleanShortcutString();

	for ( string::size_type pos = 0; pos < str.length(); pos++ )	// Search all the shortcut string.
	{
	    char c = YShortcut::normalized( str[ pos ] );
	    // y2debug( "Checking #%d '%c'", ( int ) c, c );

	    if ( YShortcut::isValid(c) && ! _used[ ( int ) c ] ) 	// Could we use this character?
	    {
		if ( _wanted[ ( int ) c ] < _wanted[ ( int ) candidate ]	// Is this a better choice than what we already have -
		     || ! YShortcut::isValid( candidate ) )		// or don't we have anything yet?
		{
		    candidate = c;			// Use this one.
		    // y2debug( "Picking %c", c );

		    if ( _wanted[ ( int ) c ] == 0 )	// It doesn't get any better than this:
			break;				// Nobody wants this shortcut anyway.
		}
	    }
	}
    }

    if ( YShortcut::isValid( candidate ) )
    {
	if ( candidate != shortcut->preferred() )
	{
	    if ( shortcut->widget()->autoShortcut() )
	    {
		y2debug( "Automatically assigning shortcut '%c' to %s( `opt( `autoShortcut ), \"%s\" )",
			 candidate, shortcut->widgetClass(), shortcut->cleanShortcutString().c_str() );
	    }
	    else
	    {
		y2debug( "Reassigning shortcut '%c' for %s \"%s\"",
			 candidate, shortcut->widgetClass(), shortcut->cleanShortcutString().c_str() );
	    }
	    shortcut->setShortcut( candidate );
	}
	else
	{
	    y2debug( "Keeping preferred shortcut '%c' for %s \"%s\"",
		     candidate, shortcut->widgetClass(), shortcut->cleanShortcutString().c_str() );
	}

	_used[ ( int ) candidate ] = true;
	shortcut->setConflict( false );
	_conflictCount--;
    }
}



void
YShortcutManager::clearShortcutList()
{
    for ( unsigned i=0; i < _shortcutList.size(); i++ )
    {
	delete _shortcutList[i];
    }

    _shortcutList.clear();
}


void
YShortcutManager::findShortcutWidgets()
{
    if ( ! _dialog )
    {
	y2error( "NULL YDialog" );
	return;
    }

    YWidgetList widgetList = _dialog->widgets();

    for ( YWidgetListIterator it = widgetList.begin(); it != widgetList.end(); ++it )
    {
	if ( ( *it )->shortcutProperty() )
	{
	    YShortcut * shortcut = new YShortcut( *it );
	    _shortcutList.push_back( shortcut );
	}
    }
}


