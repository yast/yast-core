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

  File:		YDialog.h

  Author:	Stefan Hundhammer <sh@suse.de>

/-*/


#ifndef YDialog_h
#define YDialog_h

#include "YSingleChildContainerWidget.h"
#include <stack>

class YMacroRecorder;
class YShortcutManager;


class YDialog : public YSingleChildContainerWidget
{
protected:
    /**
     * Constructor.
     **/
    YDialog( const YWidgetOpt & opt );

    /**
     * Destructor.
     * Don't delete a dialog directly, use YDialog::deleteTopmostDialog().
     **/
    virtual ~YDialog();

public:
    /**
     * Delete the topmost dialog.
     *
     * Will throw a YUINoDialogException if there is no dialog and 'doThrow' is
     * 'true'. 
     *
     * Returns 'true' if there is another open dialog after deleting,
     * 'false' if there is none.
     **/
    static bool deleteTopmostDialog( bool doThrow = true );

    /**
     * Delete all open dialogs.
     **/
    static void deleteAllDialogs();

    /**
     * Returns the number of currently open dialogs (from 1 on), i.e., the
     * depth of the dialog stack.
     **/
    static int openDialogsCount();

    /**
     * Return the current (topmost) dialog.
     *
     * If there is none, throw a YUINoDialogException if 'doThrow' is 'true'
     * and return 0 if 'doThrow' is false. 
     **/
    static YDialog * currentDialog( bool doThrow = true );

    /**
     * Alias for currentDialog().
     **/
    static YDialog * topmostDialog( bool doThrow = true )
	{ return currentDialog( doThrow ); }

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     **/
    virtual const char * widgetClass() const { return "YDialog"; }

    /**
     * Sets the initial dialog size. Honors the `defaultsize option
     * and -geometry
     **/
    void setInitialSize();

    /**
     * Returns true if the dialog has the `defaultsize option set.
     **/
    bool hasDefaultSize() { return _hasDefaultSize.value(); }

    /**
     * Returns true if the dialog has the `warncolor option set.
     **/
    bool hasWarnColor() { return _hasWarnColor.value(); }

    /**
     * Returns true if the dialog has the `infocolor option set.
     **/
    bool hasInfoColor() { return _hasInfoColor.value(); }

    /**
     * Returns true if the dialog has the `decorated option set.
     **/
    bool isDecorated() { return _isDecorated.value(); }

    /**
     * Returns true if the dialog has the `decorated option set.
     **/
    bool isCentered() { return _isCentered.value(); }

    /**
     * Returns true if the dialog has the `smallDecorations option set.
     **/
    bool hasSmallDecorations() { return _hasSmallDecorations.value(); }

    /**
     * Checks the keyboard shortcuts of all children of this dialog
     * (not for sub-dialogs!) unless shortcut checks are postponed or 'force'
     * is 'true'.
     *
     * A forced shortcut check resets postponed checking.
     **/
    void checkShortcuts( bool force = false );

    /**
     * From now on, postpone keyboard shortcut checks -
     * i.e. normal ( not forced ) checkKeyboardShortcuts() will do nothing.
     * Reset this mode by forcing a shortcut check with
     * checkKeyboardShortcuts( true ).
     **/
    void postponeShortcutCheck() { _shortcutCheckPostponed = true; }

    /**
     * Return whether or not shortcut checking is currently postponed.
     **/
    bool shortcutCheckPostponed() const { return _shortcutCheckPostponed; }

    /**
     * Implements the ui command queryWidget
     **/
    YCPValue queryWidget( const YCPSymbol & property );

    /**
     * Returns a (possibly translated) text describing this dialog for
     * debugging.
     **/
    virtual std::string dialogDebugLabel();

    /**
     * Alias for dialogDebugLabel();
     **/
    virtual std::string debugLabel()		{ return dialogDebugLabel(); }


protected:

    /**
     * Format a debug label.
     **/
    string formatDebugLabel( YWidget * widget, const string & debLabel );


    //
    // Data members
    //
    
    YBoolOpt	_hasDefaultSize;
    YBoolOpt	_hasWarnColor;
    YBoolOpt	_hasInfoColor;
    YBoolOpt	_isDecorated;
    YBoolOpt	_isCentered;
    YBoolOpt	_hasSmallDecorations;

    bool	_shortcutCheckPostponed;

    static std::stack<YDialog *> _dialogStack;
};


#endif // YDialog_h
