/*----------------------------------------------------------*- c++ -*---\
|									|
|		      __   __    ____ _____ ____			|
|		      \ \ / /_ _/ ___|_   _|___ \			|
|		       \ V / _` \___ \ | |   __) |			|
|			| | (_| |___) || |  / __/			|
|			|_|\__,_|____/ |_| |_____|			|
|									|
|			       core system				|
|							  (C) SuSE GmbH |
\-----------------------------------------------------------------------/

   File:       parser.yy

   Author:     Klaus Kämpf <kkaempf@suse.de>
   Maintainer: Klaus Kämpf <kkaempf@suse.de>

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

#include "ycp/StaticDeclaration.h"
#include "ycp/YCode.h"
#include "ycp/TypeCode.h"
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
    YCode *c;
    tokenValue v;
    TypeCode t;
    int l; // overloaded! line number, ref flag, global flag
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

static void yyerror_with_lineinfo (const char *s, Parser *parser, int lineno);
static void yywarning_with_lineinfo (const char *s, Parser *parser, int lineno);
static void yyerror_with_code (YCode *c, TypeCode &t, Parser *parser, int lineno);
static void yyerror_with_name (const char *s, Parser *parser, int lineno);
static void yyerror_with_file (const char *s, Parser *parser, int lineno);
static void yyerror_with_tableentry (Parser *parser, const char *s, int lineno, TableEntry *entry);
static void yywarning_with_tableentry (Parser *parser, TableEntry *entry);
static void yyerror_type_mismatch (Parser *parser, const TypeCode & expected_type, const TypeCode & seen_type, int lineno);
static void yyerror_missing_argument (Parser *parser, const TypeCode & type, int lineno);
//static void yyerror_decl_mismatch (Parser *parser, declaration_t *decl, const char *seen_type, int lineno);

#define yyerror(text)			yyerror_with_lineinfo (text, p_parser, -1)
#define yywarning(text,lineno)		yywarning_with_lineinfo (text, p_parser, lineno)
#define yyLerror(text,lineno)		yyerror_with_lineinfo (text, p_parser, lineno)
#define yyCerror(code,type,lineno)	yyerror_with_code (code, type, p_parser, lineno)
#define yyVerror(name,lineno)		yyerror_with_name (name, p_parser, lineno)
#define yyFerror(name,lineno)		yyerror_with_file (name, p_parser, lineno)
#define yyTerror(text,line,entry)	yyerror_with_tableentry (p_parser, text, line, entry)
#define yyTwarning(entry)		yywarning_with_tableentry (p_parser, entry)
#define yyTypeMismatch(t1,t2,lineno)	yyerror_type_mismatch (p_parser, t1, t2, lineno)
#define yyMissingArgument(type,lineno)	yyerror_missing_argument (p_parser, type, lineno)
#define yyDeclMismatch(decl,type,lineno) yyerror_decl_mismatch (p_parser, decl, type, lineno)

#include "ycp/StaticDeclaration.h"

static declaration_t *attach_parameter (Parser *parser, YCode *code, YYSTYPE *parm, YYSTYPE *parm1 = 0);
static YBlock *start_block (Parser *parser, const TypeCode & type);

extern "C" {
int yylex (YYSTYPE *, void *);
}

static void check_unary_op (YYSTYPE *result, YYSTYPE *e1, const char *op);
static void check_binary_op (YYSTYPE *result, YYSTYPE *e1, const char *op, YYSTYPE *e2);
static void i_check_compare_op(YYSTYPE *result, YYSTYPE *e1, YECompare::c_op op, YYSTYPE *e2, Parser *parser);
#define check_compare_op(result,e1,op,e2) i_check_compare_op (result, e1, op, e2, p_parser)

// for unary and binary operators
extern StaticDeclaration static_declarations;

// for logging
extern ExecutionEnvironment ee;

/*
 * DO NOT USE static or global variables!
 * They make the parser non-reentrant. You need to put them into class Parser.
 */ 

// general stack handling
//
typedef struct stack {
    struct stack *down;		// next stack element
} stack_t;
// push element to stack
static void stack_push (stack_t **stack, stack_t *element);
// pop element to stack
static stack_t *stack_pop (stack_t **stack);

// stack for blocks

struct blockstack_t : stack_t {
    YBlock *theBlock;		// pointer to block
    const char *textdomain;	// textdomain (if defined)
    TypeCode type;		// return type of block
    int includeDepth;		// block is include file, all definitions go to the outer block
};
#define blockstack_push(s,e) stack_push ((stack_t **)&(s), (stack_t *)e)
#define blockstack_pop(s) (blockstack_t *)stack_pop ((stack_t **)&(s))
#define blockstack_at_toplevel() (blockstack_depth == 1)
#define blockstack_empty() (blockstack_depth == 0)
static int blockstack_depth = 0;

enum scan_states {
    SCAN_FILE,		// a plain file
    SCAN_START_INCLUDE,	// before the first token of an include file (see start_block())
    SCAN_INCLUDE,	// inside an include file
};

// stack for scanners
struct scannerstack_t : stack_t {
    Scanner *scanner;
    string filename;
    int linenumber;
    enum scan_states state;
};
#define scannerstack_push(s,e) stack_push ((stack_t **)&(s), (stack_t *)e)
#define scannerstack_pop(s) (scannerstack_t *)stack_pop ((stack_t **)&(s))


// the current file being parsed

static YBlock *current = 0;

//----------------------------------------------------------------------------
%}

 /* expect one shift-reduce conflict (a dangling else) */
%expect 1
%pure_parser

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
%token	LOOKUP FOREACH

%token	SYM_NAMESPACE
 /* known entry  */
