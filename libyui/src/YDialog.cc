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

  File:	      YDialog.cc

  Author:     Mathias Kettner <kettner@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/


#define y2log_component "ui"
#include <ycp/y2log.h>
#include <ycp/YCPSymbol.h>
#include "YDialog.h"
#include "YWizard.h"
#include "YLabel.h"
#include "YMenuButton.h"
#include "YShortcutManager.h"

using std::string;


YDialog::YDialog( const YWidgetOpt & opt )
    : YContainerWidget( opt )
{
    _hasDefaultSize.setValue( opt.hasDefaultSize.value() );
    _hasWarnColor.setValue( opt.hasWarnColor.value() );
    _hasInfoColor.setValue( opt.hasInfoColor.value() );
    _isDecorated.setValue( opt.isDecorated.value() );
    _isCentered.setValue( opt.isCentered.value() );
    _hasSmallDecorations.setValue( opt.hasSmallDecorations.value() );
    _shortcutCheckPostponed = false;
    _debugLabelWidget = 0;
}


YDialog::~YDialog()
{
}


bool YDialog::isDialog() const
{
    return true;
}


void YDialog::setInitialSize()
{
    setSize( nicesize( YD_HORIZ ), nicesize( YD_VERT ) );
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


YWidgetList YDialog::widgets() const
{
    YWidgetList widgetList;
    fillWidgetList( widgetList, this );

    return widgetList;
}


void YDialog::fillWidgetList( YWidgetList &		widgetList,
			      const YContainerWidget *	parent 		) const
{
    if ( ! parent )
	return;

    for ( int i = 0; i < parent->numChildren(); i++ )
    {
	YWidget * child = parent->child(i);

	if ( ! child->isDialog() )
	{
	    widgetList.push_back( child );
	    YContainerWidget * container = dynamic_cast<YContainerWidget *> ( child );

	    if ( container )
		fillWidgetList( widgetList, container );
	}
	else
	{
	    y2milestone( "Found dialog: %s", child->widgetClass() );
	}
    }
}


string YDialog::dialogDebugLabel()
{
    if ( _debugLabelWidget )
    {
	string label = _debugLabelWidget->debugLabel();
	label = string( "Dialog \"" ) + label + string( "\"" );

	return label;
    }
    else
    {
	return "Dialog";
    }
}


void YDialog::addChild( YWidget * newChild )
{
    YContainerWidget::addChild( newChild );

    if ( ! _debugLabelWidget )
    {
	if 	( dynamic_cast<YWizard  	*> (newChild ) )	_debugLabelWidget = newChild;
	else if ( dynamic_cast<YLabel   	*> (newChild ) )	_debugLabelWidget = newChild;
	else if ( dynamic_cast<YMenuButton 	*> (newChild ) )	_debugLabelWidget = newChild;
    }
}


void YDialog::childDeleted( YWidget * deletedChild )
{
    if ( deletedChild == _debugLabelWidget )
	_debugLabelWidget = 0;
    
    YContainerWidget::childDeleted( deletedChild );
}


