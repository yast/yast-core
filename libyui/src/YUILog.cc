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

  File:		YUILog.cc

  Author:     	Stefan Hundhammer <sh@suse.de>

/-*/


#include <string.h>

#define YUILogComponent "ui"
#include "YUILog.h"

#include "YUIException.h"

using std::ostream;
using std::cerr;
using std::endl;
using std::string;

static void
stderrLogger( YUILogLevel_t	logLevel,
	      const char *	logComponent,
	      const char *	sourceFileName,
	      int 	 	sourceLineNo,
	      const char * 	sourceFunctionName,
	      const char *	message );



/**
 * Stream buffer class that will use the YUILog's logger function.
 *
 * See also http://blogs.awesomeplay.com/elanthis/archives/2007/12/10/
 **/
class YUILogBuffer: public std::streambuf
{
    friend class YUILog;

public:

    /**
     * Constructor.
     **/
    YUILogBuffer()
	: loggerFunction( stderrLogger )
	, enableDebugLogging( true )
	, logComponent( 0 )
	, sourceFileName( 0 )
	, lineNo( 0 )
	, functionName( 0 )
	{}

    /**
     * Destructor.
     **/
    virtual ~YUILogBuffer()
	{ flush(); }


    /**
     * Write (no more than maxLength characters of) a sequence of characters
     * and return the number of characters written.
     *
     * Reimplemented from std::streambuf.
     * This is called for all output operations on the associated ostream.
     **/
    virtual std::streamsize xsputn( const char * sequence, std::streamsize maxLength );

    /**
     * Write one character in case of buffer overflow.
     *
     * Reimplemented from std::streambuf.
     **/
    virtual int overflow( int ch = EOF );

    /**
     * Write (no more than maxLength characters of) a sequence of characters
     * and return the number of characters written.
     *
     * This is the actual worker function that uses the loggerFunction to
     * actually write characters.
     **/
    std::streamsize writeBuffer( const char * sequence, std::streamsize seqLen );

    /**
     * Flush the output buffer: Write any data unwritten so far.
     **/
    void flush();


private:

    YUILoggerFunction	loggerFunction;
    bool		enableDebugLogging;

    YUILogLevel_t	logLevel;
    const char *	logComponent;
    const char *	sourceFileName;
    int			lineNo;
    const char *	functionName;

    string		buffer;
};



std::streamsize
YUILogBuffer::writeBuffer( const char * sequence, std::streamsize seqLen )
{
    if ( logLevel != YUI_LOG_DEBUG || enableDebugLogging )
    {
	// Add new character sequence

	if ( seqLen > 0 )
	    buffer += string( sequence, seqLen );

	//
	// Output buffer contents line by line
	//

	std::size_t start       = 0;
	std::size_t newline_pos = 0;

	while ( start < buffer.length() &&
		( newline_pos = buffer.find_first_of( '\n', start ) ) != string::npos )
	{
	    string line = buffer.substr( start, newline_pos - start );

	    loggerFunction( logLevel, logComponent,
			    sourceFileName, lineNo, functionName,
			    line.c_str() );

	    start = newline_pos + 1;
	}

	if ( start < buffer.length() )
	    buffer = buffer.substr( start, string::npos );
	else
	    buffer.clear();
    }

    return seqLen;
}


std::streamsize
YUILogBuffer::xsputn( const char * sequence, std::streamsize maxLength )
{
    return writeBuffer( sequence, maxLength );
}


int
YUILogBuffer::overflow( int ch )
{
    if ( ch != EOF )
    {
	char sequence = ch;
	writeBuffer( &sequence, 1 );
    }

    return 0;
}


void YUILogBuffer::flush()
{
    writeBuffer( "\n", 1 );
}





struct YUILogPrivate
{
    YUILogPrivate()
	: logBuffer()
	, logStream( &logBuffer )
	{}

    YUILogBuffer	logBuffer;
    ostream 		logStream;
};




YUILog::YUILog()
    : priv( new YUILogPrivate() )
{
    YUI_CHECK_NEW( priv );
}


YUILog::~YUILog()
{
    priv->logBuffer.flush();
}