%token  SYM_VARIABLE
%token  SYM_FUNCTION
%token	SYM_BUILTIN

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
%left EQUALS NEQ
%left LT GT LE GE
%left LEFT RIGHT
%left '+' '-'
%left '*' '/' '%'
%right NOT
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
		p_parser->result = $1.c;
		current = 0;
		p_parser->lineno = $1.l;
		if (p_parser->parserErrors > 0)
		{
		    p_parser->parserErrors = 0;
		    $$.c = new YError ($1.l, "Parser error");
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
		    $$.c = new YError ($1.l, "EOF");
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
   * except return must have an undetermined type (TypeCode::Unspec).
   * Do not confuse TypeCode::Unspec with TypeCode::Void.
   */
  /* expressions are either 'compact' (with a defined end-token, no lookahead)
     or 'infix' (which might need a lookahead token)  */

expression:
	compact_expression
|	casted_expression
|	infix_expression
|	compact_expression '[' list_elements CLOSEBRACKET expression
	    {
		if ($1.c->isError())
		{
		    $$.c = $1.c;
		}
		else if ($3.c->isError ())
		{
		    $$.c = $3.c;
		}
		else if ($5.c->isError ())
		{
		    $$.c = $5.c;
		}
		else if (($1.t.matchtype (TypeCode::List) != 0)
			 && ($1.t.matchtype (TypeCode::Map) != 0)
			 && ($1.t.matchtype (TypeCode::Term) != 0))
		{
		    yyLerror ("Bracket operator must be applied to list, map, or term", $1.l);
		    $$.c = new YError ($1.l, "Bracket operator must be applied to list, map, or term");
		}
		else
		{
		    $$.c = new YEBracket ($1.c, $3.c, $5.c);
		    $$.t = $5.t;
		    $$.l = $1.l;
		}
	    }
;

casted_expression:
	'(' type ')' compact_expression
	{
	    int match = $2.t.matchtype ($4.t);
	    if (match > 0)
	    {
		y2milestone ("Propagated match %s -> %s at line %d", $2.t.toString().c_str(), $4.t.toString().c_str(), $2.l);
	    }
	    else if (match < 0)
	    {
		y2error ("Can't cast %s to %s at line %d", $4.t.toString().c_str(), $2.t.toString().c_str(), $2.l);
		$$.c = new YError ($2.l, "Wrong cast");
		$$.t = TypeCode::Unspec;
		$$.l = $2.l;
		break;
	    }
	    $$.c = $4.c;
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
		if ($$.t.isUnspec ())
		{
		    $$.t = TypeCode::Void;
		}
	    }
|	LOOKUP '(' expression ',' expression ',' expression ')'
	    {
		// lookup needs special treatment because we must
		// pass a 'type hint' about the default expression
		// if default is nil, the type can't be seen in a YCPValue
		if ($3.t.matchtype (TypeCode::Map) != 0)
		{
		    yyTypeMismatch (TypeCode::Map, $3.t, $3.l);
		}
		else
		{
		    $$.c = new YELookup ($3.c, $5.c, $7.c, $7.t);
		    $$.t = $7.t;
		    $$.l = $1.l;
		}
	    }
|	term
|	'(' expression ')'
	    {
		$$ = $2;
	    }
|	QUOTED_EXPRESSION expression ')'
	    {
		if (($2.c == 0)
		   || ($2.c->isError()))
		{
		    $$.c = $2.c;
		    $$.t = TypeCode::Unspec;
		}
		else
		{
		    $$.c = new YEReturn ($2.c);
		    $$.t = TypeCode::makeBlock ($2.t);
		}
	    }
|	IS '(' expression ',' type ')'
	    {
		if ($3.t.isUnspec ())		// expression error
		{
		    $$.c = new YError ($3.l, "Bad expression for 'is()'");
		    $$.t = TypeCode::Unspec;
		}
		else if ($3.t.isAny ()		// expression type unknown
			 && !$5.t.isVoid())	//   and checked type known
		{
		    $$.c = new YEIs ($3.c, $5.t);
		    $$.t = "b";
		}
		else
		{
		    yywarning ("Superfluous 'is()' expression, type is known:", $1.l);
		    yywarning ($3.t.toString().c_str(), $3.l);
		    $$.c = new YConst (YCode::ycBoolean, YCPBoolean (equaltype ($3.t, $5.t)));
		    $$.t = "b";
		}
		$$.l = $1.l;
	    }
|	TEXTDOMAIN
	    {
		if (p_parser->blockStack == 0
		    || p_parser->blockStack->textdomain == 0)
		{
		    yyLerror ("No textdomain defined", $1.l);
		    $$.c = new YError ($1.l, "No textdomain defined");
		}
		else
		{
		    $$.c = new YConst (YCode::ycString, YCPString (p_parser->blockStack->textdomain));
		    $$.t = "s";
		    $$.l = $1.l;
		}
	    }
|	I18N STRING ',' STRING ',' expression ')'
	    {
		if (p_parser->blockStack == 0
		    || p_parser->blockStack->textdomain == 0)
		{
		    yyLerror ("No textdomain defined", $1.l);
		    $$.c = new YError ($1.l, "No textdomain defined");
		}
		else if ($6.t.matchtype (TypeCode::Integer) < 0)
		{
		    yyTypeMismatch (TypeCode::Integer, $6.t, $6.l);
		    YYABORT;
		}
		else
		{
		    $$.c = new YELocale ($2.v.sval, $4.v.sval, $6.c, p_parser->blockStack->textdomain);
		    $$.t = "_";
		}
		$$.l = $1.l;
	    }
|	I18N STRING ')'
	    {
		if (p_parser->blockStack == 0
		    || p_parser->blockStack->textdomain == 0)
		{
		    yyLerror ("No textdomain defined", $1.l);
		    $$.c = new YError ($1.l, "No textdomain defined");
		}
		else
		{
		    $$.c = new YLocale ($2.v.sval, p_parser->blockStack->textdomain);
		    $$.t = "_";
		}
		$$.l = $1.l;
	    }
|	identifier
|	'&' identifier %prec UMINUS
	    {
		// reference

		$$ = $2;
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
|	expression LT expression
	    {
		check_compare_op (&($$), &($1), YECompare::C_LT, &($3));
	    }
|	expression GT expression
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
|	NOT expression
	    {
		if ($2.c->isConstant())
		{
		    YConst *c = (YConst *)$2.c;
		    if (c->code() == YCode::ycBoolean)
		    {
			$$.c = new YConst (YCode::ycBoolean, YCPBoolean (!(c->value()->asBoolean()->value())));
			$$.t = "b";
			$$.l = $1.l;
			delete c;
		    }
		    else
		    {
			yyLerror ("Bad constant for binary 'not'", $2.l);
			$$.c = new YError ($2.l, "Bad constant for binary 'not'");
		    }
		}
		else
		{
		    check_unary_op (&($$), &($2), "!");
		}
	    }
|	'-' expression %prec UMINUS
	    {
		if ($2.c->isConstant())
		{
		    YConst *c = (YConst *)$2.c;
		    if (c->code() == YCode::ycInteger)
		    {
			$$.c = new YConst (YCode::ycInteger, YCPInteger (-(c->value()->asInteger()->value())));
			$$.t = TypeCode::Integer;
			$$.l = $1.l;
			delete c;
		    }
		    else if ($2.c->code() == YCode::ycFloat)
		    {
			$$.c = new YConst (YCode::ycFloat, YCPFloat (-(c->value()->asFloat()->value())));
			$$.t = TypeCode::Float;
			$$.l = $1.l;
			delete c;
		    }
		    else
		    {
			yyLerror ("Bad constant for negate", $2.l);
			$$.c = new YError ($2.l, "Bad constant for negate");
		    }
		}
		else
		{
		    check_unary_op (&($$), &($2), "-");
		}
	    }
|	expression '?' expression ':' expression
	    {
		if (!$1.t.isBoolean())
		{
		    yyLerror ("Expression before '?' must be boolean", $1.l);
		    yyTypeMismatch ("b", $1.t, $1.l);
		    $$.c = new YError ($1.l, "Expression before '?' must be boolean");
		}
		else if ($1.c->code() == YCode::ycBoolean)
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
		$$.t = $3.t.commontype ($5.t); // FIXME LEAK
		$$.l = $1.l;
	    }
;

block:
	'{'
	    {
		start_block (p_parser, TypeCode::Unspec);
	    }
	block_end
	    {
		$$ = $3;
		y2debug ("block: ([%p]%s:%s)", $$.c, $$.c?$$.c->toString().c_str():"<nil>", $$.t.toString().c_str());
	    }
|	QUOTED_BLOCK
	    {
		start_block (p_parser, TypeCode::Unspec);
	    }
	block_end
	    {
		$$.c = $3.c;
		// See the comment about types at the "expression" rule.
		$$.t = TypeCode::makeBlock (($3.t).isUnspec() ? TypeCode::Void : ($3.t));
		$$.l = $1.l;
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
		y2debug ("Pop %s block %p[stack %p], top block %p", is_include?"include":"normal", b, top, p_parser->blockStack?p_parser->blockStack->theBlock:0);
		y2debug ("blockStack %p", p_parser->blockStack);

		SymbolTable *localTable = p_parser->scanner()->localTable();
		y2debug ("table before (%s)", localTable->toString().c_str());

		if (top->includeDepth == 0)
		{
		    b->detachEnvironment (localTable);		// detach local table
		}
		else
		{
		    top->includeDepth--;			// end of include block
		}

		y2debug ("table after (%s)", localTable->toString().c_str());

		if ($1.c == 0)			// empty block
		{
		    if (is_include)
		    {
			y2debug ("Empty include file");
		    }
		    else
		    {
			y2debug ("Empty block");
			delete b;
		    }
		    $$.c = 0;
		    $$.t = TypeCode::Unspec;
		}
		else if ($1.c->isError ())
		{
		    if (is_include)
		    {
			y2debug ("Bad include file");
		    }
		    else
		    {
			y2debug ("Error block");
			delete b;
		    }
		    $$.c = $1.c;
		    $$.t = TypeCode::Unspec;
		}
		else if (!is_include)
		{
		    $$.c = b;
		    // See the comment about types at the "expression" rule.
		    $$.t = top->type;
		    //	$$.t = TypeCode::makeBlock (top->type);

		    y2debug ("Block ([%p]%s) returns '%s'", $$.c, ((YBlock *)$$.c)->toString().c_str(), $$.t.toString().c_str());
		}
		else
		{
		    $$.c = 0;
		}
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
		if (($1.c != 0)			// first statement
		    && $1.c->isError())		//   is bad
		{
		    $$.c = $1.c;
		    $$.t = TypeCode::Unspec;
		    break;
		}

		if ($2.c == 0)
		{
		    $$.c = $1.c;
		    $$.t = TypeCode::Unspec;
		    break;
		}

		if ($2.c->isError())
		{
		    $$.c = $2.c;
		    $$.t = TypeCode::Unspec;
		    break;
		}

		    YStatement *s = (YStatement *)($2.c);
		    y2debug ("STMT[%s!%s:%s]\n", p_parser->blockStack->type.asString().c_str(), $2.t.asString().c_str(), s->toString().c_str());

		    if ((p_parser->blockStack->type.isUnspec ())	// type undefined
			&& (($2.t).notUnspec ()))			// and return statement
		    {
			y2debug ("Block type (%s)", $2.t.toString().c_str());
			p_parser->blockStack->type = $2.t; // this is the blocks type
		    }

		    $$.c = s;
		    if (($2.t).notUnspec ())
		    {
			int match = $2.t.matchtype (p_parser->blockStack->type);
			if (match > 0)
			{
			    ((YSReturn *)s)->propagate ($2.t, p_parser->blockStack->type);
			}
			else if (match < 0)
			{
			    yyLerror ("Mismatched return type in block", $2.l);
			    yyTypeMismatch (p_parser->blockStack->type, $2.t, $2.l);
			    delete s;
			    $$.c = new YError ($2.l, "Mismatched return type in block");
			}
		    }

		    if (!s->isError())
		    {
			p_parser->blockStack->theBlock->attachStatement(s);
		    }
		$$.t = p_parser->blockStack->type;
		$$.l = $1.l;
	    }
| /* empty  */
	    {
		$$.t = TypeCode::Unspec;	// default type is unknown
		$$.l = LINE_NOW;
		$$.c = 0;
	    }
;

statement:
	';'
	    {
		$$.c = 0;
	    }
|	SYM_NAMESPACE DCQUOTED_BLOCK
	    {
		start_block (p_parser, TypeCode::Unspec);

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

		if ($4.c
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
		    YYABORT;
		}
		if (current->isModule())
		{
		    yyLerror ("duplicate module statement", $1.l);
		    YYABORT;
		}

		const char *name = $2.v.sval;
		SymbolTable *globalTable = p_parser->scanner()->globalTable();
		if (globalTable->find (name, SymbolEntry::c_module) != 0)
		{
		    yyLerror ("module already declared", $1.l);
		    YYABORT;
		}

		// Remember the name that will be used for a symbol
		// table entry when we finish the module block and so
		// that other syntax rules know we are in a module.

		current->setKind (YBlock::b_module);
		current->setName (name);

		// module has private global table
		// globalTable has already been saved at IMPORT.

		y2debug ("Create module table");

		globalTable = new SymbolTable (211);
		p_parser->scanner()->initTables (globalTable, 0);
		y2debug ("overlaying globalTable %p", globalTable);
		current->setTable (globalTable);

		$$.c = 0;
		$$.t = TypeCode::Unspec;
	    }
|	INCLUDE STRING ';'
	    {
		// TODO better error reporting?
		// like: could not find foo.ycp in /include, /a/include.
		// It will return an empty string on failure
		string fn = YCPPathSearch::findInclude ($2.v.sval);
		if (fn.empty())
		{
		    yyFerror ($2.v.sval, $1.l);
		    YYABORT;
		}

		y2debug ("include %s:%s", $2.v.sval, fn.c_str());
		int fd = open (fn.c_str(), O_RDONLY);
		if (fd < 0)
		{
		    yyFerror (fn.c_str(), $1.l);
		    free ((void *)($2.v.sval));
		    YYABORT;
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

		$$.c = new YSInclude (scanner->filename, scanner->linenumber);
		$$.l = $1.l;
		$$.t = TypeCode::Unspec;

	    }
|	IMPORT STRING ';'
	    {
		const char *name = $2.v.sval;
		y2debug ("import '%s'", name);

		// check existance of module

		TableEntry *tentry = p_parser->scanner()->localTable()->find (name, SymbolEntry::c_module);
		if (tentry == 0)
		{
		    tentry = p_parser->blockStack->theBlock->importModule (string (name), $1.l);
		    if (tentry == 0)
		    {
			yyerror ("Import failed");
			YYABORT;
		    }
		    p_parser->scanner()->localTable()->enter (tentry);
		}

		$$.c = 0;
		$$.l = $1.l;
		$$.t = TypeCode::Unspec;
	    }
|	FULLNAME STRING ';'
|	TEXTDOMAIN STRING ';'
	    {
		p_parser->blockStack->textdomain = $2.v.sval;
		$$.t = TypeCode::Unspec;
		$$.c = new YSTextdomain (p_parser->blockStack->textdomain, $1.l);
		$$.l = $1.l;
	    }
|	EXPORT symbol_list ';'
	    {
		$$.c = 0;
		$$.t = TypeCode::Unspec;
		$$.l = $1.l;
	    }
|	TYPEDEF symbol type ';'
	    {
		if (($2.t).notUnspec ())
		{
		    yyLerror ("typedef symbol already declared", $2.l);
		    $$.c = new YError ($2.l, "typedef symbol already declared");
		}
		else
		{
		    TableEntry *tentry = p_parser->blockStack->theBlock->newEntry ($2.v.nval, SymbolEntry::c_typedef, $3.t, $1.l);
		    if (tentry == 0)		/* can't happen ... */
		    {
			yyLerror ("typedef symbol duplicate", $2.l);
			$$.c = new YError ($2.l, "typedef symbol already declared");
		    }
		    else
		    {
			p_parser->scanner()->localTable()->enter (tentry);
			$$.c = new YSTypedef ($2.v.nval, $3.t, $1.l);
		    }
		}
		$$.t = TypeCode::Unspec;
		$$.l = $1.l;
	    }
|	definition
	    {
		if ($1.c == 0)
		{
		    /* was prototype declaration  */
		    $$.c = 0;
		    $$.t = TypeCode::Unspec;
		}
		else if ($1.c->isError())
		{
		    $$ = $1;
		}
		else
		{
		    /* function or variable definition  */
		    $$ = $1;
		}
	    }
|	assignment ';'
	    {
		if ($1.c != 0
		    && p_parser->blockStack->theBlock->isModule ())
		{
		    yyLerror ("Assignment not allowed in a module", $1.l);
		    YYABORT;
		}
	    }
|	term ';'
	    {
		if ($1.c->isError())
		{
		    $$.c = $1.c;
		}
		else if (p_parser->blockStack->theBlock->isModule ())
		{
		    yyLerror ("Function call not allowed in a module", $1.l);
		    YYABORT;
		}
		else
		{
		    $$.c = new YSExpression ($1.c, $1.l);
		}
		$$.t = TypeCode::Unspec;
	    }
|	block
	    {
		$$ = $1;
		if (($$.c != 0)
		    && (!($$.c->isError())))
		{
		    if (p_parser->blockStack->theBlock->isModule ())
		    {
			yyLerror ("Block not allowed in a module", $1.l);
			YYABORT;
		    }
		    ((YBlock *)$$.c)->setKind (YBlock::b_statement);
		}
	    }
|	control_statement
	    {
		if ($1.c != 0
		    && (!$1.c->isError())
		    && p_parser->blockStack->theBlock->isModule ())
		{
		    yyLerror ("Statement not allowed in a module", $1.l);
		    YYABORT;
		}
		$$ = $1;
	    }
;

control_statement:
	IF '(' expression ')' statement opt_else
	    {
		$$.l = $1.l;
		$$.t = $5.t; // default for the error cases
		if (!$3.t.isBoolean())
		{
		    yyLerror ("'if' expression not boolean", $3.l);
		    yyTypeMismatch ("b", $3.t, $3.l);
		    $$.c = new YError ($3.l, "'if' expression not boolean");
		}
		else if (($5.c != 0)
			 && (($5.c->code() == YCode::ysVariable)
			    || ($5.c->code() == YCode::ysFunction)))
		{
		    yyLerror ("Declaration must be inside block", $5.l);
		    $$.c = new YError ($5.l, "Declaration must be inside block");
		}
		else if ($6.c == 0)	// no else
		{
		    $$.c = new YSIf ($3.c, $5.c, $6.c, $1.l);
		    $$.t = $5.t;
		}
		else			// else branch given
		{
		    const TypeCode & thentype = $5.t;
		    const TypeCode & elsetype = $6.t;
		    //There used to be a TypeCode::Unspec -> TypeCode::Void conversion here. It was wrong.
		    // See the comment about types at the "expression" rule.
		    $$.t = thentype.commontype (elsetype); // FIXME LEAK

		    if (false) ;					// FIXME
		    else if (($6.c->code() == YCode::ysVariable)
			     || ($6.c->code() == YCode::ysFunction))
		    {
			yyLerror ("Declaration must be inside block", $6.l);
			$$.c = new YError ($6.l, "Declaration must be inside block");
		    }
		    else
		    {
			$$.c = new YSIf ($3.c, $5.c, $6.c, $1.l);
		    }
		}

		if ($5.c == 0)
		    yywarning("Empty statement after 'if'", $1.l);
	    }
|	WHILE '(' expression ')'
	    {
		p_parser->loopCount++;

		if (!$3.t.isBoolean())
		{
		    yyLerror ("'while' expression not boolean", $3.l);
		    yyTypeMismatch ("b", $3.t, $3.l);
		    $$.c = new YError ($3.l, "'while' expression not boolean");
		}
		else
		{
		    $$ = $3;
		}
	    }
	statement
	    {
		p_parser->loopCount--;
		if ($5.c->isError())
		{
		    $$ = $5;
		}
		else
		{
		    if (($6.c != 0)
			&& (($6.c->code() == YCode::ysVariable)
			    || ($6.c->code() == YCode::ysFunction)))
		    {
			yyLerror ("Declaration must be inside block", $6.l);
			$$.c = new YError ($6.l, "Declaration must be inside block");
		    }
		    else
		    {
			$$.c = new YSWhile ($5.c, $6.c, $1.l);
		    }
		    $$.t = $6.t;
		    if ($6.c == 0)
		    {
			yywarning("Empty statement after 'while'", $1.l);
		    }
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
		if ($3.c == 0)
		    yywarning("Empty block after 'do'", $1.l);
	    }
	WHILE '(' expression ')' ';'
	    {
		if (!$7.t.isBoolean())
		{
		    yyLerror ("'do-while' expression not boolean", $7.l);
		    yyTypeMismatch ("b", $7.t, $7.l);
		    $$.c = new YError ($7.l, "'do-while' expression not boolean");
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
		if ($3.c == 0)
		    yywarning("Empty block after 'repeat'", $1.l);
	    }
	UNTIL '(' expression ')' ';'
	    {
		if (!$7.t.isBoolean())
		{
		    yyLerror ("'repeat-until' expression not boolean", $7.l);
		    yyTypeMismatch ("b", $7.t, $7.l);
		    $$.c = new YError ($7.l, "'repeat-until' expression not boolean");
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
		    $$.c = new YError ($1.l, "'break' outside of loop.");
		}
		else
		{
		    $$.c = new YStatement (YCode::ysBreak, $1.l);
		}
		$$.t = TypeCode::Unspec;
		$$.l = $1.l;
	    }
|	CONTINUE ';'
	    {
		if (p_parser->loopCount <= 0)
		{
		    yyLerror ("'continue' outside of loop.", $1.l);
		    $$.c = new YError ($1.l, "'continue' outside of loop.");
		}
		else
		{
		    $$.c = new YStatement (YCode::ysContinue, $1.l);
		}
		$$.t = TypeCode::Unspec;
		$$.l = $1.l;
	    }
|	RETURN ';'
	    {
		$$.t = TypeCode::Void;
		$$.c = new YSReturn ((YCode *)0, $1.l);
		$$.l = $1.l;
	    }
|	RETURN expression ';'
	    {
		$$.t = $2.t;
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
	    }
;

/* -------------------------------------------------------------- */
/* types  */

type:
	C_TYPE			// type set by scanner
	// C_TYPE includes already expanded typedefs
|	LIST			{ $$.t = TypeCode::List; }
|	LIST '(' type ')'	{ $$.t = TypeCode::makeList ($3.t); }
|	MAP			{ $$.t = TypeCode::Map; }
|	MAP '(' type ')'	{ $$.t = TypeCode::makeMap ($3.t); }
|	BLOCK '(' type ')'	{ $$.t = TypeCode::makeBlock ($3.t); }
;

/* -------------------------------------------------------------- */
/* Macro/Function or variable definition */

definition:
	opt_global DEFINE symbol '('
	    {
		yyLerror ("type specifier missing after 'define'", $2.l);
		$$.c = new YError ($2.l, "type specifier missing after 'define'");
	    }
|	function_start ';'		/* function declaration */
	    {
		// end parameter block
		//
		// pop block from block stack
		// unlink local symbols from symbol table

		y2debug ("parameter block end");
		blockstack_t *top = blockstack_pop (p_parser->blockStack);
		top->theBlock->detachEnvironment (p_parser->scanner()->localTable());		// detach local table

		if ($1.c
		    && $1.c->isError())
		{
		    $$ = $1;
		}
		else
		{
		    $$.c = 0;
		}
	    }
|	function_start block			/* function definition */
	    {
		// end parameter block _after_ definition block
		//
		// pop block from block stack
		// unlink local symbols from symbol table

		y2debug ("parameter block end");
		blockstack_t *top = blockstack_pop (p_parser->blockStack);
		top->theBlock->detachEnvironment (p_parser->scanner()->localTable());		// detach local table

		$$.t = TypeCode::Unspec;			// function def is typeless
		$$.l = $1.l;

		if ($1.c
		    && $1.c->isError())
		{
		    $$.c = $1.c;		// pass error up
		    break;
		}

		if ($2.c == 0)					// empty block
		{
		    y2debug ("Empty function body");
		    $$.c = new YError ($2.l, "Empty function body");
		    break;
		}

		if ($2.c->isError ())
		{
		    y2debug ("Error body");
		    $$.c = $2.c;
		    break;
		}


		SymbolEntry *entry = $1.v.tval->sentry();
		YFunction *func = (YFunction *)(entry->code());
		func->setBody ((YBlock *)$2.c);
		$$.c = new YSFunction (entry, $1.l);

		y2debug ("Func (%s) done", $$.c->toString().c_str());

	    }
|	opt_global_symbol '=' expression ';'		/* symbol definition */
	    {
		$$.c = 0;

		if ($1.v.tval == 0)	// error in opt_global_symbol
		{
		    $$.c = $1.c;	// pass error up
		}
		else
		{
		    int match = $3.t.matchtype ($1.t);
		    if (match < 0)		// no match
		    {
			yyLerror ("type mismatch in variable definition", $1.l);
			yyTypeMismatch ($1.t, $3.t, $1.l);
			$$.c = new YError ($1.l, "type mismatch in variable definition");
		    }
		    else if (match > 0)		// propagated match
		    {
			ee.setLinenumber ($3.l);
			$3.c = new YEPropagate ($3.c, $3.t, $1.t);
			match = 0;
		    }

		    if (match == 0)		// type match ok
		    {
			TableEntry *tentry = $1.v.tval;
			tentry->sentry()->setCategory (SymbolEntry::c_variable);
			$$.c = new YSVariable (tentry->sentry(), $3.c, $1.l);
		    }
		}

		$$.l = $1.l;
		$$.t = TypeCode::Unspec;
	    }
;


/*------------------------------------------------------*/
/* function definition start				*/
/* [global] [define] type symbol '(' [type symbol]* ')	*/
/*							*/
/* enter function type+symbol to local/global symbol	*/
/* table.						*/
/* Enter (list of) formal parameters type+symbol to	*/
/* private symbol table to have them available when	*/
/* parsing the (perhaps following) definition block.	*/
/*							*/
/* $$.c = 0 if ok					*/
/*	= YError() on error				*/
/* $$.v.tval = TableEntry() (->sentry->code() == YFunction*/
/* $$.t = function type					*/
/* $$.l = symbol definition line			*/
/*							*/

function_start:
	opt_global_symbol '(' tupletypes ')'
	    {
		/*
		   $1:	$1.v.tval == entry (or 0 if error)

		   $3 == tupletypes (linked list of table entries)
		 */

		// count and check formal parameters

		formalparam_t *formalp = $3.v.fpval;

		// start parameter block before parameter checking, it's popped in any case
		y2debug ("start parameter block for '%s()'", ($1.v.tval) ? $1.v.tval->sentry()->name() : "<err>");
		YBlock *parameter_block = start_block (p_parser, TypeCode::Unspec);

		if ($1.v.tval == 0)
		{
		    $$.c = $1.c;		// pass error up
		    break;
		}

		if ($3.t.isUnspec ())					// discard parameters in case of error
		{
		    formalp = 0;
		    $$.c = new YError ($1.l, "Parameter error");
		    $$.t = TypeCode::Unspec;
		    break;
		}

		// it's a function
		SymbolEntry *sentry = $1.v.tval->sentry();
		sentry->setCategory (SymbolEntry::c_function);

		// create new function
		YFunction *func = new YFunction (parameter_block, 0);
		sentry->setCode (func);

		$$.c = 0;				// code == YSFunction
		$$.v.tval = $1.v.tval;
		$$.l = $1.l;

		// loop through formalparam_t, adding each formal
		//  parameter to the function body (private block)

		TypeCode ftype = newtype ($1.t, TypeCode::Function);

		while (formalp != 0)				// while we have parameters
		{
		    y2debug ("formal param '%s %s'@%d", formalp->type.toString().c_str(), formalp->name, formalp->line);

		    // compute function type

		    ftype = newtype (ftype, formalp->type);

		    // create symbol entry for formal parameter

		    TableEntry *tentry = parameter_block->newEntry (formalp->name, SymbolEntry::c_variable, formalp->type, $1.l);
		    if (tentry == 0)
		    {
			yyLerror ("Duplicate parameter", formalp->line);
			parameter_block->detachEnvironment (p_parser->scanner()->localTable());
			$$.c = new YError (formalp->line, "Duplicate parameter");
			delete func;
			func = 0;
			break;
		    }
		    p_parser->scanner()->localTable()->enter (tentry);

		    formalparam_t *next = formalp->next;
		    delete formalp;
		    formalp = next;

		}  // while parameters present

		if ($$.c == func)			// no errors during parameter scan
		{
		    y2debug ("func '%s'", func->toString().c_str());
		    y2debug ("ftype (%s <%s> %s)", ftype.asString().c_str(), ftype.toStringSequence().c_str(), sentry->name());
		    sentry->setType (ftype);
		    $$.t = ftype;
		    y2debug ("sentry (%p: %s)", sentry, sentry->toString().c_str());
		}
	    }
;

/*--------------------------------------------------------------*/
/* symbol, optionally prepended by 'global' or			*/
/* 'define' or 'global define'					*/
/* $$.v.tval == entry, 0 on error ($$.c == YError)		*/
/* $$.c = 0 for global, -1 for local FIXME			*/
/* $$.t = type							*/
/* $$.l = line of symbol					*/

opt_global_symbol:
	opt_global opt_define type symbol
	    {
		$$.t = $3.t;
		$$.v.tval = 0;

		if ($1.l == 1)		// global
		{
		    if (($4.t).notUnspec ())		// symbol already known
		    {
			if ($4.v.tval->sentry()->block() == 0)	// global owner
			{
			    yyTerror ("Redefinition of global symbol", $4.l, $4.v.tval);
			    $$.c = new YError ($4.l, "Redefinition of global symbol");
			}
			else
			{
			    yyTerror ("Global definition shadows local symbol", $4.l, $4.v.tval);
			    $$.c = new YError ($4.l, "Global definition shadows local symbol");
			}
		    }
		    else
		    {
			$$.v.tval = p_parser->blockStack->theBlock->newEntry ($4.v.nval, SymbolEntry::c_global, $3.t, $4.l);
		    }
		}
		else			// local
		{
		    if (($4.t).notUnspec ())			// symbol already known
		    {
			if ($4.v.tval->table() == p_parser->scanner()->globalTable())	//   in global table
			{
			    yywarning ("Definition shadows global symbol", $4.l);
			    yyTwarning ($4.v.tval);

			    $$.v.tval = p_parser->blockStack->theBlock->newEntry ($4.v.tval->key(), SymbolEntry::c_unspec, $3.t, $4.l);
			}
			else if ($4.v.tval->sentry()->block() == p_parser->blockStack->theBlock)
			{
			    yyTerror ("Redefinition of local symbol", $4.l, $4.v.tval);
			    $$.c = new YError ($4.l, "Redefinition of local symbol");
			}
			else
			{
			    // yywarning ("Definition shadows local symbol", $4.l);

			    $$.v.tval = p_parser->blockStack->theBlock->newEntry ($4.v.tval->key(), SymbolEntry::c_unspec, $3.t, $4.l);
			}
		    }
		    else			// new symbol
		    {
			y2debug ("new %s", $4.v.nval);
			$$.v.tval = p_parser->blockStack->theBlock->newEntry ($4.v.nval, SymbolEntry::c_unspec, $3.t, $4.l);
		    }
		}
		if ($$.v.tval != 0)
		{
		    if ($1.l == 1)	// global
		    {
			y2debug ("enter to global table %p", p_parser->scanner()->globalTable());
			p_parser->scanner()->globalTable()->enter ($$.v.tval);
		    }
		    else
		    {
			y2debug ("enter to local table %p", p_parser->scanner()->localTable());
			p_parser->scanner()->localTable()->enter ($$.v.tval);
		    }
		}
		$$.l = $4.l;
	    }
;

opt_global:
	GLOBAL	{ $$.l = 1;
		  if (!blockstack_at_toplevel())
		  {
		    yyLerror ("'global' declaration in nested block", $1.l);
		  }
		  if (!p_parser->blockStack->theBlock->isModule())
		  {
		    yywarning ("Useless 'global' outside of module", $1.l);
		  }
		}
|		{ $$.l = 0; }
;

opt_define:
	DEFINE	{ $$.l = 1; }
|		{ $$.l = 0; }
;

/*----------------------------------------------*/
/* zero or more formal parameters		*/
/* $$.c = undef					*/
/* $$.t = TypeCode::Unspec if error, any valid type otherwise	*/
/* $$.v.fpval = pointer to formalparam_t chain	*/

tupletypes:
	/* empty  */
	    {
		$$.v.val = 0;
		$$.c = 0;
		$$.t = TypeCode::Void;
		$$.l = 0;
	    }
|	tupletype
;

/*----------------------------------------------*/
/* one or more formal parameters		*/
/* $$.c = undef					*/
/* $$.t = 0 if error				*/
/* $$.v.fpval = pointer to formalparam_t chain	*/

tupletype:
	formal_param
	    {
		if ($1.t.isUnspec())
		{
		    $$.t = TypeCode::Unspec;
		}
		else
		{
		    $$ = $1;
		}
	    }
|	tupletype ',' formal_param
	    {
		if ($1.t.isUnspec() || $3.t.isUnspec())
		{
		   $$.t = TypeCode::Unspec;
		}
		else
		{
		    $$.t = TypeCode::Void;
		    formalparam_t *formalp = $1.v.fpval;
		    while (formalp->next != 0)		// find end of list
		    {
			formalp = formalp->next;
		    }
		    formalp->next = $3.v.fpval;		// attach to last element
		    $$ = $1;
		}
	    }
;

/*----------------------------------------------*/
/* single formal function parameter		*/
/* $$.c = 0					*/
/* $$.t = 0 if error, else type			*/
/* $$.v.fpval = pointer to formalparam_t	*/

formal_param:
	type opt_ref SYMBOL
	    {
		formalparam_t *fpval = new (formalparam_t);
		fpval->next = 0;
		fpval->name = $3.v.nval;
		fpval->type = ($2.l == 1) ? TypeCode::makeReference ($1.t) : $1.t;
		fpval->line = $3.l;
		$$.v.fpval = fpval;
		$$.t = fpval->type;
		$$.c = 0;
		$$.l = $3.l;
	    }
|	type opt_ref SYM_VARIABLE
	    {
		SymbolEntry *entry = $3.v.tval->sentry();
		if (entry->category() == SymbolEntry::c_builtin)
		{
		    p_parser->parserErrors++;
		    p_parser->scanner()->logError ("Formal parameter '%s' shadows builtin function", $3.l, entry->name());
		    $$.t = TypeCode::Unspec;
		}
		else if (entry->category() == SymbolEntry::c_function)
		{
		    p_parser->parserErrors++;
		    p_parser->scanner()->logError ("Formal parameter '%s' shadows function", $3.l, entry->name());
		    $$.t = TypeCode::Unspec;
		}
		else if (entry->category() == SymbolEntry::c_unspec)
		{
		    p_parser->parserErrors++;
		    p_parser->scanner()->logError ("Parameter '%s' shadows function name", $3.l, entry->name());
		    $$.t = TypeCode::Unspec;
		}
		else
		{
		    formalparam_t *fpval = new (formalparam_t);
		    fpval->next = 0;

		    fpval->name = entry->name();
		    fpval->type = ($2.l == 1) ? TypeCode::makeReference ($1.t) : $1.t;
		    fpval->line = $3.l;

		    $$.v.fpval = fpval;
		    $$.t = fpval->type;
		}
		$$.c = 0;
		$$.l = $3.l;
	    }
;

opt_ref:
	'&'
	    {
		$$.l = 1;
	    }
|	/* empty */
	    {
		$$.l = 0;
	    }
;
/* -------------------------------------------------------------- */
/* Assignment */

assignment:
	identifier '=' expression
	    {
		int match = $3.t.matchtype ($1.t);
		if ($1.c->isError())		// bad identifier
		{
		    $$.c = $1.c;
		}
		else if ($3.c->isError())	// bad expression
		{
		    $$.c = $3.c;
		}
		else if (match < 0)
		{
		    yyLerror ("type mismatch in assignment", $1.l);
		    yyTypeMismatch ($1.t, $3.t, $1.l);
		    $$.c = new YError ($1.l, "type mismatch in assignment");
		}
		else
		{
		    if (match > 0)
		    {
			ee.setLinenumber ($3.l);
			$3.c = new YEPropagate ($3.c, $3.t, $1.t);
		    }
		    $$.c = new YSAssign ($1.v.tval->sentry(), $3.c, $1.l);
		}
		$$.t = TypeCode::Unspec;
		$$.l = $1.l;
	    }
|	identifier '[' list_elements ']' '=' expression
	    {
		if ($1.c->isError())
		{
		    $$.c = $1.c;
		}
		else if (($1.t.matchtype (TypeCode::List) != 0)
			 && ($1.t.matchtype (TypeCode::Map) != 0)
			 && ($1.t.matchtype (TypeCode::Term) != 0))
		{
		    yyLerror ("bracket operator requires list, map, or term identifier", $1.l);
		    $$.c = new YError ($1.l, "bracket operator requires list, map, or term identifier");
		}
		else if ($3.c->isError())
		{
		    $$.c = $3.c;
		}
		else if ($6.c->isError())
		{
		    $$.c = $6.c;
		}
		else
		{
		    $$.c = new YSBracket ($1.v.tval->sentry(), $3.c, $6.c, $1.l);
		    $$.t = TypeCode::Unspec;
		    $$.l = $1.l;
		}
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
		$$.t = "s";
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
		$$.t = "Lv";	// make it different from list(any) !
		$$.l = $1.l;
	    }
|	'[' list_elements opt_comma ']'
	    {
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
		    yyerror ("YEList does not evaluate to YCPList but");
		    yyerror (list->toString().c_str());
		    $$.c = new YError ($1.l, "YEList does not evaluate to YCPList");
		}
		if ($2.t.isAny())
		{
		    $$.t = TypeCode::List;
		}
		else
		{
		    $$.t = TypeCode::makeList ($2.t);
		}
		$$.l = $1.l;
	    }
;

list_elements:
	expression
	    {
		$$.c = new YEList ($1.c);
		$$.t = $1.t;
		$$.l = $1.l;
	    }
|	list_elements ',' expression
	    {
		$$.t = $1.t.commontype ($3.t); //FIXME LEAK
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
	MAPEXPR ']'
	    {
		$$.c = new YConst (YCode::ycMap, YCPMap());
		$$.t = "Mv";
		$$.l = $1.l;
	    }
|	MAPEXPR map_elements opt_comma ']'
	    {
		if ($2.c == 0)
		{
		    $$.c = new YError ($2.l, "Bad map constant");
		}
		else
		{
		    YCPValue map = $2.c->evaluate (true);
		    if (map.isNull())	// not a constant
		    {
			$$.c = $2.c;
		    }
		    else if (map->isError())
		    {
			yyerror (map->asError()->message().c_str());
			$$.c = new YError ($2.l, "Error in map constant");
			$2.t = TypeCode::Unspec;
		    }
		    else if (map->isCode())	// expression (can this happen ?)
		    {
			$$.c = map->asCode()->code();
		    }
		    else			// YCPMap constant
		    {
			$$.c = new YConst (YCode::ycMap, map->asMap());
		    }

		    if ($2.t.isAny())
		    {
			$$.t = TypeCode::Map;
		    }
		    else if ($2.t.isUnspec ())
		    {
			$$.t = $2.t;
		    }
		    else
		    {
			$$.t = TypeCode::makeMap ($2.t);
		    }
		}

		$$.l = $1.l;
	    }
;

map_elements:
	expression ':' expression
	    {
		if (!equaltype (TypeCode::Integer, $1.t)
		    && !equaltype (TypeCode::String, $1.t)
		    && !equaltype (TypeCode::Symbol, $1.t))
		{
		    yyerror ("Bad type for key");
		    $$.c = 0;
		}
		else
		{
		    $$.c = new YEMap ($1.c, $3.c);
		    $$.t = $1.t;
		}
		$$.l = $1.l;
	    }
|	map_elements ',' expression ':' expression
	    {
		if ($1.c == 0)
		{
		    $$.c = 0;
		}
		else if (!equaltype (TypeCode::Integer, $3.t)
			 && !equaltype (TypeCode::String, $3.t)
			 && !equaltype (TypeCode::Symbol, $3.t))
		{
		    yyerror ("Bad type for key");
		    $$.c = 0;
		}
		else if (equaltype ($1.t, $3.t))
		{
		    $$.t = $1.t;
		}
		else
		{
		    $$.t = TypeCode::Any;
		}

		if ($$.c != 0)
		{
		    ((YEMap *)$1.c)->attach ($3.c, $5.c);
		    $$.c = $1.c;
		}
		$$.l = $1.l;
	    }
;

/* -------------------------------------------------------------- */
/* Terms */
/* initial parse of 'term_name (' triggers first type checking */
/* and lookup of term_name so parameters can be checked against */
/* prototype. */
/* */
/*   term: term_name[$1] '('[2] {lookup prototype}[$3] parameters[$4] ')'[$5] {check parameters} */

term:
	term_name '('
	    {
		/* this is $3  */

		if ($1.c
		    && $1.c->isError())		// bad term_name
		{
		    $$ = $1;
		    $$.t = TypeCode::Unspec;
		}
		else if (equaltype ($1.t, TypeCode ("y")))	// term_name is C_SYMBOL
		{
		    y2debug ("Term %s(...)", $1.v.nval);
		    /* C_SYMBOL  */
		    $$.c = new YETerm ($1.v.nval);
		    $$.t = TypeCode::Term;
		}
		else						// term_name is function or builtin
		{
		    SymbolEntry *sentry = $1.v.tval->sentry();

		    if (sentry->category() == SymbolEntry::c_builtin)	// a builtin function
		    {
			y2debug ("Builtin! (%s)", sentry->toString().c_str());
			// found builtin declaration

			// check for overloads, those must be re-checked
			// for every parameter in order to match the
			// type-correct declaration

			declaration_t *decl = (declaration_t *)(sentry->code());
			$$.v.val = decl;
			if ((decl->next != 0)				// if overloaded
			    || (decl->flags & DECL_SYMBOL))		// or can have a symbol as parameter
			{
			    // start block for possible symbol parameters
			    YBlock *block = start_block (p_parser, TypeCode::Unspec);
			    y2debug ("opening parameter scope block %p for %s", block, sentry->name());

			    $$.c = new YESymFunc (decl, block);
			    y2debug ("SymFunc ...");
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
		    else if (equaltype (sentry->type(), TypeCode::Term)) // a term
		    {
			y2debug ("Term! (%s)", sentry->toString().c_str());
			$$.c = new YETerm (sentry->name());
			$$.t = sentry->type();
			y2debug ("Term ...");
		    }
		    else
		    {
			y2debug ("Function! (%s)", sentry->toString(true).c_str());
			$$.c = new YEFunction (sentry);			// an extern function
			$$.t = sentry->type();
			y2debug ("Function ...");
		    }
		}

		/* end of $3 */
	    }
	parameters ')'
	    {
		/* $3 == function
		   check $3.c for (YError, YETerm, YEBuiltin, YEFunction, YESymFunc)
		   $3.v.val == first matching decl for function name

		   $4.c == 0 if 'parameters' empty
		   $4.v.val == decl if YEBuiltin matched for 'term_name(parameters)' */

		if ($4.c != 0 && $4.c->isError())			// error in parameters
		{
		    $$.c = $4.c;
		    $$.t = TypeCode::Unspec;
		}
		else
		{
		    // if no parameters, use the first matched decl we looked
		    // up during the initial term_name check

		    if ($4.c == 0)
		    {
			$4.v.val = $3.v.val;
		    }

		    switch ($3.c->code())		// depends on what term_name is
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

			    if (builtin->setDecl ((declaration_t *)($4.v.val)) == 0)
			    {
				yyLerror ("Parameters don't match declaration:", $4.l);
				yyLerror (Decl2String((declaration_t *)($4.v.val), true).c_str(), $4.l);
				$$.c = new YError ($4.l, "Parameters don't match declaration");
				$$.t = TypeCode::Unspec;
			    }
			    else
			    {
				y2debug ("yeBuiltin matched");
				$$.c = $3.c;
				$$.t = builtin->returnType();
			    }
			}
			break;
			case YCode::yeSymFunc:			// an intern function
			{
			    y2debug ("YCode::yeSymFunc");
			    // yeSymFunc matched
			    YESymFunc *symfunc = (YESymFunc *)$3.c;
			    declaration_t *decl = (declaration_t *)($4.v.val);

			    // do final parameter check

			    if (symfunc->setDecl (decl) == 0)
			    {
				yyLerror ("Parameters don't match declaration:", $4.l);
				yyLerror (Decl2String(decl, true).c_str(), $4.l);
				// clean up environment
				symfunc->toBuiltin (p_parser->scanner()->localTable());
				delete symfunc;
				$$.c = new YError ($4.l, "Parameters don't match declaration");
				$$.t = TypeCode::Unspec;
			    }
			    else
			    {
				y2debug ("yeSymFunc matched");
				$$.t = symfunc->returnType();
				$$.c = symfunc->toBuiltin (p_parser->scanner()->localTable());
				delete symfunc;
			    }

			    if (decl->flags & DECL_LOOP)
			    {
				p_parser->loopCount--;
			    }
			}
			break;
			case YCode::yeFunction:			// an extern function
			{
			    y2debug ("YCode::yeFunction");
			    YEFunction *function = (YEFunction *)$3.c;
			    const TypeCode & t = function->attachFunctionParameter (0, "");	// close parameter list
			    if (t.notUnspec ()) // error: t = expected type
			    {
				yyMissingArgument (t, $1.l);
				$$.c = new YError ($1.l, "Missing argument");
			    }
			    else
			    {
				// yeFunction matched
				y2debug ("yeFunction matched");
				$$.c = $3.c;
				$$.t = $3.t.returnType ();
			    }
			}
			break;
			default:				// anything else is an error
			{
			    y2debug ("Error");
			    $$.c = new YError ($1.l, "Unknown expression");
			    $$.t = TypeCode::Unspec;
			}
			break;

		    } // switch ($3.c->code())

		} // $4.c valid


		// check if symfunc parameter block must be popped from stack

		if ($3.c->code() == YCode::yeSymFunc)
		{
		    // end block for possible symbol parameters

		    blockstack_t *top = blockstack_pop (p_parser->blockStack);
		    YESymFunc *sf = (YESymFunc *)($3.c);
		    y2debug ("closing parameter scope block %p for %s", top->theBlock, sf->decl()?sf->decl()->name:"<err>");
		}

		$$.l = $1.l;

		y2debug ("fcall (%s %s)", $$.t.toString().c_str(), $$.c->toString().c_str());
	    }
;

/*
   function name

   might be a known identifier (normal function call)
   or a symbol constant (YCP Term)

   -> $$.c = YError() if error, else 0
      $$.t = 0
      $$.v.tval = table entry
 */

term_name:
	SYM_FUNCTION
	    {
		$$ = $1;
		$$.c = 0;
		y2debug ("function <sentry %p>'[%s]:%s' !", $$.v.tval->sentry(), $$.v.tval->sentry()->type().asString().c_str(), $$.v.tval->sentry()->name());
	    }
|	SYM_BUILTIN
	    {
		$$ = $1;
		$$.c = 0;
		y2debug ("builtin '[%s]:%s' !", $$.v.tval->sentry()->type().asString().c_str(), $$.v.tval->sentry()->name());
	    }
|	SYMBOL
	    {
		/* undeclared identifier  */
		yyVerror ($1.v.sval, $1.l);
		$$.c = new YError ($1.l, "undeclared identifier");
		$$.l = $1.l;
	    }
|	C_SYMBOL
	    {
		$$.c = $1.c;
		$$.t = TypeCode ("y");
		$$.l = $1.l;
	    }
;

/*
   function call parameters

   since we're using the $0 feature of bison here, we can't
   split up this BNF further :-(

   $0 refers to $$ of the 'term' rule, ie $0.c is the function (one of 4 kinds)
 */

parameters:
	/* empty  */
	    {
		y2debug ("Empty parameters (%p)", $0.c);
		$$.t = TypeCode::Unspec;
		$$.c = 0;
	    }
|	type symbol
	    {
		y2debug ("parameters: type/name ($0.c@%p, [%s '%s'])", $0.c, $1.t.asString().c_str(), $2.t.isUnspec() ? $2.v.nval : $2.v.tval->key());

		if ($0.c->isError())
		{
		    $$.c = $0.c;
		}
		else if ($0.c->code () != YCode::yeSymFunc)
		{
		    yyLerror ("Unexpected symbol parameter", $2.l);
		    $$.c = new YError ($2.l, "Unexpected symbol parameter");
		}
		else
		{
		    /* $1.t == type, $2.v symbol*/
		    if (attach_parameter (p_parser, $0.c, &($1), &($2)) == 0)
		    {
			string s = "Bad parameter '" + $1.t.toString() + " " + ($2.t.isUnspec() ? $2.v.nval : $2.v.tval->key()) + "'";
			yyLerror (s.c_str(), $2.l);
			$$.c = new YError ($2.l, s.c_str());
		    }
		    else
		    {
			$$.c = $0.c;
		    }
		}
	    }
|	expression
	    {

		y2debug ("parameters: expression (%p)", $1.c);

		if ($0.c->isError())
		{
		    $$.c = $0.c;
		    break;
		}

		if ($1.c != 0)
		{
		    if ($1.c->isError())
		    {
			$$.c = $1.c;
			break;
		    }
		    $$.v.val = attach_parameter (p_parser, $0.c, &($1));
		}

		if ($1.c == 0)
		{
		    yyLerror ("Invalid (nil) parameter", $1.l);
		    $$.c = new YError ($1.l, "Invalid (nil) parameter");
		}
		else if ($0.c->code() == YCode::yeTerm)
		{
		    // Term allows anything
		    $$.c = $1.c;
		}
		else if ($0.c->code() == YCode::yeBuiltin)
		{
		    if ($$.v.val == 0)	// type error, no matching decl found
		    {
			yyCerror ($1.c, $1.t, $1.l);
			$$.c = new YError ($1.l, "Bad parameter");
		    }
		    else
		    {
			$$.c = $1.c;
		    }
		}
		else if ($0.c->code() == YCode::yeSymFunc)
		{
		    if ($$.v.val == 0)	// type error, no matching decl found
		    {
			yyCerror ($1.c, $1.t, $1.l);
			$$.c = new YError ($1.l, "Bad parameter");
		    }
		    else
		    {
			$$.c = $1.c;
		    }
		}
		else if ($0.c->code() == YCode::yeFunction)
		{
		    if ($$.v.val != 0)
		    {
			yyLerror ("Function type mismatch", $1.l);
			yyTypeMismatch ($$.v.nval, $1.t, $1.l);
			$$.c = new YError ($1.l, "Type mismatch");
		    }
		    else
		    {
			$$.c = $1.c;
		    }
		}
		else
		{
		    $$.c = $1.c;
		}
	    }
|	parameters ',' type symbol
	    {
		y2debug ("parameters: parameters, type/name (%p)", $0.c);

		if ($1.c == 0)
		{
		    yyerror ("Missing expression before ','");
		    $$.c = new YError ($1.l, "Missing expression before ','");
		}
		else if ($1.c->isError())
		{
		    $$.c = $1.c;
		}
		else if ($0.c->code () != YCode::yeSymFunc)
		{
		    yyLerror ("Unexpected symbol parameter", $4.l);
		    $$.c = new YError ($4.l, "Unexpected symbol parameter");
		}
		else
		{
		    /* $3.t == type, $4 symbol*/
		    if (attach_parameter (p_parser, $0.c, &($3), &($4)) == 0)
		    {
			yyLerror ("Bad parameter", $1.l);
			$$.c = new YError ($1.l, "Bad parameter");
		    }
		    else
		    {
			$$.c = $0.c;
		    }
		}
	    }
|	parameters ',' expression
	    {
		if ($1.c == 0)
		{
		    yyerror ("Missing expression before ','");
		    $$.c = new YError ($1.l, "Missing expression before ','");
		}
		else if ($1.c->isError())
		{
		    $$.c = $1.c;
		}
		else if ($3.c == 0)
		{
		    yyLerror ("Invalid (nil) parameter", $3.l);
		    $$.c = new YError ($3.l, "Invalid (nil) parameter");
		}
		else
		{
		    $$.v.val = attach_parameter (p_parser, $0.c, &($3));
		    if ($0.c->code() == YCode::yeBuiltin)
		    {
			if ($$.v.val == 0)
			{
			    yyLerror ("Bad parameter", $3.l);
			    $$.c = new YError ($3.l, "Bad parameter");
			}
			else
			{
			    $$.c = $1.c;
			}
		    }
		    else if ($0.c->code() == YCode::yeSymFunc)
		    {
			if ($$.v.val == 0)
			{
			    yyLerror ("Bad parameter", $3.l);
			    $$.c = new YError ($3.l, "Bad parameter");
			}
			else
			{
			    $$.c = $1.c;
			}
		    }
		    else	// assume yeFunction
		    {
			if ($$.v.val != 0)
			{
			    yyLerror ("Parameter type mismatch", $3.l);
			    yyTypeMismatch ($$.v.nval, $3.t, $3.l);
			    $$.c = new YError ($3.l, "Type mismatch");
			}
			else
			{
			    $$.c = $1.c;
			}
		    }
		}
	    }
;

/* -------------------------------------------------------------- */
/* Identifiers (known Locals or Globals) */
/* -> $$.c = owner (if local and $$.t valid), or 0 (if global)
      $$.t = TypeCode::Unspec if error, else known type
      $$.v.tval = table entry
 */

identifier:
	SYM_VARIABLE
	    {
		SymbolEntry *entry = $1.v.tval->sentry();
		if (entry->category() == SymbolEntry::c_unspec)
		{
		    yyVerror (entry->name(), $1.l);
		    $$.c = new YError ($1.l, "undeclared identifier");
		    $$.t = TypeCode::Unspec;
		}
		else
		{
		    $$.c = new YEVariable (entry);
		    $$.t = entry->type();
		    y2debug ("identifier '<%s>%s' !", $$.t.toString().c_str(), entry->name());
		}
		$$.l = $1.l;
	    }
|	SYMBOL
	    {
		/* undeclared identifier  */
		yyVerror ($1.v.sval, $1.l);
		$$.c = new YError ($1.l, "undeclared identifier");
		$$.t = TypeCode::Unspec;
		$$.l = $1.l;
	    }
;

/* -------------------------------------------------------------- */
/* Symbols (known and unknown names)  */
/* -> $$.v.tval == TableEntry if symbol already declared ($$.t != TypeCode::Unspec)
      $$.v.nval == charptr if symbol undefined ($$.t == TypeCode::Unspec)
      $$.t = TypeCode::Unspec for SYMBOL, "|" for builtin, else type
 */

symbol:
	SYM_VARIABLE
	    {
		// known token
		y2debug ("known variable '%s' !", $1.v.tval->sentry()->name());
		$$.v.tval = $1.v.tval;
		$$.t = $1.v.tval->sentry()->type();
		$$.l = $1.l;
	    }
|	SYM_FUNCTION
	    {
		// known token
		y2debug ("known function '%s' !", $1.v.tval->sentry()->name());
		$$.v.tval = $1.v.tval;
		$$.t = $1.v.tval->sentry()->type();
		$$.l = $1.l;
	    }
|	SYM_BUILTIN
	    {
		// known token
		y2debug ("known builtin '%s' !", $1.v.tval->sentry()->name());
		$$.v.tval = $1.v.tval;
		// builtin function, type incomplete
		$$.t = TypeCode::Function;
		$$.l = $1.l;
	    }
|	SYMBOL
	    {
		y2debug ("<new> symbol '%s' !", (const char *)$1.v.nval);
		$$.v.nval = $1.v.nval;
		$$.t = TypeCode::Unspec;
		$$.l = $1.l;
	    }
;

symbol_list:
	symbol
|	symbol ',' symbol_list
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
	if (current != 0)							// back to old filename
	{
	    current->attachStatement (new YSFilename (top->filename, top->linenumber));
	}
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
		    yyerror_with_lineinfo ("not a path constant", pr, -1);
		    return 0;
		}
		*store_here = new YConst (YCode::ycPath, path);
	    }
	    break;
	    case C_SYMBOL:	*store_here = new YConst (YCode::ycSymbol,	YCPSymbol (value.yval)); break;
	}
    }
    return token;
}
} // extern "C"


