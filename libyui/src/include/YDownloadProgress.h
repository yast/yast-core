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

  File:	      YDownloadProgress.h

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YDownloadProgress_h
#define YDownloadProgress_h

#include "YWidget.h"
#include <ycp/YCPString.h>
#include <ycp/YCPInteger.h>

/**
 * @short Implementation of the Slider widget
 */
class YDownloadProgress : public YWidget
{
public:

    /**
     * Constructor
     */
    YDownloadProgress( YWidgetOpt &		opt,
		       const YCPString &	label,
		       const YCPString &	filename,
		       int			expectedSize );

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass() { return "YDownloadProgress"; }

    /**
     * Implements the ChangeWidget() UI command.
     */
    YCPValue changeWidget( const YCPSymbol & property,
			   const YCPValue &  newValue );

    /**
     * Implements the QueryWidget() UI command.
     */
    YCPValue queryWidget( const YCPSymbol & property );

    /**
     * Change the label above the progress indicator.
     *
     * Overwrite this, but call YDownloadProgress::setLabel
     * at the end of your own method.
     */
    virtual void setLabel( const YCPString & label );

    /**
     * Change the filename.
     *
     * Overwrite this, but call YDownloadProgress::setFilename()
     * at the end of your own method.
     */
    virtual void setFilename( const YCPString &newFilename );

    /**
     * Change the expected size of the file being downloaded.
     *
     * Overwrite this, but call YDownloadProgress::setExpectedSize()
     * at the end of your own method.
     */
    virtual void setExpectedSize( int newExpectedSize );


    YCPString	label()		{ return _label;	}
    YCPString	filename()	{ return _filename;	}
    int		expectedSize()	{ return _expectedSize; }


    /**
     * Returns the current size of the file that is being downloaded.
     * Returns 0 if this file doesn't exist (yet).
     */
    long currentFileSize();


private:

    YCPString	_label;
    YCPString	_filename;
    int		_expectedSize;
};


#endif // YDownloadProgress_h
