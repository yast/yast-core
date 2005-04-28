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

  File:	      YSelectionWidget.cc

  Author:     Holger Macht <hmacht@suse.de>

/-*/


#include <ycp/YCPSymbol.h>
#include <ycp/YCPBoolean.h>
#include <ycp/YCPVoid.h>
#define y2log_component "ui"
#include <ycp/y2log.h>

#include "YUISymbols.h"
#include "YMacroRecorder.h"
#include "YSelectionBox.h"
#include "YUI.h"


YSelectionWidget::YSelectionWidget( const YWidgetOpt & opt, YCPString label )
    : YWidget( opt )
    , label( label )
    , _hasIcons( false )
{
    // y2debug( "YSelectionWidget( %s )", label->value_cstr() );
}


void YSelectionWidget::setLabel( const YCPString & label )
{
    this->label = label;
}

YCPString YSelectionWidget::getLabel()
{
    return label;
}


void YSelectionWidget::itemAdded( const YCPString & , int index, bool selected)
{
    // default dummy implementation
}


int YSelectionWidget::numItems() const
{
    return item_ids->size();
}


int YSelectionWidget::itemWithId( const YCPValue & id, bool report_error )
{
    for ( int i=0; i < numItems(); i++ )
    {
	if ( ! item_ids->value(i).isNull() && item_ids->value(i)->equal( id ) ) return i;
	else if ( item_labels->value(i)->equal( id ) ) return i;
    }
    if ( report_error )
	y2error( "SelectionWidget: No item %s existing", id->toString().c_str() );

    return -1;
}


void YSelectionWidget::parseItemList( const YCPList & item_list )
{
    _hasIcons = false;

    for ( int i=0; i < item_list->size(); i++ )
    {
	YCPValue item = item_list->value(i);

	if ( item->isString() )		// item is just a label
	{
	    addItem( YCPNull(), item->asString(), YCPNull(), false );
	}
	else if ( item->isTerm()	// `item(...)
		  && item->asTerm()->name() == YUISymbol_item )
	{
	    YCPValue  	id		= YCPNull();
	    YCPString 	label		= YCPNull();
	    YCPString 	icon		= YCPNull();
	    YCPBoolean	selected	= YCPNull();
	    YCPList	subItemList	= YCPNull();

	    if ( parseItem( item->asTerm(),				// in
			    id, label, icon, selected, subItemList ) )	// out
	    {
		if ( selected.isNull() )
		    selected = YCPBoolean( false );

		if ( ! subItemList.isNull() )
		{
		    y2error( "No sub item list permitted for %s - rejecting %s",
			     widgetClass(), subItemList->toString().c_str() );
		}

		if ( ! icon.isNull() )
		    _hasIcons = true;

		addItem( id, label, icon, selected->value() );
	    }
	    else
	    {
		y2error( "%s: Invalid item arguments: %s",
			 widgetClass(), item->toString().c_str() );
	    }
	}
	else
	{
	    y2error( "Invalid item %s: %s items must be strings or specified with `"
		     YUISymbol_item "()", widgetClass(), item->toString().c_str() );
	}
    }
}


bool YSelectionWidget::parseItem( const YCPTerm & itemTerm,
				  YCPValue  	& item_id,
				  YCPString 	& item_label,
				  YCPString 	& item_icon,
				  YCPBoolean	& item_selected,
				  YCPList	& sub_item_list )
{
    bool ok = true;

    if ( itemTerm->size() < 1 || itemTerm->size() > 4 )
    {
	y2error( "%s: Invalid argument number in %s", itemTerm->toString().c_str() );
    }

    item_id    		= YCPNull();
    item_label		= YCPNull();
    item_icon  		= YCPNull();
    item_selected	= YCPNull();
    sub_item_list	= YCPNull();

    for ( int n=0; n < itemTerm->size() && ok; n++ )
    {
	YCPValue arg = itemTerm->value( n );

	if ( arg->isTerm() )	// `id(), `icon()
	{
	    ok = arg->asTerm()->size() == 1;

	    if ( ok )
	    {
		if ( arg->asTerm()->name() == YUISymbol_id )	// `id()
		{
		    item_id = arg->asTerm()->value(0);
		}
		else if ( arg->asTerm()->name() == YUISymbol_icon )	// `icon()
		{
		    ok = arg->asTerm()->value(0)->isString();

		    if ( ok )
			item_icon = arg->asTerm()->value(0)->asString();
		}
		else
		{
		    ok = false;
		}
	    }
	}
	else if ( arg->isString() )		// label
	{
	    ok = item_label.isNull();

	    if ( ok )
		item_label = arg->asString();
	}
	else if ( arg->isBoolean() )		// "selected" flag
	{
	    ok = item_selected.isNull();

	    if ( ok )
		item_selected = arg->asBoolean();
	}
	else if ( arg->isList() )		// sub item list (for tree only)
	{
	    sub_item_list = arg->asList();
	}
	else
	{
	    ok = false;
	}
    }

    if ( ok )
	ok = ! item_label.isNull();		// the label is mandatory

    return ok;
}


void YSelectionWidget::addItem( const YCPValue & id,
				const YCPString & text,
				const YCPString & icon,
				bool selected )
{
    item_ids->add( id );
    item_labels->add( text );
    item_icons->add( icon );

    itemAdded( text, numItems()-1, selected );
}


void YSelectionWidget::deleteAllItems()
{
    item_labels = YCPList();
    item_ids	= YCPList();
}


YCPString YSelectionWidget::itemIcon( int item_no ) const
{
    if ( item_no < 0 || item_no >= item_icons.size() )
	return YCPString( "" );

    YCPString icon = item_icons->value( item_no )->asString();

    if ( icon.isNull() || icon->isVoid() )
	return YCPString( "" );

    return icon;
}



YCPValue YSelectionWidget::changeLabel( const YCPValue & newValue )
{
    if ( newValue->isString() )
    {
	setLabel( newValue->asString() );

	return YCPBoolean( true );
    }
    else
    {
	y2error( "%s: Invalid parameter %s for Label property. Must be string",
		 widgetClass(), newValue->toString().c_str() );

	return YCPBoolean( false );
    }
}


YCPValue YSelectionWidget::changeItems ( const YCPValue & newValue )
{
    if ( newValue->isList() )
    {
	deleteAllItems();
	parseItemList( newValue->asList() );
    }

    return YCPBoolean( true );
}
