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

  File:		YMacroRecorder.h

  Author:	Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YMacroRecorder_h
#define YMacroRecorder_h

#include <string>

class YWidget;

/**
 * Abstract base class for macro recorders.
 **/
class YMacroRecorder
{
protected:    

    /**
     * Constructor
     **/
    YMacroRecorder( const string & macroFileName );

public:

    /**
     * Destructor
     **/
    virtual ~YMacroRecorder();

    /**
     * Record one widget property.
     **/
    virtual void recordWidgetProperty( YWidget *	widget,
				       const char *	propertyName ) = 0;

    /**
     * Record a "UI::MakeScreenShot()" statement.
     *
     * If 'enabled' is 'false', this statement will be commented out.
     * If no file name is given, a default file name (with auto-increment) will
     * be used. 
     **/
    virtual void recordMakeScreenShot( bool enabled = false,
				       const string & filename = string() ) = 0;
};

#endif // YMacroRecorder_h
