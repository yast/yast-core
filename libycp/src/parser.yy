/*----------------------------------------------------------*- c++ -*---\
|									|
|		      __   __    ____ _____ ____			|
|		      \ \ / /_ _/ ___|_   _|___ \			|
|		       \ V / _` \___ \ | |   __) |			|
|			| | (_| |___) || |  / __/			|
|			|_|\__,_|____/ |_| |_____|			|
|									|
|			       core system				|
|						      (C) SuSE Linux AG |
\-----------------------------------------------------------------------/

   File:       parser.yy

   Author:     Klaus Kämpf <kkaempf@suse.de>
	       Stanislav Visnovsky <visnov@suse.cz>
   Maintainer: Stanislav Visnovsky <visnov@suse.cz>


   Implementation rules

   yystype is a struct with four elements

    YCodePtr c		pointer to code (where applicable)
    tokenValue v	value of token (where applicable)
    constTypePtr t	type of current syntactic element
    int l		line number of syntactic element

   c and v somehow represent a similar kind of information and are
   mostly valid alternating.

   v is used for the 'low level' (scanner) syntax like constants,
   identifiers, etc.
   c is used for the 'high level' (parser) syntax like expressions,
   statements, blocks, etc.

   t is valid everywhere since every syntactic element has a type.

   ** t == 0 means 'error', all other yystype are undefined in this case.

   l is valid everywhere since every syntactic element appears at a
   distinctive line number in the source file.
/-*/
%{
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include <fstream>
#include <string>
#include <list>

#ifndef DO_DEBUG
#define DO_DEBUG 0
#endif

#include "YCP.h"
#include "ycp/Scanner.h"
#include "ycp/y2log.h"
#include "ycp/pathsearch.h"
#include "ycp/ExecutionEnvironment.h"

#include "ycp/StaticDeclaration.h"
#include "ycp/YCode.h"
#include "ycp/Type.h"
#include "ycp/YExpression.h"
#include "ycp/YStatement.h"
#include "ycp/YBlock.h"
#include "ycp/SymbolTable.h"
#include "ycp/YSymbolEntry.h"
#include "ycp/Point.h"
#include "ycp/Bytecode.h"

#include "ycp/Parser.h"

// compile with full debug info, enable with YCP_YYDEBUG=1 in run-time env
#define YYDEBUG 0
#define YYERROR_VERBOSE 1
#define YYINITDEPTH 1000
#define YYMAXDEPTH 50000

// define type of parser values
struct yystype_type {
    YCodePtr c;			// code (for parser level syntax)
    tokenValue v;		// token (for scanner level syntax)
    constTypePtr t;		// type (NULL for error)
    int l;
};
#define YYSTYPE yystype_type

// vp_parser is void*
#define YYPARSE_PARAM vp_parser
#define YYLEX_PARAM vp_parser
#define p_parser ((Parser *) vp_parser)

#if YYLSP_NEEDED
#warning YYLSP_NEEDED
#endif

#define LINE_NOW (p_parser->m_lineno)
#define FILE_NOW (p_parser->scanner()->filename ())

// our private error function

static void yyerror_with_lineinfo	(Parser *parser, int lineno, const char *s);
static void yywarning_with_lineinfo	(Parser *parser, int lineno, const char *s);
static void yyerror_with_code		(Parser *parser, int lineno, YCodePtr c, constTypePtr t);
static void yyerror_with_name		(Parser *parser, int lineno, const char *s);
static void yyerror_with_file		(Parser *parser, int lineno, const char *s);
static void yyerror_with_tableentry	(Parser *parser, int lineno, const char *s, TableEntry *entry);
static void yywarning_with_tableentry	(Parser *parser, int lineno, TableEntry *entry);
static void yyerror_type_mismatch	(Parser *parser, int lineno, const char *s, constTypePtr expected_type, constTypePtr seen_type);
static void yyerror_missing_argument	(Parser *parser, int lineno, constTypePtr type);
static void yyerror_assign_const	(Parser *parser, int lineno, const char *s);
static void yyerror_cant_cast		(Parser *parser, int lineno, constTypePtr from, constTypePtr to);
static void yyerror_no_module		(Parser *parser, int lineno, const char *module);

//static void yyerror_decl_mismatch (Parser *parser, declaration_t *decl, const char *seen_type, int lineno);

#define yyerror(text)				yyerror_with_lineinfo (p_parser, -1, text)
#define yywarning(text,lineno)			yywarning_with_lineinfo (p_parser, lineno, text)
#define yyConstAssignError(name, lineno)	yyerror_assign_const (p_parser, lineno, name)
#define yyLerror(text,lineno)			yyerror_with_lineinfo (p_parser, lineno, text)
#define yyCerror(code,type,lineno)		yyerror_with_code (p_parser, lineno, code, type)
#define yyVerror(name,lineno)			yyerror_with_name (p_parser, lineno, name)
#define yyFerror(name,lineno)			yyerror_with_file (p_parser, lineno, name)
#define yyTerror(text,lineno,tentry)		yyerror_with_tableentry (p_parser, lineno, text, tentry)
#define yyTwarning(tentry)			yywarning_with_tableentry (p_parser, 0, tentry)
#define yyTypeMismatch(text,expected,seen,lineno)	yyerror_type_mismatch (p_parser, lineno, text, expected, seen)
#define yyMissingArgument(type,lineno)		yyerror_missing_argument (p_parser, lineno, type)
#define yyCantCast(from,to,lineno)		yyerror_cant_cast (p_parser, lineno, from, to)
#define yyNoModule(module,lineno)		yyerror_no_module (p_parser, lineno, module)

// attach a new parameter (parm) to a function call (code)
//   if the parameter is 'type symbol', type is passed in parm, symbol in parm1
// returns NULL if success, != NULL (expected type) if wrong parameter type
//  Type::Unspec if bad code (code == NULL), Type::Error if excessive parameter

static constTypePtr attach_parameter (Parser *parser, YCodePtr code, YYSTYPE *parm, YYSTYPE *parm1 = 0);

//! set by function declaration in order to predefine a definitions block return type
static constTypePtr declared_return_type = Type::Unspec;
//! set when a return statement is encountered
static constTypePtr found_return_type = Type::Unspec;
//! begin of a block
//! @param type declared return type
static YBlockPtr start_block (Parser *parser, constTypePtr type);

extern "C" {
int yylex (YYSTYPE *, void *);
}

static void i_check_unary_op (YYSTYPE *result, YYSTYPE *e1, const char *op, Parser* parser);
static void i_check_binary_op (YYSTYPE *result, YYSTYPE *e1, const char *op, YYSTYPE *e2, Parser* parser);
static void i_check_compare_op (YYSTYPE *result, YYSTYPE *e1, YECompare::c_op op, YYSTYPE *e2, Parser *parser);
static void i_check_void_assign (YYSTYPE *lhs, YYSTYPE *rhs, Parser *parser);
#define check_unary_op(result,e1,op) i_check_unary_op (result, e1, op, p_parser)
#define check_binary_op(result,e1,op,e2) i_check_binary_op (result, e1, op, e2, p_parser)
#define check_compare_op(result,e1,op,e2) i_check_compare_op (result, e1, op, e2, p_parser)
#define check_void_assign(lhs,rhs) i_check_void_assign (lhs, rhs, p_parser)

// for unary and binary operators
extern StaticDeclaration static_declarations;

// for logging
extern ExecutionEnvironment ee;

/*
 * DO NOT USE static or global variables!
 * They make the parser non-reentrant. You need to put them into class Parser.
 */ 

/*
 * this parser does not have to be reentrant
 */

//! general stack handling
//
typedef struct stack {
    struct stack *down;		//!< next stack element
} stack_t;
//! push element to stack
static void stack_push (stack_t **stack, stack_t *element);
//! pop element to stack
static stack_t *stack_pop (stack_t **stack);

//! stack for blocks

struct blockstack_t : stack_t {
    YBlockPtr theBlock;		//!< pointer to block
    const char *textdomain;	//!< textdomain (if defined)
//    constTypePtr type;		//!< return type of block
    int includeDepth;		//!< block is include file, all definitions go to the outer block
    TableEntry *self;		//!< c_self entry during module parsing
};
static void
_blockstack_push (blockstack_t **blockstackptr, blockstack_t *blockelement)
{
    stack_t **stackptr = (stack_t **)blockstackptr;
    stack_t *stackelement = (stack_t *)blockelement;
    stack_push (stackptr, stackelement);
}
#define blockstack_push(s,e) _blockstack_push(&(s), e)

static blockstack_t *
_blockstack_pop (Parser *parser, blockstack_t **blockstackptr)
{
    parser->m_blockstack_depth--;
    stack_t **stackptr = (stack_t **)blockstackptr;
    stack_t *stackelement = stack_pop (stackptr);
    return (blockstack_t *)stackelement;
}
#define blockstack_pop(s) _blockstack_pop(p_parser, &(s))

#define blockstack_at_toplevel() (p_parser->m_blockstack_depth == 1)


//! stack for switch

struct switchstack_t : stack_t {
    YSSwitchPtr statement;	//!< pointer to switch statement
};
static void
_switchstack_push (switchstack_t **switchstack, stack_t *e)
{
    stack_t **stackptr = (stack_t **)switchstack;
    stack_push (stackptr, e);
}
#define switchstack_push(s,e) _switchstack_push(&(s), e)
static switchstack_t *
_switchstack_pop (switchstack_t **switchstack)
{
    stack_t **stackptr = (stack_t **)switchstack;
    return (switchstack_t *)stack_pop (stackptr);
}
#define switchstack_pop(s) _switchstack_pop(&(s))

static bool in_switch = false;

enum scan_states {
    SCAN_FILE,		//!< a plain file
    SCAN_START_INCLUDE,	//!< before the first token of an include file (see start_block())
    SCAN_INCLUDE,	//!< inside an include file
};

//! stack for scanners
struct scannerstack_t : stack_t {
    Scanner *scanner;
    string filename;
    int linenumber;
    enum scan_states state;
    const char *old_textdomain ;	// the textdomain set before starting the include
};
static void
_scannerstack_push(scannerstack_t **scannerstackptr, stack_t *element)
{
    stack_t **stackptr = (stack_t **)scannerstackptr;
    stack_push (stackptr, element);
}
#define scannerstack_push(s,e) _scannerstack_push(&(s), e)
static scannerstack_t *
_scannerstack_pop(scannerstack_t **scannerstackptr)
{
    stack_t **stackptr = (stack_t **)scannerstackptr;
    return (scannerstack_t *)stack_pop (stackptr);
}
#define scannerstack_pop(s) _scannerstack_pop (&(s))
#define scannerstack_empty() (p_parser->m_scanner_stack == 0)

// mark here if we're parsing a module
static bool inside_module = false;

static int repeat_count = 0;
static int do_while_count = 0;
//----------------------------------------------------------------------------
%}

 /* expect one shift-reduce conflict (a dangling else) */
%expect 2
%pure_parser

  /* SCANNER_ERROR is returned when yylex does not have a valid token */
%token  SCANNER_ERROR
%token	END_OF_FILE
%token  EMPTY LIST MAP STRUCT BLOCK DEFINE UNDEFINE I18N
%token  RETURN CONTINUE BREAK IF DO WHILE REPEAT UNTIL IS ISNIL
%token  SYMBOL DCSYMBOL
%token  DCQUOTED_BLOCK QUOTED_BLOCK QUOTED_EXPRESSION
%token  CLOSEBRACKET
%token  TYPEDEF
%token  MODULE IMPORT EXPORT MAPEXPR INCLUDE GLOBAL TEXTDOMAIN
%token	CONST FULLNAME STATIC EXTERN
%token	LOOKUP SELECT 
%token	SWITCH CASE DEFAULT

%token	SYM_NAMESPACE
 /* known entry  */
%token  IDENTIFIER

 /* constants  */
 /* the C_xxx tokens return a YConst(), STRING is handled special */
 /*  since we need it also as non-ycp value */
%token  STRING
%token	C_VOID C_BOOLEAN C_INTEGER C_FLOAT
%token	C_BYTEBLOCK C_PATH C_SYMBOL C_TYPE

 /* bindings in order of precedence, lowest first  */

%right '='
%left '?'
%left OR
%left AND
%left '|'
%left '^'
%left '&'
%right CONST
%left EQUALS NEQ
%left '<' '>' LE GE
%left LEFT RIGHT
%left '+' '-'
%left '*' '/' '%'
%right '!'
%left ELSE
%right '~'
%right UMINUS
%left ':'
%left CLOSEBRACKET
%left '['

/* ---------------------------------------------------------------------- */
%%

ycp:	compact_expression
	    {
		inside_module = false; 		// reset for next call
		p_parser->m_result = ($1.t == 0) ? 0 : $1.c;
		p_parser->m_current_block = 0;
		p_parser->m_lineno = $1.l;
		if (p_parser->m_parser_errors > 0)
		{
		    p_parser->m_parser_errors = 0;
		    p_parser->m_result = 0;
		    YYABORT;
		}
		y2debug ("\n------------------------------------------- accept -------------------------------------------\n");

		YYACCEPT;
	    }
|	END_OF_FILE
	    {
		p_parser->m_result = 0;
		p_parser->m_lineno = -1;
		if (p_parser->m_parser_errors > 0)
		{
		    p_parser->m_parser_errors = 0;
		    // yyerror ("EOF");
		    p_parser->m_result = 0;
		    YYABORT;
		}
		y2debug ("\n-------------------------------------------- EOF --------------------------------------------\n");
		YYACCEPT;
	    }
|	    {
		YYABORT;
	    }
;

/* Expressions */

  /*
   * EXPRESSION vs BLOCK: type
   * An important difference between a block and an expression is that
   * block's type is determined (!= isUnspec) only if it has an explicit
   * "return".  It is then used for detecting type-mismatched return
   * statements, among other things. As a consequence, all statements
   * except return must have an undetermined type (Type::Unspec).
   * Do not confuse Type::Unspec with Type::Void.
   */
  /* expressions are either 'compact' (with a defined end-token, no lookahead)
     or 'infix' (which might need a lookahead token)  */

expression:
	compact_expression
|	casted_expression
|	infix_expression
|	bracket_expression
;

bracket_expression:
	compact_expression '[' list_elements CLOSEBRACKET expression
	    {
		if (($1.t == 0)			// any errors yet ?
		    || ($3.t == 0)
		    || ($5.t == 0))
		{
		    $$.t = 0;			// Y: break out
		    break;
		}
		else if (!$1.t->isList()
			 && !$1.t->isMap()
			 && !$1.t->isTerm())
		{
		    yyLerror ("Bracket operator must be applied to list, map, or term", $1.l);
		    $$.t = 0;
		    break;
		}
		else if ($1.t->isTerm ())
		{
		    // cannot find out anything
		    $$.t = Type::Any;
		}
		else
		{
		    // try to determine the type as far as possible, following the list of arguments,
		    // doing a type check as we go
		    // come out with $$.t == 0 if error, else determined type

		    // the currently tested structured type
		    constTypePtr cur = $1.t;

		    // index into YEList of bracket parameters, the list cannot be empty
		    int index = 0;
		    YEListPtr params = (YEListPtr)$3.c;

		    do
		    {
			constTypePtr paramType = params->value (index)->type ();	// type of bracket parameter at index
#if DO_DEBUG
			y2debug ("paramvalue (%d) '%s'", index, params->value(index)->toString().c_str());
			y2debug ("paramType '%s'", paramType->toString().c_str());
#endif
			if (paramType->isFunction())
			{
			    paramType = ((constFunctionTypePtr)paramType)->returnType ();
#if DO_DEBUG
			    y2debug ("paramType is function returning '%s'", paramType->toString().c_str());
#endif
			}
			
			// for lists, only integer is acceptable
			if (cur->isList ())
			{
			    if (! paramType->isInteger ())
			    {
				yyTypeMismatch ((string ("Bracket parameter '")
						 + params->value(index)->toString()
						 + string ("' to '")
						 + $1.c->toString()
						 + string ("' has wrong type")).c_str(), Type::Integer, paramType, $1.l);
				$$.t = 0;
				break;
			    }
			    else
			    {
				cur = ((constListTypePtr)cur)->type ();
			    }
			}
			else if (cur->isMap ())
			{
			    if (paramType->match (((constMapTypePtr)cur)->keytype ()) == -1)
			    {
				yyTypeMismatch ((string ("Bracket parameter '")
						 + params->value(index)->toString()
						 + string ("' to '")
						 + $1.c->toString()
						 + string ("' has wrong type")).c_str(), ((constMapTypePtr)cur)->keytype (), paramType, $1.l);
				$$.t = 0;
				break;
			    }
			    else
			    {
				cur = ((constMapTypePtr)cur)->valuetype ();
			    }
			}

			index++;

		    } while (index < params->count ()
			     && (cur->isList () || cur->isMap ()));

		    // quit on error
		    if ($$.t == 0)
			break;

		    if (index < params->count ())		// we hit a non-list/non-map before end of bracket
		    {
			$$.t = Type::Any;			// we can't say anything about the resulting type, must propagate to default type
		    }
		    else 
		    {
			$$.t = cur;
		    }
		}					// type determination done

#if DO_DEBUG
		    y2debug ("default type '%s', determined type '%s'", $5.t->toString().c_str(), $$.t->toString().c_str());
#endif
		// default ($5) must match for non-nil
		if (! $5.t->isVoid ()				// default is not 'nil'
		    && $5.t->match ($$.t) == -1)		// and it doesn't match the determined type
		{
		    yyTypeMismatch ((string ("Bracket default to '")
				     + $1.c->toString()
				     + string ("' has wrong type")).c_str(), $$.t, $5.t, $1.l);		// -> then we have a type error
		    $$.t = 0;
		}
		else
		{
		    check_void_assign (0, &($5));
		    $$.c = new YEBracket ($1.c, $3.c, $5.c, $$.t);
		    $$.l = $1.l;
		    if (! $5.t->isVoid ()			// default is not 'nil'
			&& $$.t->isAny ()			// and the map/list is unspecified
			&& !$5.t->isAny())			// don't cast any -> any
		    {
			// for non-nil default and cur == Any use the type of the default,
			// but with runtime type checking
			$$.c = new YEPropagate ($$.c, Type::Any, $5.t);
		    }
		    $$.t = $$.t->detailedtype ($5.t);
#if DO_DEBUG
		    y2debug ("detailed type '%s'", $$.t->toString().c_str());
#endif
		}
	    }
