/*----------------------------------------------------------------------\
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

#include <YCP.h>
#include <ycp/YCPScanner.h>
#include <y2log.h>
#include <parserret.h>
#include <ycp/YCPScope.h>

// compile with full debug info, enable with YCP_YYDEBUG=1 in run-time env
#define YYDEBUG 1
#define YYERROR_VERBOSE 1

// define type of parser values
typedef struct { YCPElement e; int l; } yystype_type;
#define YYSTYPE yystype_type

#define YYPARSE_PARAM parser_ret
#define YYLEX_PARAM parser_ret

#ifdef YYLSP_NEEDED
#warning YYLSP_NEEDED
#endif

#define LINE_NOW (((struct parserret *)parser_ret)->lineno)
#define FILE_NOW (((struct parserret *)parser_ret)->filename)

// our private error function
static void yyerror_with_lineinfo (const char *s, void *pr_void);
#define yyerror(text) yyerror_with_lineinfo(text, parser_ret)


extern "C" {
int yylex(YYSTYPE *, void *);
}

static YCPValue deepquoted(const YCPValue& v);

static string inside_module = "";

// someone out there needs this var
int yyeof_reached = 0;

// All module declarations are global

static int module_level = 0;

static YCPScope scope;
static string current_textdomain;
%}

%pure_parser

  /* SCANNER_ERROR is returned when yylex does not have a valid token */
%token  SCANNER_ERROR
%token  EMPTY LIST DEFINE UNDEFINE I18N
%token  RETURN CONTINUE BREAK IF DO WHILE REPEAT UNTIL IS ISNIL
%token  SYMBOL QUOTED_SYMBOL
%token  DCOLON UI WFM Pkg Perl SCR
%token  QUOTED_BLOCK QUOTED_EXPRESSION
%token	YCP_VOID YCP_BOOLEAN YCP_INTEGER YCP_FLOAT YCP_STRING YCP_TIME YCP_BYTEBLOCK YCP_PATH
%token  ANY YCP_DECLTYPE MODULE IMPORT EXPORT MAPEXPR INCLUDE GLOBAL TEXTDOMAIN
%token	DUMPSCOPE MEMINFO SIZE LOOKUP SELECT REMOVE FOREACH EVAL SYMBOLOF
%token  CONST FULLNAME CALLBACK UNION MERGE ADD CHANGE SORT LSORT CLOSEBRACKET

 /* bindings in order of precedence, lowest first  */

%right '='
%left '?'
%left OR
%left AND
%left EQUALS NEQ
%left ST GT SE GE
%left '+' '-'
%left '*' '/' '%'
%left LEFT RIGHT
%right NOT
%left ELSE
%left '|'
%left '&'
%right '~'
%right UMINUS
%left DCOLON

/* ---------------------------------------------------------------------- */
%%

ycp:
	module {
	    $1.e->asValue()->asBlock()->setFileName (FILE_NOW);
	    ((struct parserret *)parser_ret)->result = $1.e->asValue();
	    ((struct parserret *)parser_ret)->lineno = $1.l;
	    YYACCEPT;
	}
|	compact_expression	{
		if ($1.e->asValue()->isBlock())
		{
		    $1.e->asValue()->asBlock()->setFileName (FILE_NOW);
		}
		((struct parserret *)parser_ret)->result = $1.e->asValue();
		((struct parserret *)parser_ret)->lineno = $1.l;
		YYACCEPT;
	}
|	{ /* EOF  */
	  $$.e = YCPNull();
	}
;

/* Expressions */

  /* expressions are either 'compact' (with a defined end-token, no lookahead)
     or 'infix' (which might need a lookahead token)  */

expression:
	compact_expression
|	infix_expression
|	typecast_expression
;

typecast_expression:
	'(' typedecl ')' expression	{ $$.e = $4.e; $$.l = $2.l; }
;

compact_expression:
	block
|	'(' expression ')'	{ $$.e = $2.e; $$.l = $2.l; }
|	QUOTED_EXPRESSION expression ')'	{
		$$.e = deepquoted ($2.e->asValue()); $$.l = $2.l; }
|	quoted_block
|	term
|	tuple
|	map
|	constant
|	I18N YCP_STRING ',' YCP_STRING ',' expression ')'	{
		YCPBuiltin b (YCPB_NLOCALE);
		b->add ($2.e->asValue());
		b->add ($4.e->asValue());
		if ($2.e->asValue()->asString()->value().empty()
		    || $4.e->asValue()->asString()->value().empty())
		{
		    yyerror ("empty locale string"); YYERROR;
		}
		b->add ($6.e->asValue());
		$$.e = b;
		$$.l = LINE_NOW;
	}
|	I18N YCP_STRING ')'	{
		if ($2.e->asValue()->asString()->value().empty())
		{
		    yyerror ("empty locale string"); YYERROR;
		}
		$$.e = YCPLocale($2.e->asValue()->asString());
		$$.l = LINE_NOW;
	}
