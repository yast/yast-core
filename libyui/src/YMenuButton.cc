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

  File:	      YMenuButton.cc

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/


#include <ycp/YCPSymbol.h>
#include <ycp/YCPBoolean.h>
#define y2log_component "ui"
#include <ycp/y2log.h>

#include "YUISymbols.h"
#include "YMenuButton.h"


YMenuButton::YMenuButton( const YWidgetOpt & opt, YCPString label )
    : YWidget( opt )
    , label( label )
    , next_index(0)
{
    toplevel_menu = new YMenu( YCPString( "" ) );
}




void YMenuButton::setLabel( const YCPString & label )
{
    this->label = label;
}


YCPString YMenuButton::getLabel()
{
    return label;
}


YCPValue YMenuButton::changeWidget( const YCPSymbol & property, const YCPValue & newvalue )
{
    string s = property->symbol();

    /**
     * @property string Label the text on the MenuButton
     */
    if ( s == YUIProperty_Label )
    {
	if ( newvalue->isString() )
	{
	    setLabel( newvalue->asString() );
	    return YCPBoolean( true );
	}
	else
	{
	    y2error( "MenuButton: Invalid parameter %s for Label property. Must be string",
		     newvalue->toString().c_str() );
	    return YCPBoolean( false );
	}
    }
    else return YWidget::changeWidget( property, newvalue );
}



YCPValue YMenuButton::queryWidget( const YCPSymbol & property )
{
    string s = property->symbol();
    if ( s == YUIProperty_Label ) return getLabel();
    else return YWidget::queryWidget( property );
}



void
YMenuButton::addMenuItem( const YCPString &	item_label,
			  const YCPValue &	item_id,
			  YMenu *		parent_menu )
{
    YMenuItem * item = new YMenuItem( item_label, parent_menu, next_index++, item_id );
    
    if ( ! parent_menu )
    {
	parent_menu = toplevel_menu;
    }
    parent_menu->addMenuItem( item );
    
    items.push_back( item );
}
    

YMenu *
YMenuButton::addSubMenu( const YCPString &	sub_menu_label,
			 YMenu *		parent_menu )
{
    YMenu * menu = new YMenu( sub_menu_label, parent_menu );
    
    if ( ! parent_menu )
    {
	parent_menu = toplevel_menu;
    }
    parent_menu->addMenuItem( menu );

    return menu;
}
    

YCPValue
YMenuButton::indexToId( int index )
{
    if ( index >= 0 && ( unsigned ) index < items.size() )
    {
	YCPValue id = items[ index ]->getId();
	y2debug( "Selected menu item with ID '%s'", id->toString().c_str() );

	return id;
    }
    else
    {
	y2error( "No menu item with index %d", index );
	return YCPVoid();
    }
}


YMenuItem::YMenuItem( const YCPString &		label,
		      YMenu *			parent_menu,
		      int			index,
		      const YCPValue & 		id	)
    : label( label )
    , id( id )
    , parent( parent_menu )
    , index( index )
{
    
}


YMenu::YMenu( const YCPString & 	label,
	      YMenu *			parent_menu,
	      int			index,
	      const YCPValue &		id	)
    : YMenuItem( label, parent_menu, index, id )
{
    
}


void
YMenu::addMenuItem( YMenuItem *item )
{
    items.push_back( item );
}

