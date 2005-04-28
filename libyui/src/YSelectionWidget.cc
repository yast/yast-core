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

	if ( item->isString() ) // item is just a label
	{
	    addItem( YCPNull(), item->asString(), YCPNull(), false );
	}
	else if ( item->isTerm() // item is a term
		  && item->asTerm()->name() == YUISymbol_item )
	{
	    YCPTerm iterm = item->asTerm();

	    // `item()
	    //
	    //     Parameters:
	    //     `id()		optional
	    //     label (string)	mandatory
	    //     `icon()		optional
	    //     bool selected	optional

	    if ( iterm->size() < 1 || iterm->size() > 4 )
	    {
		y2error( "%s: Invalid argument number in %s",
			 widgetClass(), iterm->toString().c_str() );
	    }
	    else
	    {
		// Using YCPNull() as initial value to detect syntax errors

		YCPValue  	item_id    	= YCPNull();
		YCPString 	item_label	= YCPNull();
		YCPString 	item_icon  	= YCPNull();
		YCPBoolean	item_selected	= YCPNull();
		bool		ok		= true;

		for ( int n=0; n < iterm->size() && ok; n++ )
		{
		    YCPValue arg = iterm->value( n );

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
				{
				    item_icon = arg->asTerm()->value(0)->asString();
				    _hasIcons = true;
				}
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
		    else if ( arg->isBoolean() )	// selected flag
		    {
			ok = item_selected.isNull();

			if ( ok )
			    item_selected = arg->asBoolean();
		    }
		    else
		    {
			ok = false;
		    }
		}

		if ( ok )
		    ok = ! item_label.isNull();		// the label is mandatory

		if ( ! ok )
		{
		    y2error( "%s: Invalid item arguments: %s",
			     widgetClass(), iterm->toString().c_str() );
		}
		else
		{
		    if ( item_selected.isNull() )
			item_selected = YCPBoolean( false );

		    addItem( item_id, item_label, item_icon, item_selected->value() );
		}
	    }
	}
	else
	{
	    y2error( "Invalid item %s: %s items must be strings or specified with `"
		     YUISymbol_item "()", widgetClass(), item->toString().c_str() );
	}
    }
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
