/*---------------------------------------------------------*- c++ -*---\
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
   Version	3.0
   Author:	Klaus Kämpf <kkaempf@suse.de>
   Maintainer:	Klaus Kämpf <kkaempf@suse.de>

/-*/

%{

#include <stdlib.h>

#include "ycp/y2log.h"
#include "parser.h"

// We handle the line counting using a define
// in case we need to fix it again or decide tou use option yylineno
/*
// This fails after the nested scanner is destroyed
int yylineno = 1;
int *lineptr = 0;
#define LINE_VAR yylineno
#define INC_LINE ++LINE_VAR; *lineptr = LINE_VAR
*/
#define LINE_VAR m_lineNumber
#define INC_LINE ++LINE_VAR

//#define DEBUG_SCANNER
#ifndef DEBUG_SCANNER
#define debug_scanner nix
static void nix(...) { }
#else
#define debug_scanner printf
#endif

static inline unsigned
fromhex(char hex)
{
    if      (isdigit(hex)) return (unsigned)(hex - '0');
    else if (islower(hex)) return (unsigned)(hex - 'a' + 10);
    else                   return (unsigned)(hex - 'A' + 10);
}

#include "ycp/Scanner.h"
#include "ycp/StaticDeclaration.h"
#include "ycp/SymbolTable.h"
#include "ycp/SymbolEntry.h"
#include "ycp/YBlock.h"

extern StaticDeclaration static_declarations;

static SymbolTable *builtinTable = static_declarations.symbolTable();

static char *scanner_token;

static tokenValue token_value;

#define RESULT(type,token) setScannedToken(token_value, type); scanner_token = yytext; return token

%}

%option noyywrap

%option c++
%option yyclass="Scanner"

%x bybl str comment

LETTER	[a-z][A-Z]
ODIGIT	[0-7]
DIGIT	[0-9]
HDIGIT	[0-9][a-f][A-F]
PATHSEGMENT ([[:alpha:]_][[:alnum:]_-]*)|\"([^\\"]*(\\.)*)+\"
SYMBOL [[:alpha:]_][[:alnum:]_]+|[[:alpha:]][[:alnum:]_]*

%%
 /* " */
 /* ----------------------------------- */
 /*	Whitespace and Comments		*/
 /* ----------------------------------- */


^[\t ]*#.*$			{ /* Ignore Unix style Comments */ }
\/\*				{ BEGIN (comment); }
<comment>\n			{ INC_LINE; }
<comment>\*\/			{ BEGIN (INITIAL); }
<comment>.			{ /* skip  */ }

\/\/.*$				{ /* Ignore C++ style comments */ }
\n				{ /* Ignore newlines  */
				  INC_LINE;
				}
[\f\t\r\v ]+			{ /* Ignore Whitespaces according to isspace(3) */ }


 /* ----------------------------------- */
 /*	Numerical Constants		*/
 /* ----------------------------------- */

	/* floating point constant  */

[0-9]+\.[0-9]+([eE][+-]?[0-9]+)?|[0-9]+[eE][+-]?[0-9]+ {
	debug_scanner("<float>");
	token_value.fval = atof (yytext);
	RESULT ("f", C_FLOAT);
    }

	/* integer constant  */

[1-9][0-9]* {
	debug_scanner("<int>");
	token_value.ival = atoll (yytext);
	RESULT ("i", C_INTEGER);
    }

	/* octal constant  */

0[0-7]* {
	debug_scanner("<oct>");
	sscanf (yytext, "%Lo", &(token_value.ival));
	RESULT ("i", C_INTEGER);
    }

	/* hexadecimal constant  */

0x[0-9A-Fa-f]+ {
	debug_scanner("<hex>");
	sscanf (yytext, "0x%Lx", &(token_value.ival));
	RESULT ("i", C_INTEGER);
    }

 /* ----------------------------------- */
 /*	Byteblock			*/
 /* ----------------------------------- */

	/* byteblock prefix  */

\#\[ {
	if (m_scandataBuffer == 0) m_scandataBuffer = extend_scanbuffer (sizeof (long));
	if (m_scandataBuffer == 0) return 0;
	m_scandataBufferPtr = m_scandataBuffer + sizeof (long);
	BEGIN (bybl);
    }

	/* byteblock suffix  */

