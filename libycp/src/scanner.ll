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
#include "ycp/Import.h"
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
#include "ycp/Type.h"
#include "ycp/StaticDeclaration.h"
#include "ycp/SymbolTable.h"
#include "ycp/SymbolEntry.h"
#include "ycp/YBlock.h"

extern StaticDeclaration static_declarations;

SymbolTable *builtinTable = NULL;

// use during scan of qualified symbols
static SymbolTable *namespaceTable = NULL;
// the namespace prefix when BEGIN(namespace) is called.
static char *namespace_prefix = 0;

static char *scanner_token;

static tokenValue token_value;

#define RESULT(type,token) setScannedToken(token_value, type); scanner_token = yytext; return token

%}

%option noyywrap

%option c++
%option yyclass="Scanner"

%x bybl str comment namespace

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

[0-9]+\.[0-9]*([eE][+-]?[0-9]+)?|[0-9]+[eE][+-]?[0-9]+ {
	debug_scanner("<float>");
	token_value.fval = atof (yytext);
	RESULT (Type::ConstFloat, C_FLOAT);
    }

	/* integer constant  */

[1-9][0-9]* {
	debug_scanner("<int>");
	token_value.ival = atoll (yytext);
	RESULT (Type::ConstInteger, C_INTEGER);
    }

	/* octal constant  */

0[0-7]* {
	debug_scanner("<oct>");
	sscanf (yytext, "%Lo", &(token_value.ival));
	RESULT (Type::ConstInteger, C_INTEGER);
    }

	/* hexadecimal constant  */

0x[0-9A-Fa-f]+ {
	debug_scanner("<hex>");
	sscanf (yytext, "0x%Lx", &(token_value.ival));
	RESULT (Type::ConstInteger, C_INTEGER);
    }

 /* ----------------------------------- */
 /*	Byteblock			*/
 /* ----------------------------------- */

	/* byteblock prefix  */

\#\[ {
	if (m_scandataBuffer == 0) m_scandataBuffer = extend_scanbuffer (sizeof (long));
	if (m_scandataBuffer == 0) return SCANNER_ERROR;
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
	RESULT (Type::ConstByteblock, C_BYTEBLOCK);
    }

	/* byteblock data  */

<bybl>([0123456789ABCDEFabcdef][0123456789ABCDEFabcdef])+ {
	int needed   = yyleng;
	int used = m_scandataBufferPtr - m_scandataBuffer;

	if (used + needed/2 >= m_scandataBufferSize)
	{
	    if (extend_scanbuffer (needed) == 0)
		return SCANNER_ERROR;
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
	return SCANNER_ERROR;
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
	    return SCANNER_ERROR;

	m_scandataBufferPtr = m_scandataBuffer;
	BEGIN (str);
    }

	/* string suffix  */

<str>\" {
        /* " */
	debug_scanner ("<string>");
	BEGIN (INITIAL);
	*m_scandataBufferPtr = '\0';
	token_value.sval = Scanner::doStrdup (m_scandataBuffer);
	RESULT (Type::ConstString, STRING);
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
		return SCANNER_ERROR;
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
		return SCANNER_ERROR;
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
		return SCANNER_ERROR;
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
		return SCANNER_ERROR;
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
	RESULT (Type::ConstVoid, C_VOID);
    }

true  {
	debug_scanner("<true>");
	token_value.bval = true;
	RESULT (Type::ConstBoolean, C_BOOLEAN);
    }

false  {
	debug_scanner("<false>");
	token_value.bval = false;
	RESULT (Type::ConstBoolean, C_BOOLEAN);
    }

 /* -- path  */