;

castable_expression:
| 	compact_expression
|	casted_expression
|	bracket_expression
;

casted_expression:
	'(' type ')' castable_expression
	{
	    // on error, propagate it
	    if ($2.t == 0 || $4.t == 0 || $4.c == 0)
	    {
		$$.c = 0;
		$$.t = 0;
		break;
	    }
	    
	    int match = $4.t->match ($2.t);	// would casted type allow expression type ?
#if DO_DEBUG
	    y2debug ("cast '%s' to '%s' match %d", $4.t->toString().c_str(), $2.t->toString().c_str(), match);
#endif
	    if (match == 0)
	    {
		$$.c = $4.c;			// types match anyway
	    }
	    else if ($4.t->canCast ($2.t))
	    {
		$$.c = new YEPropagate ($4.c, $4.t, $2.t);
	    }
	    else 
	    {
		yyCantCast ($4.t, $2.t, $2.l);
	    	$$.t = 0;
		break;
	    }

	    $$.t = $2.t;
	    $$.l = $4.l;
	}
;

compact_expression:
	block
	    {
		$$ = $1;
#if DO_DEBUG
		y2debug ("compact_expression: block");
#endif
		/*
		 * If a block is used as an expression, we must make
		 * sure it does have a type. It is either provided by
		 * an explicit return, or there is an implicit return
		 * nil, ie. type void.
		 */
		if ($$.t
		    && $$.t->isUnspec ())
		{
		    $$.t = Type::Void;
		}

		/*
		 * we also must make sure that an empty block is treated as 'nil'
		 */
		if ($$.t != 0					// not an error
		    && $$.c == 0)				// empty block
		{
		    $$.c = new YConst (YCode::ycVoid, YCPVoid());

		    yywarning ("Empty block is treated as 'nil'", $1.l);
		}

	    }
|	LOOKUP '(' expression ',' expression ',' expression ')'
	    {
		if (($3.t == 0)
		    || ($5.t == 0)
		    || ($7.t == 0))
		{
		    $$.t = 0;
		    break;
		}

		yywarning ("'lookup()' is deprecated", $1.l);

		// lookup needs special treatment because we must
		// pass a 'type hint' about the default expression
		// if default is nil, the type can't be seen in a YCPValue
		if (!$3.t->isMap())
		{
		    yyTypeMismatch ("First parameter to 'lookup' has wrong type", Type::Map, $3.t, $3.l);
		    $$.t = 0;
		    break;
		}

		// check the type of the index
		if ($5.t->match ( (constMapTypePtr($3.t))->keytype())== -1)
		{
		    yyTypeMismatch ("Second parameter to 'lookup' has wrong type", (constMapTypePtr($3.t))->keytype(), $5.t, $1.l);		// -> then we have a type error
		    $$.t = 0;
		}

		// likely type of lookup() expression

		$$.t = (constMapTypePtr($3.t))->valuetype();

		// default ($7) must match for non-nil
		if (! $7.t->isVoid ()			// default is not 'nil'
		    && $7.t->match ($$.t) == -1)	// and it doesn't match the determined type
		{
		    yyTypeMismatch ("Third parameter to 'lookup' has wrong type", $$.t, $7.t, $1.l);	// -> then we have a type error
		    $$.t = 0;
		}
		else
		{
		    // see bracket_expression !!
		    check_void_assign (0, &($7));
		    $$.c = new YEBracket ($3.c, new YEList($5.c), $7.c, $$.t);
		    if (! $7.t->isVoid ()                       // default is not 'nil'
			&& $$.t->isAny()			// map is unspecified
			&& !$7.t->isAny())                      // don't cast any -> any
                    {
                        // for non-nil default and cur == Any use the type of the default,
                        // but with runtime type checking
                        $$.c = new YEPropagate ($$.c, Type::Any, $7.t);
                    }
                    $$.t = $$.t->detailedtype ($7.t);
#if DO_DEBUG
                    y2debug ("detailed type '%s'", $$.t->toString().c_str());
#endif
		}
		$$.l = $1.l;
	    }
|	SELECT '(' expression ',' expression ',' expression ')'
	    {
		if (($3.t == 0)
		    || ($5.t == 0)
		    || ($7.t == 0))
		{
		    $$.t = 0;
		    break;
		}

		yywarning ("'select ()' is deprecated", $1.l);

		// select needs special treatment because we must
		// pass a 'type hint' about the default expression
		// if default is nil, the type can't be seen in a YCPValue
		if ($3.t->isList())
		{
		    // likely type of select() expression

		    $$.t = (constListTypePtr($3.t))->type();
		}
		else if ($3.t->isTerm())
		{
		    // likely type of select() expression

		    $$.t = Type::Any;
		}
		else
		{
		    yyLerror ("First parameter to 'select' must have list or map type", $3.l);
		    $$.t = 0;
		    break;
		}

		// check the type of the index
		if ($5.t->match (Type::Integer) < 0)
		{
		    yyTypeMismatch ("Second parameter to 'select' has wrong type", Type::Integer, $5.t, $1.l);	// -> then we have a type error
		    $$.t = 0;
		}

		// default ($7) must match for non-nil
		if (! $7.t->isVoid ()			// default is not 'nil'
		    && !$$.t->isUnspec()		// and we have a determined type (list only, term doesnt have it)
		    && $7.t->match ($$.t) == -1)	// and it doesn't match the determined type
		{
		    yyTypeMismatch ("Third parameter to 'select' has wrong type", $$.t, $7.t, $1.l);	// -> then we have a type error
		    $$.t = 0;
		}
		else
		{
		    // see bracket_expression !!
		    check_void_assign (0, &($7));
		    $$.c = new YEBracket ($3.c, new YEList($5.c), $7.c, $$.t);
		    if (! $7.t->isVoid ()                       // default is not 'nil'
			&& $$.t->isAny()			// list is unspecified
			&& !$7.t->isAny())                      // don't cast any -> any
                    {
                        // for non-nil default and cur == Any use the type of the default,
                        // but with runtime type checking
                        $$.c = new YEPropagate ($$.c, Type::Any, $7.t);
                    }
                    $$.t = $$.t->detailedtype ($7.t);
#if DO_DEBUG
                    y2debug ("detailed type '%s'", $$.t->toString().c_str());
#endif
		}
		$$.l = $1.l;
	    }
|	function_call
	    {
		$$ = $1;
	    }
|	'(' expression ')'
	    {
		$$ = $2;
	    }
|	QUOTED_EXPRESSION expression ')'
	    {
		if ($2.t == 0)
		{
		    $$.t = 0;
		    break;
		}
		$$.c = new YEReturn ($2.c);
		$$.t = BlockTypePtr ( new BlockType ($2.t));
	    }
|	IS '(' expression ',' type ')'
	    {
		if ($3.t == 0)		// expression error
		{
		    $$.t = 0;
		    break;
		}
		$$.c = new YEIs ($3.c, $5.t);
		$$.t = Type::Boolean;
		$$.l = $1.l;
	    }
|	TEXTDOMAIN
	    {
		if (p_parser->m_block_stack == 0
		    || p_parser->m_block_stack->textdomain == 0)
		{
		    yyLerror ("No textdomain defined", $1.l);
		    $$.t = 0;
		    break;
		}

		$$.c = new YConst (YCode::ycString, YCPString (p_parser->m_block_stack->textdomain));
		$$.t = Type::String;
		$$.l = $1.l;
	    }
|	I18N string ',' string ',' expression ')'
	    {
		if ($6.t == 0)
		{
		    $$.t = 0;
		    break;
		}

		if (p_parser->m_block_stack == 0
		    || p_parser->m_block_stack->textdomain == 0)
		{
		    yyLerror ("No textdomain defined", $1.l);
		    $$.t = 0;
		    break;
		}

		if ($6.t->match (Type::Integer) < 0)
		{
		    yyTypeMismatch ("Last parameter to _(...) has wrong type", Type::Integer, $6.t, $6.l);
		    $$.t = 0;
		    break;
		}
		// don't free the .sval, YELocale keeps pointers
		$$.c = new YELocale ($2.v.sval, $4.v.sval, $6.c, p_parser->m_block_stack->textdomain);
		$$.t = Type::Locale;
		$$.l = $1.l;
	    }
|	I18N string ')'
	    {
		if (p_parser->m_block_stack == 0
		    || p_parser->m_block_stack->textdomain == 0)
		{
		    yyLerror ("No textdomain defined", $1.l);
		    $$.t = 0;
		}
		else if (*($2.v.sval) == 0)		// empty string ?
		{
		    yywarning ("Empty locale constant", $1.l);

		    // treat empty locale as normal string
		    // else dgettext would return the complete .po file header !

		    $$.c = new YConst (YCode::ycString, YCPString ($2.v.sval));
		    delete[] $2.v.sval;
		    $$.t = Type::String;
		}
		else
		{
		    // don't free $2.v.sval, YLocale keeps the pointer
		    $$.c = new YLocale ($2.v.sval, p_parser->m_block_stack->textdomain);
		    $$.t = Type::Locale;
		}
		$$.l = $1.l;
	    }
|	identifier
	    {
		if ($1.t == 0
		    || $1.t->isUnspec ())		// new (undeclared) identifier
		{
		    // FIXME: we should free the identifier memory
		    yyVerror ($1.v.nval, $1.l);
		    $$.t = 0;
		}
		else
		{
		    SymbolEntryPtr sentry = $1.v.tval->sentry();
		    $$.c = new YEVariable (sentry);
		    $$.t = sentry->type();
#if DO_DEBUG
		    y2debug ("identifier '<%s>%s' !", $$.t->toString().c_str(), sentry->name());
#endif
		    $$.l = $1.l;
		}
	    }

|	list
|	map
|	constant
;

infix_expression:
	expression '+' expression
	    {
		check_binary_op (&($$), &($1), "+", &($3));
	    }
|	expression '-' expression
	    {
		check_binary_op (&($$), &($1), "-", &($3));
	    }
|	expression '*' expression
	    {
		check_binary_op (&($$), &($1), "*", &($3));
	    }
|	expression '/' expression
	    {
		check_binary_op (&($$), &($1), "/", &($3));
	    }
|	expression '%' expression
	    {
		check_binary_op (&($$), &($1), "%", &($3));
	    }
|	expression LEFT expression
	    {
		check_binary_op (&($$), &($1), "<<", &($3));
	    }
|	expression RIGHT expression
	    {
		check_binary_op (&($$), &($1), ">>", &($3));
	    }
|	expression '&' expression
	    {
		check_binary_op (&($$), &($1), "&", &($3));
	    }
|	expression '^' expression
	    {
		check_binary_op (&($$), &($1), "^", &($3));
	    }
|	expression '|' expression
	    {
		check_binary_op (&($$), &($1), "|", &($3));
	    }
|	'~' expression
	    {
		check_unary_op (&($$), &($2), "~");
	    }
|	expression AND expression
	    {
		check_binary_op (&($$), &($1), "&&", &($3));
	    }
|	expression OR expression
	    {
		check_binary_op (&($$), &($1), "||", &($3));
	    }
|	expression EQUALS expression
	    {
		check_compare_op (&($$), &($1), YECompare::C_EQ, &($3));
	    }
|	expression '<' expression
	    {
		check_compare_op (&($$), &($1), YECompare::C_LT, &($3));
	    }
|	expression '>' expression
	    {
		check_compare_op (&($$), &($1), YECompare::C_GT, &($3));
	    }
|	expression LE expression
	    {
		check_compare_op (&($$), &($1), YECompare::C_LE, &($3));
	    }
|	expression GE expression
	    {
		check_compare_op (&($$), &($1), YECompare::C_GE, &($3));
	    }
|	expression NEQ expression
	    {
		check_compare_op (&($$), &($1), YECompare::C_NEQ, &($3));
	    }
|	'!' expression
	    {
		if ($2.t == 0)
		{
		    $$.t = 0;
		    break;
		}

		if ($2.c->isConstant())
		{
		    YConstPtr c = (YConstPtr)$2.c;
		    if (c->kind() == YCode::ycBoolean)
		    {
			$$.c = new YConst (YCode::ycBoolean, YCPBoolean (!(c->value()->asBoolean()->value())));
			$$.t = Type::Boolean;
			$$.l = $1.l;
		    }
		    else
		    {
			yyLerror ("Bad constant for binary 'not'", $2.l);
			$$.t = 0;
		    }
		}
		else
		{
		    check_unary_op (&($$), &($2), "!");
		}
	    }
|	'-' expression %prec UMINUS
	    {
		if ($2.t == 0)
		{
		    $$.t = 0;
		    break;
		}

		if ($2.c->isConstant())
		{
		    YConstPtr c = (YConstPtr)$2.c;
		    if (c->kind() == YCode::ycInteger)
		    {
			$$.c = new YConst (YCode::ycInteger, YCPInteger (-(c->value()->asInteger()->value())));
			$$.t = Type::Integer;
			$$.l = $1.l;
		    }
		    else if ($2.c->kind() == YCode::ycFloat)
		    {
			$$.c = new YConst (YCode::ycFloat, YCPFloat (-(c->value()->asFloat()->value())));
			$$.t = Type::Float;
			$$.l = $1.l;
		    }
		    else
		    {
			yyLerror ("Bad constant for negate", $2.l);
			$$.t = 0;
		    }
		}
		else
		{
		    check_unary_op (&($$), &($2), "-");
		}
	    }
|	expression '?' expression ':' expression
	    {
		if (($1.t == 0)
		    || ($3.t == 0)
		    || ($5.t == 0))
		{
		    $$.t = 0;
		    break;
		}

		if (!$1.t->isBoolean())
		{
		    yyTypeMismatch ("Expression before '?' must be boolean", Type::Boolean, $1.t, $1.l);
		    $$.t = 0;
		}
		else if ($1.c->kind() == YCode::ycBoolean)
		{
		    if ($1.c->evaluate (true)->asBoolean()->value() == true)
		    {
			$$.c = $3.c;
		    }
		    else
		    {
			$$.c = $5.c;
		    }
		}
		else
		{
		    $$.c = new YETriple ($1.c, $3.c, $5.c);
		}
		$$.t = $3.t->commontype ($5.t);
		$$.l = $1.l;
	    }
;

block:
	'{'
	    {
		if (declared_return_type == 0)
		{
		    // this is error, propagate
		    $$.t = 0;
		    break;
		}
		
		constTypePtr b_t = declared_return_type;
		
		if ( ! declared_return_type->isUnspec ())
		{
		    declared_return_type = Type::Unspec;
		}
		else if (p_parser->m_block_stack != NULL)
		{
		    b_t = p_parser->m_block_stack->theBlock->type ();
		}

		start_block (p_parser, b_t);

		// verify, if we are in switch. if yes, say we are
		// the block of the switch
		
		if (in_switch) {
#if DO_DEBUG
		    y2debug ("Setting block for switch");
#endif
		    p_parser->m_switch_stack->statement->
			setBlock (p_parser->m_block_stack->theBlock);
		    in_switch = false;
		}
		
	    }
	block_end
	    {
		$$ = $3;
#if DO_DEBUG
		y2debug ("block: (%s:%s)", $$.c ? $$.c->toString().c_str() : "<nil>", $$.t ? $$.t->toString().c_str() : "<ERR>");
#endif
	    }
