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

#include <stdio.h>
#include <string>
#include <ycp/YCPSymbol.h>
#include <ycp/YCPTerm.h>

class YWidget;

class YMacroRecorder
{
public:

    /**
     * Constructor
     **/
    YMacroRecorder( const string & macroFileName );


    /**
     * Destructor
     **/
    virtual ~YMacroRecorder();


    /**
     * Record one user input.
     **/
    void recordUserInput( const YCPValue & input );


    /**
     * Record one widget property.
     **/
    void recordWidgetProperty( YWidget *	widget,
			       const char *	propertyName );

    /**
     * Record a "UI::MakeScreenShot()" statement.
     *
     * If 'enabled' is 'false', this statement will be commented out.
     * If no file name is given, a default file name (with auto-increment) will
     * be used. 
     **/
    void recordMakeScreenShot( bool enabled = false, const char * filename=0 );

    /**
     * Record a text as a comment.
     **/
    void recordComment( string comment_text );
    
    /**
     * Begin a macro block that will be executed as a whole.
     **/
    void beginBlock();

    /**
     * End a macro block that will be executed as a whole.
     **/
    void endBlock();

protected:

    /**
     * Open the internally used macro file for writing.
     *
     * On error a message is issued to the log file.
     * All write functions check the internally used macro file prior to using
     * it, so it is perfectly safe to use them even if the initial open failed.
     **/
    void openMacroFile( const string & macroFileName );

    /**
     * Close the internally used macro file.
     **/
    void closeMacroFile();

    /**
     * Write a standardized macro file header.
     **/
    void writeMacroFileHeader();

    /**
     * Write a standardized macro file footer.
     **/
    void writeMacroFileFooter();

    /**
     * Record file name and line number of current YCP code
     **/
    void recordYcpCodeLocation();

    /**
     * Record a time stamp to match with y2logs
     **/
    void recordTimeStamp();

    /**
     * The internally used macro file; valid if non-null.
     **/
    FILE * _macroFile;

    /**
     * A counter for making screen shots.
     **/
    int _screenShotCount;
};

#endif // YMacroRecorder_h
