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
class YDialogPrivate;

// See YTypes.h for enum YDialogType and enum YDialogColorMode 


class YDialog : public YSingleChildContainerWidget
{
protected:
    /**
     * Constructor.
     *
     * 'dialogType' is one of YMainDialog or YPopupDialog.
     *
     * 'colorMode' can be set to YDialogWarnColor to use very bright "warning"
     * colors or YDialogInfoColor to use more prominent, yet not quite as
     * bright as "warning" colors. Use both only very rarely.
     **/
    YDialog( YDialogType 	dialogType,
	     YDialogColorMode	colorMode = YDialogNormalColor );

    /**
     * Destructor.
     * Don't delete a dialog directly, use YDialog::deleteTopmostDialog().
     **/
    virtual ~YDialog();

public:
    /**
     * Return a descriptive name of this widget class for logging,
     * debugging etc.
     **/
    virtual const char * widgetClass() const { return "YDialog"; }

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
     * Set the initial dialog size, depending on dialogType:
     * YMainDialog dialogs get the UI's "default main window" size,
     * YPopupDialog dialogs use their content's preferred size.
     **/
    void setInitialSize();

    /**
     * Return this dialog's type (YMainDialog / YPopupDialog).
     **/
    YDialogType dialogType() const;

    /**
     * Return this dialog's color mode.
     **/
    YDialogColorMode colorMode() const;

    /**
     * Checks the keyboard shortcuts of widgets in this dialog unless shortcut
     * checks are postponed or 'force' is 'true'.
     *
     * A forced shortcut check resets postponed checking.
     **/
    void checkShortcuts( bool force = false );

    /**
     * From now on, postpone keyboard shortcut checks - i.e. normal (not
     * forced) checkKeyboardShortcuts() will do nothing.  Reset this mode by
     * forcing a shortcut check with checkKeyboardShortcuts( true ).
     **/
    void postponeShortcutCheck();

    /**
     * Return whether or not shortcut checking is currently postponed.
     **/
    bool shortcutCheckPostponed() const;

    /**
     * Implements the ui command queryWidget
     **/
    YCPValue queryWidget( const YCPSymbol & property );

protected:

    static std::stack<YDialog *> _dialogStack;
    
private:

    ImplPtr<YDialogPrivate> priv;
};


#endif // YDialog_h
