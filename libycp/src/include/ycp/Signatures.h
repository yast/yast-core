/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|					Copyright 2003, SuSE Linux AG  |
\----------------------------------------------------------------------/

   File:	Signatures.h

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/
// -*- c++ -*-

#ifndef Signatures_h
#define Signatures_h


//
// Signature elements
//

// basic signatures

#define anyT		"a"
#define booleanT	"b"
#define byteblockT	"o"
#define floatT		"f"
#define integerT	"i"
#define localeT		"l"
#define pathT		"p"
#define stringT		"s"
#define symbolT		"y"
#define termT		"t"
#define voidT		"v"
#define wildT		"w"

// constructed signatures
#define blockC		"B"
#define listC		"L"
#define mapC		"M"
#define variableC	"Y"
#define tupleC		"T"

// function
#define functionC	"|"

// modifiers
#define constM		"C"
#define referenceM	"R"
#define staticM		"S"

typedef const char * signature_t;

#endif // Signatures_h
