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

  File:	      YDumbTab.cc

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/


#include <ycp/YCPSymbol.h>
#include <ycp/YCPBoolean.h>
#include <ycp/YCPVoid.h>
#define y2log_component "ui"
#include <ycp/y2log.h>

#include "YUISymbols.h"
#include "YDumbTab.h"
#include "YShortcut.h"


YDumbTab::YDumbTab( const YWidgetOpt & opt )
    : YContainerWidget( opt )
{
    setDefaultStretchable( YD_HORIZ, true );
    setDefaultStretchable( YD_VERT,  true );
}


YCPValue YDumbTab::changeWidget( const YCPSymbol & property, const YCPValue & newValue )
{
    string s = property->symbol();

    /**
     * @property any CurrentItem the currently selected tab
     */
    if ( s == YUIProperty_CurrentItem )
    {
	int index = findTab( newValue );

	if ( index < 0 )
	{
	    y2error( "No DumbTab item with ID %s", newValue->toString().c_str() );
	    return YCPBoolean( false );
	}
	else
	{
	    setSelectedTab( index );
	    return YCPBoolean( true );
	}
    }
    else return YWidget::changeWidget( property, newValue );
}



YCPValue YDumbTab::queryWidget( const YCPSymbol & property )
{
    string s = property->symbol();
    if ( s == YUIProperty_CurrentItem )
    {
	int index = getSelectedTabIndex();

	if ( index >= 0 && index < (int) _tabs.size() )
	    return _tabs[ index ].id();
	else
	    return YCPVoid();
    }
    else return YWidget::queryWidget( property );
}


void YDumbTab::addTab( const YCPValue & 	id,
		       const YCPString &	label,
		       bool 			selected )
{
    _tabs.push_back( Tab( id, label ) );

    // As long as the automatic shortcut conflict resolver cannot handle wigets
    // with multiple shortcuts, it is better to avoid that situation
    // => strip shortcuts for now
    YCPString strippedLabel = YCPString( YShortcut::cleanShortcutString( label->value() ) );
		   
    addTab( strippedLabel ); // notify derived class

    if ( selected )
	setSelectedTab( (int) _tabs.size() - 1 );
}


int YDumbTab::findTab( const YCPValue & id )
{
    for ( unsigned i=0; i < _tabs.size(); i++ )
    {
	if ( _tabs[i].id()->equal( id ) )
	    return (int) i;
    }

    return -1;
}


void YDumbTab::addTab( const YCPString & label )
{
    y2error( "Virtual method not reimplemented!" );
}


int YDumbTab::getSelectedTabIndex()
{
    y2error( "Virtual method not reimplemented!" );
    return 0;
}


void YDumbTab::setSelectedTab( int index )
{
    y2error( "Virtual method not reimplemented!" );
}

