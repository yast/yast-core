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

  File:		YApplication.h

  Author:	Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YApplication_h

#include <string>
#include "YUI.h"
#include "ImplPtr.h"

using std::string;

class YWidget;
class YWidgetID;
class YApplicationPrivate;


/**
 * Class for application-wide values and functions.
 * This is a singleton. Access and create it via its static functions.
 **/
class YApplication
{
public:

    /**
     * Find a widget in the topmost dialog by its ID.
     *
     * If there is no widget with that ID (or no dialog at all), this function
     * throws a YUIWidgetNotFoundException if 'doThrow' is 'true'. It returns 0
     * if 'doThrow' is 'false'.
     **/
    YWidget * findWidget( YWidgetID * id, bool doThrow = true ) const;

    /**
     * Get the base path for icons used by the UI. Selection widgets like
     * YSelectionBox, YComboBox, etc. or YWizard prepend this to icon
     * specifications that don't use an absolute path.
     **/
    virtual string iconBasePath() const;

    /**
     * Set the icon base path.
     **/
    virtual void setIconBasePath( const string & newIconBasePath );

    /**
     * Return the default function key number for a widget with the specified
     * label or 0 if there is none. Any keyboard shortcuts that may be
     * contained in 'label' are stripped away before any comparison.
     *
     * The basic idea behind this concept is to have an easy default mapping
     * from buttons etc. with the same semantics to function keys:
     *
     * "OK"	-> F10
     * "Accept"	-> F10
     * "Yes"	-> F10
     * "Next"	-> F10
     *
     * "Cancel"	-> F9
     * "No"	-> F9
     * ...
     *
     * This function returns 10 for F10, F for F9 etc.;
     * 0 means "no function key". 
     **/
    int defaultFunctionKey( const string & label ) const;

    /**
     * Add a mapping from the specified label to the specified F-key number.
     * This is the counterpart to defaultFunctionKey().
     *
     * This only affects widgets that are created after this call.
     **/
    void setDefaultFunctionKey( const string & label, int fkey );

    /**
     * Clear all previous label-to-function-key mappings.
     **/
    void clearDefaultFunctionKeys();

  
protected:
    
    friend class YUI;
    /**
     * Constructor.
     *
     * Use YUI::app() to get the singleton for this class.
     **/
    YApplication();

    /**
     * Destructor.
     **/
    virtual ~YApplication();

private:

    ImplPtr<YApplicationPrivate> priv;
};

#define YApplication_h

#endif // YApplication_h