|	IS '(' expression ',' typedecl ')'	{
		YCPBuiltin b (YCPB_IS);
		b->add ($3.e->asValue());
		b->add ($5.e->asValue());
		$$.e = b;
		$$.l = LINE_NOW;
	}
|	ISNIL '(' expression ')'	{
		YCPBuiltin b (YCPB_ISNIL);
		YCPValue v = $3.e->asValue();
		if (!(v->valuetype() == YT_SYMBOL))
		{
		    yyerror ("isnil can only check symbols"); YYERROR;
		}
		yyerror ("Warning: use 'x == nil' instead of isnil(x)");
		b->add (v);
		$$.e = b;
		$$.l = LINE_NOW;
	}
|	CALLBACK '(' expression ')'	{
		$$.e = YCPBuiltin (YCPB_CALLBACK, $3.e->asValue());
		$$.l = LINE_NOW;
	}
|	TEXTDOMAIN '(' expression ')'	{
		$$.e = YCPBuiltin (YCPB_TEXTDOMAIN, $3.e->asValue());
		$$.l = LINE_NOW;
	}
;

infix_expression:
	expression '+' expression {
		YCPBuiltin b(YCPB_PLUS);
		b->add($1.e->asValue());
		b->add($3.e->asValue());
		$$.e = b; $$.l = LINE_NOW; }
|	expression '-' expression {
		YCPBuiltin b(YCPB_MINUS);
		b->add($1.e->asValue());
		b->add($3.e->asValue());
		$$.e = b; $$.l = LINE_NOW; }
|	expression '*' expression {
		YCPBuiltin b(YCPB_MULT);
		b->add($1.e->asValue());
		b->add($3.e->asValue());
		$$.e = b; $$.l = LINE_NOW; }
|	expression '/' expression    {
		YCPBuiltin b(YCPB_DIV);
		b->add($1.e->asValue());
		b->add($3.e->asValue());
		$$.e = b; $$.l = LINE_NOW; }
|	expression '%' expression    {
		YCPBuiltin b(YCPB_MOD);
		b->add($1.e->asValue());
		b->add($3.e->asValue());
		$$.e = b; $$.l = LINE_NOW; }
|	expression LEFT expression    {
		YCPBuiltin b(YCPB_LEFT);
		b->add($1.e->asValue());
		b->add($3.e->asValue());
		$$.e = b; $$.l = LINE_NOW; }
|	expression RIGHT expression    {
		YCPBuiltin b(YCPB_RIGHT);
		b->add($1.e->asValue());
		b->add($3.e->asValue());
		$$.e = b; $$.l = LINE_NOW; }
|	expression '&' expression    {
		YCPBuiltin b(YCPB_AND);
		b->add($1.e->asValue());
		b->add($3.e->asValue());
		$$.e = b; $$.l = LINE_NOW; }
|	expression '|' expression    {
		YCPBuiltin b(YCPB_OR);
		b->add($1.e->asValue());
		b->add($3.e->asValue());
		$$.e = b; $$.l = LINE_NOW; }
|	'~' expression    {
		YCPBuiltin b(YCPB_BNOT);
		b->add($2.e->asValue());
		$$.e = b; $$.l = LINE_NOW; }
|	expression AND expression {
		YCPBuiltin b(YCPB_LOGAND);
		b->add($1.e->asValue());
		b->add($3.e->asValue());
		$$.e = b; $$.l = LINE_NOW; }
|	expression OR expression {
		YCPBuiltin b(YCPB_LOGOR);
		b->add($1.e->asValue());
		b->add($3.e->asValue());
		$$.e = b; $$.l = LINE_NOW; }
|	expression EQUALS expression  {
		YCPValue e1 = $1.e->asValue();
		YCPValue e2 = $3.e->asValue();

		/* map "x == nil" to builtin "isnil (x)"  */

		if ((e1->valuetype() == YT_VOID)
		    && (e2->valuetype() == YT_SYMBOL))
		{
		    $$.e = YCPBuiltin (YCPB_ISNIL, e2);
		}
		else if ((e2->valuetype() == YT_VOID)
			 && (e1->valuetype() == YT_SYMBOL))
		{
		    $$.e = YCPBuiltin (YCPB_ISNIL, e1);
		}
		else
		{
		    YCPBuiltin b (YCPB_EQ, e1);
		    b->add(e2);
		    $$.e = b;
		}
		$$.l = LINE_NOW; }
|	expression ST expression  {
		YCPBuiltin b(YCPB_ST);
		b->add($1.e->asValue());
		b->add($3.e->asValue());
		$$.e = b; $$.l = LINE_NOW; }
|	expression GT expression  {
		YCPBuiltin b(YCPB_GT);
		b->add($1.e->asValue());
		b->add($3.e->asValue());
		$$.e = b; $$.l = LINE_NOW; }
