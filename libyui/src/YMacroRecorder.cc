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

  File:		YMacroRecorder.cc

  Author:	Stefan Hundhammer <sh@suse.de>

/-*/


#include <stdio.h>
#include <sys/time.h>
#include <ycp/YCPSymbol.h>
#include <ycp/YCPString.h>
#include <ycp/YCPTerm.h>
#include <ycp/YCPVoid.h>

#define y2log_component "ui-macro"
#include <ycp/y2log.h>
#include <ycp/ExecutionEnvironment.h>

#include "YUISymbols.h"
#include "YWidget.h"
#include "YDialog.h"
#include "YMacroRecorder.h"
#include "YUIComponent.h"
#include "YCPValueWidgetID.h"
#include "YUI.h"

#ifndef Y2LOG_DATE
#   define Y2LOG_DATE	"%Y-%m-%d %H:%M:%S"	/* The date format */
#endif



#define YMACRO_INDENT "	   "	// 4 blanks


YMacroRecorder::YMacroRecorder( const string & macroFileName )
{
    _screenShotCount = 0;
    openMacroFile( macroFileName );
    writeMacroFileHeader();
}


YMacroRecorder::~YMacroRecorder()
{
    writeMacroFileFooter();
    closeMacroFile();
}


void YMacroRecorder::openMacroFile( const string & macroFileName )
{
    _macroFile = fopen( macroFileName.c_str(), "w" );

    if ( _macroFile )
    {
	y2milestone( "Recording macro to %s", macroFileName.c_str() );

    }
    else
    {
	y2error( "Can't record to macro file %s", macroFileName.c_str() );
    }
}


void YMacroRecorder::closeMacroFile()
{
    if ( _macroFile )
    {
	fclose( _macroFile );
	_macroFile = 0;
	y2milestone( "Macro recording done." );
    }
}


void YMacroRecorder::writeMacroFileHeader()
{
    if ( ! _macroFile )
	return;

    fprintf( _macroFile,
	     "// YaST2 UI macro file generated by UI macro recorder\n"
	     "//\n"
	     "//     Qt UI: Alt-Ctrl-Shift-M: start/stop Macro recorder\n"
	     "//	    Alt-Ctrl-Shift-P: Play macro\n"
	     "//\n"
	     "// Each block will be executed just before the next UserInput().\n"
	     "// 'return' before the closing brace ( '}' ) of each block relinquishes control\n"
	     "// back to the YCP source.\n"
	     "// Inside each block arbitrary YCP code can be added manually.\n"
	     "\n"
	     "{\n"
	     );
}


void YMacroRecorder::writeMacroFileFooter()
{
    if ( ! _macroFile )
	return;

    fprintf( _macroFile, "}\n" );
}


void YMacroRecorder::recordYcpCodeLocation()
{
    extern ExecutionEnvironment ee;	// YCP interpreter status
    ExecutionEnvironment::CallStack callStack( ee.callstack() );

    if ( ! callStack.empty() )
    {
	const CallFrame * frame = callStack.back();
	string functionName;

	if ( frame && frame->called_function.find( "Wizard::UserInput" ) == string::npos  )
	    functionName = frame->called_function;

	if ( frame )
	{
	    if ( functionName.empty() )
	    {
		fprintf( _macroFile, "%s%s// Source: %s:%d\n",
			 YMACRO_INDENT, YMACRO_INDENT,
			 frame->filename.c_str(),
			 frame->linenumber );
	    }
	    else
	    {
		fprintf( _macroFile, "%s%s// Source: %s(%s):%d\n",
			 YMACRO_INDENT, YMACRO_INDENT,
			 frame->filename.c_str(),
			 functionName.c_str(),
			 frame->linenumber );
	    }
	}
    }
}


void YMacroRecorder::recordTimeStamp()
{
    time_t now_seconds = time (NULL);
    struct tm *tm_now = localtime( &now_seconds );
    char timeStamp[80];		// that's big enough
    strftime( timeStamp, sizeof( timeStamp ), Y2LOG_DATE, tm_now );

    fprintf( _macroFile, "%s%s// %s\n",
	     YMACRO_INDENT, YMACRO_INDENT,
	     timeStamp );
}


void YMacroRecorder::recordDialogDebugLabel()
{
    fprintf( _macroFile, "%s%s// %s\n",
	     YMACRO_INDENT, YMACRO_INDENT,
	     YDialog::currentDialog()->debugLabel().c_str() );
}


void YMacroRecorder::recordComment( string text )
{
    fprintf( _macroFile, "%s%s// %s\n",
	     YMACRO_INDENT, YMACRO_INDENT,
	     text.c_str() );
}


