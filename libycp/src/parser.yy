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
   Maintainer: Klaus Kämpf <kkaempf@suse.de>


   Implementation rules

   yystype is a struct with four elements

    YCode *c		pointer to code (where applicable)
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
#include "ycp/SymbolEntry.h"
#include "ycp/Bytecode.h"

#include "ycp/Parser.h"

// compile with full debug info, enable with YCP_YYDEBUG=1 in run-time env
#define YYDEBUG 1
#define YYERROR_VERBOSE 1

// define type of parser values
struct yystype_type {
    YCode *c;			// code (for parser level syntax)
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

#define LINE_NOW (p_parser->lineno)
#define FILE_NOW (p_parser->filename ())

// our private error function

static void yyerror_with_lineinfo	(Parser *parser, int lineno, const char *s);
static void yywarning_with_lineinfo	(Parser *parser, int lineno, const char *s);
static void yyerror_with_code		(Parser *parser, int lineno, YCode *c, constTypePtr t);
static void yyerror_with_name		(Parser *parser, int lineno, const char *s);
static void yyerror_with_file		(Parser *parser, int lineno, const char *s);
static void yyerror_with_tableentry	(Parser *parser, int lineno, const char *s, TableEntry *entry);
static void yywarning_with_tableentry	(Parser *parser, int lineno, TableEntry *entry);
static void yyerror_type_mismatch	(Parser *parser, int lineno, constTypePtr expected_type, constTypePtr seen_type);
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
#define yyTypeMismatch(expected,seen,lineno)	yyerror_type_mismatch (p_parser, lineno, expected, seen)
#define yyMissingArgument(type,lineno)		yyerror_missing_argument (p_parser, lineno, type)
#define yyDeclMismatch(decl,type,lineno)	yyerror_decl_mismatch (p_parser, lineno, decl, type)
#define yyCantCast(from,to,lineno)		yyerror_cant_cast (p_parser, lineno, from, to)
#define yyNoModule(module,lineno)		yyerror_no_module (p_parser, lineno, module)
#include "ycp/StaticDeclaration.h"

// attach a new parameter (parm) to a function call (code)
//   if the parameter is 'type symbol', type is passed in parm, symbol in parm1
// returns NULL if success, != NULL (expected type) if wrong parameter type
//  Type::Unspec if bad code (code == NULL), Type::Error if excessive parameter

static constTypePtr attach_parameter (Parser *parser, YCode *code, YYSTYPE *parm, YYSTYPE *parm1 = 0);

//! set by function declaration in order to predefine a definitions block return type
static constTypePtr declared_return_type = Type::Unspec;
//! begin of a block
//! @param type declared return type
static YBlock *start_block (Parser *parser, constTypePtr type);

extern "C" {
int yylex (YYSTYPE *, void *);
}

static void i_check_unary_op (YYSTYPE *result, YYSTYPE *e1, const char *op, Parser* parser);
static void i_check_binary_op (YYSTYPE *result, YYSTYPE *e1, const char *op, YYSTYPE *e2, Parser* parser);
static void i_check_compare_op(YYSTYPE *result, YYSTYPE *e1, YECompare::c_op op, YYSTYPE *e2, Parser *parser);
#define check_unary_op(result,e1,op) i_check_unary_op (result, e1, op, p_parser)
#define check_binary_op(result,e1,op,e2) i_check_binary_op (result, e1, op, e2, p_parser)
#define check_compare_op(result,e1,op,e2) i_check_compare_op (result, e1, op, e2, p_parser)

// for unary and binary operators
extern StaticDeclaration static_declarations;

// for logging
extern ExecutionEnvironment ee;

/*
 * DO NOT USE static or global variables!
 * They make the parser non-reentrant. You need to put them into class Parser.
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
    YBlock *theBlock;		//!< pointer to block
    const char *textdomain;	//!< textdomain (if defined)
    constTypePtr type;		//!< return type of block
    int includeDepth;		//!< block is include file, all definitions go to the outer block
    TableEntry *self;		//!< c_self entry during module parsing
};
#define blockstack_push(s,e) stack_push ((stack_t **)&(s), (stack_t *)e)
#define blockstack_pop(s) (p_parser->blockstack_depth--, (blockstack_t *)stack_pop ((stack_t **)&(s)))
#define blockstack_at_toplevel() (p_parser->blockstack_depth == 1)

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
};
#define scannerstack_push(s,e) stack_push ((stack_t **)&(s), (stack_t *)e)
#define scannerstack_pop(s) (scannerstack_t *)stack_pop ((stack_t **)&(s))
#define scannerstack_empty() (p_parser->scannerStack == 0)

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
%token  UI WFM SCR
%token  DCQUOTED_BLOCK QUOTED_BLOCK QUOTED_EXPRESSION
%token  CLOSEBRACKET
%token  TYPEDEF
%token  MODULE IMPORT EXPORT MAPEXPR INCLUDE GLOBAL TEXTDOMAIN
%token	CONST FULLNAME STATIC EXTERN
%token	LOOKUP 

%token	SYM_NAMESPACE
 /* known entry  */
%token  IDENTIFIER

