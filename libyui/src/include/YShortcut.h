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

  File:	      YShortcut.h

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/


#ifndef YShortcut_h
#define YShortcut_h

#include "YWidget.h"
#include <string>


/**
 * @short Helper class for shortcut management:
 * This class holds data about the shortcut for one single widget.
 **/
class YShortcut
{
public:
    /**
     * Constructor
     **/
    YShortcut( YWidget *shortcut_widget );

    /**
     * Destructor
     **/
    virtual ~YShortcut();

    /**
     * Returns the YWidget this shortcut data belong to.
     **/
    YWidget * 	widget() { return _widget; }

    
    /**
     * Returns the textual representation of the widget class of the widget
     * this shortcut data belongs to.
     **/
    const char *widgetClass() { return widget()->widgetClass(); }

    /**
     * Returns the complete shortcut string (which may or may not contain "&"),
     * i.e. the value of the widget's shortcut property. For PushButtons, this
     * is the label on the button (e.g., "&Details..."), for other widgets
     * usually the caption above it.
     *
     * This value is chached, i.e. this isn't a too expensive operation.
     **/
    string shortcutString();

    /**
     * Returns the shortcut string (from the widget's shortcut property)
     * without any "&" markers.
     **/
    string cleanShortcutString();

    /**
     * Static version of the above for general use:
     * Returns the specified string without any "&" markers.
     **/
    static string cleanShortcutString( string shortcutString );

    /**
     * The preferred shortcut character, i.e. the character that had been
     * preceded by "&" before checking / resolving conflicts began.
     **/
    char preferred();

    /**
     * The actual shortcut character.
     *
     * This may be different from preferred() if it is overridden.
     **/
    char shortcut();

    /**
     * Set (override) the shortcut character.
     **/
    void setShortcut( char new_shortcut );

    /**
     * Query the internal 'conflict' marker. This class doesn't care about that
     * flag, it just stores it for the convenience of higher-level classes.
     **/
    bool conflict() { return _conflict; }

    /**
     * Set or unset the internal 'conflict' marker.
     **/
    void setConflict( bool newConflictState = true ) { _conflict = newConflictState; }

    /**
     * Obtain the number of distinct valid shortcut characters in the shortcut string,
     * i.e. how many different shortcuts that widget could get.
     **/
    int distinctShortcutChars();
    
    /**
     * Static function: Returns the character used for marking keyboard shortcuts.
     **/
    static char shortcutMarker() { return '&'; }

    /**
     * Static function: Find the next occurrence of the shortcut marker ('&') in a
     * string, beginning at starting position start_pos.
     *
     * Returns string::npos if not found or the position of the shortcut marker (!)
     * if found.
     **/
    static string::size_type findShortcutPos( const string &str, string::size_type start_pos = 0 );

    /**
     * Static function: Find the next shortcut marker in a string, beginning at
     * starting position start_pos.
     *
     * Returns the shortcut character or 0 if none found.
     **/
    static char findShortcut( const string &str, string::size_type start_pos = 0 );

    /**
     * Returns 'true' if 'c' is a valid shortcut character, i.e. [a-zA-Z0-9],
     * 'false' otherwise.
     **/
    static bool isValid( char c );

    /**
     * Return the normalized version of shortcut character 'c', i.e. a
     * lowercase letter or a digit [a-z0-9]. Returns 0 if 'c' is invalid.
     **/
    static char normalized( char c );

    
protected:

    /**
     * Obtain the widget's shortcut property - the string that contains "&" to
     * designate a shortcut.
     **/
    string getShortcutString();


    // Data members
    
    YWidget *	_widget;
    string	_shortcut_string;
    bool	_shortcut_string_chached;
    
    string	_clean_shortcut_string;
    bool	_clean_shortcut_string_chached;
    
    int		_preferred;	// int to enable initializing with invalid char (-1)
    int		_shortcut;	// int to enable initializing with invalid char (-1)
    
    bool	_conflict;
    int		_distinctShortcutChars;
};

typedef vector<YShortcut *>		YShortcutList;
typedef YShortcutList::iterator		YShortcutListIterator;


#endif // YShortcut_h
