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

  File:	      YProgressBar.cc

  Author:     Mathias Kettner <kettner@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/


#include <ycp/YCPSymbol.h>
#include <ycp/YCPBoolean.h>
#define y2log_component "ui"
#include <ycp/y2log.h>

#include "YUISymbols.h"
#include "YProgressBar.h"


YProgressBar::YProgressBar( YWidgetOpt & opt,
			   const YCPString & label,
			   const YCPInteger & maxProgress,
			   const YCPInteger & progress )
    : YWidget( opt )
    , label( label )
    , maxProgress( maxProgress )
    , progress( progress )
{
    setDefaultStretchable( YD_HORIZ, true );
    setStretchable( YD_VERT, false );
}


void YProgressBar::setLabel( const YCPString & label )
{
    this->label = label;
}


void YProgressBar::setProgress( const YCPInteger & progress )
{
    this->progress = progress;
}


YCPString YProgressBar::getLabel()
{
    return label;
}


YCPInteger YProgressBar::getProgress()
{
    return progress;
}


YCPValue YProgressBar::changeWidget( const YCPSymbol & property, const YCPValue & newvalue )
{
    string s = property->symbol();

    /**
     * @property integer Value the current progress
     */
    if ( s == YUIProperty_Value )
    {
	if ( newvalue->isInteger() )
	{
	    YCPInteger p = newvalue->asInteger();
	    if ( p->value() < 0 )
	    {
		y2warning( "Negative value %s for progress bar", p->toString().c_str() );
		p = YCPInteger( 0LL );
	    }
	    else if ( p->value() > maxProgress->value() )
	    {
		y2warning( "Too big value %s for progress bar ( max is %s )",
			  p->toString().c_str(), maxProgress->toString().c_str() );
		p = maxProgress;
	    }
	    setProgress( p );
	    this->progress = p;
	    return YCPBoolean( true );
	}
	else
	{
	    y2error( "ProgressBar: Invalid parameter %s for Value property. Must be integer",
		    newvalue->toString().c_str() );
	    return YCPBoolean( false );
	}
    }

    /**
     * @property string Label the label above the progress bar
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
	    y2error( "ProgressBar: Invalid parameter %s for Label property. Must be string",
		    newvalue->toString().c_str() );
	    return YCPBoolean( false );
	}
    }
    else return YWidget::changeWidget( property, newvalue );
}



YCPValue YProgressBar::queryWidget( const YCPSymbol & property )
{
    string s = property->symbol();
    if      ( s == YUIProperty_Value ) return getProgress();
    else if ( s == YUIProperty_Label ) return getLabel();
    else return YWidget::queryWidget( property );
}

