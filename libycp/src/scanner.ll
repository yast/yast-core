/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                        (C) SuSE GmbH |
\----------------------------------------------------------------------/

   File:	scanner.ll
   Version	2.0
   Author:	Klaus Kämpf <kkaempf@suse.de>
   Maintainer:	Klaus Kämpf <kkaempf@suse.de>

/-*/

%{

#include "YCP.h"
#include "y2log.h"
#include "parser.h"
#include <stdlib.h>

int yylineno = 1;
//#define DEBUG_SCANNER
#ifndef DEBUG_SCANNER
#define debug_scanner nix
#else
#define debug_scanner printf
#endif

static void nix(...) { }

static inline unsigned
fromhex(char hex)
{
    if      (isdigit(hex)) return (unsigned)(hex - '0');
    else if (islower(hex)) return (unsigned)(hex - 'a' + 10);
    else                   return (unsigned)(hex - 'A' + 10);
}

#include "YCPScanner.h"

static char *scanner_token;

#define RESULT(val,type) setScannedValue(val, yylineno); scanner_token = yytext; return type

%}

%option noyywrap

%option c++
%option yyclass="YCPScanner"

%x bybl str comment

LETTER	[a-z][A-Z]
ODIGIT	[0-7]
DIGIT	[0-9]
HDIGIT	[0-9][a-f][A-F]
PATHSEGMENT [[:alnum:]_-]+|\"([^\\"]*(\\.)*)+\"
SYMBOL [[:alpha:]_][[:alnum:]_]+|[[:alpha:]][[:alnum:]_]*

%%

^[\t ]*#.*$			{ /* Ignore Unix style Comments */ }
\/\*				{ BEGIN (comment); }
<comment>\n			{ yylineno++; }
<comment>\*\/			{ BEGIN (INITIAL); }
<comment>.			{ /* skip  */ }

\/\/.*$				{ /* Ignore C++ style comments */ }
\n				{ /* Ignore newlines  */
				  yylineno++; }
[\f\t\r\v ]+			{ /* Ignore Whitespaces according to isspace(3) */ }

  /* -- constants  */

	/* floating point constant  */

[0-9]+\.[0-9]+([eE][+-]?[0-9]+)?|[0-9]+[eE][+-]?[0-9]+ {
	  debug_scanner("<YCPFloat>");
	  RESULT (YCPFloat(yytext), YCP_FLOAT);
	}

	/* integer constant  */

[1-9][0-9]* {
	  debug_scanner("<YCPInteger int>");
	  RESULT (YCPInteger(yytext), YCP_INTEGER);
	}

	/* octal constant  */

0[0-7]* {
	  debug_scanner("<YCPInteger oct>");
	  long long i;
	  sscanf (yytext, "%Lo", &i);
	  RESULT (YCPInteger(i), YCP_INTEGER);
	}

	/* hexadecimal constant  */

0x[0-9A-Fa-f]+ {
	  debug_scanner("<YCPInteger hex>");
	  long long i;
	  sscanf (yytext, "0x%Lx", &i);
	  RESULT (YCPInteger(i), YCP_INTEGER);
	}

	/* -- double colon  */

::	{ return DCOLON; }


	/* -- byteblock  */

	/* byteblock prefix  */

\#\[    {
	  if (scandata_buffer == 0) scandata_buffer = extend_scanbuffer (1);
	  if (scandata_buffer == 0) return 0;
	  scandata_buffer_ptr = scandata_buffer;
	  BEGIN (bybl);
        }

	/* byteblock suffix  */

<bybl>\] {
	  debug_scanner ("<YCPByteblock>");
	  BEGIN (INITIAL);
	  const unsigned char *bytes = (const unsigned char *)scandata_buffer;
	  long length = (long)(scandata_buffer_ptr - scandata_buffer);
	  RESULT (YCPByteblock(bytes, length), YCP_BYTEBLOCK);
        }

	/* byteblock data  */

<bybl>([0123456789ABCDEFabcdef][0123456789ABCDEFabcdef])+ {
	  int ylen   = strlen (yytext);
	  int offset = scandata_buffer_ptr - scandata_buffer;

	  if (offset + ylen/2 >= scandata_buffer_size) {
	    if (extend_scanbuffer (ylen) == 0) return 0;
	    scandata_buffer_ptr = scandata_buffer + offset;
	  }
	  for (int i=0; i<ylen/2; i++) {
	    *scandata_buffer_ptr++ = (fromhex(yytext[i*2]) << 4) | (fromhex(yytext[i*2+1]));
	  }
        };

<bybl>[\t\r ]+ {
	  /* Ignore Whitespaces */
	}

<bybl>\n {
	  /* Ignore Newline */
	  yylineno++;
	}

<bybl>. {
          logError("bad character in byteblock constant", yylineno);
          return 0;
        }

        /* -- string  */

        /* string prefix  */