 /* constant  */
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
%right '['

/* ---------------------------------------------------------------------- */
%%

ycp:	compact_expression
	    {
		p_parser->result = ($1.t == 0) ? 0 : $1.c;
		p_parser->current_block = 0;
		p_parser->lineno = $1.l;
		if (p_parser->parserErrors > 0)
		{
		    p_parser->parserErrors = 0;
		    p_parser->result = new YError ($1.l, "Parser error");
		    YYABORT;
		}
		y2debug ("\n------------------------------------------- accept -------------------------------------------\n");
		YYACCEPT;
	    }
|	END_OF_FILE
	    {
		p_parser->result = 0;
		p_parser->lineno = -1;
		if (p_parser->parserErrors > 0)
		{
		    p_parser->parserErrors = 0;
		    // yyerror ("EOF");
		    p_parser->result = new YError ($1.l, "EOF");
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
|	compact_expression '[' list_elements CLOSEBRACKET expression
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
		    YEList* params = (YEList*)$3.c;

		    do
		    {
			constTypePtr paramType = params->value (index)->type ();	// type of bracket parameter at index

			if (paramType->isFunction())
			{
			    paramType = ((constFunctionTypePtr)paramType)->returnType ();
			}
			
			// for lists, only integer is acceptable
			if (cur->isList ())
			{
			    if (! paramType->isInteger ())
			    {
				yyTypeMismatch (Type::Integer, paramType, $1.l);
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
				yyTypeMismatch (((constMapTypePtr)cur)->keytype (), paramType, $1.l);
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
			$$.t = Type::Any;			// why's that ?
		    }
		    else 
		    {
			$$.t = cur;
		    }
		}					// type determination done

		// default ($5) must match for non-nil
		if (! $5.t->isVoid ()				// default is not 'nil'
		    && $5.t->match ($$.t) == -1)		// and it doesn't match the determined type
		{
		    yyTypeMismatch ($$.t, $5.t, $1.l);		// -> then we have a type error
		    $$.t = 0;
		}
		else
		{
		    $$.c = new YEBracket ($1.c, $3.c, $5.c, $$.t);
		    $$.l = $1.l;
#if 0
		    if (! $5.t->isVoid ()			// default is not 'nil'
			&& $$.t->isAny ())			// and the map/list is unspecified
		    {
			// for non-nil default and cur == Any use the type of the default,
			// but with runtime type checking
			$$.c = new YEPropagate ($$.c, $$.t, $5.t);
			$$.t = $5.t;
		    }
#endif
		}
	    }
;

casted_expression:
	'(' type ')' compact_expression
	{
	    // on error, propagate it
	    if ($2.t == 0 || $4.t == 0 )
	    {
		$$.c = 0;
		$$.t = 0;
		break;
	    }
	    
	    y2debug ("cast %s to %s", $4.t->toString().c_str(), $2.t->toString().c_str());

	    int match = $4.t->match ($2.t);	// would casted type allow expression type ?
	    if (match > 0)
	    {
		y2milestone ("Propagated match %s -> %s at line %d", $2.t->toString().c_str(), $4.t->toString().c_str(), $2.l);
	    }
	    else if (match < 0)
	    {
		if ($4.t->canCast($2.t))
		{
		    $$.c = new YEPropagate ($4.c, $4.t, $2.t);
		}
		else 
		{
		    yyCantCast ($4.t, $2.t, $2.l);
	    	    $$.t = 0;
		    break;
		}		
	    }
	    else 
	    {
		$$.c = $4.c;
	    }

	    $$.t = $2.t;
	    $$.l = $4.l;
	}
;

compact_expression:
	block
	    {
		$$ = $1;

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

		// lookup needs special treatment because we must
		// pass a 'type hint' about the default expression
		// if default is nil, the type can't be seen in a YCPValue
		if (!$3.t->isMap())
		{
		    yyTypeMismatch (Type::Map(), $3.t, $3.l);
		    $$.t = 0;
		    break;
		}

		$$.c = new YELookup ($3.c, $5.c, $7.c, $7.t);
		$$.t = $7.t;
		$$.l = $1.l;
	    }
|	function_call
	    {
		$$ = $1;
		y2debug ("expression: function call");
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
		$$.t = BlockTypePtr ( new BlockType ($2.t) );
	    }
|	IS '(' expression ',' type ')'
	    {
		if ($3.t == 0)		// expression error
		{
		    $$.t = 0;
		    break;
		}

		if ( ($3.t->isAny () || $3.t->isUnspec ()) // expression type unknown
			 && !$5.t->isVoid())	//   and checked type known
		{
		    $$.c = new YEIs ($3.c, $5.t);
		    $$.t = Type::Boolean;
		}
		else
		{
		    yywarning ("Superfluous 'is()' expression, type is known:", $1.l);
		    yywarning ($3.t->toString().c_str(), $3.l);
		    // is (<expr>, any) only evalutes to true if <expr> is also any
		    //   match() can't be used in this case since any->match(<expr>) would return true
		    // is (void, <type>) only evalutes to true if <type> is also void
		    //   match() can't be used in this case since type->match(void) would return true (type always accepts nil)
		    $$.c = new YConst (YCode::ycBoolean, YCPBoolean ($5.t->isAny()
								     ? $3.t->isAny()
								     : ($3.t->isVoid()
									? $5.t->isVoid()
									: $3.t->match ($5.t) == 0)));
		    $$.t = Type::Boolean;
		}
		$$.l = $1.l;
	    }
|	TEXTDOMAIN
	    {
		if (p_parser->blockStack == 0
		    || p_parser->blockStack->textdomain == 0)
		{
		    yyLerror ("No textdomain defined", $1.l);
		    $$.t = 0;
		    break;
		}

		$$.c = new YConst (YCode::ycString, YCPString (p_parser->blockStack->textdomain));
		$$.t = Type::String;
		$$.l = $1.l;
	    }
|	I18N STRING ',' STRING ',' expression ')'
	    {
		if ($6.t == 0)
		{
		    $$.t = 0;
		    break;
		}

		if (p_parser->blockStack == 0
		    || p_parser->blockStack->textdomain == 0)
		{
		    yyLerror ("No textdomain defined", $1.l);
		    $$.t = 0;
		    break;
		}

		if ($6.t->match (Type::Integer) < 0)
		{
		    yyTypeMismatch (Type::Integer, $6.t, $6.l);
		    $$.t = 0;
		    break;
		}

		$$.c = new YELocale ($2.v.sval, $4.v.sval, $6.c, p_parser->blockStack->textdomain);
		$$.t = Type::Locale;
		$$.l = $1.l;
	    }
|	I18N STRING ')'
	    {
		if (p_parser->blockStack == 0
		    || p_parser->blockStack->textdomain == 0)
		{
		    yyLerror ("No textdomain defined", $1.l);
		    $$.t = 0;
		}
		else
		{
		    $$.c = new YLocale ($2.v.sval, p_parser->blockStack->textdomain);
		    $$.t = Type::Locale;
		}
		$$.l = $1.l;
	    }
|	identifier
	    {
		if ($1.t->isUnspec ())		// new (undeclared) identifier
		{
		    yyVerror ($1.v.nval, $1.l);
		    $$.t = 0;
		}
		else
		{
		    SymbolEntry *sentry = $1.v.tval->sentry();
		    $$.c = new YEVariable (sentry);
		    $$.t = sentry->type();
		    y2debug ("identifier '<%s>%s' !", $$.t->toString().c_str(), sentry->name());
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
		    YConst *c = (YConst *)$2.c;
		    if (c->kind() == YCode::ycBoolean)
		    {
			$$.c = new YConst (YCode::ycBoolean, YCPBoolean (!(c->value()->asBoolean()->value())));
			$$.t = Type::Boolean;
			$$.l = $1.l;
			delete c;
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
		    YConst *c = (YConst *)$2.c;
		    if (c->kind() == YCode::ycInteger)
		    {
			$$.c = new YConst (YCode::ycInteger, YCPInteger (-(c->value()->asInteger()->value())));
			$$.t = Type::Integer;
			$$.l = $1.l;
			delete c;
		    }
		    else if ($2.c->kind() == YCode::ycFloat)
		    {
			$$.c = new YConst (YCode::ycFloat, YCPFloat (-(c->value()->asFloat()->value())));
			$$.t = Type::Float;
			$$.l = $1.l;
			delete c;
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
		    yyLerror ("Expression before '?' must be boolean", $1.l);
		    yyTypeMismatch (Type::Boolean, $1.t, $1.l);
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
		
		if ( ! declared_return_type->isUnspec () )
		{
		    declared_return_type = Type::Unspec;
		}
		else if (p_parser->blockStack != NULL)
		{
		    b_t = p_parser->blockStack->type;
		}

		start_block (p_parser, b_t);
	    }
	block_end
	    {
		$$ = $3;
		y2debug ("block: ([%p]%s:%s)", $$.c, $$.c ? $$.c->toString().c_str() : "<nil>", $$.t ? $$.t->toString().c_str() : "<ERR>");
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

		if ( ! declared_return_type->isUnspec () )
		{
		    declared_return_type = Type::Unspec;
		}

		start_block (p_parser, b_t);
	    }
	block_end
	    {
		$$ = $3;
		y2debug ("block: ([%p]%s:%s)", $$.c, $$.c ? $$.c->toString().c_str() : "<nil>", $$.t ? $$.t->toString().c_str() : "<ERR>");
	    }
;

block_end:
	statements '}'
	    {
		// end of file block
		//
		// pop block from block stack
		// unlink local symbols from symbol table

		blockstack_t *top = p_parser->blockStack;
		bool is_include = true;

		if (top == 0
		    || (top->includeDepth == 0))
		{
		    y2debug ("block end");
		    top = blockstack_pop (p_parser->blockStack);
		    is_include = false;
		}

		YBlock *b = top->theBlock;
		y2debug ("Pop %s block %p[stack %p], top block %p, depth %d", is_include ? "include" : "normal", b, top, p_parser->blockStack ? p_parser->blockStack->theBlock : 0, p_parser->blockstack_depth);
		y2debug ("blockStack %p", p_parser->blockStack);

		SymbolTable *localTable = p_parser->scanner()->localTable();
		y2debug ("table before (%s)", localTable->toString().c_str());

		if (top->self != 0)
		{
		    top->self->remove();			// remove c_self entry
		}

		if (top->includeDepth == 0)
		{
		    b->detachEnvironment (localTable);		// detach local table
		}
		else
		{
		    top->includeDepth--;			// end of include block
		}

		y2debug ("table after (%s)", localTable->toString().c_str());

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
			delete b;
			break;
		    }
		    $$.c = 0;
		    delete b;
		}
		else if (is_include)		// this was an include block
		{
		    $$.c = 0;			// pass it up as 'empty'
		}
		else
		{
		    b->finishBlock ();
		    $$.c = b;			// normal block
		}

		// See the comment about types at the "expression" rule.
		$$.t = top->type;
		$$.l = $1.l;

		if (!is_include)
		{
		    delete top;
		}
	    }
;

/* -------------------------------------------------------------- */
/* Statements */
/* statements are always inside a block, so p_parser->blockStack is valid !  */

statements:
	statements statement
	    {
		if (($1.t == 0)
		    || ($2.t == 0))
		{
		    y2debug ("bad statements (%p) or statement (%p)", (const void *)$1.t, (const void *)$2.t);
		    $$.t = 0;
		    break;
		}

		if ($2.c == 0)			// empty statement
		{
		    y2debug ("Empty statement");
		    $$.t = Type::Unspec;
		    break;
		}

		YStatement *stmt = (YStatement *)($2.c);
		y2debug ("STMT[%s!%s:%s:%d]\n", p_parser->blockStack->type->toString().c_str(), $2.t->toString().c_str(), stmt->toString().c_str(), stmt->line());

		if (!($2.t)->isUnspec ())			// return statement
		{
		    y2debug ("Return in block: %s", $2.t->toString().c_str());
		    if (p_parser->blockStack->type->isUnspec ())	// type undefined yet
		    {
			if (!($2.t)->isNil())				// "return nil;" does not define the block type !
			{
			    y2debug ("Block type (%s)", $2.t->toString().c_str());
			    p_parser->blockStack->type = $2.t;		// this is the block type
			}
		    }
		    else						// type is already defined, check it
		    {
			// default: no match
			int match = -1;

			// since nil (void) matches everything, handle this separately
			if ($2.t->isVoid())				// "return;"
			{
			    if (p_parser->blockStack->type->isVoid())
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
			    match = $2.t->match (p_parser->blockStack->type);
			}

			if (match > 0)
			{
			    ((YSReturn *)stmt)->propagate ($2.t, p_parser->blockStack->type);
			}
			else if (match < 0)
			{
			    yyLerror ("Mismatched return type in block", $2.l);
			    yyTypeMismatch (p_parser->blockStack->type, $2.t, $2.l);
			    delete stmt;
			    $$.t = 0;
			    break;
			}
		    }
		}

		p_parser->blockStack->theBlock->attachStatement (stmt);

		$$.c = stmt;
		$$.t = p_parser->blockStack->type;
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

		SymbolTable *globalTable = p_parser->scanner()->globalTable();
		SymbolTable *localTable = p_parser->scanner()->localTable();
		// evaluate following block in different name space
		p_parser->scanner()->initTables ($1.v.tval->sentry()->table(), 0);

		// save environment tables for later restore
		$2.c = (YCode *)localTable;
		$2.v.val = (void *)globalTable;
	    }
	block_end
	    {
		// restore environment
		p_parser->scanner()->initTables ((SymbolTable *)($2.v.val), (SymbolTable *)($2.c));

		if ($4.t == 0)
		{
		    $$.t = 0;
		    break;
		}

		if ($4.c				// block not empty
		    && $4.c->isBlock())
		{
		    YBlock *block = (YBlock *)($4.c);
		    block->setKind (YBlock::b_using);
		    block->setName (string ($1.v.tval->key()));
		}
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
		if (p_parser->current_block->isModule())
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

		p_parser->current_block->setKind (YBlock::b_module);
		p_parser->current_block->setName (name);

		// enter 'self' entry so namespace references to current module get ignored
		SymbolEntry *sself = new SymbolEntry (0, 0, name, SymbolEntry::c_self, Type::Unspec);
		TableEntry *self = new TableEntry (name, sself, $1.l);
		p_parser->scanner()->localTable()->enter (self);

		// module has private global table
		// globalTable has already been saved at IMPORT.

		y2debug ("Create module table");

		globalTable = p_parser->current_block->table ();
		p_parser->scanner()->initTables (globalTable, 0);
		y2debug ("overlaying globalTable %p", globalTable);

		$$.c = 0;
		$$.t = Type::Unspec;
	    }
|	INCLUDE STRING ';'
	    {
	    
		// check, if it is not included yet in the current block
		
		if (p_parser->blockStack->theBlock->isIncluded ($2.v.sval)) 
		{
		    y2debug ("Skipping reinclude of the file %s in block %p", $2.v.sval, p_parser->current_block);
		    $$.c = new YSInclude ($2.v.sval, $2.l, true);
		    $$.l = $1.l;
		    $$.t = Type::Unspec;
		    break;
		}

		// TODO better error reporting?
		// like: could not find foo.ycp in /include, /a/include.
		// It will return an empty string on failure
		string fn = YCPPathSearch::findInclude ($2.v.sval);
		if (fn.empty())
		{
		    yyFerror ($2.v.sval, $1.l);
		    $$.t = 0;
		    break;
		}

		y2debug ("include %s:%s", $2.v.sval, fn.c_str());
		int fd = open (fn.c_str(), O_RDONLY);
		if (fd < 0)
		{
		    yyFerror (fn.c_str(), $1.l);
		    free ((void *)($2.v.sval));
		    $$.t = 0;
		    break;
		}
		

		scannerstack_t *scanner = new (scannerstack_t);
		scanner->down = 0;
		scanner->filename = string (FILE_NOW);		// save current filename
		scanner->linenumber = $1.l;
		scanner->scanner = p_parser->scanner();
		scanner->state = SCAN_START_INCLUDE;	// see start_block()

		scannerstack_push (p_parser->scannerStack, scanner);

		p_parser->setScanner (new Scanner (fd, $2.v.sval));
		p_parser->SetFilename ($2.v.sval);

		// pass the outer scanner's tables
		p_parser->scanner()->initTables (scanner->scanner->globalTable(), scanner->scanner->localTable());

		y2debug ("new scanner at %s:%d, yychar [%d], now %p for %s", FILE_NOW, $1.l, yychar, p_parser->scanner(), $2.v.sval);

		$$.c = new YSInclude ($2.v.sval, scanner->linenumber);
		p_parser->blockStack->theBlock->addIncluded ($2.v.sval);
		$$.l = $1.l;
		$$.t = Type::Unspec;

	    }
|	IMPORT STRING ';'
	    {
		const char *name = $2.v.sval;
		y2debug ("import '%s'", name);

		$$.c = 0;
		$$.l = $1.l;
		$$.t = Type::Unspec;

		// check existance of module

		TableEntry *tentry = p_parser->scanner()->localTable()->find (name, SymbolEntry::c_module);
		if (tentry == 0)
		{
		    if (string (name) == p_parser->current_block->name())
		    {
			yywarning("Ignoring self-import", $1.l);
			break;
		    }

		    string module = name;
		    $$.c = new YSImport (module, $1.l);
		    
		    Y2Namespace *block = ((YSImport *)$$.c)->block();
		    if (block == 0)
		    {
			yyNoModule (name, $1.l);
			$$.t = 0;
			break;
		    }
		    
		    tentry = p_parser->blockStack->theBlock->newNamespace (module, ((YSImport *)$$.c), $1.l);
		    if (tentry == 0)
		    {
			yyLerror ("Import failed", $1.l);
			$$.t = 0;
			break;
		    }
		    p_parser->scanner()->localTable()->enter (tentry);
		}

	    }
|	FULLNAME STRING ';'
|	TEXTDOMAIN STRING ';'
	    {
		p_parser->blockStack->textdomain = $2.v.sval;
		$$.t = Type::Unspec;
		$$.c = new YSTextdomain (p_parser->blockStack->textdomain, $1.l);
		$$.l = $1.l;
	    }
|	EXPORT identifier_list ';'
	    {
		$$.c = 0;
		$$.t = Type::Unspec;
	    }
|	TYPEDEF identifier type ';'
	    {
		if (!($2.t)->isUnspec ())		// known identifier
		{
		    yyLerror ("typedef symbol already declared", $2.l);
		    $$.t = 0;
		    break;
		}

		TableEntry *tentry = p_parser->blockStack->theBlock->newEntry ($2.v.nval, SymbolEntry::c_typedef, $3.t, $1.l);
		if (tentry == 0)		/* can't happen ... */
		{
		    yyLerror ("typedef symbol duplicate", $2.l);
		    $$.t = 0;
		    break;
		}

		p_parser->scanner()->localTable()->enter (tentry);
		$$.c = new YSTypedef ($2.v.nval, $3.t, $1.l);
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

		if (p_parser->blockStack->theBlock->isModule ())
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
		y2debug ("statement: function call");
		if ($1.t == 0)
		{
		    $$.t = 0;
		    break;
		}

		if (p_parser->blockStack->theBlock->isModule ())
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
		if (p_parser->blockStack->theBlock->isModule () && $1.c != 0 )
		{
		    yyLerror ("Block not allowed in a module", $1.l);
		    $$.t = 0;
		    break;
		}

		if ($1.c != 0)			// block not empty
		{
		    ((YBlock *)$1.c)->setKind (YBlock::b_statement);
		}

		$$ = $1;
	    }
|	control_statement
	    {
		if ($1.t != 0
		    && p_parser->blockStack->theBlock->isModule ())
		{
		    yyLerror ("Statement not allowed in a module", $1.l);
		    $$.t = 0;
		    break;
		}
		$$ = $1;
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
		    yyLerror ("'if' expression not boolean", $3.l);
		    yyTypeMismatch (Type::Boolean, $3.t, $3.l);
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
		    $$.t = thentype->commontype (elsetype);

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

		p_parser->loopCount++;

		if (!$3.t->isBoolean())
		{
		    yyLerror ("'while' expression not boolean", $3.l);
		    yyTypeMismatch (Type::Boolean, $3.t, $3.l);
		    $$.t = 0;
		}
		else
		{
		    $$ = $3;
		}
	    }
	statement
	    {
		p_parser->loopCount--;
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
		p_parser->loopCount++;
	    }
	block
	    {
		p_parser->loopCount--;
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
		    yyLerror ("'do-while' expression not boolean", $7.l);
		    yyTypeMismatch (Type::Boolean, $7.t, $7.l);
		    $$.t = 0;
		}
		else
		{
		    $$.c = new YSDo ((YBlock *)$3.c, $7.c, $1.l);
		}
		$$.t = $3.t;
		$$.l = $1.l;
	    }
|	REPEAT
	    {
		p_parser->loopCount++;
	    }
	block
	    {
		p_parser->loopCount--;
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
		    yyLerror ("'repeat-until' expression not boolean", $7.l);
		    yyTypeMismatch (Type::Boolean, $7.t, $7.l);
		    $$.t = 0;
		}
		else
		{
		    $$.c = new YSRepeat ((YBlock *)$3.c, $7.c, $1.l);
		}
		$$.t = $3.t;
		$$.l = $1.l;
	    }
|	BREAK ';'
	    {
		if (p_parser->loopCount <= 0)
		{
		    yyLerror ("'break' outside of loop.", $1.l);
		    $$.t = 0;
		    break;
		}
		$$.c = new YStatement (YCode::ysBreak, $1.l);
		$$.t = Type::Unspec;
		$$.l = $1.l;
	    }
|	CONTINUE ';'
	    {
		if (p_parser->loopCount <= 0)
		{
		    yyLerror ("'continue' outside of loop.", $1.l);
		    $$.t = 0;
		    break;
		}
		$$.c = new YStatement (YCode::ysContinue, $1.l);
		$$.t = Type::Unspec;
		$$.l = $1.l;
	    }
|	RETURN ';'
	    {
		$$.t = Type::Void;			// differentiate "return;" from "return nil;" for type checking
		$$.c = new YSReturn ((YCode *)0, $1.l);
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
|	LIST				{ $$.t = ListTypePtr ( new ListType (Type::Any) ); }
|	LIST '<' type '>'		{ $$.t = ListTypePtr ( new ListType ($3.t) ); }
|	MAP				{ $$.t = MapTypePtr ( new MapType (Type::Any, Type::Any) ); }
|	MAP '<' type ',' type '>'	{ $$.t = MapTypePtr ( new MapType ($3.t, $5.t) ); }
|	BLOCK '<' type '>'		{ $$.t = BlockTypePtr ( new BlockType ($3.t) ); }
|	CONST type			{ TypePtr t = $2.t->clone(); t->asConst();
					  if (!t->isConst())  yywarning ("Bogus 'const'", $2.l);
					  $$.t = t;
					}
|	type '&'			{ TypePtr t = $1.t->clone(); t->asReference();
					  if (!t->isReference())  yywarning ("Bogus '&'", $1.l);
					  $$.t = t;
					}
|	type '(' ')'			{ $$.t = FunctionTypePtr ( new FunctionType ($1.t) ); }
|	type '(' types ')'		{ $$.t = new FunctionType ( $1.t, (constFunctionTypePtr)$3.t); }
;

types:
	type			{ FunctionTypePtr t = Type::Function(); t->concat ($1.t); $$.t = t; }
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
		if ($1.t == 0)
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

		y2debug ("parameter block end");
		blockstack_t *top = blockstack_pop (p_parser->blockStack);
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
		    break;
		}

		// end parameter block _after_ definition block
		//
		// pop block from block stack
		// unlink local symbols from symbol table

		y2debug ("parameter block end");
		blockstack_t *top = blockstack_pop (p_parser->blockStack);
		top->theBlock->detachEnvironment (p_parser->scanner()->localTable());		// detach local table

		if ($2.t == 0)			// error in block
		{
		    $$.t = 0;
		    break;
		}

		if ($2.c == 0)
		{
		    yyLerror ("Empty function definition", $1.l);
		    $$.t = 0;
		    break;
		}
								// link the function entry with the function definition
		SymbolEntry *entry = $1.v.tval->sentry();
		YFunction *func = (YFunction *)(entry->code());
		func->setDefinition ((YBlock *)$2.c);
		$$.c = new YSFunction (entry, $1.l);
		$$.t = Type::Unspec;			// function def is typeless
		$$.l = $1.l;

		y2debug ("Func (%s) done", $$.c->toString().c_str());

	    }
|	opt_global_identifier '=' expression ';'		/* variable definition */
	    {
		if (($1.t == 0)
		    || ($3.t == 0))
		{
		    y2debug ("Bad identifier (%p) or expression (%p)", (const void *)$1.t, (const void *)$3.t);
		    $$.t = 0;
		    break;
		}

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
			    yyLerror ("type mismatch in variable definition", $1.l);
			    yyTypeMismatch ($1.t, $3.t, $1.l);
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

		    if (match == 0)		// type match ok
		    {
			$$.c = new YSAssign (true, tentry->sentry(), $3.c, $1.l);
			tentry->sentry()->setCode($3.c);
		    }
		    if (tentry->sentry()->category() == SymbolEntry::c_unspec)
		    {
			tentry->sentry()->setCategory ($1.t->isReference() ? SymbolEntry::c_reference : SymbolEntry::c_variable);
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
		    y2debug ("Bad identifier (%p) or parameters (%p)", (const void *)$1.t, (const void *)$3.t);
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
		y2debug ("start parameter block for '%s()'", ($1.t) ? $1.v.tval->sentry()->name() : "<err>");

		// start with the declared return type
		YBlock *parameter_block = start_block (p_parser, $1.t);

		// get the functions symbol entry
		SymbolEntry *fentry = $1.v.tval->sentry();

		// remember the prototype, if set for later checking
		constTypePtr prototype = Type::Unspec;
		if (fentry->onlyDeclared())
		{
		    prototype = fentry->type();
		    y2debug ("prototype: %s", prototype->toString().c_str());
		}

		// it's a function
		fentry->setCategory (SymbolEntry::c_function);

		// create new function
		YFunction *func = new YFunction (parameter_block, fentry);
		fentry->setCode (func);

		$$.c = func;
		$$.v.tval = $1.v.tval;
		$$.l = $1.l;

		// build function type

		FunctionTypePtr ftype (new FunctionType ($1.t));

		// save the current declared_return_type, start with the declared return type
		$$.t = declared_return_type;
		declared_return_type = $1.t;

		// loop through formalparam_t, adding each formal
		//  parameter to the function definition (private block)

		while (formalp != 0)				// while we have parameters
		{
		    y2debug ("formal param '%s %s'@%d", formalp->type->toString().c_str(), formalp->name, formalp->line);

		    // compute function type

		    ftype->concat (formalp->type);

		    // create symbol entry for formal parameter

		    TableEntry *tentry = parameter_block->newEntry (formalp->name, formalp->type->isReference() ? SymbolEntry::c_reference : SymbolEntry::c_variable, formalp->type, $1.l);
		    if (tentry == 0)
		    {
			yyLerror ("Duplicate parameter", formalp->line);
			parameter_block->detachEnvironment (p_parser->scanner()->localTable());
			$$.t = 0;
			delete func;
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
			    yyTypeMismatch (prototype, ftype, $1.l);
			    $$.t = 0;
			    break;
			}
		    }

		    y2debug ("func '%s'", func->toString().c_str());
		    y2debug ("ftype (%s:%s)", fentry->name(), ftype->toString().c_str());
		    fentry->setType (ftype);
		    y2debug ("sentry (%p: %s)", fentry, fentry->toString().c_str());
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

		    y2debug ("new %s symbol <%s>'%s'", ($1.v.bval) ? "global" : "local", $3.t->toString().c_str(), $4.v.nval);
		    $$.v.tval = p_parser->blockStack->theBlock->newEntry ($4.v.nval, ($1.v.bval) ? SymbolEntry::c_global : SymbolEntry::c_unspec, $3.t, $4.l);
		}
		else if ($4.v.tval->sentry()->onlyDeclared())
		{
		    SymbolEntry *fentry = $4.v.tval->sentry();

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
			yyTypeMismatch (ftype->returnType(), $3.t, $4.l);
			$$.t = 0;
			break;
		    }

		    $4.v.tval->setLine ($4.l);
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
			    $$.v.tval = p_parser->blockStack->theBlock->newEntry ($4.v.tval->key(), SymbolEntry::c_unspec, $3.t, $4.l);
			}
		    }
		    else if ($4.v.tval->sentry()->block() == p_parser->blockStack->theBlock)
		    {
			yyTerror ("Redefinition of local symbol", $4.l, $4.v.tval);
			$$.t = 0;
			break;
		    }
		    else
		    {
			// yywarning ("Definition shadows local symbol", $4.l);

			$$.v.tval = p_parser->blockStack->theBlock->newEntry ($4.v.tval->key(), SymbolEntry::c_unspec, $3.t, $4.l);
		    }
		}

		if ($1.v.bval)
		{
		    y2debug ("enter (%s) to global table %p", $$.v.tval->toString().c_str(), p_parser->scanner()->globalTable());
		    p_parser->scanner()->globalTable()->enter ($$.v.tval);
		}
		else
		{
		    y2debug ("enter (%s) to local table %p", $$.v.tval->toString().c_str(), p_parser->scanner()->localTable());
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
		    y2debug ("Nesting level is %d", p_parser->blockstack_depth);
		    $$.v.bval = false;
		}
		if (!p_parser->blockStack->theBlock->isModule())
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
		   y2debug ("Bad tupletype (%p) or formal_param (%p)", (const void *)$1.t, (const void *)$3.t);
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
		    SymbolEntry *entry = $2.v.tval->sentry();
		    switch (entry->category())
		    {
			case SymbolEntry::c_builtin:
			{
			    p_parser->parserErrors++;
			    p_parser->scanner()->logError ("Formal parameter '%s' shadows builtin function", $2.l, entry->name());
			    $$.t = 0;
			}
			break;
			case SymbolEntry::c_function:
			{
			    p_parser->parserErrors++;
			    p_parser->scanner()->logError ("Formal parameter '%s' shadows function", $2.l, entry->name());
			    $$.t = 0;
			}
			break;
			case SymbolEntry::c_unspec:
			{
			    p_parser->parserErrors++;
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
			yyLerror ("type mismatch in assignment", $1.l);
			yyTypeMismatch ($1.t, $3.t, $1.l);
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

		$$.c = new YSAssign (false, $1.v.tval->sentry(), $3.c, $1.l);
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

		$$.c = new YSBracket ($1.v.tval->sentry(), $3.c, $6.c, $1.l);
		$$.t = Type::Unspec;
		$$.l = $1.l;
	    }
;

/* ----------------------------------------------------------*/

constant:
	C_VOID
|	C_BOOLEAN
|	C_INTEGER
|	C_FLOAT
|	STRING
	    {
		$$.c = new YConst (YCode::ycString, YCPString ($1.v.sval));
		free ((void *)($1.v.sval));
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
		$$.t = ListTypePtr (new ListType (Type::Unspec));			// make it different from list(any) !
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
		((YEList *)$1.c)->attach ($3.c);
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
		$$.t = MapTypePtr (new MapType (Type::Unspec, Type::Unspec));			// make it different from map<any,any> !
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

		((YEMap *)$1.c)->attach ($3.c, $5.c);
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
		    y2debug ("Term %s(...)", $1.v.nval);
		    /* C_SYMBOL  */
		    $$.c = new YETerm ($1.v.nval);
		    $$.t = Type::Term;
		}
		else							// function_name is function or builtin
		{


		    SymbolEntry *sentry = $1.v.tval->sentry();

		    if (sentry->isBuiltin())	// a builtin function
		    {
			// found builtin declaration

			// check for overloads, those must be re-checked
			// for every parameter in order to match the
			// type-correct declaration

			declaration_t *decl = sentry->declaration();

			y2debug ("Builtin! (%s)%s", sentry->toString().c_str(), (decl->next == 0) ? "!" : "?");

			$$.v.val = decl;
			if ((decl->next != 0)				// if overloaded
			    || (decl->flags & DECL_SYMBOL))		// or can have a symbol as parameter
			{
			    // start block for possible symbol parameters
			    YBlock *block = start_block (p_parser, Type::Unspec);
			    y2debug ("opening parameter scope block %p for %s", block, sentry->name());

			    $$.c = new YEBuiltin (decl, block);
			    y2debug ("Builtin with block ...");
			}
			else
			{
			    $$.c = new YEBuiltin (decl);
			    y2debug ("Builtin ...");
			}

			if (decl->flags & DECL_LOOP)			// allow break in code parameter
			{
			    p_parser->loopCount++;
			}
			$$.t = decl->type;
		    }
/*
		    else if (sentry->type() == Type::Term) // a term
		    {
			y2debug ("Term! (%s)", sentry->toString().c_str());
			$$.c = new YETerm (sentry->name());
			$$.t = sentry->type();
			y2debug ("Term ...");
		    }
*/
		    else
		    {
			y2debug ("Doing function call, starting params");
			$$.c = new YEFunction (sentry);			// an extern function
			$$.t = sentry->type();
			y2debug ("Function! (%s)@%p", sentry->toString(true).c_str(), $$.c);
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
		    y2debug ("Bad function (%p) or parameters %p)", (const void *)$3.t, (const void *)$4.t);
		    $$.t = 0;

		    // check if symfunc parameter block must be popped from stack
		    
		    if ($3.t != 0 && ($3.c->kind() == YCode::yeBuiltin)
			&& (((YEBuiltin *)$3.c)->parameterBlock() != NULL))
		    {
			// end block for possible symbol parameters

			blockstack_t *top = blockstack_pop (p_parser->blockStack);
			top->theBlock->detachEnvironment (p_parser->scanner()->localTable());		// detach local table
			YEBuiltin *bf = (YEBuiltin *)($3.c);
			y2debug ("closing parameter scope block %p for %s", top->theBlock, bf->decl() ? bf->decl()->name : "<err>");
		    }
		    break;
		}

		switch ($3.c->kind())		// depends on what function_name is
		{
		    case YCode::yeTerm:			// a plain term
		    {
			y2debug ("YCode::yeTerm");
			$$.c = $3.c;
			$$.t = $3.t;
		    }
		    break;
		    case YCode::yeBuiltin:			// a builtin function
		    {
			y2debug ("YCode::yeBuiltin");
			// yeBuiltin matched
			//   do final check for parameters

			YEBuiltin *builtin = (YEBuiltin *)$3.c;

			if (builtin->decl()->flags & DECL_LOOP)
			{
			    p_parser->loopCount--;
			}

			constTypePtr finalT = builtin->finalize ();
			if (finalT != 0)
			{
			    if (finalT->isError())
			    {
				yyLerror ("Parameters:", $4.l);
				yyLerror (builtin->toString().c_str(), $4.l);
				yyLerror ("  don't match declaration:", $4.l);
				yyLerror (StaticDeclaration::Decl2String((declaration_t *)($3.v.val), true).c_str(), $4.l);
			    }
			    else
			    {
				yyMissingArgument (finalT, $1.l);
			    }
			    delete builtin;

			    $$.t = 0;
			}
			else
			{
			    y2debug ("yeBuiltin matched");
			    $$.c = $3.c;
			    $$.t = builtin->returnType();
			}
		    }
		    break;
		    case YCode::yeFunction:			// an extern function
		    {
			y2debug ("YCode::yeFunction (%s)@%p", $3.c->toString().c_str(), $3.c);
			YEFunction *function = (YEFunction *)$3.c;

			// close parameter list
			constTypePtr finalT = function->finalize ();
			if (finalT != 0)
			{
			    if (finalT->isError())
			    {
				yyLerror ("Parameters:", $4.l);
				yyLerror (function->toString().c_str(), $4.l);
				yyLerror ("  don't match declaration:", $4.l);
				yyLerror (StaticDeclaration::Decl2String((declaration_t *)($3.v.val), true).c_str(), $4.l);
			    }
			    else
			    {
				yyMissingArgument (finalT, $1.l);
			    }
			    $$.t = 0;
			}
			else
			{
			     // yeFunction matched
			     y2debug ("yeFunction matched");
			     $$.c = $3.c;
			     constFunctionTypePtr ft = $3.t;
			     $$.t = ft->returnType ();
			}

		    }
		    break;
		    default:				// anything else is an error
		    {
			y2debug ("Error");
			$$.t = 0;
		    }
		    break;

		} // switch ($3.c->kind())

		// check if symfunc parameter block must be popped from stack

		if (($3.c->kind() == YCode::yeBuiltin)
		    && (((YEBuiltin *)$3.c)->parameterBlock() != NULL))
		{
		    // end block for possible symbol parameters

		    blockstack_t *top = blockstack_pop (p_parser->blockStack);
		    top->theBlock->detachEnvironment (p_parser->scanner()->localTable());		// detach local table
		    YEBuiltin *bf = (YEBuiltin *)($3.c);
		    y2debug ("closing parameter scope block %p for %s", top->theBlock, bf->decl() ? bf->decl()->name : "<err>");
		}

		if ($$.t == 0)
		{
		    break;
		}

		$$.l = $1.l;

		y2debug ("fcall (%s:%s)", $$.t->toString().c_str(), $$.c->toString().c_str());
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
		y2debug ("Empty parameters (%p)", ($0.t != NULL)? $0.c: NULL);
		$$.c = 0;
		$$.t = Type::Unspec;
	    }
|	type identifier
	    {
		y2debug ("parameters: type/name ($0.c@%p, [%s '%s'])", ($0.t != NULL)? $0.c: NULL, $1.t->toString().c_str(), $2.t->isUnspec () ? $2.v.nval : $2.v.tval->key());


		/* if the function was not found, better fail now */
		if ($0.t != 0 ) {

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
		y2debug ("parameters: expression (%p)", $1.c);

		if ($1.t == 0)			// parameter 'expression' is bad
		{
		    $$.t = 0;
		    break;
		}

		if ($1.c == 0)
		{
		    yyLerror ("Empty expression (block) as a parameter", $1.l);
		    $$.t = 0;
		    break;
		}

		/* if the function was not found, better fail now */
		if ($0.t != 0 ) {
		    // attach parameter ($1) to function ($0), checking types

		    constTypePtr t = attach_parameter (p_parser, $0.c, &($1));

		    if (t != 0)
		    {
			$$.t = 0;
			break;
		    }
		}

		$$.c = $1.c;		// default return value
		$$.t = $1.t;
	    }
|	parameters ',' type identifier
	    {
		y2debug ("parameters: parameters, type/name (%p)", $0.c);

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
		if ($0.t != 0 ) {

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
		if ($0.t != 0 ) {

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
		y2debug ("<new> symbol '%s' !", (const char *)$1.v.nval);
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
	scannerstack_t *top = scannerstack_pop (pr->scannerStack);		// eof of include ?
	y2debug ("EOF, top %p, current %p, yychar ?\n", top, currentScanner);
	if (top == 0)
	{
	    break;								// no
	}
	y2debug ("EOF, back to %s:%d\n", top->filename.c_str(), top->linenumber);
	currentScanner = top->scanner;
	pr->setScanner (currentScanner);
	pr->SetFilename (top->filename);
	
	YSFilename* fn = new YSFilename (top->filename);
	pr->current_block->attachStatement (fn);
	
	delete top;			    // free scannerstack_t new'd at INCLUDE
	token = currentScanner->yylex();
    }

    if (token != END_OF_FILE)
    {
	// store the value of the lexical token
	YCode **store_here = (YCode **) &(lvalp_void->c);
	lvalp_void->t	   = currentScanner->scannedType();

	tokenValue value   = currentScanner->scannedValue();
	lvalp_void->v	   = value;
	pr->lineno	   = lvalp_void->l = currentScanner->lineNumber();

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
	}
    }
    return token;
}
} // extern "C"


static void
yyerror_with_lineinfo (Parser *parser, int lineno, const char *s)
{
    parser->parserErrors++;
    parser->scanner()->logError (s, (lineno > 0) ? lineno : parser->lineno);
}


static void
yywarning_with_lineinfo (Parser *parser, int lineno, const char *s)
{
    parser->scanner()->logWarning (s, (lineno > 0) ? lineno : parser->lineno);
}


static void
yyerror_with_code (Parser *parser, int lineno, YCode *c, constTypePtr t)
{
    parser->parserErrors++;
    if (c == 0)
    {
	parser->scanner()->logError ("Bad parameter", (lineno > 0) ? lineno : parser->lineno);
    }
    else
    {
	parser->scanner()->logError ("Bad parameter '<%s> %s'", (lineno > 0) ? lineno : parser->lineno, t->toString().c_str(), c ? c->toString().c_str() : "<err>");
    }
}


static void
yyerror_with_name (Parser *parser, int lineno, const char *name)
{
    parser->parserErrors++;
    parser->scanner()->logError ("Undeclared identifier '%s'", (lineno > 0) ? lineno : parser->lineno, name);
}


static void
yyerror_assign_const (Parser *parser, int lineno, const char *name)
{
    parser->parserErrors++;
    if (*name == 0)
	parser->scanner()->logError ("Assignment to const", (lineno > 0) ? lineno : parser->lineno);
    else
	parser->scanner()->logError ("Assignment to const identifier '%s'", (lineno > 0) ? lineno : parser->lineno, name);
}


static void
yyerror_with_file (Parser *parser, int lineno, const char *name)
{
    parser->parserErrors++;
    parser->scanner()->logError ("Bad or unknown file '%s'", (lineno > 0) ? lineno : parser->lineno, name);
}


static void
yyerror_with_tableentry (Parser *parser, int lineno, const char *s, TableEntry *entry)
{
    parser->parserErrors++;
    parser->scanner()->logError (s, (lineno > 0) ? lineno : parser->lineno);
    parser->scanner()->logError ("'%s' defined here.", entry->line(), entry->key());
}


static void
yywarning_with_tableentry (Parser *parser, int lineno, TableEntry *entry)
{
    parser->scanner()->logWarning ("'%s' defined here.", entry->line(), entry->key());
}


static void
yyerror_type_mismatch (Parser *parser, int lineno, constTypePtr expected_type, constTypePtr seen_type)
{
    parser->parserErrors++;
    if (expected_type->isUnspec())
    {
	parser->scanner()->logError ("No matching function", lineno);
    }
    else if (expected_type->isError())
    {
	parser->scanner()->logError ("Bad parameter type '%s'.", lineno, seen_type->toString().c_str());
    }
    else
    {
	parser->scanner()->logError ("Expected '%s', seen '%s'.", lineno, expected_type->toString().c_str(), seen_type->toString().c_str());
    }
}


static void
yyerror_missing_argument (Parser *parser, int lineno, constTypePtr type)
{
    parser->parserErrors++;
    parser->scanner()->logError ("Missing '%s' parameter.", lineno, type->toString().c_str());
}


static void yyerror_cant_cast (Parser *parser, int lineno, constTypePtr from, constTypePtr to)
{
    parser->parserErrors++;
    parser->scanner()->logError ("Can't cast from '%s' to '%s'.", lineno, from->toString().c_str(), to->toString().c_str());
}

static void yyerror_no_module (Parser *parser, int lineno, const char *module)
{
    parser->parserErrors++;
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

    FunctionTypePtr ft = Type::Function();
    ft->concat (e1->t);
    declaration_t *decl = static_declarations.findDeclaration (op, ft);

    if (decl == 0)
    {
	yyerror_with_lineinfo (p, e1->l, "Operator not defined for this type");
	result->t = 0;
	return;
    }

    result->c = new YEUnary(decl, e1->c);
    result->t = ((constFunctionTypePtr)(decl->type))->returnType ();
    result->l = e1->l;

    y2debug ("check_unary_op (%s/%s) good (ret = %s)", op, e1->t->toString().c_str(), result->t->toString().c_str());

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

    FunctionTypePtr ft = Type::Function ();
y2debug ("check_binary_op %p, e1 %p, e2 %p", &ft, &(e1->t), &(e2->t));
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
	    FunctionTypePtr ft1 = Type::Function();		// propagate e1->t --> e2->t
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
	    FunctionTypePtr ft1 = Type::Function();		// propagate e2->t --> e1->t
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
	    static_declarations.findDeclaration (op, ft);	// trigger error output
	}
    }

    if (decl != 0)		// if plain or propagation matched
    {
	if (decl->flags & DECL_FLEX)
	{
	    ft = Type::determineFlexType (ft, decl->type, Type::Unspec);
	}
	else
	{
	    ft = decl->type->clone();
	}
	result->c = new YEBinary (decl, e1->c, e2->c);
	result->t = ft->returnType ();
	result->l = e1->l;
    }

    y2debug ("check_binary_op (%s/%s/%s) good (ret = '%s')", e1->t->toString().c_str(), op, e2->t->toString().c_str(), result->t->toString().c_str());
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

    y2debug ("check_compare_op e1'%s' op %d e2'%s'", e1->t->toString().c_str(), (int)op, e2->t->toString().c_str());
    int e1_match_e2 = e1->t->match (e2->t);
    y2debug ("e1_match_e2 %d", e1_match_e2);
    int e2_match_e1 = e2->t->match (e1->t);
    y2debug ("e2_match_e1 %d", e2_match_e1);

    if ((e1_match_e2 < 0)						// not comparable types
	&& (e2_match_e1 < 0))
    {
	yyerror_type_mismatch (parser, e2->l, e1->t, e2->t);
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

    y2debug ("check_compare_op '%s'", result->c->toString().c_str());
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
attach_parameter (Parser *parser, YCode *code, YYSTYPE *parm, YYSTYPE *parm1)
{
    if ((code == 0)
	|| (parm->t == 0))
    {
	return Type::Unspec;
    }

    y2debug ("attach_parameter (code %p, p %p(%s), p1 %p)", code, parm, parm1 ? parm->t->toString().c_str() : parm->c->toString().c_str(), parm1);

    ee.setLinenumber (parm->l);

    constTypePtr t;

    switch (code->kind())
    {
	case YCode::yeBuiltin:
	{
	    y2debug ("YCode::yeBuiltin:");
	    YEBuiltin *builtin = (YEBuiltin *)code; 
	    
	    // FIXME: new symbol could be probably devised, but it is a debug, anyway
	    y2debug ("attach_parameter builtin ([%s]%s:%s)", builtin->toString().c_str(), parm1 ? "new symbol" : parm->c->toString().c_str(), parm->t->toString().c_str());
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
			 && ((YConst *)(parm->c))->value().isNull())))	//   and nilsymbol
	    {
		constTypePtr type = parm->t;
		y2debug ("attach_parameter builtin expr ([%s]%s:%s)", builtin->toString().c_str(), parm->c->toString().c_str(), type->toString().c_str());
		if (parm->c->kind() == YCode::yeBlock)
		{
		    // parameter is block
		    y2debug ("parameter is block");

		    YBlock *b = (YBlock *)(parm->c);
		    YSReturn *ret = b->justReturn();
		    if (ret != 0 && ret->value () != 0)
		    {
			parm->c = new YEReturn (ret->value());
			ret->clearValue ();
			delete b;
		    }
		    type = BlockTypePtr ( new BlockType (type->isUnspec () ? Type::Void : type));
		}
		t = builtin->attachParameter (parm->c, type);
	    }
	    else							// attach type/name or symbol expression
	    {
		TableEntry *tentry = 0;
		y2debug ("check for 'type name' or 'symbol' expression");

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
		    y2debug ("Enter %s to local table", tentry->toString().c_str());
		    parser->scanner()->localTable()->enter (tentry);
		}
	    }
	}
	break;
        case YCode::yeFunction:
	{
	    y2debug ("YCode::yeFunction:");
	    YEFunction *func = (YEFunction *)code; 
	    y2debug ("attach_parameter func ([%s]%s:%s)", func->toString().c_str(), parm->c->toString().c_str(), parm->t->toString().c_str());

	    t = func->attachParameter (parm->c, parm->t);
	}
	break;
	case YCode::yeTerm:
	{
	    y2debug ("YCode::yeTerm:");
	    YETerm *term = (YETerm *)code; 
	    y2debug ("attach_parameter term ([%s]%s)", term->toString().c_str(), parm->c->toString().c_str());
	    term->attachParameter (parm->c);
	    return 0;
	}
	default:
	{
	    fprintf (stderr, "attach_parameter to unknown (%s)\n", code->toString().c_str());
	    t = Type::Error;
	}
	break;
    }

    if (t != 0)
    {
	y2debug ("attach_parameter error, t:%s", t->toString().c_str());
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
		yyerror_with_lineinfo (parser, parm->l, "Duplicate symbol");
	    }
	    else
	    {
		yyerror_with_lineinfo (parser, parm->l, "Excessive parameter");
	    }
	    yyerror_with_code (parser, parm->l, parm->c, parm->t);
	}
	else
	{
	    yyerror_with_lineinfo (parser, parm->l, "Parameter type mismatch");
	    yyerror_type_mismatch (parser, parm->l, t, parm->t);
	    yyerror_with_code (parser, parm->l, parm->c, parm->t);
	}
    }
    y2debug ("attach_parameter() -> %s", t ? t->toString().c_str() : "OK");
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
	    y2debug ("STACK EMPTY NOW");
	}
    }
    else
    {
	y2debug ("POP EMPTY STACK");
    }
    return element;
}