void YMacroRecorder::beginBlock()
{
    if ( ! _macroFile )
	return;

    fprintf( _macroFile, "%s{\n", YMACRO_INDENT );
    recordDialogDebugLabel();
    fprintf( _macroFile, "%s%s//\n", YMACRO_INDENT, YMACRO_INDENT );
    recordYcpCodeLocation();
    recordTimeStamp();
    fprintf( _macroFile, "\n" );
}


void YMacroRecorder::endBlock()
{
    if ( ! _macroFile )
	return;

    fprintf( _macroFile, "\n" );
    fprintf( _macroFile, "%s%sreturn;\n", YMACRO_INDENT, YMACRO_INDENT );
    fprintf( _macroFile, "%s}\n\n", YMACRO_INDENT );
}


void YMacroRecorder::recordUserInput( const YCPValue & input )
{
    if ( ! _macroFile )
	return;

    fprintf( _macroFile, "\n" );

    recordMakeScreenShot();

    if ( input->isVoid() )
    {
	fprintf( _macroFile, "%s%sUI::%s();\n",
		 YMACRO_INDENT, YMACRO_INDENT,
		 YUIBuiltin_FakeUserInput );
    }
    else
    {
	fprintf( _macroFile, "%s%sUI::%s( %s );\n",
		 YMACRO_INDENT, YMACRO_INDENT,
		 YUIBuiltin_FakeUserInput,
		 input->toString().c_str() );
    }

    fflush( _macroFile );	// sync to disk at this point - for debugging

    y2debug( "Input: %s", input->isVoid() ? "(nil)" : input->toString().c_str() );
}


void YMacroRecorder::recordMakeScreenShot( bool enabled, const char * filename )
{
    if ( ! _macroFile )
	return;

    // Automatically add a (commented out) UI::MakeScreenShot() statement.
    //
    // The screenshot goes to /tmp/yast2-*.png , but since that statement is
    // commented out anyway this is not a security hazard - the user has to
    // remove the comment characters to actually trigger any action upon macro
    // replay, and while he is at it, he can also chose some other directory.
    //
    // All this is mainly for the SuSE documentation department anyway who will
    // use that in a testing environment, not on some ultra-sensitive
    // server. We need a good default directory here, not some academic
    // bullshit discussion about creating temp directories with awkward names
    // and unusable permissions on the fly. Been there, done that, works only
    // for security theoreticians, not in real life. Those who devise such
    // schemes never seem to use them in reality.
    //
    // End of discussion before it even starts. ;-)

    char buffer[256];

    if ( ! filename )
    {
	sprintf( buffer, "/tmp/yast2-%04d", _screenShotCount++ );
	filename = buffer;
    }

    fprintf( _macroFile, "%s%s%sUI::%s( \"%s\" );\n",
	     YMACRO_INDENT, YMACRO_INDENT,
	     enabled ? "" : "// ",
	     YUIBuiltin_MakeScreenShot, filename );
}


void YMacroRecorder::recordWidgetProperty( YWidget *	widget,
					   const char *	propertyName )
{
    if ( ! _macroFile )
	return;

    if ( ! widget )
    {
	y2error( "Null widget" );
	return;
    }

    if ( ! widget->isValid() )
    {
	y2error( "Invalid widget" );
	return;
    }

    if ( ! propertyName )
    {
	y2error ( "Null property name" );
	return;
    }

    if ( ! widget->hasId() )
    {
	// It's pointless to save properties if the widget doesn't have an ID -
	// there is no way to restore the property without an ID.

	return;
    }

    YCPValueWidgetID * widgetId = dynamic_cast<YCPValueWidgetID *> ( widget->id() );

    if ( ! widgetId )
	return;
    
    YCPTerm idTerm( YUISymbol_id );		// `id()
    idTerm->add( widgetId->value() );	// `id( `something )

    YCPValue val = widget->queryWidget( YCPSymbol( propertyName ) );

    fprintf( _macroFile, "%s%sUI::%s( %s,\t`%s,\t%s );\t// %s \"%s\"\n",
	     // UI::ChangeWidget( `id( `something ), `Value, 42 ) // YWidget
	     YMACRO_INDENT, YMACRO_INDENT,
	     YUIBuiltin_ChangeWidget,
	     idTerm->toString().c_str(),
	     propertyName,
	     val->toString().c_str(),
	     widget->widgetClass(),
	     widget->debugLabel().c_str() );

    y2debug( "Recording %s status: %s: %s",
	     widget->widgetClass(), propertyName, val->toString().c_str() );
}

