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

  File:	      YPushButton.cc

  Author:     Mathias Kettner <kettner@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/


#include <ycp/YCPSymbol.h>
#include <ycp/YCPBoolean.h>
#define y2log_component "ui"
#include <ycp/y2log.h>

#include "YUISymbols.h"
#include "YPushButton.h"


YPushButton::YPushButton(YWidgetOpt &opt, YCPString label)
    : YWidget(opt)
    , label(label)
{
}




void YPushButton::setLabel(const YCPString &label)
{
    this->label = label;
}


YCPString YPushButton::getLabel()
{
    return label;
}


YCPValue YPushButton::changeWidget(const YCPSymbol & property, const YCPValue & newvalue)
{
    string s = property->symbol();

    /**
     * @property string Label the text on the PushButton
     */
    if (s == YUIProperty_Label)
    {
	if (newvalue->isString())
	{
	    setLabel(newvalue->asString());
	    return YCPBoolean(true);
	}
	else
	{
	    y2error("PushButton: Invalid parameter %s for Label property. Must be string",
		    newvalue->toString().c_str());
	    return YCPBoolean(false);
	}
    }
    else return YWidget::changeWidget(property, newvalue);
}



YCPValue YPushButton::queryWidget(const YCPSymbol & property)
{
    string s = property->symbol();
    if (s == YUIProperty_Label) return getLabel();
    else return YWidget::queryWidget(property);
}