<bybl>\] {
	debug_scanner ("<byteblock>");
	BEGIN (INITIAL);
	const char *bytes = (const char *)(m_scandataBuffer + sizeof (long));
	long length = (long)(m_scandataBufferPtr - bytes);
	memcpy (m_scandataBuffer, &length, sizeof (long));
	token_value.cval = (unsigned char *)m_scandataBuffer;
	RESULT ("o", C_BYTEBLOCK);
    }

	/* byteblock data  */

<bybl>([0123456789ABCDEFabcdef][0123456789ABCDEFabcdef])+ {
	int needed   = yyleng;
	int used = m_scandataBufferPtr - m_scandataBuffer;

	if (used + needed/2 >= m_scandataBufferSize)
	{
	    if (extend_scanbuffer (needed) == 0)
		return 0;
	    m_scandataBufferPtr = m_scandataBuffer + used; // might be realloced
	}
	for (int i=0; i < needed/2; i++)
	{
	    *m_scandataBufferPtr++ = (fromhex(yytext[i*2]) << 4) | (fromhex(yytext[i*2+1]));
	}
    }

<bybl>[\t\r ]+ {
	/* Ignore Whitespaces */
    }

<bybl>\n {
	  /* Ignore Newline */
	INC_LINE;
    }

<bybl><<EOF>> {
	logError("unterminated byteblock constant", LINE_VAR);
    }

<bybl>. {
	logError("bad character in byteblock constant", LINE_VAR);
	return 0;
    }

 /* ----------------------------------- */
 /*	String				*/
 /* ----------------------------------- */

	/* string prefix  */

\"  {
        /* " */
	if (m_scandataBuffer == 0)
	    m_scandataBuffer = extend_scanbuffer (1);
	if (m_scandataBuffer == 0)
	    return 0;

	m_scandataBufferPtr = m_scandataBuffer;
	BEGIN (str);
    }

	/* string suffix  */

<str>\" {
        /* " */
	debug_scanner ("<string>");
	BEGIN (INITIAL);
	*m_scandataBufferPtr = '\0';
	token_value.sval = strdup (m_scandataBuffer);
	RESULT ("s", STRING);
    }

	/* string octal character  */

<str>\\[0-7]{1,3} {
	debug_scanner("<octal escape in string>");

	/* octal escape sequence */
	/* mis-use result as m_scandataBuffer offset  */

	int result = m_scandataBufferPtr - m_scandataBuffer;
	if (result >= m_scandataBufferSize)
	{
	    if (extend_scanbuffer (1) == 0)
		return 0;
	    m_scandataBufferPtr = m_scandataBuffer + result;
	}
	(void) sscanf (yytext + 1, "%o", &result);
	if (( result > 0xff )
	    || (result == 0))
	{
	    logError("bad octal constant", LINE_VAR);
	}
	else
	{
	    *m_scandataBufferPtr++ = result;
	}
    }

	/* string escaped character  */

<str>\\(.|\n) {
	debug_scanner("<escape in string>");
	int offset = m_scandataBufferPtr - m_scandataBuffer;
	if (offset >= m_scandataBufferSize)
	{
	    if (extend_scanbuffer (1) == 0)
		return 0;
	    m_scandataBufferPtr = m_scandataBuffer + offset;
	}
	switch (yytext[1])
	{
	    case 'n': *m_scandataBufferPtr++ = '\n'; break;
	    case 't': *m_scandataBufferPtr++ = '\t'; break;
	    case 'r': *m_scandataBufferPtr++ = '\r'; break;
	    case 'b': *m_scandataBufferPtr++ = '\b'; break;
	    case 'f': *m_scandataBufferPtr++ = '\f'; break;
	    case '\n': INC_LINE; break;
	    default:  *m_scandataBufferPtr++ = yytext[1]; break;
	}
    }

	/* string escaped " char  */

<str>[^\\\"\n]+ {
        /* " */
	char *yptr = yytext;
	int needed = yyleng;
	int used = m_scandataBufferPtr - m_scandataBuffer;

	if (used + needed >= m_scandataBufferSize)
	{
	    if (extend_scanbuffer (needed) == 0)
		return 0;
	    m_scandataBufferPtr = m_scandataBuffer + used;  // might be realloced
	}
	strcpy (m_scandataBufferPtr, yptr);
	m_scandataBufferPtr += needed;
    }

	/* \n in string */

<str>\n {
	int offset = m_scandataBufferPtr - m_scandataBuffer;
	if (offset >= m_scandataBufferSize)
	{
	    if (extend_scanbuffer (1) == 0)
		return 0;
	    m_scandataBufferPtr = m_scandataBuffer + offset;
	}
	*m_scandataBufferPtr++ = '\n';
	INC_LINE;
    }

