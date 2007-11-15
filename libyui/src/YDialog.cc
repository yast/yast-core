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


#define y2log_component "ui"
#include <ycp/y2log.h>
#include <ycp/YCPSymbol.h>
#include "YDialog.h"
#include "YShortcutManager.h"
#include "YUIException.h"

using std::string;

#define VERBOSE_DIALOGS	0



std::stack<YDialog *> YDialog::_dialogStack;


YDialog::YDialog( const YWidgetOpt & opt )
    : YSingleChildContainerWidget( 0 )
{
    _hasDefaultSize.setValue( opt.hasDefaultSize.value() );
    _hasWarnColor.setValue( opt.hasWarnColor.value() );
    _hasInfoColor.setValue( opt.hasInfoColor.value() );
    _isDecorated.setValue( opt.isDecorated.value() );
    _isCentered.setValue( opt.isCentered.value() );
    _hasSmallDecorations.setValue( opt.hasSmallDecorations.value() );
    _shortcutCheckPostponed = false;

    _dialogStack.push( this );

#if VERBOSE_DIALOGS
    y2debug( "New YDialog at %p", this );
#endif
}


YDialog::~YDialog()
{
#if VERBOSE_DIALOGS
    y2debug( "Destroying YDialog at %p", this );
#endif
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


void YDialog::setInitialSize()
{
#if VERBOSE_DIALOGS
    y2debug( "Setting initial size for YDialog at %p", this );
#endif
    
    // Trigger geometry management
    setSize( preferredWidth(), preferredHeight() );
}


void YDialog::checkShortcuts( bool force )
{
    if ( _shortcutCheckPostponed && ! force )
    {
	y2debug( "shortcut check postponed" );
	return;
    }

    YShortcutManager shortcutManager( this );
    shortcutManager.checkShortcuts();

    _shortcutCheckPostponed = false;
}


YCPValue YDialog::queryWidget( const YCPSymbol & property )
{
    string symbol = property->symbol();

    if ( symbol == YUIProperty_DebugLabel ||
	 symbol == YUIProperty_DialogDebugLabel )	return YCPString( dialogDebugLabel() );
    else
    {
	return YWidget::queryWidget( property );
    }
}


string YDialog::dialogDebugLabel()
{
#if 0
    // FIXME
    // FIXME
    // FIXME

    if ( _debugLabelWidget )
    {
	string label = _debugLabelWidget->debugLabel();

	if ( ! label.empty() )
	    return formatDebugLabel( _debugLabelWidget, label );
    }

    for ( YWidgetListConstIterator * it = parent->childrenBegin();
	  it != parent->childrenEnd();
	  ++it )
    {
	string label = (*it)->debugLabel();

	if ( ! label.empty() )
	    return formatDebugLabel( child(i), label );
    }

    // FIXME
    // FIXME
    // FIXME
#endif

    return widgetClass();
}


string YDialog::formatDebugLabel( YWidget * widget, const string & debLabel )
{
    if ( debLabel.empty() )
	return "";

    string label = "Dialog with ";

    if ( widget->hasChildren() )
    {
	label += debLabel;
    }
    else
    {
	label += widget->widgetClass();
	label += " \"";
	label += debLabel;
	label += "\"";
    }

    return label;
}