|	expression SE expression  {
		YCPBuiltin b(YCPB_SE);
		b->add($1.e->asValue());
		b->add($3.e->asValue());
		$$.e = b; $$.l = LINE_NOW; }
|	expression GE expression  {
		YCPBuiltin b(YCPB_GE);
		b->add($1.e->asValue());
		b->add($3.e->asValue());
		$$.e = b; $$.l = LINE_NOW; }
|	expression NEQ expression  {
		YCPValue e1 = $1.e->asValue();
		YCPValue e2 = $3.e->asValue();

		/* map "x != nil" to "!isnil (x)"  */

		if (e1->valuetype() == YT_VOID)
		{
		    if (e2->valuetype() == YT_SYMBOL)
		    {
			$$.e = YCPBuiltin (YCPB_NISNIL, e2);
		    }
		    else		// ensure nil is 2nd arg
		    {
			YCPBuiltin b (YCPB_NEQ, e2);
			b->add (e1);
			$$.e = b;
		    }
		}
		else if (e2->valuetype() == YT_VOID)
		{
		    if (e1->valuetype() == YT_SYMBOL)
		    {
			$$.e = YCPBuiltin (YCPB_NISNIL, e1);
		    }
		    else		// ensure nil is 2nd arg
		    {
			YCPBuiltin b (YCPB_NEQ, e1);
			b->add (e2);
			$$.e = b;
		    }
		}
		else
		{
		    YCPBuiltin b (YCPB_NEQ, e1);
		    b->add(e2);
		    $$.e = b;
		}
		$$.l = LINE_NOW; }
|	NOT expression {
		YCPBuiltin b(YCPB_NOT);
		b->add($2.e->asValue());
		$$.e = b; $$.l = LINE_NOW; }
|	expression '?' expression ':' expression {
		YCPBuiltin b(YCPB_TRIPLE);
		b->add($1.e->asValue());
		b->add($3.e->asValue());
		b->add($5.e->asValue());
		$$.e = b; $$.l = LINE_NOW;
	}
|	'-' expression %prec UMINUS {
		  if ($2.e->asValue()->isInteger()) {
		    $$.e = YCPInteger (-($2.e->asValue()->asInteger()->value()));
		  }
		  else if ($2.e->asValue()->isFloat()) {
		    $$.e = YCPFloat (-($2.e->asValue()->asFloat()->value()));
		  }
		  else {
		    YCPBuiltin b(YCPB_NEG);
		    b->add($2.e->asValue());
		    $$.e = b;
		  }
		  $$.l = LINE_NOW;
		}
;

module:
	'{' MODULE YCP_STRING ';'
	    {   if (!inside_module.empty())
		{
		    string where = string ("still inside module ")+inside_module;
		    yyerror (where.c_str());
		}
		if (getenv ("Y2ALLGLOBAL") != 0)
		{
		    module_level = 1;
		}
		inside_module = $3.e->asValue()->asString()->value();
	    }
	    module_statements '}'	 {
		$$.e = YCPBlock ($3.e->asValue()->asString()->value(), $6.e->asValue()->asBlock()->getStatements());
		$$.l = LINE_NOW;
		inside_module = "";
		module_level = 0;
	}
;

block:
	'{' block_statements '}'	 { $$.e = $2.e; $$.l = LINE_NOW; }
;

/* -------------------------------------------------------------- */
/* Statements */

module_statements:  module_statements novalue_statement {
		if (!$2.e.isNull()) {
		    $1.e->asValue()->asBlock()->add ($2.e->asValue()->asStatement());
		}
		$$.e = $1.e;
		$$.l = LINE_NOW;
	}
|		module_statements value_statement {
		    yyerror ("Only declarations allowed inside a module."); YYERROR;
	}
| /* empty  */ {
		$$.e = YCPBlock();
	}
;

block_statements:  block_statements statement {
		if (!$2.e.isNull()) {
		    $1.e->asValue()->asBlock()->add ($2.e->asValue()->asStatement());
		}
		$$.e = $1.e;
		$$.l = LINE_NOW;
	}
| /* empty  */ {
		$$.e = YCPBlock();
	}
;

// quoted block statements

quoted_block:	QUOTED_BLOCK qb_statements '}'	{
		$$.e = deepquoted ($2.e->asValue()); $$.l = $2.l;
	}
;

qb_statements:  qb_statements statement {
		if (!$2.e.isNull()) {
		    $1.e->asValue()->asBlock()->add ($2.e->asValue()->asStatement());
		}
		$$.e = $1.e;
		$$.l = LINE_NOW;
	}
|	/* empty  */	{
		YCPBlock b;
		b->setFileName (FILE_NOW);
		if (!current_textdomain.empty())
		{
		    YCPStatement s = YCPBuiltinStatement (LINE_NOW, YCPB_LOCALDOMAIN, YCPString (current_textdomain));
		    b->add (s);
		}
		$$.e = b;
	}
;

novalue_statement:
	';'				{ $$.e = YCPNull(); }