|	QUOTED_BLOCK
	    {
		if (declared_return_type == 0)
		{
		    // this is error, propagate
		    $$.t = 0;
		    break;
		}
		
		// this differs from the non-quoted block that it will not
		// inherit the return type from the parent block, never

		constTypePtr b_t = declared_return_type;

		if ( ! declared_return_type->isUnspec ())
		{
		    declared_return_type = Type::Unspec;
		}

		start_block (p_parser, b_t);
	    }
	block_end
	    {
		$$ = $3;
#if DO_DEBUG
		y2debug ("block: (%s:%s)", $$.c ? $$.c->toString().c_str() : "<nil>", $$.t ? $$.t->toString().c_str() : "<ERR>");
#endif
	    }
;

block_end:
	statements '}'
	    {
		// end of block
		//
		// pop block from block stack
		// unlink local symbols from symbol table

		blockstack_t *top = p_parser->m_block_stack;
		bool is_include = true;

		if (top == 0
		    || (top->includeDepth == 0))
		{
#if DO_DEBUG
		    y2debug ("block end");
#endif
		    top = blockstack_pop (p_parser->m_block_stack);
		    is_include = false;
		}

		YBlockPtr b = top->theBlock;

		SymbolTable *localTable = p_parser->scanner()->localTable();
		extern SymbolTable *builtinTable;	// for predefined namespaces
#if 0
		y2debug ("table before (%s)", localTable->toString().c_str());
#endif

		if (top->self != 0)
		{
		    top->self->remove();			// remove c_self entry
		}

		if (top->includeDepth == 0)
		{
#if DO_DEBUG
		    y2debug ("Detaching local table");
#endif
		    b->detachEnvironment (localTable);		// detach local table
#if DO_DEBUG
		    y2debug ("Detaching local table done");
#endif
		}
		else
		{
		    top->includeDepth--;			// end of include block
		    b->endInclude ();
		}

		if (b->isModule()
		    || b->isFile())
		{
		    // end of topleve 'module' or 'file' block
		    // check if any predefined namespace where activated in StaticDeclaration
		    //   or auto-imported by the scanner
		    // add import statements for such namespaces
		    Y2Namespace *name_space;

		    const std::list<std::pair<std::string, Y2Namespace *> > & active_predefined = static_declarations.active_predefined();
		    std::list<std::pair<std::string, Y2Namespace *> >::const_iterator it;
		    for (it = active_predefined.begin(); it != active_predefined.end(); it++)
		    {
			name_space = it->second;
			if (name_space->table()->countUsage() > 0)
			{
#if DO_DEBUG
			    y2debug ("active_predefined: import '%s', %d symbols needed", it->first.c_str(), name_space->table()->countUsage());
#endif
			    p_parser->m_current_block->pretachStatement (new YSImport (it->first, name_space));
			}
		    }
		    const std::list<std::pair<std::string, Y2Namespace *> > & autoimport_predefined= p_parser->scanner()->autoimport_predefined();
		    for (it = autoimport_predefined.begin(); it != autoimport_predefined.end(); it++)
		    {
			name_space = it->second;
			if (name_space->table()->countUsage() > 0)
			{
#if DO_DEBUG
			    y2debug ("autoimport_predefined: import '%s', %d symbols needed", it->first.c_str(), name_space->table()->countUsage());
#endif
			    p_parser->m_current_block->pretachStatement (new YSImport (it->first, name_space));
			    
			    // reset symbol entry category - BEWARE: this makes it non-reentrant
                    	    TableEntry* tentry = builtinTable->find (it->first.c_str ());
                    	    if (tentry && tentry->sentry ())
                    	    {
                        	tentry->sentry ()->setCategory (SymbolEntry::c_predefined);
                    	    }
			}
		    }
		}

//		y2debug ("table after (%s)", localTable->toString().c_str());

		if ($1.t == 0)			// error block
		{
		    $$.t = 0;
		    break;
		}

		if ($1.c == 0)			// empty block
		{
		    if (is_include)
		    {
			yyLerror ("Bad (empty ?) include file", $1.l);
			$$.t = 0;
			break;
		    }
		    if (b->isModule())
		    {
			yyLerror ("Empty module", $1.l);
			$$.t = 0;
			break;
		    }
		    $$.c = 0;
		}
		else if (is_include)		// this was an include block
		{
		    $$.c = 0;			// pass it up as 'empty'
		}
		else
		{
		    b->finish ();
		    $$.c = b;			// normal block
		}

		// See the comment about types at the "expression" rule.
		$$.t = top->theBlock->type ();
		$$.l = $1.l;

		if (!is_include)
		{
		    delete top;
		}
	    }
;

/* -------------------------------------------------------------- */
/* Statements */
/* statements are always inside a block, so p_parser->m_block_stack is valid !  */

statements:
	statements statement
	    {
		if (($1.t == 0)
		    || ($2.t == 0))
		{
#if DO_DEBUG
		    y2debug ("bad statements (%p) or statement (%p)", (const void *)$1.t, (const void *)$2.t);
#endif
		    $$.t = 0;
		    break;
		}

		if ($2.c == 0)			// empty statement
		{
#if DO_DEBUG
		    y2debug ("Empty statement");
#endif
		    $$.t = Type::Unspec;
		    break;
		}

		YStatementPtr stmt = static_cast<YStatementPtr>($2.c);
		YBlockPtr theblock = p_parser->m_block_stack->theBlock;
		constTypePtr typeofblock = theblock->type ();
#if DO_DEBUG
		y2debug ("STMT[%s!%s]\n", typeofblock->toString().c_str(), $2.t->toString().c_str());
		y2debug ("STMT[kind %d]\n", $2.c->kind());
		if (stmt) y2debug ("STMT[%s:%d]\n", stmt->toString().c_str(), stmt->line());
#endif

		if (!($2.t)->isUnspec ())			// return statement
		{
#if DO_DEBUG
		    y2debug ("Return in block: %s, typeofblock %s, declared_return_type %s", $2.t->toString().c_str(), typeofblock->toString().c_str(), declared_return_type->toString().c_str());
#endif
		    found_return_type = $2.t;

		    if (typeofblock->isUnspec ())	// type undefined yet
		    {
			if (!($2.t)->isNil())				// "return nil;" does not define the block type !
			{
#if DO_DEBUG
			    y2debug ("Block type (%s)", $2.t->toString().c_str());
#endif
			    theblock->setType ($2.t);
			    typeofblock = $2.t;
			}
		    }
		    else						// type is already defined, check it
		    {
			// default: no match
			int match = -1;

			// since nil (void) matches everything, handle this separately
			if ($2.t->isVoid())				// "return;"
			{
			    if (typeofblock->isVoid())
			    {
				match = 0;
			    }
			}
			else if ($2.t->isNil())				// "return nil;"
			{
			    match = 0;
			}
			else						// "return <expression>;"
			{
			    match = $2.t->match (typeofblock);
			}

			if (match > 0)
			{
			    ((YSReturnPtr)stmt)->propagate ($2.t, typeofblock);
			}
			else if (match < 0)
			{
			    yyTypeMismatch ("Mismatched return type in block", typeofblock, $2.t, $2.l);
			    $$.t = 0;
			    break;
			}
		    }
		}

		theblock->attachStatement (stmt);
		y2debug ("attached statement '%s'", stmt == 0 ? "<NULL>" : stmt->toString().c_str());
		$$.c = stmt;
		$$.t = typeofblock;
		$$.l = $1.l;
	    }
| /* empty  */
	    {
		$$.t = Type::Unspec;	// default type is unknown
		$$.c = 0;		// empty statement
		$$.l = LINE_NOW;
	    }
;

statement:
	';'
	    {
		$$.t = Type::Unspec;		// empty statement is allowed
		$$.c = 0;
	    }
|	SYM_NAMESPACE DCQUOTED_BLOCK
	    {
		start_block (p_parser, Type::Unspec);

		SymbolTable **saved_table = new SymbolTable *[2];
		saved_table[0] = p_parser->scanner()->globalTable();
		saved_table[1] = p_parser->scanner()->localTable();
		// evaluate following block in different name space
		// FIXME: maybe the SymbolEntry is not correct
		p_parser->scanner()->initTables (((YSymbolEntryPtr)$1.v.tval->sentry())->table(), 0);

		y2warning ("Using incompatible coversion");
		// save environment tables for later restore
		$2.v.val = (void *)saved_table;
	    }
	block_end
	    {
		// restore environment
		SymbolTable **saved_table = (SymbolTable **)$2.v.val;
		p_parser->scanner()->initTables (saved_table[0], saved_table[1]);
		delete[] saved_table;

		if ($4.t == 0)
		{
		    $$.t = 0;
		    break;
		}

		if ($4.c				// block not empty
		    && $4.c->isBlock())
		{
		    YBlockPtr block = (YBlockPtr)($4.c);
		    block->setKind (YBlock::b_using);
		    block->setName (string ($1.v.tval->key()));
		}
#if DO_DEBUG
		y2debug ("block_end");
#endif
		$$ = $4;
	    }
|	MODULE STRING ';'
	    {
		if (!blockstack_at_toplevel()) // must be outermost block
		{
		    yyLerror ("module statement in sub-block", $1.l);
		    $$.t = 0;
		    break;
		}

		if (p_parser->m_current_block->isModule())
		{
		    yyLerror ("duplicate module statement", $1.l);
		    $$.t = 0;
		    break;
		}

		const char *name = $2.v.sval;
		SymbolTable *globalTable = p_parser->scanner()->globalTable();
		if (globalTable->find (name, SymbolEntry::c_module) != 0)
		{
		    yyLerror ("module already declared", $1.l);
		    $$.t = 0;
		    break;
		}

		// Remember the name that will be used for a symbol
		// table entry when we finish the module block and so
		// that other syntax rules know we are in a module.

		p_parser->m_current_block->setKind (YBlock::b_module);
		p_parser->m_current_block->setName (name);

		// enter 'self' entry so namespace references to current module get ignored
		SymbolEntryPtr sself = new YSymbolEntry (0, 0, name, SymbolEntry::c_self, Type::Unspec);
		TableEntry *self = new TableEntry (sself->name(), sself, new Point (sself, $1.l, p_parser->m_current_block->point()));
		p_parser->scanner()->localTable()->enter (self);

		// module has private global table
		// globalTable has already been saved at IMPORT.

#if DO_DEBUG
		y2debug ("Create module table");
#endif

		globalTable = p_parser->m_current_block->table ();
		p_parser->scanner()->initTables (globalTable, 0);
#if DO_DEBUG
		y2debug ("overlaying globalTable %p", globalTable);
#endif

		inside_module = true;
		
		delete[] name; // SymbolEntry uses Ustring

		$$.c = 0;
		$$.t = Type::Unspec;
	    }
|	INCLUDE STRING ';'
	    {
	    
		// check, if it is not included yet in the current block
		
		// FIXME: where is the $2.v.sval freed?
		
		if (p_parser->m_block_stack->theBlock->isIncluded ($2.v.sval)) 
		{
#if DO_DEBUG
		    y2debug ("Skipping reinclude of the file %s in block", $2.v.sval);
#endif
		    $$.c = new YSInclude ($2.v.sval, $2.l, true);
		    $$.l = $1.l;
		    $$.t = Type::Unspec;
		    
		    delete[] $2.v.sval;

		    break;
		}

		// TODO better error reporting?
		// like: could not find foo.ycp in /include, /a/include.
		// It will return an empty string on failure
		string fn = YCPPathSearch::findInclude ($2.v.sval);
		if (fn.empty())
		{
		    yyFerror ($2.v.sval, $1.l);
		    delete[] $2.v.sval;
		    $$.t = 0;
		    break;
		}

#if DO_DEBUG
		y2debug ("include %s:%s", $2.v.sval, fn.c_str());
#endif
		int fd = open (fn.c_str(), O_RDONLY);
		if (fd < 0)
		{
		    yyFerror (fn.c_str(), $1.l);
		    delete[] $2.v.sval;
		    $$.t = 0;
		    break;
		}
		

		scannerstack_t *scanner = new (scannerstack_t);
		scanner->down = 0;
		scanner->filename = FILE_NOW;		// save current filename
		scanner->linenumber = $1.l;
		scanner->scanner = p_parser->scanner();
		scanner->state = SCAN_START_INCLUDE;	// see start_block()
		scanner->old_textdomain = p_parser->m_block_stack->textdomain;

		scannerstack_push (p_parser->m_scanner_stack, scanner);

#if DO_DEBUG
		y2debug ("new scanner at %s:%d, yychar [%d], now %p for %s", FILE_NOW.c_str(), $1.l, yychar, p_parser->scanner(), $2.v.sval);
#endif
		p_parser->m_block_stack->theBlock->newEntry ($2.v.sval, SymbolEntry::c_filename, Type::Unspec, $1.l);

		// we duplicate the value, so scanner gets its own instance
		p_parser->setScanner (new Scanner (fd, Scanner::doStrdup($2.v.sval)));

		// pass the outer scanner's tables
		p_parser->scanner()->initTables (scanner->scanner->globalTable(), scanner->scanner->localTable());

#if DO_DEBUG
		y2debug ("new scanner at %s:%d, yychar [%d], now %p for %s", FILE_NOW.c_str(), $1.l, yychar, p_parser->scanner(), $2.v.sval);
#endif
		$$.c = new YSInclude ($2.v.sval, scanner->linenumber);
		p_parser->m_block_stack->theBlock->addIncluded ($2.v.sval);
		$$.l = $1.l;
		$$.t = Type::Unspec;
		
		delete[] $2.v.sval;
	    }
|	IMPORT STRING ';'
	    {
		const char *name = $2.v.sval;
#if DO_DEBUG
		y2debug ("import '%s'", name);
#endif
		$$.c = 0;
		$$.l = $1.l;
		$$.t = Type::Unspec;

		// check existance of module

		TableEntry *tentry = p_parser->scanner()->localTable()->find (name, SymbolEntry::c_module);

		string module = name;
		delete [] name;

		if (tentry == 0)
		{
		    if (module == p_parser->m_current_block->name())
		    {
			yywarning("Ignoring self-import", $1.l);
			break;
		    }

		    ee.setLinenumber ($1.l);			// if YSImport logs an error
		    ee.setFilename (p_parser->scanner ()->filename ());
		    YSImportPtr imp = new YSImport (module, $1.l);
		    if (imp->name().empty())
		    {
			yyNoModule (module.c_str(), $1.l);
			$$.t = 0;
			break;
		    }

		    Y2Namespace *name_space = imp->nameSpace();
		    if (name_space == 0)
		    {
			yyNoModule (module.c_str(), $1.l);
			$$.t = 0;
			break;
		    }
		    
		    tentry = p_parser->m_block_stack->theBlock->newNamespace (module, name_space, $1.l);
		    if (tentry == 0)
		    {
			yyNoModule (module.c_str(), $1.l);
			$$.t = 0;
			break;
		    }
		    p_parser->scanner()->localTable()->enter (tentry);
		    $$.c = imp;
		}

	    }
|	FULLNAME STRING ';'
|	TEXTDOMAIN STRING ';'
	    {
		$$.t = Type::Unspec;
		YSTextdomainPtr c = new YSTextdomain ($2.v.sval, $1.l);		// YSTextdomain will copy $2.v.sval into an Ustring
		delete [] $2.v.sval;
		p_parser->m_block_stack->textdomain = c->domain();		// get the Ustring char pointer
		$$.c = c;
		$$.l = $1.l;
	    }
|	EXPORT identifier_list ';'
	    {
		$$.c = 0;
		$$.t = Type::Unspec;
	    }
|	TYPEDEF type identifier ';'
	    {
		if ($2.t->isUnspec())			// bad type
		{
		    $$.t = 0;
		    break;
		}

		if (!($3.t)->isUnspec ())		// known identifier
		{
		    yyLerror ("typedef symbol already declared", $3.l);
		    $$.t = 0;
		    break;
		}

		TableEntry *tentry = p_parser->m_block_stack->theBlock->newEntry ($3.v.nval, SymbolEntry::c_typedef, $2.t, $1.l);
		if (tentry == 0)		/* can't happen ... since we check above for known identifier */
		{
		    yyLerror ("typedef symbol duplicate", $3.l);
		    $$.t = 0;
		    break;
		}

		p_parser->scanner()->localTable()->enter (tentry);
		$$.c = new YSTypedef ($3.v.nval, $2.t, $1.l);
		$$.t = Type::Unspec;
	    }
