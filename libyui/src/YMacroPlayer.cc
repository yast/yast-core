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

  File:	      YMacroPlayer.cc

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/


#include <stdio.h>
#include <ycp/YCPString.h>
#include <ycp/YCPVoid.h>
#include <ycp/Parser.h>
#include <ycp/YBlock.h>
#include <ycp/YCPCode.h>

#define y2log_component "ui-macro"
#include <ycp/y2log.h>

#include "YUISymbols.h"
#include "YWidget.h"
#include "YMacroPlayer.h"



YMacroPlayer::YMacroPlayer( const string & macroFileName )
    : _macro( 0 )
{
    _nextBlockNo = -1;
    readMacroFile( macroFileName );
}


YMacroPlayer::~YMacroPlayer()
{
    y2debug( "Deleting macro player." );
}



void YMacroPlayer::readMacroFile( const string & macroFileName )
{
    clearError();
    FILE * macroFile = fopen( macroFileName.c_str(), "r" );

    if ( ! macroFile )
    {
	setError();
	y2error( "Can't open macro file %s", macroFileName.c_str() );
	return ;
    }

    y2milestone( "Loading macro file %s", macroFileName.c_str() );

    Parser parser( macroFile, macroFileName.c_str() );
    YCode *parsed = parser.parse();

    if ( !parsed || parsed->isError() )
    {
	setError();
	y2error( "Error parsing macro file %s - macro execution aborted",
		 macroFileName.c_str() );
	return;
    }

    if ( !parsed->isBlock() )
    {
	setError();
	y2error( "Macro syntax error in file %s - expected YCP block",
		 macroFileName.c_str() );
	return;
    }
    
    _macro = static_cast <YBlock *> (parsed) ;

    y2debug( "Playing macro from file %s - %d macro blocks",
	     macroFileName.c_str(), _macro->statementCount() );
    _nextBlockNo = 0;

    fclose( macroFile );
}


bool YMacroPlayer::finished()
{
    if ( error() || !_macro || _nextBlockNo < 0 )
    {
	y2warning( "Test for error() first before testing finished() !" );
	return true;
    }
    y2debug( "_nextBlockNo: %d, size: %d, finished(): %s",
	      _nextBlockNo ,_macro->statementCount(),
	      _nextBlockNo >= _macro->statementCount() ? "true" : "false" );

    return _nextBlockNo >= _macro->statementCount();
}


YCPValue YMacroPlayer::evaluateNextBlock()
{
    if ( error() || finished() )
    {
	return YCPNull();
    }

    y2milestone( "Evaluating macro block #%d", _nextBlockNo );

    return _macro->evaluate( _nextBlockNo++ );
}


void YMacroPlayer::rewind()
{
    _nextBlockNo = 0;
}

