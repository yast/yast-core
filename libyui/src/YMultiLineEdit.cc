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

  File:       YMultiLineEdit.cc

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/


#include <ycp/YCPSymbol.h>
#include <ycp/YCPBoolean.h>
#define y2log_component "ui"
#include <ycp/y2log.h>

#include "YUISymbols.h"
#include "YMacroRecorder.h"
#include "YMultiLineEdit.h"


YMultiLineEdit::YMultiLineEdit(YWidgetOpt &opt, const YCPString &label)
    : YWidget(opt)
    , _label(label)
{
    setDefaultStretchable(YD_HORIZ, true);
    setDefaultStretchable(YD_VERT,  true);
}


void YMultiLineEdit::setLabel(const YCPString & newLabel)
{
    _label = newLabel;
}



YCPValue YMultiLineEdit::changeWidget(const YCPSymbol & property, const YCPValue & newValue)
{
    string s = property->symbol();

    /**
     * @property string Value
     * The text contents as one large string containing newlines.
     */
    if (s == YUIProperty_Value )
    {
	if (newValue->isString())
	{
	    setText( newValue->asString() );
	    return YCPBoolean(true);
	}
	else
	{
	    y2error( "MultiLineEdit: Invalid Value property - string expected, not %s",
		     newValue->toString().c_str() );

	    return YCPBoolean(false);
	}
    }
    /**
     * @property string Label The label above the log text.
     */
    else if (s == YUIProperty_Label )
    {
	if (newValue->isString())
	{
	    setLabel( newValue->asString() );
	    return YCPBoolean(true);
	}
	else
	{
	    y2error( "MultiLineEdit: Invalid Label property - string expected, not %s",
		     newValue->toString().c_str() );

	    return YCPBoolean(false);
	}
    }
    else return YWidget::changeWidget( property, newValue );
}



YCPValue YMultiLineEdit::queryWidget(const YCPSymbol & property)
{
    string s = property->symbol();
    if      (s == YUIProperty_Label) return label();
    else if (s == YUIProperty_Value) return text();
    else return YWidget::queryWidget(property);
}


void YMultiLineEdit::saveUserInput( YMacroRecorder *macroRecorder )
{
    macroRecorder->recordWidgetProperty( this, YUIProperty_Value );
}
