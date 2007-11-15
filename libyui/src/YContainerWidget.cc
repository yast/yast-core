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

  File:		YContainerWidget.cc

  Author:	Stefan Hundhammer <sh@suse.de>

/-*/


#define y2log_component "ui"
#include <ycp/y2log.h>
#include <ycp/YCPSymbol.h>
#include "YContainerWidget.h"
#include "YWizard.h"
#include "YLabel.h"
#include "YMenuButton.h"
#include "YPushButton.h"


YContainerWidget::YContainerWidget( const YWidgetOpt & opt )
    : YWidget( opt )
{
    _debugLabelWidget = 0;
    setChildrenManager( new YWidgetChildrenManager( this ) );
}


YContainerWidget::~YContainerWidget()
{
}


void YContainerWidget::addChild( YWidget *child )
{
#if 0
    if ( ! _debugLabelWidget )
    {
	if 	( dynamic_cast<YWizard  	*> (child ) )	_debugLabelWidget = child;
	else if ( dynamic_cast<YLabel   	*> (child ) )	_debugLabelWidget = child;
	else if ( dynamic_cast<YMenuButton 	*> (child ) )	_debugLabelWidget = child;
	else if ( dynamic_cast<YPushButton 	*> (child ) )	_debugLabelWidget = child;
	else if ( dynamic_cast<YLabel		*> (child ) )	_debugLabelWidget = child;
    }
#endif

    YWidget::addChild( child );
}

	
int YContainerWidget::preferredWidth()
{
    if ( hasChildren() )
	return firstChild()->preferredWidth();
    else
	return 0;
}


int YContainerWidget::preferredHeight()
{
    if ( hasChildren() )
	return firstChild()->preferredHeight();
    else
	return 0;
}


void YContainerWidget::setSize( int width, int height )
{
    if ( hasChildren() )
	firstChild()->setSize( width , height );
}


bool YContainerWidget::stretchable( YUIDimension dim ) const
{
    if ( hasChildren() )
	return firstChild()->stretchable( dim );
    else
	return false;
}


string YContainerWidget::debugLabel()
{
    if ( _debugLabelWidget )
    {
	string label;
	
	if ( _debugLabelWidget == this )
	    label = YWidget::debugLabel();
	else
	    label = _debugLabelWidget->debugLabel();

	if ( ! label.empty() )
	    return formatDebugLabel( _debugLabelWidget, label );
    }

    for ( YWidgetListConstIterator it = childrenBegin();
	  it != childrenEnd();
	  ++it )
    {
	YWidget * child = *it;
	
	if ( child->isValid() )
	{
	    string label = child->debugLabel();

	    if ( ! label.empty() )
		return formatDebugLabel( child, label );
	}
    }


    return "";
}


string YContainerWidget::formatDebugLabel( YWidget * widget, const string & debLabel )
{
    if ( ! widget || debLabel.empty() )
	return "";

    string label;

    if ( widget->hasChildren() )
    {
	label = debLabel;
    }
    else
    {
	label = "YContainerWidget with ";
	label = widget->widgetClass();
	label += " \"";
	label += debLabel;
	label += "\"";
    }

    return label;
}


