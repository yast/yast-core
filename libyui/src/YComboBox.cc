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

  File:	      YComboBox.cc

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/


#include <ycp/YCPSymbol.h>
#include <ycp/YCPBoolean.h>
#include <ycp/YCPVoid.h>
#define y2log_component "ui"
#include <ycp/y2log.h>

#include "YUISymbols.h"
#include "YMacroRecorder.h"
#include "YComboBox.h"


YComboBox::YComboBox( YWidgetOpt & opt, YCPString label )
    : YWidget( opt )
    , label( label )
    , validChars( "" )
{
    y2debug( "YComboBox( %s )", label->value_cstr() );

    _editable = opt.isEditable.value();
}


YCPValue YComboBox::changeWidget( const YCPSymbol & property, const YCPValue & newvalue )
{
    string s = property->symbol();

    /**
     * @property string Label
     * The label above the ComboBox describing what it is all about
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
	    y2error( "ComboBox: Invalid parameter %s for Label property. Must be string",
		    newvalue->toString().c_str() );
	    return YCPBoolean( false );
	}
    }

    /**
     * @property string Value
     * The current value of the ComboBox. If this is a list item which
     * has an ID, the ID will be used rather than the corresponding text.
     * For editable ComboBox widgets it is OK to set <em>any</em> value - even
     * if it doesn't exist in the list.
     */
    else if ( s == YUIProperty_Value )		// Select item with that id
    {
	int index = itemWithId( newvalue, ! editable() ); // true: log error

	if ( index >= 0 )			// Item already exists in list
	{
	    setCurrentItem( index );
	    return YCPBoolean( true );		// OK
	}
	else					// Item not in list
	{
	    if ( editable() )
	    {
		setValue( newvalue->asString() );
		return YCPBoolean( true );	// OK
	    }
	    else 				// ! editable()
	    {
		return YCPBoolean( false );	// Error
	    }
	}
    }

    /**
     * @property string ValidChars valid input characters
     */
    else if ( s == YUIProperty_ValidChars )
    {
	if ( editable() )
	{
	    if ( newvalue->isString() )
	    {
		setValidChars( newvalue->asString() );
		return YCPBoolean( true );
	    }
	    else
	    {
		y2error( "ComboBox: Invalid parameter %s for ValidChars property. Must be string.",
			newvalue->toString().c_str() );
		return YCPBoolean( false );
	    }
	}
	else
	{
	    y2error( "ComboBox: Setting the ValidChars property doesn't make sense "
		    "for a non-editable ComboBox!" );
	    return YCPBoolean( false );
	}
    }
    else return YWidget::changeWidget( property, newvalue ); // handle default properties
}



YCPValue YComboBox::queryWidget( const YCPSymbol & property )
{
    string s = property->symbol();
    if		( s == YUIProperty_Label ) 	return getLabel();
    else if	( s == YUIProperty_Value )
    {
	YCPString val = getValue();
	YCPValue id = IdForValue( val );

	if ( id->isVoid() )	return val;
	else 			return id;
    }
    else if ( s == YUIProperty_ValidChars )	return getValidChars();
    else return YWidget::queryWidget( property );
}


void YComboBox::setLabel( const YCPString & label )
{
    this->label = label;
}


YCPString YComboBox::getLabel()
{
    return label;
}


void YComboBox::setValidChars( const YCPString & newValidChars )
{
    this->validChars= newValidChars;
}


YCPString YComboBox::getValidChars()
{
    return validChars;
}


void YComboBox::addItem( const YCPValue & id, const YCPString & text, bool selected )
{
    item_ids->add( id );
    item_labels->add( text );
    itemAdded( text, numItems()-1, selected );

    if ( selected )
    {
	setCurrentItem( numItems()-1 );
    }
}


void YComboBox::itemAdded( const YCPString & , int, bool )
{
    // default dummy implementaion
}


int YComboBox::numItems() const
{
    return item_ids->size();
}


int YComboBox::itemWithId( const YCPValue & id, bool report_error )
{
    for ( int i=0; i < numItems(); i++ )
    {
	if ( ! item_ids->value( i ).isNull() && item_ids->value( i )->equal( id ) ) return i;
	else if ( item_labels->value( i )->equal( id ) ) return i;
    }
    if ( report_error )
	y2error( "ComboBox: No item %s existing", id->toString().c_str() );

    return -1;
}


YCPValue YComboBox::IdForValue( const YCPValue & val )
{
    for ( int i=0; i < numItems(); i++ )
    {
	if ( ! item_ids->value( i ).isNull() && item_labels->value( i )->equal( val ) )
	    return item_ids->value( i );
    }

    return YCPVoid();
}



void YComboBox::saveUserInput( YMacroRecorder *macroRecorder )
{
    macroRecorder->recordWidgetProperty( this, YUIProperty_Value );
}

