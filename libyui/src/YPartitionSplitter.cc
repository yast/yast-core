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

  File:	      YPartitionSplitter.cc

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/


#include <ycp/YCPSymbol.h>
#include <ycp/YCPBoolean.h>
#define y2log_component "ui"
#include <ycp/y2log.h>

#include "YUISymbols.h"
#include "YMacroRecorder.h"
#include "YPartitionSplitter.h"


YPartitionSplitter::YPartitionSplitter( const YWidgetOpt &	opt,
					int 			usedSize,
					int 			totalFreeSize,
					int 			newPartSize,
					int 			minNewPartSize,
					int 			minFreeSize,
					const YCPString &	usedLabel,
					const YCPString &	freeLabel,
					const YCPString &	newPartLabel,
					const YCPString &	freeFieldLabel,
					const YCPString &	newPartFieldLabel )
    : YWidget			( opt			)
    , _usedSize			( usedSize 		)
    , _totalFreeSize		( totalFreeSize 	)
    , _newPartSize 		( newPartSize 		)
    , _minNewPartSize 		( minNewPartSize	)
    , _minFreeSize 		( minFreeSize 		)
    , _usedLabel 		( usedLabel 		)
    , _freeLabel 		( freeLabel 		)
    , _newPartLabel 		( newPartLabel 		)
    , _freeFieldLabel 		( freeFieldLabel	)
    , _newPartFieldLabel	( newPartFieldLabel 	)
{
    setDefaultStretchable( YD_HORIZ, true );
    setStretchable( YD_VERT, false );
}


void YPartitionSplitter::setValue( int newValue )
{
    _newPartSize = newValue;
}


YCPValue YPartitionSplitter::changeWidget( const YCPSymbol & property,
					   const YCPValue  & newValue )
{
    string sym = property->symbol();

    /**
     * @property integer Value the numerical value
     */
    if ( sym == YUIProperty_Value )
    {
	if ( newValue->isInteger() )
	{
	    int val = newValue->asInteger()->value();

	    if ( val < minNewPartSize() )
	    {
		y2warning( "YPartitionSplitter::changeWidget( `Value ): "
			   "Warning: New value %d below minNewPartSize ( %d )",
			   val, minNewPartSize() );
		setValue( minNewPartSize() );
	    }
	    else if ( val > maxNewPartSize() )
	    {
		y2warning( "YPartitionSplitter::changeWidget( `Value ): "
			   "Warning: New value %d above maxNewPartSize ( %d )",
			   val, maxNewPartSize() );
		setValue( maxNewPartSize() );
	    }
	    else
	    {
		setValue( val );
	    }

	    return YCPBoolean( true );
	}
	else
	{
	    y2error( "YPartitionSplitter::changeWidget( `Value ): "
		     "Error: Expecting integer value, not %s",
		     newValue->toString().c_str() );

	    return YCPBoolean( false );
	}
    }
    else return YWidget::changeWidget( property, newValue );
}



YCPValue YPartitionSplitter::queryWidget( const YCPSymbol & property )
{
    string s = property->symbol();
    if 		( s == YUIProperty_Value )	return YCPInteger( newPartSize() );
    else return YWidget::queryWidget( property );
}



void YPartitionSplitter::saveUserInput( YMacroRecorder *macroRecorder )
{
    macroRecorder->recordWidgetProperty( this, YUIProperty_Value );
}

