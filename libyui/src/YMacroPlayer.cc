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
#include <ycp/YCPParser.h>
#include <ycp/YCPBlock.h>

#define y2log_component "ui-macro"
#include <ycp/y2log.h>

#include "YUISymbols.h"
#include "YWidget.h"
#include "YMacroPlayer.h"



YMacroPlayer::YMacroPlayer( const string & macroFileName )
    : _macro( YCPNull() )
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

    YCPParser parser( macroFile, macroFileName.c_str() );
    _macro = parser.parse();

    if ( _macro.isNull() )
    {
	setError();
	y2error( "Error parsing macro file %s - macro execution aborted",
		 macroFileName.c_str() );
	return;
    }

    if ( ! _macro->isBlock() )
    {
	setError();
	y2error( "Macro syntax error in file %s - expected YCP block",
		 macroFileName.c_str() );
	return;
    }

    y2debug( "Playing macro from file %s - %d macro blocks",
	     macroFileName.c_str(), _macro->asBlock()->size() );
    _nextBlockNo = 0;

    fclose( macroFile );
}


bool YMacroPlayer::finished()
{
    if ( error() || _macro.isNull() || _nextBlockNo < 0 )
    {
	y2warning( "Test for error() first before testing finished() !" );
	return true;
    }
    else
    {
	y2debug( "_nextBlockNo: %d, size: %d, finished(): %s",
		 _nextBlockNo ,_macro->asBlock()->size(),
		 _nextBlockNo >= _macro->asBlock()->size() ? "true" : "false" );

	return _nextBlockNo >= _macro->asBlock()->size();
    }
}


YCPBlock YMacroPlayer::nextBlock()
{
    if ( error() || finished() )
    {
	return YCPNull();
    }

    YCPStatement macroBlock = _macro->asBlock()->statement( _nextBlockNo++ );

    if ( ! macroBlock->isNestedStatement() )
    {
	setError();
	y2error( "Invalid macro syntax - block expected, not\n%s",
		 macroBlock->toString().c_str() );

	return YCPNull();
    }
    else
    {
	y2debug( "Next macro block to execute:\n%s",
		 macroBlock->toString().c_str() );
    }

    return macroBlock->asNestedStatement()->value();
}


void YMacroPlayer::rewind()
{
    _nextBlockNo = 0;
}