\"      {
          if (scandata_buffer == 0)
            scandata_buffer = extend_scanbuffer (1);
          if (scandata_buffer == 0)
            return 0;

          scandata_buffer_ptr = scandata_buffer;
          BEGIN (str);
        }

        /* string suffix  */

<str>\" {
          debug_scanner ("<YCPString>");
          BEGIN (INITIAL);
          *scandata_buffer_ptr = '\0';
          RESULT (YCPString(scandata_buffer), YCP_STRING);
        }

        /* string octal character  */

<str>\\[0-7]{1,3} {
	  debug_scanner("<YCPOctal escape in string>");
          /* octal escape sequence */
          /* mis-use result as scandata_buffer offset  */
          int result = scandata_buffer_ptr - scandata_buffer;
          if (result >= scandata_buffer_size) {
              if (extend_scanbuffer (1) == 0)
		  return 0;
	      scandata_buffer_ptr = scandata_buffer + result;
	  }
          (void) sscanf( yytext + 1, "%o", &result );
	  if (( result > 0xff ) || (result == 0))
	      logError("bad octal constant", yylineno);
	  else
	      *scandata_buffer_ptr++ = result;
	  }

	/* string escaped character  */

<str>\\(.|\n) {
	  debug_scanner("<escape in string>");
	  int offset = scandata_buffer_ptr - scandata_buffer;
	  if (offset >= scandata_buffer_size) {
	    if (extend_scanbuffer (1) == 0)
	      return 0;
	    scandata_buffer_ptr = scandata_buffer + offset;
	  }
	  switch (yytext[1]) {
	    case 'n': *scandata_buffer_ptr++ = '\n'; break;
	    case 't': *scandata_buffer_ptr++ = '\t'; break;
	    case 'r': *scandata_buffer_ptr++ = '\r'; break;
	    case 'b': *scandata_buffer_ptr++ = '\b'; break;
	    case 'f': *scandata_buffer_ptr++ = '\f'; break;
	    case '\n': yylineno++; break;
	    default:  *scandata_buffer_ptr++ = yytext[1]; break;
	  }
	}

        /* string escaped " char  */

<str>[^\\\"\n]+ {
	  char *yptr = yytext;
	  int ylen = strlen (yptr);
	  int offset = scandata_buffer_ptr - scandata_buffer;
	  if (offset + ylen >= scandata_buffer_size) {
	    if (extend_scanbuffer (ylen) == 0)
	      return 0;
	    scandata_buffer_ptr = scandata_buffer + offset;
	  }
	  strcpy (scandata_buffer_ptr, yptr);
	  scandata_buffer_ptr += ylen;
	}

        /* \n in string */

<str>\n {
	  int offset = scandata_buffer_ptr - scandata_buffer;
	  if (offset >= scandata_buffer_size) {
	    if (extend_scanbuffer (1) == 0)
	      return 0;
	    scandata_buffer_ptr = scandata_buffer + offset;
	  }
          *scandata_buffer_ptr++ = '\n';
          yylineno++;
        }

nil|nilboolean|nilinteger|nilfloat|nilstring|nillocale|nilbyteblock|nilpath|nillist|nilmap|nilsymbol|nilterm	{
	 	debug_scanner("<YCPNil>");
		RESULT (YCPVoid(), YCP_VOID);			};
true|false  {	debug_scanner("<YCPBoolean>");
		RESULT (YCPBoolean(yytext), YCP_BOOLEAN);		};


 /* -- statement keywords  */

return		{ return RETURN;	};
define		{ return DEFINE;	};
undefine	{ return UNDEFINE;	};
continue	{ return CONTINUE;	};
break		{ return BREAK;		};
if		{ return IF;		};
is		{ return IS;		};
else		{ return ELSE;		};
do		{ return DO;		};
while		{ return WHILE;		};
repeat		{ return REPEAT;	};
until		{ return UNTIL;		};
empty		{ return EMPTY;		};
list		{ return LIST;		};
include		{ return INCLUDE;	};
import		{ return IMPORT;	};
export		{ return EXPORT;	};
global		{ return GLOBAL;	};
_dump_scope	{ return DUMPSCOPE;	};
_dump_meminfo	{ return MEMINFO;	};
_fullname	{ return FULLNAME;	};
_callback	{ return CALLBACK;	};
size		{ return SIZE;		};
lookup		{ return LOOKUP;	};
select		{ return SELECT;	};
remove		{ return REMOVE;	};
foreach		{ return FOREACH;	};
eval		{ return EVAL;		};
symbolof	{ return SYMBOLOF;	};
module		{ return MODULE;	};
textdomain	{ return TEXTDOMAIN;	};
const		{ return CONST;		};
isnil		{ return ISNIL;		};
union		{ return UNION;		};
merge		{ return MERGE;		};
add		{ return ADD;		};
change		{ return CHANGE;	};
sort		{ return SORT;		};

  /* -- basetype  */