|	definition
|	assignment ';'
	    {
		if ($1.t == 0)
		{
		    $$.t = 0;
		    break;
		}

		if (p_parser->m_block_stack->theBlock->isModule ())
		{
		    yyLerror ("Assignment not allowed in a module", $1.l);
		    $$.t = 0;
		    break;
		}
		$$.c = $1.c;
		$$.t = Type::Unspec;
		$$.l = $1.l;
	    }
|	function_call ';'
	    {
#if DO_DEBUG
		y2debug ("statement: function call");
#endif
		if ($1.t == 0)
		{
		    $$.t = 0;
		    break;
		}

		if (p_parser->m_block_stack->theBlock->isModule ())
		{
		    yyLerror ("Function call not allowed in a module", $1.l);
		    $$.t = 0;
		    break;
		}

		$$.c = new YSExpression ($1.c, $1.l);
		$$.t = Type::Unspec;
		$$.l = $1.l;
	    }
|	block
	    {
		if ($1.t == 0)
		{
		    $$.t = 0;
		    break;
		}

		// include block has code NULL
		if (p_parser->m_block_stack->theBlock->isModule () && $1.c != 0)
		{
		    yyLerror ("Block not allowed in a module", $1.l);
		    $$.t = 0;
		    break;
		}

		if ($1.c == 0)				// block empty
		{
		    $$.c = 0;
		    $$.t = Type::Unspec;
		    break;
		}

		$$.c = new YSBlock ($1.c, $1.l);
		$$.t = $1.t;				// might contain a 'return' statement
		$$.l = $1.l;
	    }
|	control_statement
	    {
		if ($1.t != 0
		    && p_parser->m_block_stack->theBlock->isModule ())
		{
		    yyLerror ("Statement not allowed in a module", $1.l);
		    $$.t = 0;
		    break;
		}
		$$ = $1;
	    }
|	CASE expression ':'
	    {
		if ($2.t == 0)
		{
		    $$.t = 0;
		    break;
		}
		
		// verify that we are in switch
		if (! p_parser->m_switch_stack)
		{
		    yyLerror ("case expression not allowed outside of switch statement", $1.l);
		    $$.t = 0;
		    break;
		}

		YCPValue val = $2.c->evaluate (true);
		if (val.isNull ())
		{
		    yyLerror ("case expression must be constant", $1.l);

		    $$.t = 0;
		    break;
		}

		YSSwitchPtr st = p_parser->m_switch_stack->statement;
		
		// check type
		if (st->conditionType ()->match ($2.c->type ()) != 0)
		{
		    yyTypeMismatch("Invalid constant type in the case statement"
			,st->conditionType (), $2.c->type (), $1.l);
		    $$.t = 0;
		    break;
		}
		
		bool non_duplicate = st->setCase (val);

		if (! non_duplicate)
		{
		    yyLerror ("Duplicate case expression", $1.l);
		    $$.t = 0;
		    break;
		}
		
		$$.t = Type::Void;
		$$.c = 0;	// no code
	    }
|	DEFAULT  ':'
	    {
		if ($2.t == 0)
		{
		    $$.t = 0;
		    break;
		}

		// verify that we are in switch
		if (! p_parser->m_switch_stack)
		{
		    yyLerror ("'default' not allowed outside of switch statement", $1.l);
		    $$.t = 0;
		    break;
		}

		bool non_duplicate = 
		    p_parser->m_switch_stack->statement->setDefaultCase ();

		if (! non_duplicate)
		{
		    yyLerror ("Duplicate 'default' expression", $1.l);
		    $$.t = 0;
		    break;
		}
		
		$$.t = Type::Void;
		$$.c = 0;	// no code
	    }
;

control_statement:
	IF '(' expression ')' statement opt_else
	    {
		if (($3.t == 0)			// bad expression
		    || ($5.t == 0)		// bad 'then' statement
		    || ($6.t == 0))		// bad 'else' statement
		{
		    $$.t = 0;
		    break;
		}

		if (!$3.t->isBoolean())
		{
		    yyTypeMismatch ("'if' expression not boolean", Type::Boolean, $3.t, $3.l);
		    $$.t = 0;
		    break;
		}

		if (($5.c != 0)				// 'then' statement not empty
		    && (($5.c->kind() == YCode::ysVariable)
			|| ($5.c->kind() == YCode::ysFunction)))
		{
		    yyLerror ("Declaration must be inside block", $5.l);
		    $$.t = 0;
		    break;
		}

		if ($6.c == 0)			// no else
		{
		    $$.c = new YSIf ($3.c, $5.c, $6.c, $1.l);
		    $$.t = $5.t;
		}
		else			// else branch given
		{
		    constTypePtr thentype = $5.t;
		    constTypePtr elsetype = $6.t;
		    
		    //There used to be a Type::Unspec -> Type::Void conversion here. It was wrong.
		    // See the comment about types at the "expression" rule.
		    
		    // if one of types is void (=nil), use the other one 
		    if (thentype->isNil ())
		    {
			$$.t = elsetype;
		    }
		    else if (elsetype->isNil ())
		    {
			$$.t = thentype;
		    }
		    else $$.t = thentype->commontype (elsetype);

		    if (false) ;					// FIXME
		    else if (($6.c->kind() == YCode::ysVariable)
			     || ($6.c->kind() == YCode::ysFunction))
		    {
			yyLerror ("Declaration must be inside block", $6.l);
			$$.t = 0;
		    }
		    else
		    {
			$$.c = new YSIf ($3.c, $5.c, $6.c, $1.l);
		    }
		}

		if ($5.c == 0)
		{
		    yywarning("Empty statement after 'if'", $1.l);
		}
	    }
|	WHILE '(' expression ')'
	    {
		if ($3.t == 0)
		{
		    $$.t = 0;
		    break;
		}

		p_parser->m_loop_count++;

		if (!$3.t->isBoolean())
		{
		    yyTypeMismatch ("'while' expression not boolean", Type::Boolean, $3.t, $3.l);
		    $$.t = 0;
		}
		else
		{
		    $$ = $3;
		}
	    }
	statement
	    {
		p_parser->m_loop_count--;
		if (($5.t == 0)
		    || ($6.t == 0))
		{
		    $$.t = 0;
		}
		else
		{
		    if (($6.c != 0)				// statement not empty
			&& (($6.c->kind() == YCode::ysVariable)
			    || ($6.c->kind() == YCode::ysFunction)))
		    {
			yyLerror ("Declaration must be inside block", $6.l);
			$$.t = 0;
		    }
		    else
		    {
			$$.c = new YSWhile ($5.c, $6.c, $1.l);
		    }
		    if ($6.c == 0)
		    {
			yywarning("Empty statement after 'while'", $1.l);
		    }
		    $$.t = $6.t;
		}
		$$.l = $1.l;
	    }
|	DO
	    {
		p_parser->m_loop_count++;
		do_while_count++;
	    }
	block
	    {
		p_parser->m_loop_count--;
		do_while_count--;
		if ($3.t == 0)
		{
		    $$.t = 0;
		    break;
		}

		if ($3.c == 0)
		    yywarning("Empty block after 'do'", $1.l);
	    }
	WHILE '(' expression ')' ';'
	    {
		if ($7.t == 0)
		{
		    $$.t = 0;
		    break;
		}

		if (!$7.t->isBoolean())
		{
		    yyTypeMismatch ("'do-while' expression not boolean", Type::Boolean, $7.t, $7.l);
		    $$.t = 0;
		}
		else
		{
		    $$.c = new YSDo ((YBlockPtr)$3.c, $7.c, $1.l);
		}
		$$.t = $3.t;
		$$.l = $1.l;
	    }
|	REPEAT
	    {
		p_parser->m_loop_count++;
		repeat_count++;
	    }
	block
	    {
		p_parser->m_loop_count--;
		repeat_count--;
		if ($3.t == 0)
		{
		    $$.t = 0;
		    break;
		}
		if ($3.c == 0)
		{
		    yywarning("Empty block after 'repeat'", $1.l);
		}
	    }
	UNTIL '(' expression ')' ';'
	    {
		if ($7.t == 0)
		{
		    $$.t = 0;
		    break;
		}

		if (!$7.t->isBoolean())
		{
		    yyTypeMismatch ("'repeat-until' expression not boolean", Type::Boolean, $7.t, $7.l);
		    $$.t = 0;
		    break;
		}
		else
		{
		    $$.c = new YSRepeat ((YBlockPtr)$3.c, $7.c, $1.l);
		}
		$$.t = $3.t;
		$$.l = $1.l;
	    }
|	BREAK ';'
	    {
		if (p_parser->m_loop_count <= 0
		    && p_parser->m_switch_stack == 0)
		{
		    yyLerror ("'break' outside of loop.", $1.l);
		    $$.t = 0;
		    break;
		}
		$$.c = new YSBreak ($1.l);
		$$.t = Type::Unspec;
		$$.l = $1.l;
	    }
|	CONTINUE ';'
	    {
		if (do_while_count > 0)
		{
		    if (getenv ("Y2CONTINUE") != 0) yywarning ("CONTINUE IN DO-WHILE", $1.l);
		}
		if (repeat_count > 0)
		{
		    if (getenv ("Y2CONTINUE") != 0) yywarning ("CONTINUE IN REPEAT", $1.l);
		}
		if (p_parser->m_loop_count <= 0)
		{
		    yyLerror ("'continue' outside of loop.", $1.l);
		    $$.t = 0;
		    break;
		}
		$$.c = new YSContinue ($1.l);
		$$.t = Type::Unspec;
		$$.l = $1.l;
	    }
|	RETURN ';'
	    {
		$$.t = Type::Void;			// differentiate "return;" from "return nil;" for type checking
		$$.c = new YSReturn ((YCodePtr)0, $1.l);
		$$.l = $1.l;
	    }
|	RETURN expression ';'
	    {
		if ($2.t == 0)
		{
		   $$.t = 0;
		   break;
		}
		$$.t = $2.t->isVoid() ? Type::Nil : $2.t;	// "return nil;" is of type 'Nil'
		$$.c = new YSReturn ($2.c, $1.l);
		$$.l = $1.l;
	    }
|	SWITCH '(' expression ')'
	    {
		// propagate errors
		if ($3.t == 0)
		{
		    $$.t = 0;
		    break;
		}
		
		YSSwitchPtr sw = new YSSwitch ($3.c);

		// push to stack
		switchstack_t* top = new switchstack_t;
		top->statement = sw;
		switchstack_push (p_parser->m_switch_stack, top);
		
		in_switch = true;
	    }
	block
	    {
		// cleanup, just in case
		in_switch = false;
		
		// pop from stack
		switchstack_t* top = switchstack_pop (p_parser->m_switch_stack);
		YSSwitchPtr sw = top->statement;
		delete top;

		// propagate errors
		if ($6.t == 0)
		{
		    $$.t = 0;
		    break;
		}

#if DO_DEBUG
		y2debug ("Finished switch: %s", sw->toString ().c_str ());
#endif
		$$.t = Type::Unspec;
		$$.c = sw;
	    }
;

opt_else:
	ELSE statement
	    {
		$$ = $2;
	    }
|	/* empty */
	    {
		$$.c = 0;
		$$.t = Type::Unspec;
	    }
;

/* -------------------------------------------------------------- */
/* types  */

type:
	C_TYPE				// type ($$.t) is set by scanner
					// C_TYPE includes already expanded typedefs
|	LIST				{ $$.t = Type::List; }
|	LIST '<' type_gt		{ $$.t = ListTypePtr ( new ListType ($3.t)); }
|	MAP				{ $$.t = Type::Map; }
|	MAP '<' type ',' type_gt	{ $$.t = MapTypePtr ( new MapType ($3.t, $5.t)); }
|	BLOCK '<' type_gt		{ $$.t = BlockTypePtr ( new BlockType ($3.t)); }
|	CONST type			{ TypePtr t = $2.t->clone(); t->asConst();
					  if (!t->isConst())  yywarning ("Bogus 'const'", $2.l);
					  $$.t = t;
					}
|	type '&'			{ TypePtr t = $1.t->clone(); t->asReference();
					  if (!t->isReference())  yywarning ("Bogus '&'", $1.l);
					  $$.t = t;
					}
|	type '(' ')'			{ $$.t = FunctionTypePtr ( new FunctionType ($1.t)); }
|	type '(' types ')'		{ $$.t = new FunctionType ( $1.t, (constFunctionTypePtr)$3.t); }
;

/* recognize "type >" vs "type >>" */
type_gt:
	type '>'			{ $$.t = $1.t; }
|	type RIGHT			{ yyLerror ("Missing blank between '>' and '>'", $2.l); $$.t = $1.t; }
;

types:
	type			{ FunctionTypePtr t = Type::Function(Type::Unspec);	// not a real function, just a container for the parameter types
				  t->concat ($1.t); $$.t = t; }
|	types ',' type		{ FunctionTypePtr t = $1.t->clone(); t->concat ($3.t); $$.t = t; }
;
/* -------------------------------------------------------------- */
/* Macro/Function or variable definition */

/*

 */
definition:
	opt_global DEFINE identifier '('
	    {
		yyLerror ("type specifier missing after 'define'", $2.l);
		$$.t = 0;
	    }
|	function_start ';'		/* function declaration */
	    {
		if ($1.t == 0)	// error in declaration, parameter block not on stack
		{
		    $$.t = 0;
		    break;
		}

		// back to old type restriction for current block
		declared_return_type = $1.t;

		// end parameter block
		//
		// pop block from block stack
		// unlink local symbols from symbol table

#if DO_DEBUG
		y2debug ("parameter block end");
#endif
		blockstack_t *top = blockstack_pop (p_parser->m_block_stack);
		top->theBlock->detachEnvironment (p_parser->scanner()->localTable());		// detach local table

		$$.c = 0;
		$$.t = Type::Unspec;
		$$.l = $1.l;
	    }
|	function_start block			/* function definition */
	    {
		// back to old type restriction for current block
		declared_return_type = $1.t;

		if ($1.t == 0)			// error in function_start, parameter block not on stack
		{
		    $$.t = 0;
		    declared_return_type = Type::Unspec;
		    y2debug ("error in function_start, parameter block not on stack");
		    break;
		}

		// end parameter block _after_ definition block
		//
		// pop block from block stack
		// unlink local symbols from symbol table

#if DO_DEBUG
		y2debug ("parameter block end");
#endif
		blockstack_t *top = blockstack_pop (p_parser->m_block_stack);
		top->theBlock->detachEnvironment (p_parser->scanner()->localTable());		// detach local table

		if ($2.t == 0)			// error in block
		{
		    $$.t = 0;
		    break;
		}
#if DO_DEBUG
		y2debug ("block type '%s'", $2.t->toString().c_str());
#endif
		if ($2.c == 0)						// empty function definition
		{
		    constTypePtr type = $1.v.tval->sentry ()->type ();
		    if (!type->isFunction () )
		    {
			y2internal ("Internal error: Expected function type, found %s", type->toString ().c_str());
			$$.t = 0;
			break;
		    }
		    
		    // get the return type
		    type = ((constFunctionTypePtr)type)->returnType ();
		    if (type->isVoid ())
		    {
			yywarning ("Empty function definition", $1.l);
			YBlockPtr block = new YBlock (p_parser->m_current_block->point());
			block->attachStatement (new YSReturn((YCodePtr)0, $1.l));
			$2.c = block;
		    }
		    else
		    {
			yyLerror ("Empty function definition for function not returning void", $1.l);
			$$.t = 0;
			break;
		    }
		}
		else
		{
		    if (!$2.t->isVoid()					// non-void function
			&& found_return_type->isUnspec())		// no 'return' statement found
		    {
			yyLerror ("No return statement in non-void function definition", $2.l);
			$$.t = 0;
			break;
		    }
		}
								// link the function entry with the function definition
		YSymbolEntryPtr entry = (YSymbolEntryPtr)$1.v.tval->sentry();
		YFunctionPtr func = (YFunctionPtr)(entry->code());
		func->setDefinition ((YBlockPtr)$2.c);
		$$.c = new YSFunction (entry, $1.l);
		$$.t = Type::Unspec;			// function def is typeless
		$$.l = $1.l;

#if DO_DEBUG
		y2debug ("Func (%s) done", $$.c->toString().c_str());
#endif
	    }