|	MODULE YCP_STRING ';'	{
		    yyerror ("module must be the first statement."); YYERROR;
	}
|	INCLUDE YCP_STRING ';'		{ $$.e = YCPBuiltinStatement (LINE_NOW, YCPB_INCLUDE, $2.e->asValue());
					  $$.l = LINE_NOW; }
|	IMPORT YCP_STRING ';'		{ $$.e = YCPBuiltinStatement (LINE_NOW, YCPB_IMPORT, $2.e->asValue());
					  $$.l = LINE_NOW; }
|	EXPORT symbollist ';'		{ $$.e = YCPBuiltinStatement (LINE_NOW, YCPB_EXPORT, $2.e->asValue());
					  $$.l = LINE_NOW; }
|	UNDEFINE symbollist ';'		{ $$.e = YCPBuiltinStatement (LINE_NOW, YCPB_UNDEFINE, $2.e->asValue());
					  $$.l = LINE_NOW; }
|	FULLNAME YCP_STRING ';'		{ $$.e = YCPBuiltinStatement (LINE_NOW, YCPB_FULLNAME, $2.e->asValue());
					  $$.l = LINE_NOW; }
|	TEXTDOMAIN YCP_STRING ';'	{ $$.e = YCPBuiltinStatement (LINE_NOW, YCPB_TEXTDOMAIN, $2.e->asValue());
					  current_textdomain = $2.e->asValue()->asString()->value();
					  $$.l = LINE_NOW;
					}
|	vardecl	';'			{ $$.e = YCPEvaluationStatement(LINE_NOW, $1.e->asValue());
					  $$.l = LINE_NOW; }
|	definition			{ $$.e = YCPEvaluationStatement(LINE_NOW, $1.e->asValue());
					  $$.l = LINE_NOW; }
;

value_statement:
	assignment ';'
|	block	{
		  $$.e = YCPNestedStatement(LINE_NOW, $1.e->asValue()->asBlock());
 		  $$.l = LINE_NOW;
		}
|	term ';'
		{
		  $$.e = YCPEvaluationStatement(LINE_NOW, $1.e->asValue());
		  $$.l = LINE_NOW;
		}
|	IF '(' expression ')' { $$.l = LINE_NOW; }
	  statement_as_block opt_else
		{ if ($7.e.isNull())
		    $$.e = YCPIfThenElseStatement($5.l, $3.e->asValue(), $6.e->asValue()->asBlock());
		  else
		    $$.e = YCPIfThenElseStatement($5.l, $3.e->asValue(), $6.e->asValue()->asBlock(), $7.e->asValue()->asBlock());
		}
|	WHILE { $$.l = LINE_NOW; }
	  '(' expression ')' statement_as_block
		{
		  $$.e = YCPEvaluationStatement($2.l, YCPWhileBlock($6.e->asValue()->asBlock(), $4.e->asValue()));
		}
|	DO { $$.l = LINE_NOW; }
	  block WHILE '(' expression ')' ';'
		{
		  $$.e = YCPEvaluationStatement($2.l, YCPDoWhileBlock($3.e->asValue()->asBlock(), $6.e->asValue(), false));
		}
|	REPEAT { $$.l = LINE_NOW; }
	  block UNTIL '(' expression ')' ';'
		{
		  $$.e = YCPEvaluationStatement($2.l, YCPDoWhileBlock($3.e->asValue()->asBlock(), $6.e->asValue(), true));
		}
|	BREAK ';'		{ $$.e = YCPBreakStatement(LINE_NOW); }
|	CONTINUE ';'		{ $$.e = YCPContinueStatement(LINE_NOW); }
|	RETURN ';'		{ $$.e = YCPReturnStatement(LINE_NOW); }
|	RETURN expression ';'	{ $$.e = YCPReturnStatement(LINE_NOW, $2.e->asValue()); }
;

statement:
	novalue_statement
|	value_statement
;

opt_else:
	ELSE statement_as_block	{ $$.e = $2.e; }
|	/* empty */		{ $$.e = YCPNull(); }
;

statement_as_block:
	statement
		{ if ($1.e.isNull())
		  {
		     yyerror ("Warning: Empty statement instead of expected block");
		     $$.e = YCPBlock();
		  }
		  else if (!$1.e->asValue()->asStatement()->isNestedStatement())
		  {
		    YCPBlock b;
		    b->add ($1.e->asValue()->asStatement());
		    $$.e = b;
		  }
		  else
		  {
		    $$.e = $1.e->asStatement()->asNestedStatement()->value();
		  }
		  $$.l = LINE_NOW;
		}
;

/* -------------------------------------------------------------- */
/* Declarations. They have a small arithmetic themselves  */

typedecl:
	ANY
