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

  File:	      YDownloadProgress.cc

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/


#include <sys/stat.h>
#include <ycp/YCPSymbol.h>
#include <ycp/YCPBoolean.h>
#define y2log_component "ui"
#include <ycp/y2log.h>

#include "YUISymbols.h"
#include "YDownloadProgress.h"


YDownloadProgress::YDownloadProgress( YWidgetOpt &	opt,
				      const YCPString &	label,
				      const YCPString &	filename,
				      int		expectedSize )
    : YWidget( opt )
    , _label( label )
    , _filename( filename )
    , _expectedSize( expectedSize )
{
    setDefaultStretchable( YD_HORIZ, true );
    setStretchable( YD_VERT, false );
}


void YDownloadProgress::setLabel( const YCPString & newLabel )
{
    _label = newLabel;
}


void YDownloadProgress::setFilename( const YCPString & newFilename )
{
    _filename = newFilename;
}


void YDownloadProgress::setExpectedSize( int newExpectedSize )
{
    _expectedSize = newExpectedSize;
}


YCPValue YDownloadProgress::changeWidget( const YCPSymbol & property,
					  const YCPValue  & newValue )
{
    string sym = property->symbol();

    /**
     * @property string Label the label above the progress indicator
     */
    if ( sym == YUIProperty_Label )
    {
	if ( newValue->isString() )
	{
	    setLabel( newValue->asString() );
	    return YCPBoolean( true );
	}
	else
	{
	    y2error( "YDownloadProgress::changeWidget( `Value ): "
		     "Error: Expecting string, not %s",
		     newValue->toString().c_str() );

	    return YCPBoolean( false );
	}
    }
    /**
     * @property string Filename file name with full path of the file to poll
     */
    else if ( sym == YUIProperty_Filename )
    {
	if ( newValue->isString() )
	{
	    setFilename( newValue->asString() );
	    return YCPBoolean( true );
	}
	else
	{
	    y2error( "YDownloadProgress::changeWidget( `Filename ): "
		     "Error: Expecting string, not %s",
		     newValue->toString().c_str() );

	    return YCPBoolean( false );
	}
    }
    /**
     * @property integer ExpectedSize expected final size of the file in bytes
     */
    else if ( sym == YUIProperty_ExpectedSize )
    {
	if ( newValue->isInteger() )
	{
	    int val = newValue->asInteger()->value();

	    if ( val < 1 )
	    {
		y2warning( "YDownloadProgress::changeWidget( `expectedSize ): "
			   "Warning: New value %d below minValue ( %d )",
			   val, 0 );
		setExpectedSize(0);
	    }
	    else
	    {
		setExpectedSize( val );
	    }

	    return YCPBoolean( true );
	}
	else
	{
	    y2error( "YDownloadProgress::changeWidget( `expectedSize ): "
		     "Error: Expecting integer value, not %s",
		     newValue->toString().c_str() );

	    return YCPBoolean( false );
	}
    }
    else return YWidget::changeWidget( property, newValue );
}



YCPValue YDownloadProgress::queryWidget( const YCPSymbol & property )
{
    string s = property->symbol();
    if 		( s == YUIProperty_ExpectedSize )	return YCPInteger( expectedSize() );
    else if	( s == YUIProperty_Label ) 	return label();
    else if	( s == YUIProperty_Filename ) 	return filename();
    else return YWidget::queryWidget( property );
}


long YDownloadProgress::currentFileSize()
{
    struct stat stat_info;

    if ( stat( filename()->value().c_str(), & stat_info ) == 0 )
	return ( long ) stat_info.st_size;
    else
	return 0L;
}