// start a block
// pushes new element on blockStack
// opens new scope for block-local definitions

static YBlock *
start_block (Parser *parser, constTypePtr type)
{
    // check if this block is starting an include file. This means all definitions go to the
    // including (the current blockStack) block -> don't open up a new block !

    if ((parser->scannerStack != 0)
	&& (parser->scannerStack->state == SCAN_START_INCLUDE))
    {
	y2debug ("Include !");
	// now we're inside the include file
	parser->scannerStack->state = SCAN_INCLUDE;
	parser->blockStack->includeDepth++;

	return parser->blockStack->theBlock;
    }

    // start new block
    // push block on block stack
    blockstack_t *top = new (blockstack_t);
    top->theBlock = new YBlock (parser->filename(), parser->blockstack_depth == 0 ? YBlock::b_file : YBlock::b_unknown);

    if (parser->blockstack_depth == 0)		// initial block
    {
	parser->current_block = top->theBlock;
	
	if (parser->preload_namespaces)
	{
	    // implicit Pkg import
	    YSImport* pkg = new YSImport ("Pkg", 0);
		    
	    Y2Namespace *block = pkg->block();
	    if (block == 0)
	    {
		yyerror_no_module (parser, 0, "Pkg");
	    }
	    else
	    {	    
		TableEntry* tentry = top->theBlock->newNamespace ("Pkg", pkg, 0);
		if (tentry == 0)
		{
		    y2error ("Import Pkg failed");
		}
		else
		{
		    parser->scanner()->localTable()->enter (tentry);
		    top->theBlock->attachStatement (pkg);
		}
	    }
	}
    }

    // inherit textdomain from outer block
    top->textdomain = (parser->blockStack ? parser->blockStack->textdomain : 0);
    top->type = type;
    top->includeDepth = 0;
    top->self = 0;

    blockstack_push (parser->blockStack, top);
    parser->blockstack_depth++;

    y2debug ("Push block#%d:%s, top %p, down %p", parser->blockstack_depth, type->toString().c_str(), parser->blockStack->theBlock, parser->blockStack->down ? ((blockstack_t *)parser->blockStack->down)->theBlock : 0);

    return top->theBlock;
}