|	YCP_DECLTYPE
|	LIST			{ $$.e = YCPDeclType(YT_LIST); }
|	LIST ST typedecl GT	{ $$.e = YCPDeclList($3.e->asValue()->asDeclaration()); }
|	LIST '(' typedecl ')'	{ $$.e = YCPDeclList($3.e->asValue()->asDeclaration()); }
|	'[' EMPTY ']'		{ $$.e = YCPDeclStruct(); }
|	'[' tupledecl ']'	{ $$.e = $2.e; }
|	'(' typedecl ')'	{ $$.e = $2.e; }
;

tupledecl:
	typedecl SYMBOL
		{ YCPSymbol s = $2.e->asValue()->asSymbol();
		  if (0 && s->isQuoted ()) {
		    yyerror ("Can't declare quoted symbol"); YYERROR;
		  }
		  YCPDeclStruct d;
		  d->add (s, $1.e->asValue());
		  $$.e = d;
		}
|	tupledecl ',' typedecl SYMBOL
		{ YCPSymbol s = $4.e->asValue()->asSymbol();
		  if (0 && s->isQuoted ()) {
		    yyerror ("Can't declare quoted symbol"); YYERROR;
		  }
		  YCPDeclStruct d = $1.e->asValue()->asDeclaration()->asDeclStruct();
		  d->add (s, $3.e->asValue());
		  $$.e = d;
		}
;

/* optional type, defaults to 'any'  */

opt_type:
	typedecl
|		  { $$.e = YCPDeclAny(); }
;
/* -------------------------------------------------------------- */
/* Variable declaration */

vardecl:
	opt_global typedecl SYMBOL '=' expression {
		YCPBuiltin b(($1.l == 1)?YCPB_GLOBALDECLARE:YCPB_LOCALDECLARE);
		b->add($2.e->asValue());
		YCPSymbol s = $3.e->asValue()->asSymbol();
		if (0 && s->isQuoted ()) {
		    yyerror ("Can't declare quoted symbol"); YYERROR;
		}
		b->add(YCPSymbol (s->symbol(), true));
		b->add($5.e->asValue());
		$$.l = LINE_NOW;
		$$.e = b;
		if (module_level > 1)
		{
		    module_level--;
		}
	    }
;

/* -------------------------------------------------------------- */
/* Assignment */

assignment:
	identifier '=' expression
	    {
		YCPBuiltin b(YCPB_ASSIGN);
		YCPValue v = $1.e->asValue();
		if (v->isIdentifier())
		{
		    YCPIdentifier id (v->asIdentifier()->symbol()->symbol(), v->asIdentifier()->module(), true);
		    b->add(id);
		}
		else
		{
		    YCPSymbol s = v->asSymbol();
		    b->add(YCPSymbol (s->symbol(), true));
		}
		b->add($3.e->asValue());
		$$.e = YCPEvaluationStatement(LINE_NOW, b);
	    }
|	identifier '[' tuple_elements ']' '=' expression
	    {
		YCPBuiltin b(YCPB_BASSIGN);
		YCPValue v = $1.e->asValue();
		if (v->isIdentifier())
		{
		    // lvalue is always quoted
		    YCPIdentifier id (v->asIdentifier()->symbol()->symbol(), v->asIdentifier()->module(), true);
		    b->add(id);
		}
		else
		{
		    // lvalue is always quoted
		    YCPSymbol s = v->asSymbol();
		    b->add(YCPSymbol (s->symbol(), true));
		}
		b->add($3.e->asValue());	// bracket args list
		b->add($6.e->asValue());	// expression
		$$.e = YCPEvaluationStatement(LINE_NOW, b);
	    }
;

/* -------------------------------------------------------------- */
/* Macro defintion */

definition:
	opt_global DEFINE opt_type definition_prefix ')' expression
	    {
		YCPBuiltin b(($1.l == 1)?YCPB_GLOBALDEFINE:YCPB_LOCALDEFINE);
		b->add($4.e->asValue());
		b->add($6.e->asValue());
		$$.e = b;
		if (module_level > 1)
		{
		    module_level--;
		}
	    }
|	opt_global DEFINE opt_type UI DCOLON definition_prefix ')' expression
	    {
		if ($1.l != 1)
		{
		    yyerror ("Useless local UI define"); YYERROR;
		}
		YCPBuiltin b(YCPB_GLOBALDEFINE);
		b->add($6.e->asValue());
		b->add($8.e->asValue());
		$$.e = YCPBuiltin (YCPB_UI, b);
		if (module_level > 1)
		{
		    module_level--;
		}
	    }
;

opt_global:
	GLOBAL	{ $$.l = 1; }
|	    {	if (module_level == 1)
		{
		    module_level++;
		    $$.l = 1;
		}
		else
		{
		    $$.l = 0;
		}
	    }
;

definition_prefix:
	definition_symbol
|	definition_symbol typedecl SYMBOL
		{ YCPSymbol s = $3.e->asValue()->asSymbol();
		  if (0 && s->isQuoted ()) {
		    yyerror ("Can't declare quoted symbol"); YYERROR;
		  }
		  YCPDeclTerm d = $1.e->asValue()->asDeclaration()->asDeclTerm();
		  d->add (s, $2.e->asValue());
		  $$.e = d;
		}