YUILog *
YUILog::instance()
{
    static YUILog * instance = 0;

    if ( ! instance )
    {
	instance = new YUILog();
	YUI_CHECK_NEW( instance );
    }

    return instance;
}


void
YUILog::enableDebugLogging( bool debugLogging )
{
    instance()->priv->logBuffer.enableDebugLogging = debugLogging;
}


bool
YUILog::debugLoggingEnabled()
{
    return instance()->priv->logBuffer.enableDebugLogging;
}


void
YUILog::setLoggerFunction( YUILoggerFunction loggerFunction )
{
    if ( ! loggerFunction )
	loggerFunction = stderrLogger;

    instance()->priv->logBuffer.loggerFunction = loggerFunction;
}


YUILoggerFunction
YUILog::loggerFunction()
{
    YUILoggerFunction logger = instance()->priv->logBuffer.loggerFunction;

    if ( logger == stderrLogger )
	logger = 0;

    return logger;
}


ostream &
YUILog::log( YUILogLevel_t	logLevel,
	     const char *	logComponent,
	     const char *	sourceFileName,
	     int 		lineNo,
	     const char * 	functionName )
{
    if ( ! priv->logBuffer.buffer.empty() )	// Leftovers from previous logging?
    {
	if ( priv->logBuffer.logLevel != logLevel ||
	     priv->logBuffer.lineNo   != lineNo   ||
	     strcmp( priv->logBuffer.logComponent,   logComponent   ) != 0 ||
	     strcmp( priv->logBuffer.sourceFileName, sourceFileName ) != 0 ||
	     strcmp( priv->logBuffer.functionName,   functionName   ) != 0   )
	{
	    priv->logBuffer.flush();
	}
    }

    priv->logBuffer.logLevel		= logLevel;
    priv->logBuffer.logComponent	= logComponent;
    priv->logBuffer.sourceFileName	= sourceFileName;
    priv->logBuffer.lineNo		= lineNo;
    priv->logBuffer.functionName	= functionName;

    return priv->logStream;
}


ostream &
YUILog::debug( const char * logComponent, const char * sourceFileName, int lineNo, const char * functionName )
{
    return instance()->log( YUI_LOG_DEBUG, logComponent, sourceFileName, lineNo, functionName );
}


ostream &
YUILog::milestone( const char * logComponent, const char * sourceFileName, int lineNo, const char * functionName )
{
    return instance()->log( YUI_LOG_MILESTONE, logComponent, sourceFileName, lineNo, functionName );
}


ostream &
YUILog::warning( const char * logComponent, const char * sourceFileName, int lineNo, const char * functionName )
{
    return instance()->log( YUI_LOG_WARNING, logComponent, sourceFileName, lineNo, functionName );
}


ostream &
YUILog::error( const char * logComponent, const char * sourceFileName, int lineNo, const char * functionName )
{
    return instance()->log( YUI_LOG_ERROR, logComponent, sourceFileName, lineNo, functionName );
}


static void
stderrLogger( YUILogLevel_t	logLevel,
	      const char *	logComponent,
	      const char *	sourceFileName,
	      int 	 	sourceLineNo,
	      const char * 	sourceFunctionName,
	      const char *	message )
{
    const char * logLevelStr = "";

    switch ( logLevel )
    {
	case YUI_LOG_DEBUG:	logLevelStr = "dbg";	break;
	case YUI_LOG_MILESTONE:	logLevelStr = "_M_";	break;
	case YUI_LOG_WARNING:	logLevelStr = "WRN";	break;
	case YUI_LOG_ERROR:	logLevelStr = "ERR";	break;
    }

    if ( ! logComponent )
	logComponent = "??";

    if ( ! sourceFileName )
	sourceFileName = "??";

    if ( ! sourceFunctionName )
	sourceFunctionName = "??";

    if ( ! message )
	message = "";

    cerr << "<" << logLevelStr  << "> "
	 << "[" << logComponent << "] "
	 << sourceFileName	<< ":" << sourceLineNo << " "
	 << sourceFunctionName	<< "(): "
	 << message
	 << endl;
}
