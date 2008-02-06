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

  File:		YMacroPlayer.h

  Author:	Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YMacroPlayer_h
#define YMacroPlayer_h

#include <string>

/**
 * Abstract base class for macro player.
 **/
class YMacroPlayer
{
protected:
    /**
     * Constructor
     **/
    YMacroPlayer( const string & macroFileName );

public:
    /**
     * Destructor
     **/
    virtual ~YMacroPlayer();

};

#endif // YMacroPlayer_h
