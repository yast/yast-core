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

  File:	      YDialog.h

  Author:     Mathias Kettner <kettner@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/


#ifndef YDialog_h
#define YDialog_h

#include "YContainerWidget.h"

class YMacroRecorder;
class YShortcutManager;


/**
 * @short Realizes a dialog A dialog itself is a container widget. It is never
 * contained in another widget.
 */
class YDialog : public YContainerWidget
{
public:
    /**
     * Constructor
     */
    YDialog( YWidgetOpt &opt);

    /**
     * Cleanup
     */
    virtual ~YDialog( );

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass( ) { return "YDialog"; }

    /**
     * Returns true, since this is a dialog widget.
     */
    bool isDialog( ) const;

    /**
     * Sets the initial dialog size. Honors the `defaultsize option
     * and -geometry
     */
    void setInitialSize( );

    /**
     * Returns true if the dialog has the `defaultsize option set.
     */
    bool hasDefaultSize( ) { return _hasDefaultSize.value( ); }

    /**
     * Returns true if the dialog has the `warncolor option set.
     */
    bool hasWarnColor( ) { return _hasWarnColor.value( ); }

    /**
     * Returns true if the dialog has the `infocolor option set.
     */
    bool hasInfoColor( ) { return _hasInfoColor.value( ); }

    /**
     * Returns true if the dialog has the `decorated option set.
     */
    bool isDecorated( ) { return _isDecorated.value( ); }

    /**
     * Checks the keyboard shortcuts of all children of this dialog
     * ( not for sub-dialogs!) unless shortcut checks are postponed or 'force'
     * is 'true'.
     *
     * A forced shortcut check resets postponed checking.
     */
    void checkShortcuts( bool force = false );

    /**
     * From now on, postpone keyboard shortcut checks -
     * i.e. normal ( not forced) checkKeyboardShortcuts( ) will do nothing.
     * Reset this mode by forcing a shortcut check with 
     * checkKeyboardShortcuts( true ).
     **/
    void postponeShortcutCheck( ) { _shortcutCheckPostponed = true; }

    /**
     * Return whether or not shortcut checking is currently postponed.
     **/
    bool shortcutCheckPostponed( ) const { return _shortcutCheckPostponed; }

    /**
     * Return a list of all widgets that belong to this dialog.
     **/
    YWidgetList widgets( ) const;
    
    
protected:

    /**
     * Recursively fill a widgets list with all children and grandchildren of
     * 'parent' that are in the same dialog.   
     **/
    void fillWidgetList( YWidgetList &			widgetList,
			 const YContainerWidget * 	parent )	const;

	
    /*
     * The dialog options
     */
    YBoolOpt	_hasDefaultSize;
    YBoolOpt	_hasWarnColor;
    YBoolOpt	_hasInfoColor;
    YBoolOpt	_isDecorated;

    bool	_shortcutCheckPostponed;
};


#endif // YDialog_h