static void
yyerror_with_lineinfo(const char *s, Parser *parser, int lineno)
{
    parser->parserErrors++;
    parser->scanner()->logError (s, (lineno>0)?lineno:parser->lineno);
}


static void
yywarning_with_lineinfo(const char *s, Parser *parser, int lineno)
{
    parser->scanner()->logWarning (s, (lineno>0)?lineno:parser->lineno);
}


static void
yyerror_with_code (YCode *c, TypeCode &t, Parser *parser, int lineno)
{
    parser->parserErrors++;
    parser->scanner()->logError ("Bad parameter '<%s> %s'", (lineno>0)?lineno:parser->lineno, t.toString().c_str(), c ? c->toString().c_str() : "<err>");
}


static void
yyerror_with_name(const char *name, Parser *parser, int lineno)
{
    parser->parserErrors++;
    parser->scanner()->logError ("Undeclared identifier '%s'", (lineno>0) ? lineno : parser->lineno, name);
}


static void
yyerror_with_file(const char *name, Parser *parser, int lineno)
{
    parser->parserErrors++;
    parser->scanner()->logError ("Bad or unknown file '%s'", (lineno>0)?lineno:parser->lineno, name);
}


static void
yyerror_with_tableentry (Parser *parser, const char *s, int lineno, TableEntry *entry)
{
    parser->parserErrors++;
    parser->scanner()->logError (s, (lineno>0)?lineno:parser->lineno);
    parser->scanner()->logError ("'%s' defined here.", entry->line(), entry->key());
}