any         { RESULT (YCPDeclAny(), ANY);			};
void        { RESULT (YCPDeclType(YT_VOID), YCP_DECLTYPE);	};
boolean     { RESULT (YCPDeclType(YT_BOOLEAN), YCP_DECLTYPE);	};
integer     { RESULT (YCPDeclType(YT_INTEGER), YCP_DECLTYPE);	};
float       { RESULT (YCPDeclType(YT_FLOAT), YCP_DECLTYPE);	};
string      { RESULT (YCPDeclType(YT_STRING), YCP_DECLTYPE);	};
byteblock   { RESULT (YCPDeclType(YT_BYTEBLOCK), YCP_DECLTYPE);	};
 /* list is a keyword and done by parser.yy  */
map         { RESULT (YCPDeclType(YT_MAP), YCP_DECLTYPE);	};
locale      { RESULT (YCPDeclType(YT_LOCALE), YCP_DECLTYPE);	};
term        { RESULT (YCPDeclType(YT_TERM), YCP_DECLTYPE);	};
path        { RESULT (YCPDeclType(YT_PATH), YCP_DECLTYPE);	};
block       { RESULT (YCPDeclType(YT_BLOCK), YCP_DECLTYPE);	};
declaration { RESULT (YCPDeclType(YT_DECLARATION), YCP_DECLTYPE); };
symbol      { RESULT (YCPDeclType(YT_SYMBOL), YCP_DECLTYPE);	};

 /* common mistyped names  */
int	    { logError ("Seen 'int', use 'integer' instead", yylineno); return 0; };
char	    { logError ("Seen 'char', use 'string' instead", yylineno); return 0; };
bool	    { logError ("Seen 'bool', use 'boolean' instead", yylineno); return 0; };

UI	{ return UI; }
WFM	{ return WFM; }
Pkg	{ return Pkg; }
SCR	{ return SCR; }

 /* -- path  */

\.|(\.{PATHSEGMENT})+ {
	    debug_scanner("<YCPPath>");
	    if (yyleng == 1) {
		RESULT (YCPPath(yytext), YCP_PATH);
	    }
	    else {
		if (yytext[yyleng-1] == '-') {
		    logError ("dash behind path constant not allowed", yylineno);
		    return 0;
		}
		YCPPath p (yytext);
		if (p->length() == 0) {
		    logError ("not a path constant", yylineno);
		    return 0;
		}
		RESULT (p, YCP_PATH);
	    }
	}

 /* -- quoted symbol  */

\`([[:alpha:]_][[:alnum:]_]+|[[:alpha:]][[:alnum:]_]*) {
		debug_scanner("<YCPSymbol quoted>");
		RESULT (YCPSymbol(yytext+1, true), SYMBOL);
	}

 /* -- double quoted symbol  */

\`\`([[:alpha:]_][[:alnum:]_]+|[[:alpha:]][[:alnum:]_]*) {
		debug_scanner("<YCPSymbol dquoted>");
		logError ("*** Don't use ``symbol... but ``(symbol ... ***", yylineno);
		RESULT (YCPSymbol(yytext+2, false), QUOTED_SYMBOL);
	}

  /* -- symbol  */

{SYMBOL} {
		debug_scanner("<YCPSymbol>");
		RESULT (YCPSymbol(yytext, false), SYMBOL);
	}

  /* -- locale  */
_\(	{ return I18N;		}

  /* -- map expression  */
\$\[	{ return MAPEXPR;	}

  /* -- comparison operators  */
==	{ return EQUALS;	}
\<	{ return ST;		}
\>	{ return GT;		}
\<=	{ return SE;		}
\>=	{ return GE;		}
\!=	{ return NEQ;		}

  /* -- boolean operators  */
&&	{ return AND;		}
\|\|	{ return OR; 		}
\!	{ return NOT;		}
\<\<	{ return LEFT;		}
\>\>	{ return RIGHT;		}

  /* -- quotes  */

\`\`\{	{ return QUOTED_BLOCK;		}
\`\`\(	{ return QUOTED_EXPRESSION;	}
\`\`	{ logError ("Lonely doubleqoute", yylineno); return 0; }
\`	{ logError ("Lonely qoute", yylineno); return 0; }


  /* -- bit operators  */
\||\&|~	{ return yytext[0];	}

  /* -- math operators  */
\+|\-|\*|\/|\% {
	  return yytext[0];
	}

\]\:	{
	    return CLOSEBRACKET;
	}

\[|\]|\(|\)|,|\{|\}|:|\;|\=|\? {
	  return yytext[0];
	}

.	{
		debug_scanner("?%s?", yytext);
	  	logError ("Unexpected char '%s'", yylineno, yytext);
		return 0;
	}