<str><<EOF>> {
	logError("unterminated string constant", LINE_VAR);
    }

 /* ----------------------------------- */
 /*      Misc Constants			*/
 /* ----------------------------------- */

nil {
	debug_scanner("<nil>");
	RESULT ("v", C_VOID);
    }

true  {
	debug_scanner("<true>");
	token_value.bval = true;
	RESULT ("b", C_BOOLEAN);
    }

false  {
	debug_scanner("<false>");
	token_value.bval = false;
	RESULT ("b", C_BOOLEAN);
    }

 /* -- path  */

\.|(\.{PATHSEGMENT})+ {
	debug_scanner("<path>");
	token_value.pval = yytext;
	if (yytext[yyleng-1] == '-')
	{
	    logError ("dash behind path constant not allowed", LINE_VAR);
	    return 0;
	}
	RESULT ("p", C_PATH);
    }

 /* ----------------------------------- */
 /*	Statement Keywords		*/
 /* ----------------------------------- */

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
map		{ return MAP;		};
struct		{ return STRUCT;	};
block		{ return BLOCK;		};
include		{ return INCLUDE;	};
import		{ return IMPORT;	};
export		{ return EXPORT;	};
global		{ return GLOBAL;	};
static		{ return STATIC;	};
extern		{ return EXTERN;	};
module		{ return MODULE;	};
textdomain	{ return TEXTDOMAIN;	};
const		{ return CONST;		};
typedef		{ return TYPEDEF;	};
lookup		{ return LOOKUP;	};

 /* ----------------------------------- */
 /*	Type Keywords			*/
 /* ----------------------------------- */

any		{ RESULT ("a", C_TYPE);	};
void		{ RESULT ("v", C_TYPE); };
boolean		{ RESULT ("b", C_TYPE); };
integer		{ RESULT ("i", C_TYPE); };
float		{ RESULT ("f", C_TYPE); };
string		{ RESULT ("s", C_TYPE); };
byteblock	{ RESULT ("o", C_TYPE); };
 /* list, map are keywords and done by parser.yy  */
locale		{ RESULT ("_", C_TYPE); };
term		{ RESULT ("t", C_TYPE); };
path		{ RESULT ("p", C_TYPE); };
symbol		{ RESULT ("y", C_TYPE); };

 /* common mistyped names  */
int	{ logError ("Seen 'int', use 'integer' instead", LINE_VAR); return 0; };
char	{ logError ("Seen 'char', use 'string' instead", LINE_VAR); return 0; };
bool	{ logError ("Seen 'bool', use 'boolean' instead", LINE_VAR); return 0; };

 /* ----------------------------------- */
 /*	Quotes				*/
 /* ----------------------------------- */

 /* -- quoted symbol  */

\`{SYMBOL} {
	debug_scanner("<`symbol>");
	token_value.yval = strdup (yytext + 1);
	RESULT ("y", C_SYMBOL);
    }

 /* -- double quoted symbol  */

