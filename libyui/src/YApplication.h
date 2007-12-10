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

    /**
     * Set language and encoding for the locale environment ($LANG).
     *
     * This affects UI-internal translations (e.g. for predefined dialogs like
     * file selection), encoding and fonts.
     *
     * 'language' is the ISO short code ("de_DE", "en_US", ...).
     *
     * 'encoding' an (optional) encoding ("utf8", ...) that will be appended if
     * present.
     *
     * Derived classes can overwrite this method, but they should call this
     * base class method at the beginning of the new implementation.
     **/
    virtual void setLanguage( const string & language,
			      const string & encoding = string() );

    /**
     * Return the current language from the locale environment ($LANG).
     * If 'stripEncoding' is true, any encoding (".utf8" etc.) is removed.
     **/
    string language( bool stripEncoding = false ) const;

    /**
     * Open a directory selection box and prompt the user for an existing
     * directory.
     *
     * 'startDir' is the initial directory that is displayed.
     *
     * 'headline' is an explanatory text for the directory selection box.
     * Graphical UIs may omit that if no window manager is running.
     *
     * Returns the selected directory name
     * or an empty string if the user canceled the operation.
     *
     * Derived classes are required to implement this.
     **/
    virtual string askForExistingDirectory( const string & startDir,
					    const string & headline ) = 0;

    /**
     * Open a file selection box and prompt the user for an existing file.
     *
     * 'startWith' is the initial directory or file.
     *
     * 'filter' is one or more blank-separated file patterns, e.g.
     * "*.png *.jpg"
     *
     * 'headline' is an explanatory text for the file selection box.
     * Graphical UIs may omit that if no window manager is running.
     *
     * Returns the selected file name
     * or an empty string if the user canceled the operation.
     *
     * Derived classes are required to implement this.
     **/
    virtual string askForExistingFile( const string & startWith,
				       const string & filter,
				       const string & headline ) = 0;

    /**
     * Open a file selection box and prompt the user for a file to save data
     * to.  Automatically asks for confirmation if the user selects an existing
     * file.
     *
     * 'startWith' is the initial directory or file.
     *
     * 'filter' is one or more blank-separated file patterns, e.g.
     * "*.png *.jpg"
     *
     * 'headline' is an explanatory text for the file selection box.
     * Graphical UIs may omit that if no window manager is running.
     *
     * Returns the selected file name
     * or an empty string if the user canceled the operation.
     *
     * Derived classes are required to implement this.
     **/
    virtual string askForSaveFileName( const string & startWith,
				       const string & filter,
				       const string & headline ) = 0;

private:

    ImplPtr<YApplicationPrivate> priv;
};

#define YApplication_h

#endif // YApplication_h