static void
yywarning_with_tableentry (Parser *parser, TableEntry *entry)
{
    parser->scanner()->logWarning ("'%s' defined here.", entry->line(), entry->key());
}


static void
yyerror_type_mismatch (Parser *parser, const TypeCode & expected_type, const TypeCode & seen_type, int lineno)
{
    parser->parserErrors++;
    parser->scanner()->logError ("Expected '%s', seen '%s'.", lineno, expected_type.toString().c_str(), seen_type.toString().c_str());
}


static void
yyerror_missing_argument (Parser *parser, const TypeCode & type, int lineno)
{
    parser->parserErrors++;
    parser->scanner()->logError ("Missing '%s' parameter.", lineno, type.toString().c_str());
}


/*
  check unary operator

  result = pointer to $$ for return value
  e1 = expression
  op = unary operator (i.e "!", "-", ...)

*/

static void
check_unary_op (YYSTYPE *result, YYSTYPE *e1, const char *op)
{
    const TypeCode & t = TypeCode::makeFunction (e1->t);
    declaration_t *decl = static_declarations.findDeclaration (op, t);

    if (decl == 0)
    {
	result->c = new YError (e1->l, "Operator not defined for this type");
	result->t = TypeCode::Unspec;
	return;
    }

    result->c = new YEUnary(decl, e1->c);
    result->t = static_declarations.returnType (decl);
    result->l = e1->l;

    y2debug ("check_unary_op (%s/%s) good (ret = %s)", op, e1->t.asString().c_str(), result->t.asString().c_str());

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
check_binary_op (YYSTYPE *result, YYSTYPE *e1, const char *op, YYSTYPE *e2)
{
    TypeCode t = newtype (TypeCode::makeFunction (e1->t), e2->t);
    declaration_t *decl = static_declarations.findDeclaration (op, t, true);

    if (decl == 0)		// plain failed, try propagation
    {
	int e1_to_e2 = e1->t.matchtype (e2->t);
	int e2_to_e1 = e2->t.matchtype (e1->t);
	if ((e1_to_e2 > e2_to_e1)
	    && (e2_to_e1 > 0))
	{
	    TypeCode t1 = newtype (TypeCode::makeFunction (e2->t), e2->t);		// propagate e1->t --> e2->t
	    decl = static_declarations.findDeclaration (op, t1);
	    if (decl != 0)
	    {
		ee.setLinenumber (e1->l);
		e1->c = new YEPropagate (e1->c, e1->t, e2->t);
		t = t1;
	    }
	}
	else if ((e2_to_e1 > e1_to_e2)
		  && (e1_to_e2 > 0))
	{
	    TypeCode t1 = newtype (TypeCode::makeFunction (e1->t), e1->t);		// propagate e2->t --> e1->t
	    decl = static_declarations.findDeclaration (op, t1);
	    if (decl != 0)
	    {
		ee.setLinenumber (e2->l);
		e2->c = new YEPropagate (e2->c, e2->t, e1->t);
		t = t1;
	    }
	}

	if (decl == 0)
	{
	    result->c = new YError (e1->l, "Binary operator not defined for this type");
	    result->t = TypeCode::Unspec;
	    static_declarations.findDeclaration (op, t);	// trigger error output
	}
    }

    if (decl != 0)		// if plain or propagation matched
    {
	result->c = new YEBinary (decl, e1->c, e2->c);
	result->t = static_declarations.returnType (decl);
	result->l = e1->l;
    }

    y2debug ("check_binary_op (%s/%s/%s) good (ret = '%s')", e1->t.toString().c_str(), op, e2->t.toString().c_str(), result->t.toString().c_str());
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
    result->t = TypeCode::Unspec;

    if (e1->c == 0)
    {
	yyerror_with_lineinfo ("Bad (nil) expression", parser, e1->l);
	result->c = new YError (e1->l, "Bad (nil) expression");
    }
    else if (e2->c == 0)
    {
	yyerror_with_lineinfo ("Bad (nil) expression", parser, e2->l);
	result->c = new YError (e2->l, "Bad (nil) expression");
    }
    else if (e1->c->isError())
    {
	result->c = e1->c;
    }
    else if (e2->c->isError())
    {
	result->c = e2->c;
    }
    else
    {
	int e1_match_e2 = e1->t.matchtype (e2->t);
	int e2_match_e1 = e2->t.matchtype (e1->t);

	result->c = 0;

	if ((e1_match_e2 < 0)						// not comparable types
	    && (e2_match_e1 < 0))
	{
	    yyerror_type_mismatch (parser, e1->t, e2->t, e2->l);
	    result->c = new YError (e2->l, "Type mismatch");
	}
	else if ((e1_match_e2 > e2_match_e1)
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

	if (result->c == 0)		// not set yet
	{
	    result->c = new YECompare (e1->c, op, e2->c);
	    result->t = "b";
	}

	result->l = e1->l;
    }

    y2debug ("check_compare_op '%s'", result->c->toString().c_str());
    return;
}


// attach parameter 'parm' to YEBuiltin/YEFunction/YETerm 'code'
//
// for YEBuiltin:
//   return declaration_t if type matched
//   return 0 if type did not match
// for YESymFunc:
//   return declaration_t if type matched
//   return 0 if type did not match
// for YEFunction:
//   return == 0 if type matched
//   return type if type did not match
// for YETerm:
//   return 0
//
// if parameter is 'type symbol': parm->t == type,
//				    parm1.t.isUnspec() ? parm1.v.nval = symbol name : parm1->v.tval == table entry
// if parameter is 'expression':  parm->c == code, parm1 == 0

static declaration_t *
attach_parameter (Parser *parser, YCode *code, YYSTYPE *parm, YYSTYPE *parm1)
{
    y2debug ("attach_parameter (code %p, p %p, p1 %p)", code, parm, parm1);
    ee.setLinenumber (parm->l);
    switch (code->code())
    {
	case YCode::yeBuiltin:
	{
	    y2debug ("YCode::yeBuiltin:");
	    YEBuiltin *builtin = (YEBuiltin *)code; 
	    y2debug ("attach_parameter builtin ([%s]%s:%s)", builtin->toString().c_str(), parm->c->toString().c_str(), parm->t.asString().c_str());
	    return builtin->attachBuiltinParameter (parm->c, parm->t);
	}
	break;
	case YCode::yeSymFunc:
	{
	    y2debug ("YCode::yeSymFunc");
	    YESymFunc *symfunc = (YESymFunc *)code;
 
	    // check if 'func (..., `x, ...)' is to be interpreted as
	    //	func (..., `x, ...) or func (..., any x, ...)
	    if ((parm1 == 0)						// parameter is an expression (not 'type symbol')
		&& ((parm->c->code() != YCode::ycSymbol)		//  not a symbol
		     || (parm->c->isConstant()				//   or a constant
			 && ((YConst *)(parm->c))->value().isNull())))	//   and nilsymbol
	    {
		TypeCode type = parm->t;
		y2debug ("attach_parameter symfunc expr ([%s]%s:%s)", symfunc->toString().c_str(), parm->c->toString().c_str(), type.asString().c_str());
		if (parm->c->code() == YCode::yeBlock)
		{
		    // parameter is block
		    y2debug ("parameter is block");

		    YBlock *b = (YBlock *)(parm->c);
		    YSReturn *ret = b->justReturn();
		    if (ret != 0)
		    {
			parm->c = new YEReturn (ret->value());
			ret->clearValue ();
			delete b;
		    }
		    type = type.codify ();		// code as parameter is always codified
		}
		return symfunc->attachSymValue (type, parm->l, parm->c);
	    }
	    else								// attach type/name or symbol expression
	    {
		declaration_t *decl = 0;
		TableEntry *tentry = 0;
		y2debug ("check for 'type name' or 'symbol' expression");

		// check for "type name" or "symbol" expression

		if (parm1 != 0)		// symbol expression
		{
		    // attach entry to parameter block of function
		    if (parm1->t.isUnspec())
		    {
			decl = symfunc->attachSymVariable (parm1->v.nval, parm->t, parm->l, tentry);
		    }
		    else
		    {
			decl = symfunc->attachSymVariable (parm1->v.tval->key(), parm->t, parm->l, tentry);
		    }
		}
		else
		{
		    // possibly convert "func(`x, ...)" to "func (any x, ...)" depending on function prototype
		    decl = symfunc->attachSymVariable (parm->v.nval, TypeCode::Unspec, parm->l, tentry);
		}

		if ((decl != 0)
		    && (tentry != 0))
		{
		    y2debug ("Enter %s to local table", tentry->toString().c_str());
		    parser->scanner()->localTable()->enter (tentry);
		}

		return decl;
	    }
	}
	break;
        case YCode::yeFunction:
	{
	    y2debug ("YCode::yeFunction:");
	    YEFunction *func = (YEFunction *)code; 
	    y2debug ("attach_parameter func ([%s]%s:%s)", func->toString().c_str(), parm->c->toString().c_str(), parm->t.asString().c_str());
	    // not checked anyway? FIXME
	    TypeCode expected = func->attachFunctionParameter (parm->c, parm->t);
	    return expected.isUnspec() ? 0 : (declaration_t *) strdup (expected.asString().c_str());
	}
	break;
	case YCode::yeTerm:
	{
	    y2debug ("YCode::yeTerm:");
	    YETerm *term = (YETerm *)code; 
	    y2debug ("attach_parameter term ([%s]%s)", term->toString().c_str(), parm->c->toString().c_str());
	    term->attachTermParameter (parm->c);
	    return 0;
	}
	default:
	{
	    y2error ("attach_parameter to unknown (%s)", code->toString().c_str());
	}
	break;
    }
    return 0;
}

//-------------------------------------------------------------------
// stack handling

static void
stack_push (stack_t **stack, stack_t *element)
{
    element->down = *stack;
    *stack = element;
}

// pop element to stack
static stack_t *stack_pop (stack_t **stack)
{
    stack_t *element = 0;
    if (*stack)
    {
	element = *stack;
	*stack = element->down;
	blockstack_depth--;
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
start_block (Parser *parser, const TypeCode & type)
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
    top->theBlock = new YBlock ();
    if (blockstack_empty())
    {
	current = top->theBlock;
	current->attachStatement (new YSFilename (parser->filename()));
    }
    // inherit textdomain from outer block
    top->textdomain = (parser->blockStack ? parser->blockStack->textdomain : 0);
    top->type = type;
    top->includeDepth = 0;

    blockstack_push (parser->blockStack, top);
    blockstack_depth++;

    y2debug ("Push block#%d, top %p, down %p", blockstack_depth, parser->blockStack->theBlock, parser->blockStack->down?((blockstack_t *)parser->blockStack->down)->theBlock:0);

    return top->theBlock;
}
