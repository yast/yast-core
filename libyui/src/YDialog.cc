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
	{}

    YDialogType		dialogType;
    YDialogColorMode	colorMode;
    bool		shortcutCheckPostponed;
    YPushButton *	defaultButton;
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
	_dialogStack.pop();
    }

    return ! _dialogStack.empty();
}


void
YDialog::deleteAllDialogs()
{
    while ( deleteTopmostDialog( false ) )
    {}
}


int
YDialog::openDialogsCount()
{
    return _dialogStack.size();
}