\.|(\.{PATHSEGMENT})+ {
	debug_scanner("<path>");
	token_value.pval = yytext;
	if (yytext[yyleng-1] == '-')
	{
	    logError ("dash behind path constant not allowed", LINE_VAR);
	    return SCANNER_ERROR;
	}
	RESULT (Type::ConstPath, C_PATH);
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

any		{ RESULT (Type::Any, C_TYPE);	};
void		{ RESULT (Type::Void, C_TYPE); };
boolean		{ RESULT (Type::Boolean, C_TYPE); };
integer		{ RESULT (Type::Integer, C_TYPE); };
float		{ RESULT (Type::Float, C_TYPE); };
string		{ RESULT (Type::String, C_TYPE); };
byteblock	{ RESULT (Type::Byteblock, C_TYPE); };
 /* list, map are keywords and done by parser.yy  */
locale		{ RESULT (Type::Locale, C_TYPE); };
term		{ RESULT (Type::Term, C_TYPE); };
path		{ RESULT (Type::Path, C_TYPE); };
symbol		{ RESULT (Type::Symbol, C_TYPE); };

 /* common mistyped names  */
int	{ logError ("Seen 'int', use 'integer' instead", LINE_VAR); return SCANNER_ERROR; };
char	{ logError ("Seen 'char', use 'string' instead", LINE_VAR); return SCANNER_ERROR; };
bool	{ logError ("Seen 'bool', use 'boolean' instead", LINE_VAR); return SCANNER_ERROR; };

 /* ----------------------------------- */
 /*	Quotes				*/
 /* ----------------------------------- */

 /* -- quoted symbol  */

\`{SYMBOL} {
	debug_scanner("<`symbol>");
	token_value.yval = Scanner::doStrdup (yytext + 1);
	RESULT (Type::ConstSymbol, C_SYMBOL);
    }

 /* -- double quoted symbol  */

