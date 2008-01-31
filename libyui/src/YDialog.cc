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

  File:		YDialog.cc

  Author:	Stefan Hundhammer <sh@suse.de>

/-*/


#define YUILogComponent "ui"
#include "YUILog.h"
#include <ycp/YCPSymbol.h>
#include "YDialog.h"
#include "YShortcutManager.h"
#include "YPushButton.h"
#include "YUIException.h"

using std::string;

#define VERBOSE_DIALOGS	0


std::stack<YDialog *> YDialog::_dialogStack;


struct YDialogPrivate
{
    YDialogPrivate( YDialogType dialogType, YDialogColorMode colorMode )
	: dialogType( dialogType )
	, colorMode( colorMode )
	, shortcutCheckPostponed( false )
	, defaultButton( 0 )
	, isOpen( false )
	{}

    YDialogType		dialogType;
    YDialogColorMode	colorMode;
    bool		shortcutCheckPostponed;
    YPushButton *	defaultButton;
    bool		isOpen;
};



YDialog::YDialog( YDialogType dialogType, YDialogColorMode colorMode )
    : YSingleChildContainerWidget( 0 )
    , priv( new YDialogPrivate( dialogType, colorMode ) )
{
    YUI_CHECK_NEW( priv );

    _dialogStack.push( this );

#if VERBOSE_DIALOGS
    yuiDebug() << "New YDialog " << this << endl;
#endif
}


YDialog::~YDialog()
{
#if VERBOSE_DIALOGS
    yuiDebug() << "Destroying " << this << endl;
#endif

    if ( ! _dialogStack.empty() && _dialogStack.top() == this )
    {
	_dialogStack.pop();

	if ( ! _dialogStack.empty() )
	    _dialogStack.top()->activate();
    }
    else
	yuiError() << "Not top of dialog stack: " << this << endl;
}


void
YDialog::open()
{
    if ( priv->isOpen )
	return;

    checkShortcuts();
    setInitialSize();
    openInternal();	// Make sure this is only called once!

    priv->isOpen = true;
}


bool
YDialog::isOpen() const
{
    return priv->isOpen;
}


bool
YDialog::destroy( bool doThrow )
{
    YUI_CHECK_WIDGET( this );

    if ( _dialogStack.empty() ||
	 _dialogStack.top() != this )
    {
	if ( doThrow )
	    YUI_THROW( YUIDialogStackingOrderException() );

	return false;
    }

    delete this;

    return true;
}


YDialogType
YDialog::dialogType() const
{
    return priv->dialogType;
}


YDialogColorMode
YDialog::colorMode() const
{
    return priv->colorMode;
}


void
YDialog::postponeShortcutCheck()
{
    priv->shortcutCheckPostponed = true;
}


bool
YDialog::shortcutCheckPostponed() const
{
    return priv->shortcutCheckPostponed;
}


void
YDialog::checkShortcuts( bool force )
{
    if ( priv->shortcutCheckPostponed && ! force )
    {
	yuiDebug() << "Shortcut check postponed" << endl;
    }
    else
    {

	YShortcutManager shortcutManager( this );
	shortcutManager.checkShortcuts();

	priv->shortcutCheckPostponed = false;
    }
}


YPushButton *
YDialog::defaultButton() const
{
    return priv->defaultButton;
}


void
YDialog::setDefaultButton( YPushButton * newDefaultButton )
{
    if ( newDefaultButton && priv->defaultButton ) // already have one?
    {
	yuiError() << "Too many `opt(`default) PushButtons: ["
		   << newDefaultButton->label()
		   << "]" << endl;
    }

    priv->defaultButton = newDefaultButton;
}


void
YDialog::setInitialSize()
{
#if VERBOSE_DIALOGS
    yuiDebug() << "Setting initial size for " << this << endl;
#endif

    // Trigger geometry management
    setSize( preferredWidth(), preferredHeight() );
}


void
YDialog::recalcLayout()
{
    yuiDebug() << "Recalculating layout for " << this << endl;

    setSize( preferredWidth(), preferredHeight() );
}




YDialog *
YDialog::currentDialog( bool doThrow )
{
    if ( _dialogStack.empty() )
    {
	if ( doThrow )
	    YUI_THROW( YUINoDialogException() );
	return 0;
    }
    else
	return _dialogStack.top();
}


bool
YDialog::deleteTopmostDialog( bool doThrow )
{
    if ( _dialogStack.empty() )
    {
	if ( doThrow )
	    YUI_THROW( YUINoDialogException() );
    }
    else
    {
	delete _dialogStack.top();
    }

    return ! _dialogStack.empty();
}


void
YDialog::deleteAllDialogs()
{
    while ( ! _dialogStack.empty() )
    {
	delete _dialogStack.top();
    }
}


void
YDialog::deleteTo( YDialog * targetDialog )
{
    YUI_CHECK_WIDGET( targetDialog );

    while ( ! _dialogStack.empty() )
    {
	YDialog * dialog = _dialogStack.top();

	delete dialog;

	if ( dialog == targetDialog )
	    return;
    }

    // If we ever get here, targetDialog was nowhere in the dialog stack.

    YUI_THROW( YUIDialogStackingOrderException() );
}


int
YDialog::openDialogsCount()
{
    return _dialogStack.size();
}