|	opt_global_identifier '=' expression ';'		/* variable definition */
	    {
		if (($1.t == 0)
		    || ($3.t == 0))
		{
#if DO_DEBUG
		    y2debug ("Bad identifier (%p) or expression (%p)", (const void *)$1.t, (const void *)$3.t);
#endif
		    $$.t = 0;
		    break;
		}

		check_void_assign (&($1), &($3));

		TableEntry *tentry = $1.v.tval;

		if (tentry->sentry()->isFunction()
		    || tentry->sentry()->isBuiltin())
		{
		    yyTerror ("variable definition shadows function", $1.l, tentry);
		    $$.t = 0;
		    break;
	//	    tentry->remove();
		}
		else
		{
		    int match = $3.t->match ($1.t);
#if DO_DEBUG
		    y2debug ("Assign '%s' = '%s' -> %d", $1.t->toString().c_str(), $3.t->toString().c_str(), match);
#endif
		    if (match < 0)				// no match
		    {
			if ($1.t->isBlock()			// lhs is block
			    && $3.c->isBlock())			// rhs is also block
			{
			    TypePtr bt = BlockTypePtr (new BlockType ($3.t));		// don't evaluate block
			    $3.c = new YEReturn ($3.c);

			    match = bt->match ($1.t);
			    if (match == 0)
			    {
				$3.t = bt;
			    }
			    else if (match > 0)
			    {
				$3.c = new YEPropagate ($3.c, $3.t, $1.t);
				$3.t = bt;
			    }
			    
			}
		        if (match < 0)
			{
			    yyTypeMismatch ("type mismatch in variable definition", $1.t, $3.t, $1.l);
			    $$.t = 0;
			    break;
//			    tentry->remove();
			}
		    }
		    else if (match > 0)		// propagated match
		    {
			ee.setLinenumber ($3.l);
			$3.c = new YEPropagate ($3.c, $3.t, $1.t);
			match = 0;
		    }
		    
		    YSymbolEntryPtr sentry = (YSymbolEntryPtr) tentry->sentry ();

		    if (match == 0)		// type match ok
		    {
			$$.c = new YSVariable (tentry->sentry(), $3.c, $1.l);
			sentry->setCode($3.c);
		    }
		    if (sentry->category() == SymbolEntry::c_unspec)
		    {
			sentry->setCategory ($1.t->isReference() ? SymbolEntry::c_reference : SymbolEntry::c_variable);
		    }
		}

		$$.l = $1.l;
		$$.t = Type::Unspec;
	    }
;


/*------------------------------------------------------
  function definition start
  [global] [define] type identifier '(' [type identifier]* ')

  enter function type+identifier to local/global symbol
  table.
  Enter (list of) formal parameters type+symbol to
  private symbol table to have them available when
  parsing the (perhaps following) definition block.

  $$.c = YFunction
  $$.v.tval = TableEntry() (->sentry->code() == YFunction
  $$.t = declared_return_type for current block
  $$.l = symbol definition line
*/

function_start:
	opt_global_identifier '(' tupletypes ')'
	    {
		if (($1.t == 0)
		    || ($3.t == 0))
		{
#if DO_DEBUG
		    y2debug ("Bad identifier (%p) or parameters (%p)", (const void *)$1.t, (const void *)$3.t);
#endif
		    $$.t = 0;
		    break;
		}

		/*
		   $1:	$1.v.tval == entry

		   $3 == tupletypes (linked list of table entries)
		 */

		// count and check formal parameters

		formalparam_t *formalp = $3.v.fpval;

		// start parameter block before parameter checking, it's popped in any case
#if DO_DEBUG
		y2debug ("start parameter block for '%s()'", ($1.t) ? $1.v.tval->sentry()->name() : "<err>");
#endif
		// if the scanner still think it's an include, teach him
		// this allows include without closed in a block
		if ((p_parser->m_scanner_stack != 0)
    		    && (p_parser->m_scanner_stack->state == SCAN_START_INCLUDE))
		{
		    p_parser->m_scanner_stack->state = SCAN_INCLUDE;
		}
		
		// start with the declared return type
		YBlockPtr parameter_block = start_block (p_parser, $1.t);

		// get the functions symbol entry
		YSymbolEntryPtr fentry = (YSymbolEntryPtr) $1.v.tval->sentry();

		// remember the prototype, if set for later checking
		constTypePtr prototype = Type::Unspec;
		if (fentry->onlyDeclared())
		{
		    prototype = fentry->type();
#if DO_DEBUG
		    y2debug ("prototype: %s", prototype->toString().c_str());
#endif
		}

		// it's a function
		fentry->setCategory (SymbolEntry::c_function);

		// create new function
		YFunctionPtr func = new YFunction (parameter_block, fentry);
		fentry->setCode (func);

		$$.c = func;
		$$.v.tval = $1.v.tval;
		$$.l = $1.l;

		// build function type

		FunctionTypePtr ftype (new FunctionType ($1.t));

		// save the current declared_return_type on the '$$' stack
		$$.t = declared_return_type;
		// start with this functions declared return type
		declared_return_type = $1.t;

		// used for tracking 'return' statements inside the function definition
		found_return_type = Type::Unspec;

		// loop through formalparam_t, adding each formal
		//  parameter to the function definition (private block)

		while (formalp != 0)				// while we have parameters
		{
#if DO_DEBUG
		    y2debug ("formal param '%s %s'@%d", formalp->type->toString().c_str(), formalp->name, formalp->line);
#endif

		    if (formalp->type->isReference()
			&& formalp->type->isAny())
		    {
			// #97956
			yyLerror ("Reference to 'any' not allowed", formalp->line);

			// to properly delete a function, also the corresponding SymbolEntry must be
			// adapted (so it does not try to access the YFunction anymore)
			fentry->setCategory (SymbolEntry::c_unspec);
			func = 0;

			$$.t = 0;
			break;
		    }

		    // compute function type

		    ftype->concat (formalp->type);

		    // create symbol entry for formal parameter

		    TableEntry *tentry = parameter_block->newEntry (formalp->name, formalp->type->isReference() ? SymbolEntry::c_reference : SymbolEntry::c_variable, formalp->type, $1.l);
		    if (tentry == 0)
		    {
			yyLerror ("Duplicate parameter", formalp->line);
			blockstack_pop (p_parser->m_block_stack);
			parameter_block->detachEnvironment (p_parser->scanner()->localTable());
			$$.t = 0;
			
			// to properly delete a function, also the corresponding SymbolEntry must be
			// adapted (so it does not try to access the YFunction anymore)
			fentry->setCategory (SymbolEntry::c_unspec);
			func = 0;
			break;
		    }
		    p_parser->scanner()->localTable()->enter (tentry);

		    formalparam_t *next = formalp->next;
		    delete formalp;
		    formalp = next;

		}  // while parameters present

		if (func != 0)			// no errors during parameter scan
		{

		    if (!prototype->isUnspec())			// if we had a prototype before
		    {
			if (! ftype->equals (prototype))		// check if definition is equivalent
			{
			    yyTerror ("Redeclaration with different type", $1.l, $1.v.tval);
			    yyTypeMismatch ("", prototype, ftype, $1.l);
			    blockstack_pop (p_parser->m_block_stack);
			    parameter_block->detachEnvironment (p_parser->scanner()->localTable());
			    fentry->setCategory (SymbolEntry::c_unspec);
			    $$.t = 0;
			    break;
			}
		    }

#if DO_DEBUG
		    y2debug ("func '%s'", func->toString().c_str());
		    y2debug ("ftype (%s:%s)", fentry->name(), ftype->toString().c_str());
#endif
		    fentry->setType (ftype);
#if DO_DEBUG
		    y2debug ("sentry (%s)", fentry->toString().c_str());
#endif
		}
	    }
;

/*--------------------------------------------------------------
  identifier, optionally prepended by 'global' or
  'define' or 'global define'
  $$.v.tval == entry
  $$.t = type
  $$.l = line of identifier
*/

opt_global_identifier:
	opt_global opt_define type identifier
	    {
		$$.t = $3.t;
		$$.l = $4.l;

		if (($4.t)->isUnspec ())
		{
		    // new symbol

#if DO_DEBUG
		    y2debug ("new %s symbol <%s>'%s'", ($1.v.bval) ? "global" : "local", $3.t->toString().c_str(), $4.v.nval);
#endif
		    $$.v.tval = p_parser->m_block_stack->theBlock->newEntry ($4.v.nval, ($1.v.bval) ? SymbolEntry::c_global : SymbolEntry::c_unspec, $3.t, $4.l);
		}
		else if ($4.v.tval->sentry()->onlyDeclared())
		{
		    SymbolEntryPtr fentry = $4.v.tval->sentry();

		    // declared, but not defined function (!) symbol
		    // check if the current declaration matches

		    if ($1.v.bval != (fentry->isGlobal()))
		    {
			yyTerror ("Redeclaration has different global scope", $4.l, $4.v.tval);
			$$.t = 0;
			break;
		    }

		    // onlyDeclared() above ensures that this is a function

		    constFunctionTypePtr ftype = (constFunctionTypePtr)(fentry->type());

		    if (ftype->returnType()->match ($3.t) != 0)
		    {
			yyTerror ("Redeclaration with different type", $4.l, $4.v.tval);
			yyTypeMismatch ("", ftype->returnType(), $3.t, $4.l);
			$$.t = 0;
			break;
		    }

		    $4.v.tval->makeDefinition ($4.l);
		    $$.v.tval = $4.v.tval;

		    break;					// don't re-enter it to the table !
		}
		else if ($1.v.bval)		// global redeclaration
		{
		    if ($4.v.tval->sentry()->isGlobal())
		    {
			yyTerror ("Redefinition of global symbol", $4.l, $4.v.tval);
			$$.t = 0;
			break;
		    }
		    else
		    {
			yyTerror ("Global definition shadows local symbol", $4.l, $4.v.tval);
			$$.t = 0;
			break;
		    }
		}
		else				// local redeclaration
		{
		    if ($4.v.tval->sentry()->isGlobal())
		    {
			if ($4.v.tval->sentry()->isBuiltin())
			{
			    yyTerror ("Definition shadows builtin", $4.l, $4.v.tval);
			    $$.t = 0;
			    break;
			}
			else
			{
			    yywarning ("Definition shadows global symbol", $4.l);
			    yyTwarning ($4.v.tval);
			    $$.v.tval = p_parser->m_block_stack->theBlock->newEntry ($4.v.tval->key(), SymbolEntry::c_unspec, $3.t, $4.l);
			}
		    }
		    else if ($4.v.tval->sentry()->nameSpace() == p_parser->m_block_stack->theBlock->nameSpace())
		    {
			yyTerror ("Redefinition of local symbol", $4.l, $4.v.tval);
			$$.t = 0;
			break;
		    }
		    else
		    {
			// yywarning ("Definition shadows local symbol", $4.l);

			$$.v.tval = p_parser->m_block_stack->theBlock->newEntry ($4.v.tval->key(), SymbolEntry::c_unspec, $3.t, $4.l);
		    }
		}

		if ($$.v.tval == 0)		// bad identifier, exit here
		{
#if DO_DEBUG
		    y2debug ("bad identifier");
#endif
		    $$.t = 0;
		    break;
		}

		if ($1.v.bval)
		{
#if DO_DEBUG
		    y2debug ("enter (%s) to global table %p", $$.v.tval->toString().c_str(), p_parser->scanner()->globalTable());
#endif
		    p_parser->scanner()->globalTable()->enter ($$.v.tval);
		}
		else
		{
#if DO_DEBUG
		    y2debug ("enter (%s) to local table %p", $$.v.tval->toString().c_str(), p_parser->scanner()->localTable());
#endif
		    p_parser->scanner()->localTable()->enter ($$.v.tval);
		}
	    }
;

opt_global:
	GLOBAL
	    {
		$$.v.bval = true;
		if (!blockstack_at_toplevel())
		{
		    yyLerror ("'global' declaration in nested block", $1.l);
#if DO_DEBUG
		    y2debug ("Nesting level is %d", p_parser->m_blockstack_depth);
#endif
		    $$.v.bval = false;
		}
		if (!p_parser->m_block_stack->theBlock->isModule())
		{
		    yywarning ("Useless 'global' outside of module", $1.l);
		    $$.v.bval = false;
		}
	    }
|	    { $$.v.bval = false; }
;

opt_define:
	DEFINE	{ $$.v.bval = true; }
|		{ $$.v.bval = false; }
;

/*----------------------------------------------*/
/* zero or more formal parameters		*/
/* $$.c = undef					*/
/* $$.t = Type::Unspec if error, any valid type otherwise	*/
/* $$.v.fpval = pointer to formalparam_t chain	*/

tupletypes:
	/* empty  */
	    {
		$$.v.val = 0;
		$$.t = Type::Void;
	    }
|	tupletype
;

/*----------------------------------------------*/
/* one or more formal parameters		*/
/* $$.v.fpval = pointer to formalparam_t chain	*/

tupletype:
	formal_param
|	tupletype ',' formal_param
	    {
		if (($1.t == 0)
		    || ($3.t == 0))
		{
#if DO_DEBUG
		   y2debug ("Bad tupletype (%p) or formal_param (%p)", (const void *)$1.t, (const void *)$3.t);
#endif
		   $$.t = 0;
		   break;
		}

		formalparam_t *formalp = $1.v.fpval;
		while (formalp->next != 0)		// find end of list
		{
		    formalp = formalp->next;
		}
		formalp->next = $3.v.fpval;		// attach to last element
		$$.v.fpval = $1.v.fpval;		// pointer to start of chain
		$$.t = Type::Void;
	    }
;

/*----------------------------------------------*/
/* single formal function parameter		*/
/* $$.v.fpval = pointer to formalparam_t	*/

formal_param:
	type identifier
	    {
		if ($2.t->isUnspec ())			// new identifier
		{
		    formalparam_t *fpval = new (formalparam_t);
		    fpval->next = 0;
		    fpval->name = $2.v.nval;
		    fpval->type = $1.t;
		    fpval->line = $2.l;
		    $$.v.fpval = fpval;
		    $$.t = fpval->type;
		    $$.l = $2.l;
	        }
		else					// known identifier, check and clone it
		{
		    SymbolEntryPtr entry = $2.v.tval->sentry();
		    switch (entry->category())
		    {
			case SymbolEntry::c_builtin:
			{
			    p_parser->m_parser_errors++;
			    p_parser->scanner()->logError ("Formal parameter '%s' shadows builtin function", $2.l, entry->name());
			    $$.t = 0;
			}
			break;
			case SymbolEntry::c_function:
			{
			    p_parser->m_parser_errors++;
			    p_parser->scanner()->logError ("Formal parameter '%s' shadows function", $2.l, entry->name());
			    $$.t = 0;
			}
			break;
			case SymbolEntry::c_unspec:
			{
			    p_parser->m_parser_errors++;
			    p_parser->scanner()->logError ("Parameter '%s' shadows function name", $2.l, entry->name());
			    $$.t = 0;
			}
			break;
			default:				// clone identifier
			{
			    formalparam_t *fpval = new (formalparam_t);
			    fpval->next = 0;

			    fpval->name = entry->name();
			    fpval->type = $1.t;
			    fpval->line = $2.l;

			    $$.v.fpval = fpval;
			    $$.t = fpval->type;
			}
			break;
		    }
		}
		$$.l = $2.l;
	    }
;
/* -------------------------------------------------------------- */
/* Assignment */

assignment:
	identifier '=' expression
	    {
		if (($1.t == 0)		// bad identifier
		    || ($3.t == 0))	// bad expression
		{
		    $$.t = 0;
		    break;
		}

		if ($1.t->isUnspec())	// undefined identifier
		{
		    yyVerror ($1.v.nval, $1.l);
		    $$.t = 0;
		    break;
		}

		if ($1.t->isConst ())
		{
		    yyConstAssignError ($1.v.tval->sentry()->name(), $1.l);
		    $$.t = 0;
		    break;
		}

		check_void_assign (&($1), &($3));

		int match = $3.t->match ($1.t);

		if (match < 0)
		{
		    if ($1.t->isBlock()			// lhs is block
			&& $3.c->isBlock())			// rhs is also block
		    {
			TypePtr bt = BlockTypePtr (new BlockType ($3.t));		// don't evaluate block

			match = bt->match ($1.t);
			if (match == 0)
			{
			    $3.t = bt;
			}
			else if (match > 0)
			{
			    $3.c = new YEPropagate ($3.c, $3.t, $1.t);
			    $3.t = bt;
			}
			$3.c = new YEReturn ($3.c);
		    }
		    if (match < 0)
		    {
			yyTypeMismatch ("type mismatch in assignment", $1.t, $3.t, $1.l);
			$$.t = 0;
			break;
//			tentry->remove();
		    }
		}

		if ($1.v.tval->sentry()->isFunction()
		    || $1.v.tval->sentry()->isBuiltin())
		{
		    // rhs identifier is a defined function
		    yyLerror ("Assignment to function", $1.l);
		    $$.t = 0;
		    break;
		}

		if (match > 0)
		{
		    ee.setLinenumber ($3.l);
		    $3.c = new YEPropagate ($3.c, $3.t, $1.t);
		}

		$$.c = new YSAssign ($1.v.tval->sentry(), $3.c, $1.l);
		$$.t = Type::Unspec;
		$$.l = $1.l;
	    }