|	definition_prefix ',' typedecl SYMBOL
		{ YCPSymbol s = $4.e->asValue()->asSymbol();
		  if (0 && s->isQuoted ()) {
		    yyerror ("Can't declare quoted symbol"); YYERROR;
		  }
		  YCPDeclTerm d = $1.e->asValue()->asDeclaration()->asDeclTerm();
		  d->add (s, $3.e->asValue());
		  $$.e = d;
		}
;

definition_symbol:
	SYMBOL '('
		{ YCPSymbol s = $1.e->asValue()->asSymbol();
		  if (0 && s->isQuoted ()) {
		    yyerror ("Can't declare quoted symbol"); YYERROR;
		  }
		  $$.e = YCPDeclTerm(s);
		}
;

/* ----------------------------------------------------------*/

constant:
	YCP_VOID
|	YCP_BOOLEAN
|	YCP_INTEGER
|	YCP_FLOAT
|	YCP_STRING
|	YCP_TIME
|	YCP_BYTEBLOCK
|	YCP_PATH {
	    if ($$.e->asValue().isNull()) {
		yyerror ("Bad path constant"); YYERROR;
	    }
	    $$.e = $1.e;
	}
;

/* -------------------------------------------------------------- */
/* Tuples */

tuple:
	'[' tuple_elements ']'		{ $$.e = $2.e; }
|	'[' tuple_elements ',' ']'	{ $$.e = $2.e; }
;

tuple_elements:
	/* empty  */			{ $$.e = YCPList (); }
|	expression			{
		YCPList l;
		l->add ($1.e->asValue());
		$$.e = l; }
|	typedecl SYMBOL
	    {
		// prepare for typed syntax
		// treat typed name as quoted symbol

		YCPList l;
		YCPSymbol s = $2.e->asValue()->asSymbol();
		if (s->isQuoted())
		    l->add (s);
		else
		    l->add (YCPSymbol (s->symbol(), true));
		$$.e = l;
	    }
|	tuple_elements ',' typedecl SYMBOL
	    {
		if ($1.e->asValue()->asList()->size() == 0)
		{
		    yyerror ("Wrong or missing element at start of list"); YYERROR;
		}
		YCPList l = $1.e->asValue()->asList();
		YCPSymbol s = $4.e->asValue()->asSymbol();
		if (s->isQuoted())
		    l->add (s);
		else
		    l->add (YCPSymbol (s->symbol(), true));
		$$.e = l;
	    }
|	tuple_elements ',' expression	{
		if ($1.e->asValue()->asList()->size() == 0) {
			yyerror ("Wrong or missing element at start of list"); YYERROR;
		}
		YCPList l = $1.e->asValue()->asList();
		l->add($3.e->asValue());
		$$.e = l; }
;

/* -------------------------------------------------------------- */
/* Maps */

map:
	MAPEXPR map_elements ']'	{ $$.e = $2.e; }
|	MAPEXPR map_elements ',' ']'	{ $$.e = $2.e; }
;

map_elements:
	/* empty  */			{ $$.e = YCPMap ();  }
|	expression ':' expression	{
		YCPMap m; m->add($1.e->asValue(), $3.e->asValue());
		$$.e = m; }
|	map_elements ',' expression ':' expression {
		if ($1.e->asValue()->asMap()->size() == 0) {
			yyerror ("Wrong or missing element at start of map"); YYERROR;
		}
		YCPMap m = $1.e->asValue()->asMap();
		m->add($3.e->asValue(), $5.e->asValue());
		$$.e = m; }
;

/* -------------------------------------------------------------- */
/* Terms */


term:
	QUOTED_SYMBOL '(' tuple_elements ')'	{
		  YCPTerm t ($1.e->asValue()->asSymbol(), $3.e->asValue()->asList());
		  $$.e = deepquoted (t->asValue());
		  $$.l = LINE_NOW; }
|	DUMPSCOPE '(' tuple_elements ')'	{
		  $$.e = YCPBuiltin (YCPB_DUMPSCOPE, $3.e->asValue()->asList());
		  $$.l = LINE_NOW; }
|	MEMINFO '(' tuple_elements ')'	{
		  $$.e = YCPBuiltin (YCPB_MEMINFO, $3.e->asValue()->asList());
		  $$.l = LINE_NOW; }
|	SIZE '(' expression ')'	{
		  $$.e = YCPBuiltin (YCPB_SIZE, $3.e->asValue());
		  $$.l = LINE_NOW; }
|	SYMBOLOF '(' expression ')'	{
		  $$.e = YCPBuiltin (YCPB_SYMBOLOF, $3.e->asValue());
		  $$.l = LINE_NOW; }
