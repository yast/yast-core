/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|      	                        core system                            |
|                                                        (C) SuSE GmbH |
\----------------------------------------------------------------------/

  File:       YTextEntry.cc

  Author:     Mathias Kettner <kettner@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/


#include <ycp/YCPSymbol.h>
#include <ycp/YCPBoolean.h>
#include <ycp/YCPInteger.h>
#define y2log_component "ui"
#include <ycp/y2log.h>

#include "YUISymbols.h"
#include "YMacroRecorder.h"
#include "YTextEntry.h"


YTextEntry::YTextEntry( const YWidgetOpt & opt, const YCPString & label )
    : YWidget( opt )
    , label( label )
    , validChars( "" )
{
    _passwordMode = opt.passwordMode.value();
    setDefaultStretchable( YD_HORIZ, true );
    setDefaultStretchable( YD_VERT,  false );
}


void YTextEntry::setLabel( const YCPString & label )
{
    this->label = label;
}


YCPString YTextEntry::getLabel()
{
    return label;
}


void YTextEntry::setValidChars( const YCPString & newValidChars )
{
    this->validChars= newValidChars;
}


YCPString YTextEntry::getValidChars()
{
    return validChars;
}


YCPValue YTextEntry::changeWidget( const YCPSymbol & property, const YCPValue & newvalue )
{
    string s = property->symbol();
    /**
     * @property string Value the field's contents ( the user input )
     */
    if ( s == YUIProperty_Value )
    {
	if ( newvalue->isString() )
	{
	    setText( newvalue->asString() );
	    return YCPBoolean( true );
	}
	else
	{
	    y2error( "TextEntry: Invalid parameter %s for Value property. Must be string.",
		     newvalue->toString().c_str() );
	    return YCPBoolean( false );
	}
    }
    /**
     * @property string Label label above the field
     */
    else if ( s == YUIProperty_Label )
    {
	if ( newvalue->isString() )
	{
	    setLabel( newvalue->asString() );
	    return YCPBoolean( true );
	}
	else
	{
	    y2error( "TextEntry: Invalid parameter %s for Label property. Must be string.",
		     newvalue->toString().c_str() );
	    return YCPBoolean( false );
	}
    }
    /**
     * @property string ValidChars valid input characters
     */
    else if ( s == YUIProperty_ValidChars )
    {
	if ( newvalue->isString() )
	{
	    setValidChars( newvalue->asString() );
	    return YCPBoolean( true );
	}
	else
	{
	    y2error( "TextEntry: Invalid parameter %s for ValidChars property. Must be string.",
		     newvalue->toString().c_str() );
	    return YCPBoolean( false );
	}
    }
    /**
     * @property interger InputMaxLength limit the amount of characters
     */
    else if ( s == YUIProperty_InputMaxLength )
    {
	if ( newvalue->isInteger() )
	{
	    setInputMaxLength( newvalue->asInteger()->value() );
	    return YCPBoolean( true );
	}
	else
	{
	    y2error( "TextEntry: Invalid parameter %s for InputMaxLength. Must be integer.",
		     newvalue->toString().c_str() );
	    return YCPBoolean( false );
	}
    }
    else return YWidget::changeWidget( property, newvalue );
}

void YTextEntry::setInputMaxLength( const YCPInteger & numberOfChars ) 
{
}

YCPValue YTextEntry::queryWidget( const YCPSymbol & property )
{
    string s = property->symbol();
    if      ( s == YUIProperty_Value ) 		return getText();
    else if ( s == YUIProperty_Label ) 		return getLabel();
    else if ( s == YUIProperty_ValidChars )	return getValidChars();
    else return YWidget::queryWidget( property );
}


void YTextEntry::saveUserInput( YMacroRecorder *macroRecorder )
{
    if ( passwordMode() )
    {
	if ( hasId() )
	{
	    string text = "UI::";
	    text += YUIBuiltin_ChangeWidget;
	    text += "( " + id()->toString() + ", \t`";
	    text += YUIProperty_Value;
	    text += ", \"<not recording password in plain text>\" );\t// ";
	    text += widgetClass();
	    text += " \"" + debugLabel() + "\"";
	    
	    macroRecorder->recordComment( text );
	}
    }
    else
    {
	macroRecorder->recordWidgetProperty( this, YUIProperty_Value );
    }
}