|	identifier '[' list_elements ']' '=' expression
	    {
		$$.t = 0;				// default: error

		if (($1.t == 0)
		    || ($3.t == 0)
		    || ($6.t == 0))
		{
		    break;
		}

		if ($1.t->isUnspec ())			// undeclared identifier
		{
		    yyVerror ($1.v.nval, $1.l);
		    break;
		}

		check_void_assign (&($1), &($6));

		if (!$1.t->isList()
		     && !$1.t->isMap()
		     && !$1.t->isTerm())
		{
		    yyLerror ("bracket operator requires list, map, or term identifier", $1.l);
		    break;
		}

		if ($1.t->isConst())
		{
		    yyConstAssignError ($1.v.tval->sentry()->name(), $1.l);
		    break;
		}

		    // try to determine the type as far as possible, following the list of arguments,
		    // doing a type check as we go
		    // come out with $$.t == 0 if error, else determined type

		    // the currently tested structured type
		    constTypePtr cur = $1.t;

		    // index into YEList of bracket parameters, the list cannot be empty
		    int index = 0;
		    YEListPtr params = (YEListPtr)$3.c;

		    do
		    {
			constTypePtr paramType = params->value (index)->type ();	// type of bracket parameter at index
#if DO_DEBUG
			y2debug ("paramvalue (%d) '%s'", index, params->value(index)->toString().c_str());
			y2debug ("paramType '%s'", paramType->toString().c_str());
#endif
			if (paramType->isFunction())
			{
			    paramType = ((constFunctionTypePtr)paramType)->returnType ();
#if DO_DEBUG
			    y2debug ("paramType is function returning '%s'", paramType->toString().c_str());
#endif
			}
			
			// for lists, only integer is acceptable
			if (cur->isList ())
			{
			    if (! paramType->isInteger ())
			    {
				yyTypeMismatch ("Bracket parameter has wrong type", Type::Integer, paramType, $1.l);
				cur = 0;
				break;
			    }
			    else
			    {
				cur = ((constListTypePtr)cur)->type ();
			    }
			}
			else if (cur->isMap ())
			{
			    if (paramType->match (((constMapTypePtr)cur)->keytype ()) == -1)
			    {
				yyTypeMismatch ("Bracket parameter has wrong type", ((constMapTypePtr)cur)->keytype (), paramType, $1.l);
				cur = 0;
				break;
			    }
			    else
			    {
				cur = ((constMapTypePtr)cur)->valuetype ();
			    }
			}
			else if (cur->isTerm ())
			{
			    if (! paramType->isInteger ())
			    {
				yyTypeMismatch ("Bracket parameter has wrong type", Type::Integer, paramType, $1.l);
				cur = 0;
				break;
			    }
			    else
			    {
				cur = Type::Any;
			    }
			}

			index++;

		    } while (index < params->count ()
			     && (cur->isList () || cur->isMap ()));

		    // quit on error
		    if (cur == 0)
		    {
			$$.t = 0;
			break;
		    }

		    int match = $6.t->match (cur);
		    if (match > 0)
		    {
			$6.c = new YEPropagate ($6.c, $6.t, cur);
			match = 0;
		    }
		    if (match != 0)
		    {
			yyTypeMismatch ("type mismatch in bracket assignment", cur, $6.t, $1.l);
		    }
#if 0
		    if (index < params->count ())		// we hit a non-list/non-map before end of bracket
		    {
			$$.t = Type::Any;			// we can't say anything about the resulting type, must propagate to default type
		    }
		    else 
		    {
			$$.t = cur;
		    }
#endif
		$$.c = new YSBracket ($1.v.tval->sentry(), $3.c, $6.c, $1.l);
		$$.t = Type::Unspec;
		$$.l = $1.l;
	    }
;

/* ----------------------------------------------------------*/

/* allow multi line strings  */
string:
	STRING
|	string STRING
	    {
		int s1len = strlen ($1.v.sval);
		int s2len = strlen ($2.v.sval);
		char *s = new char [s1len + s2len + 1];
		strcpy (s, $1.v.sval);
		strcpy (s + s1len, $2.v.sval);
		delete[] $1.v.sval;
		delete[] $2.v.sval;
		$$.v.sval = s;
		$$.l = $1.l;
	    }
;

constant:
	C_VOID
|	C_BOOLEAN
|	C_INTEGER
|	C_FLOAT
|	STRING		/* can't use 'string' here, because it needs lookahead and hence is no 'compact'_expression  */
	    {
		/* convert to ycp value, like all other constants  */
		$$.c = new YConst (YCode::ycString, YCPString ($1.v.sval));
		delete[] $1.v.sval;
		$$.t = Type::String;
		$$.l = $1.l;
	    }
|       C_BYTEBLOCK
|	C_PATH
|	C_SYMBOL
;

/* -------------------------------------------------------------- */
/* List expressions */

list:
	'[' ']'
	    {
		$$.c = new YConst (YCode::ycList, YCPList());
		$$.t = Type::ListUnspec;			// make it different from list(any) !
		$$.l = $1.l;
	    }
|	'[' list_elements opt_comma ']'
	    {
		if ($2.t == 0)
		{
		    $$.t = 0;
		    break;
		}

		YCPValue list = $2.c->evaluate (true);
		if (list.isNull())
		{
		    $$.c = $2.c;
		}
		else if (list->isCode())
		{
		    $$.c = list->asCode()->code();
		}
		else if (list->isList())
		{
		    $$.c = new YConst (YCode::ycList, list->asList());
		}
		else
		{
		    yyLerror ("YEList does not evaluate to YCPList but", $2.l);
		    yyLerror (list->toString().c_str(), $2.l);
		    $$.t = 0;
		    break;
		}
		$$.t = ListTypePtr (new ListType ($2.t));
		$$.l = $1.l;
	    }
;

list_elements:
	expression
	    {
		if ($1.t == 0)
		{
		    $$.t = 0;
		    break;
		}
		$$.c = new YEList ($1.c);
		$$.t = $1.t;
		$$.l = $1.l;
	    }
|	list_elements ',' expression
	    {
		if (($1.t == 0)
		    || ($3.t == 0))
		{
		    $$.t = 0;
		    break;
		}

		$$.t = $1.t->commontype ($3.t);
		((YEListPtr)$1.c)->attach ($3.c);
		$$.c = $1.c;
		$$.l = $1.l;
	    }
;

	/* optional comma  */
opt_comma:
	','
|
;

/* -------------------------------------------------------------- */
/* Map expressions */

map:
	MAPEXPR ']'					/* empty map */
	    {
		$$.c = new YConst (YCode::ycMap, YCPMap());
		$$.t = Type::MapUnspec;			// make it different from map<any,any> !
		$$.l = $1.l;
	    }
|	MAPEXPR map_elements opt_comma ']'
	    {
		if ($2.t == 0)
		{
		    $$.t = 0;
		    break;
		}

		YCPValue map = $2.c->evaluate (true);
		if (map.isNull())	// not a constant
		{
		    $$.c = $2.c;
		    $$.t = $2.t;
		}
		else if (map->isCode())	// expression (can this happen ?)
		{
		    $$.c = map->asCode()->code();
		}
		else			// YCPMap constant
		{
		    $$.c = new YConst (YCode::ycMap, map->asMap());
		}

		$$.t = $2.t;
		$$.l = $1.l;
	    }
;

// $$.t == MapTypePtr()
map_elements:
	expression ':' expression
	    {
		if (($1.t == 0)
		    || ($3.t == 0))
		{
		    $$.t = 0;
		    break;
		}
		if (!($1.t->isInteger()
		    || $1.t->isString()
		    || $1.t->isSymbol()))
		{
		    yyLerror ("Bad type for key", $1.l);
		    $$.t = 0;
		    break;
		}
		$$.c = new YEMap ($1.c, $3.c);
		$$.t = MapTypePtr (new MapType ($1.t, $3.t));
		$$.l = $1.l;
	    }
|	map_elements ',' expression ':' expression
	    {
		if (($1.t == 0)
		    || ($3.t == 0)
		    || ($5.t == 0))
		{
		    $$.t = 0;
		    break;
		}
		if (!($3.t->isInteger()
		    || $3.t->isString()
		    || $3.t->isSymbol()))
		{
		    //yyCerror($3.c, $3.t, $3.l);
		    yyLerror ("Bad type for key", $3.l);
		    $$.t = 0;
		    break;
		}

		constMapTypePtr mt = $1.t;
		constTypePtr keytype = mt->keytype()->commontype ($3.t);
		constTypePtr valuetype = mt->valuetype()->commontype ($5.t);
		$$.t = MapTypePtr (new MapType (keytype, valuetype));

		((YEMapPtr)$1.c)->attach ($3.c, $5.c);
		$$.c = $1.c;
		$$.l = $1.l;
	    }
;

/* -------------------------------------------------------------- */
/*
   Function call

   initial parse of 'term_name (' triggers first type checking
   and lookup of term_name so parameters can be checked against
   prototype.

   function_call: term_name[$1] '('[2] {lookup prototype}[$3] parameters[$4] ')'[$5] {check parameters}

*/

function_call:
	function_name '('
	    {
		if ($1.t == 0)
		{
		    $$.t = 0;
		    break;
		}

		/* this is $3  */

		if ($1.t->isUnspec ())					// bad function_name
		{
		    yyVerror ($1.v.nval, $1.l);
		    $$.t = 0;
		    break;
		}

		if ($1.t->isSymbol())					// function_name is C_SYMBOL
		{
#if DO_DEBUG
		    y2debug ("Term %s(...)", $1.v.nval);
#endif
		    /* C_SYMBOL  */
		    $$.c = new YETerm ($1.v.nval);
		    $$.t = Type::Term;
		}
		else							// function_name is function or builtin
		{


		    SymbolEntryPtr sentry = $1.v.tval->sentry();

		    if (sentry->isBuiltin())	// a builtin function
		    {
			// found builtin declaration

			// check for overloads, those must be re-checked
			// for every parameter in order to match the
			// type-correct declaration

			declaration_t *decl = ((YSymbolEntryPtr)sentry)->declaration();

#if DO_DEBUG
//			y2debug ("Builtin! (%s)%s", sentry->toString().c_str(), (decl->next == 0) ? "!" : "?");
#endif
			$$.v.val = decl;
			if ((decl->tentry->isOverloaded())				// if overloaded
			    || (decl->flags & DECL_SYMBOL))		// or can have a symbol as parameter
			{
			    // if the scanner still think it's an include, teach him
			    // this allows include without closed in a block
			    if ((p_parser->m_scanner_stack != 0)
    				&& (p_parser->m_scanner_stack->state == SCAN_START_INCLUDE))
			    {
				p_parser->m_scanner_stack->state = SCAN_INCLUDE;
			    }
			    // start block for possible symbol parameters
			    YBlockPtr block = start_block (p_parser, Type::Unspec);
#if DO_DEBUG
			    y2debug ("opening parameter scope for %s", sentry->name());
#endif

			    $$.c = new YEBuiltin (decl, block);
#if DO_DEBUG
			    y2debug ("Builtin with block ...");
#endif
			}
			else
			{
			    $$.c = new YEBuiltin (decl);
#if DO_DEBUG
			    y2debug ("Builtin ...");
#endif
			}

			if (decl->flags & DECL_LOOP)			// allow break in code parameter
			{
			    p_parser->m_loop_count++;
			}
			$$.t = decl->type;

		    }
		    else if (sentry->type()->isFunction())
		    {
#if DO_DEBUG
			y2debug ("Doing function call, starting params");
#endif
			if (sentry->isVariable ())
			{
#if DO_DEBUG
			    y2debug ("Doing function call via function pointer, starting params");
#endif
//			    y2internal ("Doing function call via function pointer, starting params");
			    $$.c = new YEFunctionPointer ($1.v.tval);		// an extern function
			}
			else
			{
			    $$.c = new YEFunction ($1.v.tval);			// an extern function
			}
			$$.t = sentry->type();
#if DO_DEBUG
			y2debug ("Function! (%s)", sentry->toString(true).c_str());
#endif
		    }
		    else
		    {
			yyTerror ("Identifier is not a function", $1.l, $1.v.tval);
			break;
		    }
		}

		/* end of $3 */
	    }
	parameters ')'
	    {
		/* $3 == function
		   check $3.c for (YETerm, YEBuiltin, YEFunction)
		   $3.v.val == first matching decl for function name

		   $4.t == 0 if 'parameters' bad
		   $4.c == 0 if 'parameters' empty
		 */

		if (($3.t == 0)
		    || ($4.t == 0))			// error in parameters
		{
#if DO_DEBUG
		    y2debug ("Bad function (%p) or parameters %p)", (const void *)$3.t, (const void *)$4.t);
#endif
		    $$.t = 0;

		    // check if symfunc parameter block must be popped from stack
		    
		    if (($3.t != 0)
			&& ($3.c->kind() == YCode::yeBuiltin)
			&& (((YEBuiltinPtr)$3.c)->parameterBlock() != NULL))
		    {
			// end block for possible symbol parameters

			blockstack_t *top = blockstack_pop (p_parser->m_block_stack);
			top->theBlock->detachEnvironment (p_parser->scanner()->localTable());		// detach local table
			YEBuiltinPtr bf = (YEBuiltinPtr)($3.c);
#if DO_DEBUG
			y2debug ("closing parameter scope block for %s", bf->decl() ? bf->decl()->name : "<err>");
#endif
			bf->closeParameters();			// delete parameter block if not needed
		    }
		    break;
		}

		switch ($3.c->kind())		// depends on what function_name is
		{
		    case YCode::yeTerm:			// a plain term
		    {
#if DO_DEBUG
			y2debug ("YCode::yeTerm");
#endif
			$$.c = $3.c;
			$$.t = $3.t;
		    }
		    break;
		    case YCode::yeBuiltin:			// a builtin function
		    {
#if DO_DEBUG
			y2debug ("YCode::yeBuiltin");
#endif
			// yeBuiltin matched
			//   do final check for parameters

			YEBuiltinPtr builtin = (YEBuiltinPtr)$3.c;

			if (builtin->decl()->flags & DECL_LOOP)
			{
			    p_parser->m_loop_count--;
			}

			constTypePtr finalT = builtin->finalize (p_parser->scanner ());
			if (finalT != 0)
			{
			    constFunctionTypePtr bt = 0;
			    constFunctionTypePtr dt = 0;
			    if (builtin->completeType()->isFunction())
			    {
				bt = builtin->completeType();
			    }
			    if (builtin->decl()->type->isFunction())
			    {
				dt = builtin->decl()->type;
			    }

			    if (finalT != 0)				// error not shown yet
			    {
				constTypePtr seen = bt ? (constTypePtr)(bt->parameters()) : builtin->type();
				if (! seen)
				    seen = new FunctionType();

				constTypePtr expected;
				if (finalT->isError())
				{
				    expected = dt ? (constTypePtr)(dt->parameters()) : builtin->decl()->type;
				}
				else if (finalT->isFunction())
				{
				    dt = finalT;
				    expected = dt->parameters();
				}
				else
				{
				    expected = finalT;
				}
				yyTypeMismatch ((string ("Wrong parameters in call to ") + string (builtin->decl()->name) + string ("(...)")).c_str(), expected, seen, $4.l);
			    }

			    $$.t = 0;
			}
			else
			{
#if DO_DEBUG
			    y2debug ("yeBuiltin matched");
#endif
			    $$.c = $3.c;
			    $$.t = builtin->type ();

			    if (builtin->decl ()->flags & DECL_DEPRECATED)
			    {
				yywarning ((string (builtin->decl ()->name) + "(...) is deprecated, please fix").c_str(), $1.l);
			    }

			}
		    }
		    break;
		    case YCode::yeFunctionPointer:
		    case YCode::yeFunction:			// an extern function
		    {
#if DO_DEBUG
			y2debug ("YCode::yeFunction (%s)", $3.c->toString().c_str());
#endif
			YECallPtr function = (YECallPtr)$3.c;

			// close parameter list
			constTypePtr finalT = function->finalize ();
			if (finalT != 0)
			{
			    yyLerror ("Parameters don't match any declaration:", $2.l);
			    yyLerror (function->toString().c_str(), $2.l);
        		    yyLerror ("Candidates are:", $2.l);

			    // enumerate all possible calls
			    TableEntry* tentry = $1.v.tval;
			    while (tentry)
			    {
				yyLerror (tentry->sentry ()->toString ().c_str (), $2.l);
				tentry = tentry->next_overloaded ();
			    }
			    $$.t = 0;
			}
			else
			{
			     // yeFunction matched
#if DO_DEBUG
			     y2debug ("yeFunction matched");
#endif
			     $$.c = $3.c;
			     constFunctionTypePtr ft = $3.t;
			     $$.t = ft->returnType ();
			}

		    }
		    break;
		    default:				// anything else is an error
		    {
#if DO_DEBUG
			y2debug ("Error");
#endif
			$$.t = 0;
		    }
		    break;

		} // switch ($3.c->kind())

		// check if symfunc parameter block must be popped from stack

		if (($3.c->kind() == YCode::yeBuiltin)
		    && (((YEBuiltinPtr)$3.c)->parameterBlock() != NULL))
		{
		    // end block for possible symbol parameters

		    blockstack_t *top = blockstack_pop (p_parser->m_block_stack);
		    top->theBlock->detachEnvironment (p_parser->scanner()->localTable());		// detach local table
		    YEBuiltinPtr bf = (YEBuiltinPtr)($3.c);
#if DO_DEBUG
		    y2debug ("closing parameter scope block for %s", bf->decl() ? bf->decl()->name : "<err>");
#endif
		    bf->closeParameters();			// delete parameter block if not needed
		}

		if ($$.t == 0)
		{
		    break;
		}

		$$.l = $1.l;

#if DO_DEBUG
		y2debug ("fcall (%s:%s)", $$.t->toString().c_str(), $$.c->toString().c_str());
#endif
	    }