|	LOOKUP '(' tuple_elements ')'	{
		  YCPList e = $3.e->asValue()->asList();
		  if (e->size() != 3)
		  {
			yyerror ("must be lookup (<map>, <key>, <default>)"); YYERROR;
		  }
		  $$.e = YCPBuiltin (YCPB_LOOKUP, e);
		  $$.l = LINE_NOW; }
|	SELECT '(' tuple_elements ')'	{
		  YCPList e = $3.e->asValue()->asList();
		  if (e->size() < 3)
		  {
		    yyerror ("select() without default");
		  }
		  else if (e->size() > 3)
		  {
		    yyerror ("select() only accepts 3 arguments"); YYERROR;
		  }
		  $$.e = YCPBuiltin (YCPB_SELECT, e);
		  $$.l = LINE_NOW; }
|	REMOVE '(' tuple_elements ')'	{
		YCPList e = $3.e->asValue()->asList();
		if (e->size() != 2) {
			yyerror ("remove() only accepts 2 arguments"); YYERROR;
		}
		$$.e = YCPBuiltin (YCPB_REMOVE, e);
		$$.l = LINE_NOW;
	}
|	FOREACH '(' tuple_elements ')'	{
		  YCPList e = $3.e->asValue()->asList();
		  if ((e->size() < 3) || (e->size() > 4)) {
			yyerror ("foreach() only accepts 3 or 4 arguments"); YYERROR;
		  }
		  $$.e = YCPBuiltin (YCPB_FOREACH, e);
		  $$.l = LINE_NOW; }
|	EVAL '(' expression ')'	{
		  $$.e = YCPBuiltin (YCPB_EVAL, $3.e->asValue());
		  $$.l = LINE_NOW; }
|	UNION '(' expression ',' expression ')'	{
		YCPBuiltin b (YCPB_UNION);
		b->add ($3.e->asValue());
		b->add ($5.e->asValue());
		$$.e = b;
		$$.l = LINE_NOW;
	}
|	MERGE '(' expression ',' expression ')'	{
		YCPBuiltin b (YCPB_MERGE);
		b->add ($3.e->asValue());
		b->add ($5.e->asValue());
		$$.e = b;
		$$.l = LINE_NOW;
	}
|	ADD '(' tuple_elements ')'	{
		if ($3.e->asValue()->asList()->size() < 2)
		{
		    yyerror ("add() only accepts 2 or 3 arguments"); YYERROR;
		}
		$$.e = YCPBuiltin (YCPB_ADD, $3.e->asValue()->asList());
		$$.l = LINE_NOW;
	}
|	CHANGE '(' tuple_elements ')'	{
		if ($3.e->asValue()->asList()->size() < 2)
		{
		    yyerror ("change() only accepts 2 or 3 arguments"); YYERROR;
		}
		$$.e = YCPBuiltin (YCPB_CHANGE, $3.e->asValue()->asList());
		$$.l = LINE_NOW;
	}
|	SORT '(' tuple_elements ')'	{
		YCPList e = $3.e->asValue()->asList();
		if ((e->size() != 1)
		    && (e->size() != 4))
		{
			yyerror ("sort () only accepts 1 or 4 arguments"); YYERROR;
		}
		$$.e = YCPBuiltin (YCPB_SORT, e);
		$$.l = LINE_NOW;
	}
|	LSORT '(' tuple_elements ')'	{
		YCPList e = $3.e->asValue()->asList();
		if (e->size() != 1)
		{
			yyerror ("lsort () only accepts 1 argument"); YYERROR;
		}
		$$.e = YCPBuiltin (YCPB_LSORT, e);
		$$.l = LINE_NOW;
	}
|	UI other_builtin {
		$$.e = YCPBuiltin (YCPB_UI, $2.e->asValue());
		$$.l = LINE_NOW;
	}
|	SCR other_builtin {
		$$.e = YCPBuiltin (YCPB_SCR, $2.e->asValue());
		$$.l = LINE_NOW;
	}
|	WFM other_builtin {
		$$.e = YCPBuiltin (YCPB_WFM, $2.e->asValue());
		$$.l = LINE_NOW;
	}
|	Pkg other_builtin {
		if ($2.e->asValue()->isTerm())
		{
		    YCPTerm pkgterm = $2.e->asValue()->asTerm();
		    if (pkgterm->name_space().empty())
		    {
			$$.e = YCPBuiltin (YCPB_WFM, YCPTerm (pkgterm->symbol(), "Pkg", pkgterm->args()));
		    }
		    else
		    {
			yyerror ("Namespace qualifier after Pkg::"); YYERROR;
		    }
		}
		else
		{
		    yyerror ("Only Pkg::<func>(<args>...) allowed"); YYERROR;
		}
		$$.l = LINE_NOW;
	}
