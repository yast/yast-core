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

  File:	      YRadioButton.cc

  Author:     Mathias Kettner <kettner@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/


#include <ycp/YCPSymbol.h>
#include <ycp/YCPBoolean.h>
#define y2log_component "ui"
#include <ycp/y2log.h>

#include "YUISymbols.h"
#include "YMacroRecorder.h"
#include "YRadioButtonGroup.h"
#include "YRadioButton.h"


YRadioButton::YRadioButton(YWidgetOpt &opt,
			   const YCPString &label,
			   YRadioButtonGroup *radiobuttongroup)
    : YWidget(opt)
    , label(label)
    , radiobuttongroup(radiobuttongroup)
{
}


YRadioButton::~YRadioButton()
{
    if (radiobuttongroup)
	radiobuttongroup->removeRadioButton(this);
}


void YRadioButton::setLabel(const YCPString &label)
{
    this->label = label;
}


YCPString YRadioButton::getLabel()
{
    return label;
}


YCPValue YRadioButton::changeWidget(const YCPSymbol & property, const YCPValue & newvalue)
{
    string s = property->symbol();
    /*
     * @property boolean Value the state of the RadioButton (on or off)
     */
    if (s == YUIProperty_Value)
    {
	if (newvalue->isBoolean())
	{
	    setValue(newvalue->asBoolean());
	    return YCPBoolean(true);
	}
	else
	{
	    y2error("RadioButton: Invalid parameter %s for property `Value. Must be boolean",
		    newvalue->toString().c_str());
	    return YCPBoolean(false);
	}
    }

    /*
     * @property string Label the RadioButton's text
     */
    else if (s == YUIProperty_Label)
    {
	if (newvalue->isString())
	{
	    setLabel(newvalue->asString());
	    return YCPBoolean(true);
	}
	else
	{
	    y2error("RadioButton: Invalid parameter %s for property `Label. Must be string",
		    newvalue->toString().c_str());
	    return YCPBoolean(false);
	}
    }
    else return YWidget::changeWidget(property, newvalue);
}



YCPValue YRadioButton::queryWidget(const YCPSymbol & property)
{
    string s = property->symbol();
    if	   (s == YUIProperty_Value) return getValue();
    else if (s == YUIProperty_Label) return getLabel();
    else return YWidget::queryWidget(property);
}


void YRadioButton::buttonGroupIsDead()
{
    radiobuttongroup = 0;
}


YRadioButtonGroup *YRadioButton::buttonGroup()
{
    return radiobuttongroup;
}


void YRadioButton::saveUserInput( YMacroRecorder *macroRecorder )
{
    YCPBoolean isChecked = getValue();

    if ( isChecked->value() )
    {
	// Only record if this radio button is on. By definition one radio
	// button of the radio box _must_ be on if the user did anything, so we
	// don't record a lot of redundant "ChangeWidget(..., `Value, false)"
	// calls.

	macroRecorder->recordWidgetProperty( this, YUIProperty_Value );
    }
}

