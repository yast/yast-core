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

   File:       exitcodes.h

   Author:     Stanislav Visnovsky <visnov@suse.cz>
   Maintainer: Stanislav Visnovsky <visnov@suse.cz>

/-*/
// -*- c++ -*-

#ifndef exitcodes_h
#define exitcodes_h

enum error_codes {
    YAST_OK		= 0,	// process finished without errors
    YAST_FEWARGUMENTS	= 1,	// too few arguments for the commandline
    YAST_OPTIONERROR	= 5,	// error in provided arguments
    YAST_CLIENTRESULT	= 16	// client (YCP) returned special result, this is used as offset (or as generic error)
};

#endif // exitcodes_h