|	Perl other_builtin {
		if ($2.e->asValue()->isTerm())
		{
		    YCPTerm perlterm = $2.e->asValue()->asTerm();
		    if (perlterm->name_space().empty())
		    {
			$$.e = YCPBuiltin (YCPB_WFM, YCPTerm (perlterm->symbol(), "Perl", perlterm->args()));
		    }
		    else
		    {
			yyerror ("Namespace qualifier after Perl::"); YYERROR;
		    }
		}
		else
		{
		    yyerror ("Only Perl::<func>(<args>...) allowed"); YYERROR;
		}
		$$.l = LINE_NOW;
	}
|	identifier '(' tuple_elements ')'	{
		YCPValue v = $1.e->asValue();
		if (v->isIdentifier())
		{
		    YCPIdentifier id = v->asIdentifier();
		    $$.e = YCPTerm (id->symbol(), id->module(), $3.e->asValue()->asList());
		}
		else if (v->isBuiltin())
		{
		    YCPBuiltin b = v->asBuiltin();
		    $$.e = YCPBuiltin (b->builtin_code(), YCPTerm (b->args()->value(0)->asSymbol(), $3.e->asValue()->asList()));
		}
		else
		{
		    $$.e = YCPTerm (v->asSymbol(), $3.e->asValue()->asList());
		}
		$$.l = LINE_NOW;
	}
|	identifier '[' tuple_elements CLOSEBRACKET %prec DCOLON expression
	    {
		YCPValue v = $1.e->asValue();
		YCPList l = YCPList();
		l->add ($1.e->asValue());
		l->add ($3.e->asValue());
		l->add ($5.e->asValue());

		$$.e = YCPBuiltin (YCPB_BRACKET, l);
		$$.l = LINE_NOW;
	    }
|	identifier
;

opt_default:
	':' expression
	    {
		$$.e = $2.e;
	    }
| /* empty */
	    {
		$$.e = YCPNull();
	    }
;

/* -------------------------------------------------------------- */
/* after UI, SCR, or WFM */

other_builtin:
	DCOLON SYMBOL {
		$$ = $2;
	}
|	DCOLON SYMBOL '(' tuple_elements ')'
	{
		$$.e = YCPTerm ($2.e->asValue()->asSymbol(), $4.e->asValue()->asList());
	}
|	DCOLON SYMBOL DCOLON SYMBOL {
		YCPSymbol s = $4.e->asValue()->asSymbol();
		$$.e = YCPIdentifier (s->symbol(), $2.e->asValue()->asSymbol()->symbol(), s->isQuoted());
	}
|	DCOLON SYMBOL DCOLON SYMBOL '(' tuple_elements ')' {
		$$.e = YCPTerm ($4.e->asValue()->asSymbol(), $2.e->asValue()->asSymbol()->symbol(), $6.e->asValue()->asList());
	}
|	DCOLON quoted_block {
		$$ = $2;
	}
|	DCOLON block {
		$$.e = deepquoted ($2.e->asValue());
	}
|	'(' expression ')' {
		$$ = $2;
	}
;

/* -------------------------------------------------------------- */
/* Symbol lists */

symbollist:
	SYMBOL		{
		YCPList symbollist;
		symbollist->add ($1.e->asValue());
		$$.e = symbollist;
		$$.l = $1.l; }
|	symbollist ',' SYMBOL {
		$1.e->asValue()->asList()->add ($3.e->asValue());
		$$ = $1; }
;
/* -------------------------------------------------------------- */
/* Symbols   (YCPSymbol())*/

identifier:
	SYMBOL DCOLON SYMBOL {
				YCPSymbol modname = $1.e->asValue()->asSymbol();
				if (modname->isQuoted ()) {
				    yyerror ("Can't quote module qualifier"); YYERROR;
				}
				YCPSymbol symname = $3.e->asValue()->asSymbol();
				$$.e = YCPIdentifier (symname->symbol(), modname->symbol());
			}
|	DCOLON SYMBOL	{
				YCPSymbol s = $2.e->asValue()->asSymbol();
				if (s->isQuoted ()) {
				    yyerror ("Can't quote global symbol"); YYERROR;
				}
				$$.e = YCPIdentifier (s->symbol(), "_");
			}
|	SYMBOL
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
    struct parserret *pr = (struct parserret *)void_pr;

    // call 'our' scanner through the parser
    int token = pr->scanner->yylex();

    if (token == YYEOF)
    {
	yyeof_reached = 1;
	return token;
    }

    // store the value of the lexical token
    YCPValue *store_here = (YCPValue *) &(lvalp_void->e);
    *store_here = pr->scanner->getScannedValue();
    pr->lineno = pr->scanner->getLineNumber();

    return token;
}
} // extern "C"


static void yyerror_with_lineinfo(const char *s, void *pr_void)
{
    struct parserret *parser_ret = (struct parserret *)pr_void;
    parser_ret->scanner->logError(s, parser_ret->lineno);
}


static YCPValue deepquoted(const YCPValue& v)
{
    YCPBuiltin b(YCPB_DEEPQUOTE);
    b->add(v);
    return b;
}