;

/*
   function call parameters

   attach parameters directly to function, thereby using the type information
   from the function in deciding how to treat parameters.

   since we're using the $0 feature of bison here, we can't
   split up this BNF further :-(

   $0 refers to $3 of the 'function_call' rule, ie $0.c is the function (one of 4 kinds)

   return $$.t == 0 on error, $$.c == 0 if empty
 */

parameters:
	/* empty  */
	    {
#if DO_DEBUG
		y2debug ("Empty parameters");
#endif
		$$.c = 0;
		$$.t = Type::Unspec;
	    }
|	type identifier
	    {
#if DO_DEBUG
		y2debug ("parameters: type/name ([%s '%s'])", $1.t->toString().c_str(), $2.t->isUnspec () ? $2.v.nval : $2.v.tval->key());
#endif
		/* if the function was not found, better fail now */
		if ($0.t != 0)
		{

		    /* $1.t == type, $2.v symbol*/
		    constTypePtr t = attach_parameter (p_parser, $0.c, &($1), &($2));
		    if (t != 0)
		    {
			yyLerror ("Parameter error", $2.l);
			$$.t = 0;
			break;
		    }
		}

		$$.c = $0.c;
		$$.t = $1.t;
		$$.l = $1.l;
	    }
|	expression
	    {
#if DO_DEBUG
		y2debug ("parameters: expression");
#endif
		if ($1.t == 0)			// parameter 'expression' is bad
		{
#if DO_DEBUG
		    y2debug ("parameter 'expression' is bad");
#endif
		    $$.t = 0;
		    break;
		}

		/* if the function was not found, better fail now */
		if ($0.t != 0)
		{
		    // attach parameter ($1) to function ($0), checking types

		    constTypePtr t = attach_parameter (p_parser, $0.c, &($1));

		    if (t != 0)
		    {
#if DO_DEBUG
			y2debug ("attach_parameter failed");
#endif
			$$.t = 0;
			break;
		    }
		}

		$$.c = $1.c;		// default return value
		$$.t = $1.t;
	    }
|	parameters ',' type identifier
	    {
#if DO_DEBUG
		y2debug ("parameters: parameters, type/name");
#endif
		if ($1.t == 0)
		{
		    $$.t = 0;
		    break;
		}

		if ($1.c == 0)
		{
		    yyLerror ("Missing expression before ','", $1.l);
		    $$.t = 0;
		    break;
		}

		/* if the function was not found, better fail now */
		if ($0.t != 0)
		{

		    /* $3.t == type, $4 symbol*/
		    constTypePtr t = attach_parameter (p_parser, $0.c, &($3), &($4));
		    if (t != 0)
		    {
			$$.t = 0;
			break;
		    }
		}

		$$.c = $0.c;
		$$.t = Type::Unspec;
	    }
|	parameters ',' expression
	    {
		if (($1.t == 0)
		    || ($3.t == 0))
		{
		    $$.t = 0;
		    break;
		}

		if ($1.c == 0)
		{
		    yyLerror ("Missing expression before ','", $1.l);
		    $$.t = 0;
		    break;
		}
		
		if ($3.c == 0)
		{
		    yyLerror ("Empty expression (block) as a parameter", $1.l);
		    $$.t = 0;
		    break;
		}

		/* if the function was not found, better fail now */
		if ($0.t != 0 )
		{
		    constTypePtr t = attach_parameter (p_parser, $0.c, &($3));
		    if (t != 0)
		    {
			$$.t = 0;
			break;
		    }
		}

		$$.c = $1.c;
		$$.t = Type::Unspec;
	    }
;

/* -------------------------------------------------------------- */
/*
   function name

   might be a known identifier (normal function call)
   or a symbol constant (YCP Term)

/* -> $$.v.tval == TableEntry if symbol already declared ($$.t != Type::Unspec)
      $$.v.nval == charptr if symbol undefined ($$.t == Type::Unspec)
      $$.t = Type::Unspec for SYMBOL, "|" for builtin, else type
 */

function_name:
	identifier
|	C_SYMBOL
	    {
		$$.c = $1.c;
		$$.t = Type::Symbol;
		$$.l = $1.l;
	    }
;

/* -------------------------------------------------------------- */
/* Identifiers (KNOWN and UNKNOWN symbols) */
/* -> $$.v.tval == TableEntry if symbol already declared ($$.t != Type::Unspec)
      $$.v.nval == charptr if symbol undefined ($$.t == Type::Unspec)
      $$.t = Type::Unspec for SYMBOL, "|" for builtin, else type
 */

identifier:
	IDENTIFIER
|	SYMBOL
	    {
#if DO_DEBUG
		y2debug ("<new> symbol '%s' !", (const char *)$1.v.nval);
#endif
		$$.v.nval = $1.v.nval;
		$$.t = Type::Unspec;
		$$.l = $1.l;
	    }
;

identifier_list:
	identifier
|	identifier ',' identifier_list
;
/* ---------------------------------------------------------------------- */
%%


/*
  I define my own yylex, which makes scanner and parser reentrant.

  lvalp_void is a void pointer to yylval (the value of the lexical token)
  void_pr is a pointer to 'our' parser
*/

extern "C" {
int yylex(YYSTYPE *lvalp_void, void *void_pr)
{
    // get 'our' parser
    Parser *pr = (Parser *)void_pr;
    Scanner *currentScanner = pr->scanner ();

    // call 'our' scanner through the parser
    int token = currentScanner->yylex();

    while (token == END_OF_FILE)
    {
	scannerstack_t *top = scannerstack_pop (pr->m_scanner_stack);		// eof of include ?
#if DO_DEBUG
	y2debug ("EOF, top %p, current %p, yychar ?\n", top, currentScanner);
#endif
	if (top == 0)
	{
	    break;								// no
	}
#if DO_DEBUG
	y2debug ("EOF, back to %s:%d\n", top->filename.c_str(), top->linenumber);
#endif
	// close the old file
	pr->scanner ()->closeInput ();
	
	currentScanner = top->scanner;
	pr->setScanner (currentScanner);
	
	// restore the text domain
	pr->m_block_stack->textdomain = top->old_textdomain;
	
	YSFilename* fn = new YSFilename (top->filename);
	pr->m_current_block->attachStatement (fn);
	
	delete top;			    // free scannerstack_t new'd at INCLUDE
	token = currentScanner->yylex();
    }

    if (token != END_OF_FILE)
    {
	// store the value of the lexical token
	YCodePtr *store_here = (YCodePtr *) &(lvalp_void->c);
	lvalp_void->t	   = currentScanner->scannedType();

	tokenValue value   = currentScanner->scannedValue();
	lvalp_void->v	   = value;
	pr->m_lineno	   = lvalp_void->l = currentScanner->lineNumber();

	switch (token)
	{
	    case C_FLOAT:	*store_here = new YConst (YCode::ycFloat,	YCPFloat (value.fval)); break;
	    case C_INTEGER:	*store_here = new YConst (YCode::ycInteger,	YCPInteger (value.ival)); break;
	    case C_BYTEBLOCK:
	    {
		long length;
		unsigned char *ptr = value.cval + sizeof (long);
		memcpy (&length, value.cval, sizeof (long));
		*store_here = new YConst (YCode::ycByteblock, YCPByteblock (ptr, length));
	    }
	    break;
	    case C_VOID:	*store_here = new YConst (YCode::ycVoid,	YCPVoid ()); break;
	    case C_BOOLEAN:	*store_here = new YConst (YCode::ycBoolean,	YCPBoolean (value.bval)); break;
	    case C_PATH:
	    {
		YCPPath path = YCPPath (value.pval);
		if ((path->length() == 0)
		    && (strlen (value.pval) > 1))
		{
		    yyerror_with_lineinfo (pr, -1, "not a path constant");
		    return 0;
		}
		*store_here = new YConst (YCode::ycPath, path);
	    }
	    break;
	    case C_SYMBOL:
	    {
		*store_here = new YConst (YCode::ycSymbol, YCPSymbol (value.yval));
	    }
	    break;
	    case SCANNER_ERROR:
	    {
		*store_here = 0;
		lvalp_void->t = 0;
	    }
	    break;
	}
    }
    return token;
}
} // extern "C"


static void
yyerror_with_lineinfo (Parser *parser, int lineno, const char *s)
{
    parser->m_parser_errors++;
    parser->scanner()->logError (s, (lineno > 0) ? lineno : parser->m_lineno);
}


static void
yywarning_with_lineinfo (Parser *parser, int lineno, const char *s)
{
    parser->scanner()->logWarning (s, (lineno > 0) ? lineno : parser->m_lineno);
}


static void
yyerror_with_code (Parser *parser, int lineno, YCodePtr c, constTypePtr t)
{
    parser->m_parser_errors++;
    if (c == 0)
    {
	parser->scanner()->logError ("Bad parameter", (lineno > 0) ? lineno : parser->m_lineno);
    }
    else
    {
	parser->scanner()->logError ("Bad parameter '<%s> %s'", (lineno > 0) ? lineno : parser->m_lineno, t->toString().c_str(), c ? c->toString().c_str() : "<err>");
    }
}


static void
yyerror_with_name (Parser *parser, int lineno, const char *name)
{
    parser->m_parser_errors++;
    parser->scanner()->logError ("Undeclared identifier '%s'", (lineno > 0) ? lineno : parser->m_lineno, name);
}


static void
yyerror_assign_const (Parser *parser, int lineno, const char *name)
{
    parser->m_parser_errors++;
    if (*name == 0)
	parser->scanner()->logError ("Assignment to const", (lineno > 0) ? lineno : parser->m_lineno);
    else
	parser->scanner()->logError ("Assignment to const identifier '%s'", (lineno > 0) ? lineno : parser->m_lineno, name);
}


static void
yyerror_with_file (Parser *parser, int lineno, const char *name)
{
    parser->m_parser_errors++;
    parser->scanner()->logError ("Bad or unknown file '%s'", (lineno > 0) ? lineno : parser->m_lineno, name);
}


static void
yyerror_with_tableentry (Parser *parser, int lineno, const char *s, TableEntry *tentry)
{
    parser->m_parser_errors++;
    int line = (lineno > 0) ? lineno : parser->m_lineno;
    parser->scanner()->logError (s, line);
    const Point *point = tentry->point();
    bool start = true;
    while (point != 0)
    {
	const Point *next = point->point();
	if (start)
	{
	    if (next == 0)
	    {
		parser->scanner()->logError ("Definition point (%s:%d) without parent\n", line, point->filename().c_str(), point->line());
	    }
	    // definition point
	    else if (point->line() == 0)
	    {
		parser->scanner()->logError ("'%s' defined as %s.", line, tentry->key(), next->filename().c_str());
	    }
	    else
	    {
		parser->scanner()->logError ("'%s' defined in %s:%d.", line, tentry->key(), next->filename().c_str(), point->line());
	    }
	}
	else if (point->line() != 0)
	{
	    if (next == 0)
	    {
		parser->scanner()->logError ("Inclusion point (%s:%d) without parent\n", line, point->filename().c_str(), point->line());
	    }
	    else
	    {
		// inclusion point
		parser->scanner()->logError ("Included from %s:%d.", line, next->filename().c_str(), point->line());
	    }
	}
	point = next;
	start = false;
    }
    return;
}


static void
yywarning_with_tableentry (Parser *parser, int lineno, TableEntry *tentry)
{
    int line = (lineno > 0) ? lineno : parser->m_lineno;
    const Point *point = tentry->point();
    bool start = true;
    while (point != 0)
    {
	const Point *next = point->point();
	if (start)
	{
	    if (next == 0)
	    {
		parser->scanner()->logWarning ("Definition point (%s:%d) without parent\n", line, point->filename().c_str(), point->line());
	    }
	    // definition point
	    else if (point->line() == 0)
	    {
		parser->scanner()->logWarning ("'%s' defined as %s.", line, tentry->key(), next->filename().c_str());
	    }
	    else
	    {
		parser->scanner()->logWarning ("'%s' defined in %s:%d.", line, tentry->key(), next->filename().c_str(), point->line());
	    }
	}
	else if (point->line() != 0)
	{
	    if (next == 0)
	    {
		parser->scanner()->logWarning ("Inclusion point (%s:%d) without parent\n", line, point->filename().c_str(), point->line());
	    }
	    else
	    {
		// inclusion point
		parser->scanner()->logWarning ("Included from %s:%d.", line, next->filename().c_str(), point->line());
	    }
	}
	point = next;
	start = false;
    }
    return;
}


static void
yyerror_type_mismatch (Parser *parser, int lineno, const char *s, constTypePtr expected_type, constTypePtr seen_type)
{
    int linenumber = (lineno > 0) ? lineno : parser->m_lineno;
    
    parser->m_parser_errors++;
    if (s && *s)
    {
	parser->scanner()->logError (s, linenumber);
    }

    if (expected_type->isUnspec())
    {
	parser->scanner()->logError ("No matching function", linenumber);
    }
    else if (expected_type->isError())
    {
	parser->scanner()->logError ("Bad parameter type '%s'.", linenumber, seen_type->toString().c_str());
    }
    else
    {
	parser->scanner()->logError ("Expected '%s', seen '%s'.", linenumber, expected_type->toString().c_str(), seen_type->toString().c_str());
    }
}


static void
yyerror_missing_argument (Parser *parser, int lineno, constTypePtr type)
{
    parser->m_parser_errors++;
    parser->scanner()->logError ("Missing '%s' parameter.", lineno, type->toString().c_str());
}


static void yyerror_cant_cast (Parser *parser, int lineno, constTypePtr from, constTypePtr to)
{
    parser->m_parser_errors++;
    parser->scanner()->logError ("Can't cast from '%s' to '%s'.", lineno, from->toString().c_str(), to->toString().c_str());
}

static void yyerror_no_module (Parser *parser, int lineno, const char *module)
{
    parser->m_parser_errors++;
    parser->scanner()->logError ("Can't load module '%s'.", lineno, module);
}


/*
  check unary operator

  result = pointer to $$ for return value
  e1 = expression
  op = unary operator (i.e "!", "-", ...)

*/

static void
i_check_unary_op (YYSTYPE *result, YYSTYPE *e1, const char *op, Parser* p)
{
    if (e1->t == 0)
    {
	result->t = 0;
	return;
    }

    FunctionTypePtr ft = Type::Function (Type::Unspec);		// the declaration determines the return type
    ft->concat (e1->t);
    declaration_t *decl = static_declarations.findDeclaration (op, ft);

    if (decl == 0)
    {
	yyerror_with_lineinfo (p, e1->l, "Operator not defined for this type");
	result->t = 0;

	declaration_t* first_decl = static_declarations.findDeclaration(op);
	StaticDeclaration::errorNoMatch (p->scanner (), ft, first_decl);

	return;
    }

    result->c = new YEUnary(decl, e1->c);
    result->t = ((constFunctionTypePtr)(decl->type))->returnType ();
    result->l = e1->l;

#if DO_DEBUG
    y2debug ("check_unary_op (%s/%s) good (ret = %s)", op, e1->t->toString().c_str(), result->t->toString().c_str());
#endif
    return;
}