\`\`{SYMBOL} {
	debug_scanner("<``symbol>");
	logError ("*** Don't use ``symbol... but ``(symbol ... ***", LINE_VAR);
	return 0;
    }

  /* -- symbols  */

  /* plain symbol  */
{SYMBOL} {
	/*y2debug ("builtinTable %p, m_localTable %p, m_globalTable %p", builtinTable, m_localTable, m_globalTable);*/
	TableEntry *tentry = builtinTable->find (yytext);
y2debug ("builtinTable->find (%s) = %p", yytext, tentry);
	if (tentry == 0)
	{
	    tentry = m_localTable->find (yytext);
	}
	if (tentry == 0)
	{
	    tentry = m_globalTable->find (yytext);
	}
	if (tentry == 0)
	{
	    debug_scanner("<Symbol(%s)>", yytext);
	    token_value.nval = strdup (yytext);
	    RESULT ("", SYMBOL);	// symbol of unknown type
	}
	token_value.tval = tentry;
	switch (tentry->sentry()->category())
	{
	    case SymbolEntry::c_global:
	    case SymbolEntry::c_unspec:
	    {
		y2debug ("<Unspec(%s@%p)>", yytext, tentry);
		token_value.tval = tentry;
		RESULT ("", SYM_VARIABLE);
	    }
	    break;
	    case SymbolEntry::c_module:
	    {
		y2debug ("<Module(%s@%p)>", yytext, tentry);
		logError ("Missing '::' after module name '%s'", LINE_VAR, yytext);
		return 0;
	    }
	    break;
	    case SymbolEntry::c_variable:
	    {
		y2debug ("<Variable(%s@%p)>", yytext, tentry);
		token_value.tval = tentry;
		RESULT (yytext, SYM_VARIABLE);
	    }
	    break;
	    case SymbolEntry::c_function:
	    {
		y2debug ("<Function(%s@%p)>", yytext, tentry);
		token_value.tval = tentry;
		RESULT (yytext, SYM_FUNCTION);
	    }
	    break;
	    case SymbolEntry::c_builtin:
	    {
		y2debug ("<Builtin(%s@%p)>", yytext, tentry);
		token_value.tval = tentry;
		RESULT (yytext, SYM_BUILTIN);
	    }
	    break;
	    case SymbolEntry::c_typedef:
	    {
		y2debug ("<Typedef(%s@%p)>", yytext, tentry);
		RESULT (tentry->sentry()->type(), C_TYPE);
	    }
	    break;
	    case SymbolEntry::c_const:
	    {
		y2debug ("<Const(%s@%p)>", yytext, tentry);
		// FIXME
		token_value.sval = strdup (yytext);
		RESULT ("s", STRING);
	    }
	    break;
	    case SymbolEntry::c_namespace:
	    {
		y2debug ("<Namespace(%s@%p)>", yytext, tentry);
		token_value.tval = tentry;
		RESULT (yytext, SYM_NAMESPACE);
	    }
	    break;
	}
	logError ("Identifier of unknown category '%s'", LINE_VAR, yytext);
	return 0;
    }

  /* global symbol  */
::{SYMBOL}  {
	char *name = yytext + 2;
	if (builtinTable->find (name) != 0)
	{
	    logError ("Keyword used as variable '%s'", LINE_VAR, name);
	    return 0;
	}
	TableEntry *tentry = m_globalTable->find (name, SymbolEntry::c_unspec);
	if (tentry == 0)
	{
	    y2debug ("'%s' not found in %p", name, m_globalTable);
	    logError ("Unknown global variable '%s'", LINE_VAR, name);
	    return 0;
	}
	switch (tentry->sentry()->category())
	{
	    case SymbolEntry::c_global:
	    case SymbolEntry::c_unspec:
	    case SymbolEntry::c_namespace:	// can't happen since it's in builtinTable only
	    break;
	    case SymbolEntry::c_module:
	    {
		logError ("Module name '%s' after '::' ?!", LINE_VAR, name);
		return 0;
	    }
	    break;
	    case SymbolEntry::c_variable:
	    {
		token_value.tval = tentry;
		RESULT (name, SYM_VARIABLE);
	    }
	    break;
	    case SymbolEntry::c_function:
	    {
		token_value.tval = tentry;
		RESULT (name, SYM_FUNCTION);
	    }
	    break;
	    case SymbolEntry::c_builtin:
	    {
		token_value.tval = tentry;
		RESULT (name, SYM_FUNCTION);
	    }
	    break;
	    case SymbolEntry::c_typedef:
	    {
		RESULT (tentry->sentry()->type(), C_TYPE);
	    }
	    break;
	    case SymbolEntry::c_const:
	    {
		// FIXME
		token_value.sval = strdup (name);
		RESULT ("s", STRING);
	    }
	    break;
	}
	logError ("Global identifier of unknown category '%s'", LINE_VAR, name);
	return 0;
    }

  /* qualified symbol  */
{SYMBOL}(::{SYMBOL})+ {

	// check symbol for namespace
	// table points to namespace table
	// entry points to entry in table (== 0 for initial search)

	TableEntry *tentry = 0;

	// start of current SYMBOL
	char *colon = yytext;
	// end of current SYMBOL (the ':' is temporarily replaced by '\0')
	char *next = 0;

	SymbolTable *table = m_localTable;	// where imported modules are defined

	while (colon != 0)
	{
	    if (next != 0)			// skip to next symbol (does not trigger on first symbol)
	    {
		*next = ':';
		colon = next + 2;
	    }

	    next = strchr (colon, ':');
	    if (next != 0)
	    {
		*next = 0;
	    }

	    if (tentry == 0)			// first symbol, could be namespace
	    {
		tentry = builtinTable->find (colon);	// check for builtin/namespace
		if (tentry != 0)
		{
		    table = tentry->sentry()->table();
		    if (table == 0)
		    {
			logError ("Prefix '%s' is not a namespace", LINE_VAR, yytext);
			return 0;
		    }
		    y2debug ("namespace (%s) -> table %p", colon, table);
		    continue;
		}
	    }

	    // defined in table ?
	    // must be c_module if followed by '::'
	    // else find any type

	    // entry is the 'import' TableEntry in the local table
	    //   it's SymbolEntry points to the real YBlock

	    tentry = table->find (colon, (next != 0) ? SymbolEntry::c_module : SymbolEntry::c_unspec);
	    y2debug ("find ('%s'/%p in %p) = t@%p", colon, next, table, tentry);

	    if ((tentry != 0)			// found entry
		&& (next != 0))			// and must be module
	    {
		YBlock *block = (YBlock *)(tentry->sentry()->code());

		if (!block->isBlock()		// did we really find a module block code ?
		    || !block->isModule())
		{
		    logError ("Not a module '%s'", LINE_VAR, colon);
		    return 0;
		}

		// ok, this YCode defines it's own table of exported symbols
		table = block->table();
		if (table == 0)
		{
		    logError ("Module table is empty", LINE_VAR);
		    return 0;
		}

	    }

	    if (tentry == 0)
	    {
		if (next != 0)
		{
		    logError ("Unknown modules qualifier '%s'", LINE_VAR, yytext);
		}
		else
		{
		    logError ("Unknown identifier '%s'", LINE_VAR, yytext);
		}
		return 0;
	    }

	    if (next == 0)
	    {
		break;
	    }

	}  // while (colon != 0)

	y2debug ("<QualifiedSymbol(%s@%p)>", yytext, tentry);

	switch (tentry->sentry()->category())
	{
	    case SymbolEntry::c_global:
	    case SymbolEntry::c_unspec:
	    break;
	    case SymbolEntry::c_module:
	    case SymbolEntry::c_namespace:
	    {
		logError ("Module/Namespace name '%s' after '::' ?!", LINE_VAR, yytext);
		return 0;
	    }
	    break;
	    case SymbolEntry::c_variable:
	    {
		token_value.tval = tentry;
		RESULT (yytext, SYM_VARIABLE);
	    }
	    break;
	    case SymbolEntry::c_function:
	    {
		token_value.tval = tentry;
		RESULT (yytext, SYM_FUNCTION);
	    }
	    break;
	    case SymbolEntry::c_builtin:
	    {
		token_value.tval = tentry;
		RESULT (yytext, SYM_FUNCTION);
	    }
	    break;
	    case SymbolEntry::c_typedef:
	    {
		RESULT (tentry->sentry()->type(), C_TYPE);
	    }
	    break;
	    case SymbolEntry::c_const:
	    {
		// FIXME
		token_value.sval = strdup (yytext);
		RESULT ("s", STRING);
	    }
	    break;
	}
	logError ("Global identifier of unknown category '%s'", LINE_VAR, yytext);
	return 0;
    }

 /* ----------------------------------- */
 /*	Operators			*/
 /* ----------------------------------- */

  /* -- locale  */
_\(	{ return I18N;		}

  /* -- map expression  */
\$\[	{ return MAPEXPR;	}

  /* -- comparison operators  */
==	{ return EQUALS;	}
\<	{ return LT;		}
\>	{ return GT;		}
\<=	{ return LE;		}
\>=	{ return GE;		}
\!=	{ return NEQ;		}

  /* -- boolean operators  */
&&	{ return AND;		}
\|\|	{ return OR; 		}
\!	{ return NOT;		}
\<\<	{ return LEFT;		}
\>\>	{ return RIGHT;		}

  /* -- quotes  */

::\`\`\{	{ return DCQUOTED_BLOCK; }
::\{	{ return DCQUOTED_BLOCK; }
\`\`\{	{ return QUOTED_BLOCK; }
\`\`\(	{ return QUOTED_EXPRESSION; }
\`\`	{ logError ("Lonely doubleqoute", LINE_VAR); return 0; }
\`	{ logError ("Lonely qoute", LINE_VAR); return 0; }


  /* -- bit operators  */
\||\^|\&|~	{
	    return yytext[0];
	}

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

<<EOF>> {
	    return END_OF_FILE;
	}

.	{
	    debug_scanner("?%s?", yytext);
	    logError ("Unexpected char '%s'", LINE_VAR, yytext);
	    return 0;
	}