\`\`{SYMBOL} {
	debug_scanner("<``symbol>");
	logError ("*** Don't use ``symbol... but ``(symbol ... ***", LINE_VAR);
	return SCANNER_ERROR;
    }

  /* -- symbols  */

  /* qualified symbol  */
  /* originally there was {SYMBOL}:: here but it did not work */
({SYMBOL}::)+ {
	yytext[yyleng-2] = 0;

	TableEntry *tentry = builtinTable->find (yytext);	// check for builtin/namespace
	if (tentry != 0)
	{
	    y2debug ("found (%s)", tentry->toString().c_str());
	    SymbolEntryPtr sentry = tentry->sentry();
	    namespaceTable = tentry->sentry()->table();		// will be != if sentry is c_namespace

	    while (namespaceTable == 0)
	    {
		if (sentry->isPredefined())				// is a predefined namespace ?
		{
		    Import import (yytext);				// load bytecode/plugin/perl file for module

		    Y2Namespace *name_space = import.nameSpace();	// NULL if import failed
		    y2debug ("Import (%s).nameSpace = %p", yytext, name_space);
		    if (name_space != 0)
		    {
			namespaceTable = name_space->table();
			y2debug ("namespaceTable = %p", namespaceTable);
		    }

		    if (namespaceTable != 0)
		    {
			m_autoimport_predefined.push_back (std::make_pair (string(yytext), name_space));
			y2debug ("'%s' is a namespace, table %p, name_space %p", yytext, namespaceTable, name_space);
			sentry->setCategory (SymbolEntry::c_namespace);
			sentry->setTable (namespaceTable);
			break;
		    }
		    logError ("Auto-Import '%s' failed", LINE_VAR, yytext);
		}
		logError ("Prefix '%s' is not a namespace", LINE_VAR, yytext);
		return SCANNER_ERROR;
	    }
	    if (getenv (XREFDEBUG) != 0) y2milestone ("Autoimported (%s), table %p", yytext, namespaceTable);
	    else y2debug ("builtin namespace (%s) -> table %p", yytext, namespaceTable);

	    namespace_prefix = Scanner::doStrdup (yytext);
	    BEGIN (namespace);
	}
	else
	{
	    tentry = m_localTable->find (yytext, SymbolEntry::c_module);		// check for import or self reference
	    if (tentry == 0)
	    {
		tentry = m_localTable->find (yytext, SymbolEntry::c_namespace);
	    }
	    if (tentry == 0)
	    {
		tentry = m_localTable->find (yytext, SymbolEntry::c_self);
	    }
	    y2debug ("m_localTable[%p]->find (%s) = %p", m_localTable, yytext, tentry);
	    if (tentry == 0)
	    {
		logError ("Unknown namespace '%s'", LINE_VAR, yytext);
		return SCANNER_ERROR;
	    }
	    else if (tentry->sentry()->category() == SymbolEntry::c_self)	// self reference
	    {
		// ignore self-prefix inside module definition
		y2debug ("ignore self-prefix (%s)", yytext);
		BEGIN (INITIAL);
	    }
	    else
	    {
		y2debug ("found (%s)", tentry->toString().c_str());
		
		if (! tentry->sentry()->isModule())
		{
		    logError ("Not a module '%s'", LINE_VAR, yytext);
		    return SCANNER_ERROR;
		}
		
		const Y2Namespace *name_space = tentry->sentry()->payloadNamespace();

		y2debug ("Going to get a table" );
		// ok, this YCode defines it's own table of exported symbols
		namespaceTable = name_space->table();
		if (namespaceTable == 0)
		{
		    logError ("Module table is empty", LINE_VAR);
		    return SCANNER_ERROR;
		}
		y2debug ("imported namespace (%s) -> table %p", yytext, namespaceTable);

		namespace_prefix = Scanner::doStrdup (yytext);

		BEGIN (namespace);
	    }
	}
    }


  /* <namespace>{SYMBOL}(::{SYMBOL})* { */
  /* We don't need hierarchical namespaces yet, just hierarchical names
   * The colon handling code is dead for now
   */
<namespace>{SYMBOL} {

	BEGIN (INITIAL);

	// check symbol for namespace
	// table points to namespace table
	// entry points to entry in table (== 0 for initial search)

	TableEntry *tentry = 0;

	// start of current SYMBOL
	char *colon = yytext;
	// end of current SYMBOL (the ':' is temporarily replaced by '\0')
	char *next = 0;

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

	    // defined in table ?
	    // must be c_module if followed by '::'
	    // else find any type

	    // entry is the 'import' TableEntry in the local table
	    //   it's SymbolEntry points to the real YBlock

	    tentry = namespaceTable->find (colon, (next != 0) ? SymbolEntry::c_module : SymbolEntry::c_unspec);
	    y2debug ("find ('%s'<next @ %p> in %p) = t@%p", colon, next, namespaceTable, tentry);

	    if ((tentry != 0)			// found entry
		&& (next != 0))			// and must be module
	    {
		if (! tentry->sentry()->isModule())
		{
		    logError ("Not a module '%s'", LINE_VAR, colon);
		    delete[] namespace_prefix;
		    return SCANNER_ERROR;
		}
		
		const Y2Namespace *name_space = tentry->sentry()->nameSpace();

		// ok, this YCode defines it's own table of exported symbols
		namespaceTable = name_space->table();
		if (namespaceTable == 0)
		{
		    logError ("Module table is empty", LINE_VAR);
		    delete[] namespace_prefix;
		    return SCANNER_ERROR;
		}

	    }

	    if (tentry == 0)
	    {
		if (next != 0)
		{
		    logError ("Unknown namespace '%s::%s'", LINE_VAR, namespace_prefix, yytext);
		}
		else
		{
		    logError ("Unknown identifier '%s::%s'", LINE_VAR, namespace_prefix, yytext);
		}
		delete[] namespace_prefix;
		return SCANNER_ERROR;
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
	    case SymbolEntry::c_self:
	    case SymbolEntry::c_filename:
	    case SymbolEntry::c_predefined:
	    {
		logError ("Module/Namespace name '%s' after '::' ?!", LINE_VAR, yytext);
		return SCANNER_ERROR;
	    }
	    break;
	    case SymbolEntry::c_variable:
	    case SymbolEntry::c_reference:
	    case SymbolEntry::c_function:
	    case SymbolEntry::c_builtin:
	    {
		token_value.tval = tentry;
		RESULT (tentry->sentry()->type(), IDENTIFIER);
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
		token_value.sval = Scanner::doStrdup (yytext);
		RESULT (Type::ConstString, STRING);
	    }
	    break;
	}
	logError ("Global identifier of unknown category '%s'", LINE_VAR, yytext);
	return SCANNER_ERROR;
    }

<namespace>. {
	    debug_scanner("?%s?", yytext);
	    logError ("Unexpected char '%s'", LINE_VAR, yytext);
	    return SCANNER_ERROR;
	}

  /* plain symbol  */
{SYMBOL} {
	/*y2debug ("builtinTable %p, m_localTable %p, m_globalTable %p", builtinTable, m_localTable, m_globalTable);*/
	TableEntry *tentry = builtinTable->find (yytext);
if (tentry!=0) y2debug ("'%s' is builtin", yytext);
	if (tentry == 0)
	{
	    tentry = m_localTable->find (yytext);
if (tentry!=0) y2debug ("'%s' is local", yytext);
	}
	if (tentry == 0)
	{
	    tentry = m_globalTable->find (yytext);
if (tentry!=0) y2debug ("'%s' is global", yytext);
	}
	if (tentry == 0)
	{
	    debug_scanner("<Symbol(%s)>", yytext);
	    token_value.nval = Scanner::doStrdup (yytext);
	    RESULT (Type::Unspec, SYMBOL);	// symbol of unknown type
	}
	token_value.tval = tentry;
	switch (tentry->sentry()->category())
	{
	    case SymbolEntry::c_module:
	    case SymbolEntry::c_self:
	    case SymbolEntry::c_predefined:
	    {
		y2debug ("<Symbol equals module(%s@%p)>", yytext, tentry);
		token_value.nval = Scanner::doStrdup (yytext);
		RESULT (Type::Unspec, SYMBOL);	// symbol of unknown type
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
		token_value.sval = Scanner::doStrdup (yytext);
		RESULT (Type::ConstString, STRING);
	    }
	    break;
	    case SymbolEntry::c_namespace:
	    {
		y2debug ("<Namespace(%s@%p)>", yytext, tentry);
		token_value.tval = tentry;
		RESULT (tentry->sentry()->type(), SYM_NAMESPACE);
	    }
	    break;
	    case SymbolEntry::c_global:
	    case SymbolEntry::c_unspec:
	    case SymbolEntry::c_variable:
	    case SymbolEntry::c_reference:
	    case SymbolEntry::c_function:
	    case SymbolEntry::c_builtin:
	    {
		y2debug ("<identifier(%s@%p)>", yytext, tentry);
		token_value.tval = tentry;
		RESULT (tentry->sentry()->type(), IDENTIFIER);
	    }
	    break;
	    case SymbolEntry::c_filename:
	    break;
	}
	logError ("Identifier of unknown category '%s'", LINE_VAR, yytext);
	return SCANNER_ERROR;
    }

  /* global symbol  */
::{SYMBOL}  {
	char *name = yytext + 2;
	if (builtinTable->find (name) != 0)
	{
	    logError ("Keyword used as variable '%s'", LINE_VAR, name);
	    return SCANNER_ERROR;
	}
	TableEntry *tentry = m_globalTable->find (name, SymbolEntry::c_unspec);
	if (tentry == 0)
	{
	    y2debug ("'%s' not found in %p", name, m_globalTable);
	    logError ("Unknown global variable '%s'", LINE_VAR, name);
	    return SCANNER_ERROR;
	}
	switch (tentry->sentry()->category())
	{
	    case SymbolEntry::c_global:
	    case SymbolEntry::c_unspec:
	    case SymbolEntry::c_namespace:	// can't happen since it's in builtinTable only
	    case SymbolEntry::c_self:
	    case SymbolEntry::c_predefined:
	    break;
	    case SymbolEntry::c_module:
	    {
		logError ("Module name '%s' after '::' ?!", LINE_VAR, name);
		return SCANNER_ERROR;
	    }
	    break;
	    case SymbolEntry::c_variable:
	    case SymbolEntry::c_reference:
	    case SymbolEntry::c_function:
	    case SymbolEntry::c_builtin:
	    {
		token_value.tval = tentry;
		RESULT (tentry->sentry()->type(), IDENTIFIER);
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
		token_value.sval = Scanner::doStrdup (name);
		RESULT (Type::ConstString, STRING);
	    }
	    break;
	    case SymbolEntry::c_filename:
	    break;
	}
	logError ("Global identifier of unknown category '%s'", LINE_VAR, name);
	return SCANNER_ERROR;
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
\<	{ return yytext[0];	}
\>	{ return yytext[0];	}
\<=	{ return LE;		}
\>=	{ return GE;		}
\!=	{ return NEQ;		}

  /* -- boolean operators  */
&&	{ return AND;		}
\|\|	{ return OR; 		}
\!	{ return yytext[0];	}
\<\<	{ return LEFT;		}
\>\>	{ return RIGHT;		}

  /* -- quotes  */

::\`\`\{	{ return DCQUOTED_BLOCK; }
::\{	{ return DCQUOTED_BLOCK; }
\`\`\{	{ return QUOTED_BLOCK; }
\`\`\(	{ return QUOTED_EXPRESSION; }
\`\`	{ logError ("Lonely doubleqoute", LINE_VAR); return SCANNER_ERROR; }
\`	{ logError ("Lonely qoute", LINE_VAR); return SCANNER_ERROR; }


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
	    return SCANNER_ERROR;
	}