/*
  check binary operator

  result = pointer to $$ for return value
  e1 = left expression
  op = compare operator (i.e "+", "-", ...)
  e2 = right expression

*/

static void
i_check_binary_op (YYSTYPE *result, YYSTYPE *e1, const char *op, YYSTYPE *e2, Parser* p)
{
    if ((e1->t == 0)
	|| (e2->t == 0))
    {
	result->t = 0;
	return;
    }

    FunctionTypePtr ft = Type::Function (Type::Unspec);		// the declaration determines the return type
#if DO_DEBUG
    y2debug ("i_check_binary_op e1 %p, e2 %p", &(e1->t), &(e2->t));
#endif
    ft->concat (e1->t);
    ft->concat (e2->t);
    declaration_t *decl = static_declarations.findDeclaration (op, ft, true);

    if (decl == 0)		// plain failed, try propagation
    {
	int e1_to_e2 = e1->t->match (e2->t);
	int e2_to_e1 = e2->t->match (e1->t);
	if ((e1_to_e2 > e2_to_e1)
	    && (e2_to_e1 > 0))
	{
	    FunctionTypePtr ft1 = Type::Function (Type::Unspec);		// propagate e1->t --> e2->t
	    ft1->concat (e2->t);
	    ft1->concat (e2->t);
	    decl = static_declarations.findDeclaration (op, ft1);
	    if (decl != 0)
	    {
		ee.setLinenumber (e1->l);
		e1->c = new YEPropagate (e1->c, e1->t, e2->t);
		ft = ft1;
	    }
	}
	else if ((e2_to_e1 > e1_to_e2)
		  && (e1_to_e2 > 0))
	{
	    FunctionTypePtr ft1 = Type::Function (Type::Unspec);		// propagate e2->t --> e1->t
	    ft1->concat (e1->t);
	    ft1->concat (e1->t);
	    decl = static_declarations.findDeclaration (op, ft1);
	    if (decl != 0)
	    {
		ee.setLinenumber (e2->l);
		e2->c = new YEPropagate (e2->c, e2->t, e1->t);
		ft = ft1;
	    }
	}

	if (decl == 0)
	{
	    yyerror_with_lineinfo (p, e1->l, "Binary operator not defined for this type");
	    result->t = 0;

	    declaration_t* first_decl = static_declarations.findDeclaration(op);
	    StaticDeclaration::errorNoMatch (p->scanner (), ft, first_decl);
	}
    }

    if (decl != 0)		// if plain or propagation matched
    {
	constFunctionTypePtr cft = decl->type;
	if (decl->flags & DECL_FLEX)
	{
	    cft = Type::determineFlexType (ft, decl->type);
	    if (cft == 0)					// failed
	    {
		result->t = 0;
		return;
	    }
	    else if (cft->isFunction()				// template function, check final types
		     && ft->isFunction())
	    {
		if (ft->parameters()->match (cft->parameters()) != 0)		//  or realtype does not match actual type
		{
		    result->t = 0;
		    return;
		}
	    }
#if DO_DEBUG
	    y2debug ("cft '%s', decl->type '%s'", cft ? cft->toString().c_str() : "NULL", decl->type->toString().c_str());
#endif
	}
	result->c = new YEBinary (decl, e1->c, e2->c);
	result->t = cft->returnType ();
	result->l = e1->l;
    }

#if DO_DEBUG
    y2debug ("i_check_binary_op (%s/%s/%s) good (ret = '%s')", e1->t->toString().c_str(), op, e2->t->toString().c_str(), result->t->toString().c_str());
#endif
    return;
}


/*
  check compare operator

  result = pointer to $$ for return value
  e1 = left expression
  op = compare operator (see YECompare)
  e2 = right expression

*/

static void
i_check_compare_op (YYSTYPE *result, YYSTYPE *e1, YECompare::c_op op, YYSTYPE *e2, Parser *parser)
{
    if ((e1->t == 0)
	|| (e2->t == 0))
    {
	result->t = 0;
	return;
    }

#if DO_DEBUG
    y2debug ("check_compare_op e1'%s' op %d e2'%s'", e1->t->toString().c_str(), (int)op, e2->t->toString().c_str());
#endif

    int e1_match_e2 = e1->t->match (e2->t);

#if DO_DEBUG
    y2debug ("e1_match_e2 %d", e1_match_e2);
#endif

    int e2_match_e1 = e2->t->match (e1->t);

#if DO_DEBUG
    y2debug ("e2_match_e1 %d", e2_match_e1);
#endif

    if ((e1_match_e2 < 0)						// not comparable types
	&& (e2_match_e1 < 0))
    {
	yyerror_type_mismatch (parser, e2->l, "Types are not comparable", e1->t, e2->t);
	result->t = 0;
	return;
    }

    if ((e1_match_e2 > e2_match_e1)
	     && (e2_match_e1 > 0))
    {
	ee.setLinenumber (e1->l);
	e1->c = new YEPropagate (e1->c, e1->t, e2->t);	// propagate e1
    }
    else if ((e2_match_e1 > e1_match_e2)
	     && (e1_match_e2 > 0))
    {
	ee.setLinenumber (e2->l);
	e2->c = new YEPropagate (e2->c, e2->t, e1->t);	// propagate e2
    }

    result->c = new YECompare (e1->c, op, e2->c);
    result->t = Type::Boolean;
    result->l = e1->l;

#if DO_DEBUG
    y2debug ("check_compare_op '%s'", result->c->toString().c_str());
#endif
    return;
}


/*
  check assign operator for 'void' on rhs

  lhs = left hand side (might be NULL)
  rhs = right hand side

*/

static void
i_check_void_assign (YYSTYPE *lhs, YYSTYPE *rhs, Parser *parser)
{
    if (!rhs->t->isVoid()			// rhs isn't void
	|| rhs->c->isConstant())		//   or is the 'nil' constant
    {
	return;
    }

    if (lhs
	&& (lhs->t->isVoid()			// lhs accepts void
	    || lhs->t->isAny()))		//   or any
    {
	return;
    }

    if (rhs->c->kind() == YCode::yeBracket)	// rhs is bracket operator
    {
	if (lhs != 0)				// bracket operator checked void before
	{
	    return;
	}

	YEBracketPtr b = rhs->c;
	if (b == 0)
	{
	    yyerror_with_lineinfo (parser, rhs->l, "rhs isn't yeBracket but pretends to be");
	    return;
	}

	YYSTYPE bracket_default_as_yystype = { b->def(), {0}, b->type(), rhs->l };

	return i_check_void_assign (0, &bracket_default_as_yystype, parser);
    }

    if (rhs->c->kind() == YCode::yeBuiltin)	// rhs is builtin function
    {
	YEBuiltinPtr b = rhs->c;

	if (b == 0)
	{
	    yyerror_with_lineinfo (parser, rhs->l, "rhs isn't yeBuiltin but pretends to be");
	    return;
	}
    }

    if (lhs == 0)
    {
	yywarning_with_lineinfo (parser, rhs->l, "default of bracket expression is undefined");
    }
    else
    {
	yywarning_with_lineinfo (parser, rhs->l, "rhs of assignment is undefined");
    }

    return;
}


/*
  attach parameter 'parm' to YEBuiltin/YEFunction/YETerm 'code'


  if parameter is 'type identifier': parm->t == type,
				    parm1.t->isUnspec () ? parm1.v.nval = symbol name : parm1->v.tval == table entry
  if parameter is 'expression':  parm->c == code, parm1 == 0

  return NULL if success,
      != NULL (expected type) if wrong parameter type
      Type::Unspec if bad code (NULL)
      Type::Error if excessive parameter
*/

static constTypePtr
attach_parameter (Parser *parser, YCodePtr code, YYSTYPE *parm, YYSTYPE *parm1)
{
    if ((code == 0)
	|| (parm->t == 0))
    {
	return Type::Unspec;
    }

#if DO_DEBUG
    y2debug ("attach_parameter (p %p(%s:%s), p1 %p)", parm, parm1 ? parm->t->toString().c_str() : parm->c->toString().c_str(), parm1 ? "" : parm->t->toString().c_str(), parm1);
#endif

    ee.setLinenumber (parm->l);

    constTypePtr t;
    string name;

    switch (code->kind())
    {
	case YCode::yeBuiltin:
	{
#if DO_DEBUG
	    y2debug ("YCode::yeBuiltin:");
#endif
	    YEBuiltinPtr builtin = (YEBuiltinPtr)code; 

	    name = builtin->decl()->name;
	    
	    // FIXME: new symbol could be probably devised, but it is a debug, anyway
#if DO_DEBUG
	    y2debug ("attach_parameter builtin ([%s]%s:%s)", builtin->toString().c_str(), parm1 ? "new symbol" : parm->c->toString().c_str(), parm->t->toString().c_str());
#endif
	    if (builtin->parameterBlock() == 0)
	    {
		t = builtin->attachParameter (parm->c, parm->t);
		break;
	    }

	    // check if 'func (..., `x, ...)' is to be interpreted as
	    //	func (..., `x, ...) or func (..., any x, ...)
	    if ((parm1 == 0)						// parameter is an expression (not 'type identifier')
		&& ((parm->c->kind() != YCode::ycSymbol)		//  not a symbol
		     || (parm->c->isConstant()				//   or a constant
			 && ((YConstPtr)(parm->c))->value().isNull())))	//   and nilsymbol
	    {
		constTypePtr type = parm->t;
#if DO_DEBUG
		y2debug ("attach_parameter builtin expr ([%s]%s:%s)", builtin->toString().c_str(), parm->c->toString().c_str(), type->toString().c_str());
#endif
		if (parm->c->isBlock())
		{
		    // parameter is block

		    YBlockPtr b = (YBlockPtr)(parm->c);
		    YSReturnPtr ret = b->justReturn();
		    if (ret != 0 && ret->value () != 0)
		    {
			parm->c = new YEReturn (ret->value());
			ret->clearValue ();
		    }
		    type = BlockTypePtr ( new BlockType (type->isUnspec () ? Type::Void : type));
		}
		t = builtin->attachParameter (parm->c, type);
	    }
	    else							// attach type/name or symbol expression
	    {
		TableEntry *tentry = 0;
#if DO_DEBUG
		y2debug ("check for 'type name' or 'symbol' expression");
#endif
		// check for "type name" or "symbol" expression

		if (parm1 != 0)						// 'type name' expression
		{
		    // attach entry to parameter block of function
		    if (parm1->t->isUnspec ())
		    {
			// parameter name is unknown
			t = builtin->attachSymVariable (parm1->v.nval, parm->t, parm->l, tentry);
		    }
		    else
		    {
			// parameter name overlays known TableEntry
			t = builtin->attachSymVariable (parm1->v.tval->key(), parm->t, parm->l, tentry);
		    }
		}
		else							// 'symbol' expression
		{
		    // possibly convert "func(`x, ...)" to "func (any x, ...)" depending on function prototype
		    //  pass is as '<unspec>' and let attachSymVariable() find it out
		    t = builtin->attachSymVariable (parm->v.nval, Type::Unspec, parm->l, tentry);
		}

		if ((t == 0)
		    && (tentry != 0))
		{
#if DO_DEBUG
		    y2debug ("Enter %s to local table", tentry->toString().c_str());
#endif
		    parser->scanner()->localTable()->enter (tentry);
		}
	    }
	}
	break;
	case YCode::yeFunctionPointer:
        case YCode::yeFunction:
	{
#if DO_DEBUG
	    y2debug ("YCode::yeFunction:");
#endif
	    // parameter declaration is not allowed for functions
	    if (parm1 != 0)
	    {
		yyerror_with_lineinfo (parser, parm->l
		    , "Newly declared symbol as a parameter allowed only for builtins");
		t = Type::Error;
	    }
	    
	    YECallPtr func = (YECallPtr)code; 
	    name = func->qualifiedName();
#if DO_DEBUG
	    y2debug ("attach_parameter func ([%s]%s:%s)", func->toString().c_str(), parm->c->toString().c_str(), parm->t->toString().c_str());
#endif

	    t = func->attachParameter (parm->c, parm->t);
	}
	break;
	case YCode::yeTerm:
	{
#if DO_DEBUG
	    y2debug ("YCode::yeTerm:");
#endif
	    // parameter declaration is not allowed for terms
	    if (parm1 != 0)
	    {
		yyerror_with_lineinfo (parser, parm->l
		    , "Newly declared symbol as a parameter allowed only for builtins");
		t = Type::Error;
	    }
	    
	    YETermPtr term = (YETermPtr)code; 
	    name = term->name();
#if DO_DEBUG
	    y2debug ("attach_parameter term ([%s]%s)", term->toString().c_str(), parm->c->toString().c_str());
#endif
	    term->attachParameter (parm->c);
	    return 0;
	}
	default:			// not a builtin/term/function
	{
	    t = Type::Error;
	}
	break;
    }

    if (t != 0)
    {
#if DO_DEBUG
	y2debug ("attach_parameter error, t:%s", t->toString().c_str());
#endif
	if (!name.empty())
	{
	    name = string (" in call to ") + name + string ("(...)");
	}

	if (t->isUnspec())
	{
//	    yyerror_with_lineinfo (parser, parm->l, "No matching function");
	    yyerror_with_code (parser, parm->l, parm->c, parm->t);
	}
	else if (t->isError())
	{
	    if (parm->t->isSymbol()
		&& parm->t->isConst())
	    {
		yyerror_with_lineinfo (parser, parm->l, (string ("Duplicate symbol") + name).c_str());
	    }
	    else
	    {
		yyerror_with_lineinfo (parser, parm->l, (string ("Excessive parameter") + name).c_str());
	    }
	    yyerror_with_code (parser, parm->l, parm->c, parm->t);
	}
	else
	{
	    yyerror_type_mismatch (parser, parm->l, (string ("Parameter type mismatch") + name).c_str(), t, parm->t);
	    yyerror_with_code (parser, parm->l, parm->c, parm->t);
	}
    }
#if DO_DEBUG
    y2debug ("attach_parameter() -> %s", t ? t->toString().c_str() : "OK");
#endif
    return t;
}


//-------------------------------------------------------------------
// stack handling

static void
stack_push (stack_t **stack, stack_t *element)
{
    element->down = *stack;
    *stack = element;
}

// pop element to stack, return the old top
static stack_t *stack_pop (stack_t **stack)
{
    stack_t *element = 0;
    if (*stack)
    {
	element = *stack;
	*stack = element->down;
	if (*stack == 0)
	{
#if DO_DEBUG
	    y2debug ("STACK EMPTY NOW");
#endif
	}
    }
    else
    {
#if DO_DEBUG
	y2debug ("POP EMPTY STACK");
#endif
    }
    return element;
}


// start a block
// pushes new element on m_block_stack
// opens new scope for block-local definitions

static YBlockPtr
start_block (Parser *parser, constTypePtr type)
{
#if DO_DEBUG
    y2debug ("start_block");
#endif

    // check if this block is starting an include file. This means all definitions go to the
    // including (the current m_block_stack) block -> don't open up a new block !

    if ((parser->m_scanner_stack != 0)
	&& (parser->m_scanner_stack->state == SCAN_START_INCLUDE))
    {
#if DO_DEBUG
	y2debug ("Include !");
#endif
	// now we're inside the include file
	parser->m_scanner_stack->state = SCAN_INCLUDE;
	parser->m_block_stack->includeDepth++;

	return parser->m_block_stack->theBlock;
    }

    // start new block
    // push block on block stack
    blockstack_t *top = new blockstack_t;

    if (parser->m_blockstack_depth == 0)	// initial block
    {
	top->theBlock = new YBlock (parser->scanner()->filename(), YBlock::b_file);
	parser->m_current_block = top->theBlock;
    }
    else					// intermediate block
    {
	top->theBlock = new YBlock (parser->m_current_block->point());
    }
	
#if DO_DEBUG
    y2debug ("YBlock created");
#endif
    // inherit textdomain from outer block
    top->textdomain = (parser->m_block_stack ? parser->m_block_stack->textdomain : 0);
    top->theBlock->setType (type);
    top->includeDepth = 0;
    top->self = 0;

    blockstack_push (parser->m_block_stack, top);
    parser->m_blockstack_depth++;

#if DO_DEBUG
    y2debug ("Push block#%d:%s", parser->m_blockstack_depth, type->toString().c_str());
#endif

    return top->theBlock;
}
