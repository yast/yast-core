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

   File:	YExpression.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

$Id$
/-*/
// -*- c++ -*-

#include <libintl.h>

#include "ycp/y2log.h"
#include "ycp/StaticDeclaration.h"

#include "ycp/YExpression.h"
#include "ycp/YStatement.h"
#include "ycp/YBlock.h"
#include "ycp/YCPCode.h"
#include "ycp/YCPError.h"
#include "ycp/YCPTerm.h"
#include "ycp/YCPBoolean.h"
#include "ycp/YCPInteger.h"
#include "ycp/YCPFloat.h"
#include "ycp/YCPMap.h"

#include "ycp/Bytecode.h"

#include "ycp/y2log.h"

typedef YCPValue (*v2) ();
typedef YCPValue (*v2v) (const YCPValue &);
typedef YCPValue (*v2vv) (const YCPValue &, const YCPValue &);
typedef YCPValue (*v2vvv) (const YCPValue &, const YCPValue &, const YCPValue &);
typedef YCPValue (*v2vvvv) (const YCPValue &, const YCPValue &, const YCPValue &, const YCPValue &);

// ------------------------------------------------------------------
// variable ref (-> SymbolEntry)

YEVariable::YEVariable (SymbolEntry *entry)
    : YCode (yeVariable)
    , m_entry (entry)
{
}


YEVariable::YEVariable (std::istream & str)
    : YCode (yeVariable)
{
    m_entry = Bytecode::readEntry (str);
}


SymbolEntry *
YEVariable::entry() const
{
    return m_entry;
}


const char *
YEVariable::name() const
{
    return m_entry->name();
}


string
YEVariable::toString() const
{
    return m_entry->toString(false);
}


YCPValue
YEVariable::evaluate (bool cse)
{
    if (cse) return YCPNull();

    YCPValue value = m_entry->value();		// get current value

    if (value.isNull())				// oops, no value yet
    {
	YCode *code = m_entry->code();		// evaluate code to value
	y2debug ("m_entry %p", m_entry);
	y2debug ("= %s", m_entry->toString().c_str());
	y2debug ("m_entry->code() %p", code);
	if (code) y2debug ("= %s", code->toString().c_str());
	value = m_entry->setValue (code ? code->evaluate () : YCPNull());
    }
    y2debug ("YEVariable::evaluate (%s) = %s", toString().c_str(), value.isNull() ? "NULL" : value->toString().c_str());
    return value;
}


std::ostream &
YEVariable::toStream (std::ostream & str) const
{
    YCode::toStream (str);
    Bytecode::writeEntry (str, m_entry);
    return str;
}

// ------------------------------------------------------------------
// term (-> name, parameters)

YETerm::YETerm (const char *name)
    : YCode (yeTerm)
    , m_name (name)
    , m_parameters (0)
    , m_last (0)
{
}


YETerm::YETerm (std::istream & str)
    : YCode (yeTerm)
    , m_parameters (0)
    , m_last (0)
{
    m_name = Bytecode::readCharp (str);
    if (m_name)
    {
	if (!Bytecode::readYCodelist (str, &m_parameters, &m_last))
	{
	    delete m_name;
	    m_name = 0;
	}
    }
}


YETerm::~YETerm ()
{
    ycodelist_t *parm = m_parameters;
    while (parm)
    {
	ycodelist_t *next = parm->next;
	delete parm->code;
	delete parm;
	parm = next;
    }
}


void
YETerm::attachTermParameter (YCode *code)
{
    if ((code == 0)
	|| (code->isError()))
    {
	y2debug ("YETerm::attachTermParameter (Error)");
	return;
    }

    y2debug ("YETerm::attachTermParameter (%s)", code->toString().c_str());

    ycodelist_t *element = new ycodelist_t;
    element->code = code;
    element->next = 0;

    if (m_parameters == 0)
    {
	m_parameters = element;
    }
    else
    {
	m_last->next = element;
    }
    m_last = element;

    return;
}


string
YETerm::toString () const
{
    string s = "`" + string (m_name) + " (";

    ycodelist_t *parm = m_parameters;
    while (parm)
    {
	s += parm->code->toString().c_str();
	if (parm->next != 0)
	    s += ", ";
	parm = parm->next;
    }
    s += ")";
    return s;
}


YCPValue
YETerm::evaluate (bool cse)
{
    YCPTerm term (m_name);

    ycodelist_t *actualp = m_parameters;

    while (actualp != 0)
    {
	YCPValue value = actualp->code->evaluate (cse);
	if (value.isNull())
	{
	    if (cse)		// parse time
	    {
		return value;
	    }
	    return YCPError ("Term parameter evaluates to 'NULL'");
	}
	term->add (value);
	actualp = actualp->next;
    }

    return term;
}


std::ostream &
YETerm::toStream (std::ostream & str) const
{
    YCode::toStream (str);
    Bytecode::writeCharp (str, m_name);
    return Bytecode::writeYCodelist (str, m_parameters);
}

// ------------------------------------------------------------------
// Compare (-> left, right, type)

YECompare::YECompare (YCode *left, c_op op, YCode *right)
    : YCode (yeCompare)
    , m_left (left)
    , m_op (op)
    , m_right (right)
{
}


YECompare::YECompare (std::istream & str)
    : YCode (yeCompare)
{
    m_left = Bytecode::readCode (str);
    char c;
    str.get (c);
    m_op = (c_op) c;
    m_right = Bytecode::readCode (str);
}


YECompare::~YECompare ()
{
    delete m_left;
    delete m_right;
}


string
YECompare::toString () const
{
    string s = "(" + m_left->toString();
    switch (m_op)
    {
	case C_EQ:  s += " == "; break;
	case C_NEQ: s += " != "; break;
	case C_LT:  s += " < ";  break;
	case C_GE:  s += " >= "; break;
	case C_LE:  s += " <= "; break;
	case C_GT:  s += " > ";  break;
	default:
		s += " ?compare? ";
		break;
    }
    s += m_right->toString();
    return s + ")";
}


YCPValue
YECompare::evaluate (bool cse)
{
    if (cse)		// parse time
    {
	return YCPNull();
    }

    YCPValue vl = m_left->evaluate (cse);
    YCPValue vr = m_right->evaluate (cse);
y2debug ("YECompare::evaluate (%s, '%d', %s)", vl.isNull() ? "NULL" : vl->toString().c_str(), m_op, vr.isNull() ? "NULL" : vr->toString().c_str());

    // left value is nil
    // HUH? nil is isVoid! FIXME
    if (vl.isNull())
    {
	return YCPBoolean (vr.isNull() && (m_op == C_EQ));	// 'nil == nil'
    }

    // left != nil, right == nil

    if (vr.isNull())
    {
	return YCPBoolean (m_op == C_NEQ);			// 'x != nil'
    }

    enum YCPOrder order = vl->compare (vr);
    switch (m_op)
    {
	case C_EQ:  return YCPBoolean (order == YO_EQUAL); break;
	case C_NEQ: return YCPBoolean (order != YO_EQUAL); break;
	case C_LT:  return YCPBoolean (order == YO_LESS);  break;
	case C_GE:  return YCPBoolean (order != YO_LESS);  break;
	case C_LE:  return YCPBoolean ((order == YO_EQUAL)||(order == YO_LESS)); break;
	case C_GT:  return YCPBoolean (order == YO_GREATER); break;
	default:
	    break;
    }
    return YCPError ("YECompare unknown type", YCPBoolean (false));
}


std::ostream &
YECompare::toStream (std::ostream & str) const
{
    YCode::toStream (str);
    if (!m_left->toStream (str))
	return str;

    str.put ((char)m_op);
    return m_right->toStream (str);
}

// ------------------------------------------------------------------
// builtin function ref (-> declaration_t, type, parameters)

YEBuiltin::YEBuiltin (declaration_t *decl, const TypeCode & type)
    : YCode (yeBuiltin)
    , m_decl (decl)
    , m_type (type)
    , m_parameters (0)
    , m_last (0)
{
}


YEBuiltin::YEBuiltin (std::istream & str)
    : YCode (yeBuiltin)
{
    extern StaticDeclaration static_declarations;
    m_decl = static_declarations.readDeclaration (str);
    if (m_decl)
    {
	m_type = m_decl->type;
	Bytecode::readYCodelist (str, &m_parameters, &m_last);
    }
}


YEBuiltin::~YEBuiltin ()
{
    ycodelist_t *parm = m_parameters;
    while (parm)
    {
	ycodelist_t *next = parm->next;
	delete parm->code;
	delete parm;
	parm = next;
    }
}


declaration_t *
YEBuiltin::decl () const
{
    return m_decl;
}


// set final (type matching) declaration
// return 0 if all parameters match type,
// return type if parameter matching type is missing

declaration_t *
YEBuiltin::setDecl (declaration_t *decl)
{
    extern StaticDeclaration static_declarations;

    y2debug ("YEBuiltin::setDecl (%s)", Decl2String(decl,true).c_str());

    // final type check for all parameters
    decl = static_declarations.findDeclaration (decl, m_type, false);
    if (decl != 0)
    {
	m_decl = decl;

	if (m_type.isArgs ())		// if not set by attachBuiltinParameter ('A' type)
	{
	    m_type = decl->type;
	}
    }
    return decl;
}


TypeCode
YEBuiltin::type () const
{
    return m_type;
}


TypeCode
YEBuiltin::returnType () const
{
    y2debug ("YEBuiltin::returnType %s", m_type.asString().c_str());
    return m_type.returnType ();
}


/**
 * determine actual type if declared type contains 'A' (flex type)
 * Returns actual - unchanged or fixed
 * @param symbol type of a symbol parameter for YESymFunc, isUnspec for YEBuiltin
 */
static TypeCode
determineFlexType (const TypeCode & actual, const TypeCode & declared, const TypeCode & symbol)
{
    // if builtin decl returns 'A', the parameter deduces the return type

    if (!actual.isReturnUnspec ()	// not yet set
	// FIXME LA|...
	|| !declared.isFlex ())		// decl returns 'A'
    {
	// already determined
	return actual;
    }

    y2debug ("determineFlexType (actual %s, declared %s, symbol %s)", actual.asString().c_str(), declared.asString().c_str(), symbol.asString().c_str());

    TypeCode result = actual;
    y2debug ("ANY type check (decl '%s', m_type '%s'!", declared.asString().c_str(), actual.asString().c_str());

    TypeCode dtype = declared.args (); // decl type, points behind '|'
    TypeCode btype = actual.args ();   // builtin type, points behind '|'

    while (!dtype.isFlex ())		// search for 'A' in decl parameter list
    {
	y2debug ("scanning dtype '%s' for 'A', btype '%s'", dtype.asString().c_str(), btype.asString().c_str());
	if (btype.isEnd ())		// oops, we dont have the matching parameter yet
	    break;

	// matching has already been done. we are only skipping to the A
	// TYPECODE FIXME
	// a particular type of matching, must end at declared A,
	// ie maybe in the middle of an actual argument
	// (consider A|iLA, given iLs)

	const char *cdtype = dtype.asString().c_str ();
	const char *cbtype = btype.asString().c_str ();

	if ((*cdtype == *cbtype)	// normal matching parameter, skip
	    || (*cbtype == 'a')	//huh?
	    || (*cdtype == 'a'))
	{
	    dtype = cdtype + 1;
	    btype = cbtype + 1;
	}
	else
	{				// FIXME cleanup. without this, Select_List loops on nil
	    break;
	}
    }

    y2debug ("dtype '%s', btype '%s'", dtype.asString().c_str(), btype.asString().c_str());

    if (dtype.isFlex ()
	&& !btype.isEnd ())		// there we are !
    {
	TypeCode flextype = symbol.isCode() ? symbol.returnType() : btype.firstT ();

	if (flextype.isVoid())			// void (nil) propagates to any
	{
	    flextype = TypeCode::Any;
	}

	y2debug ("cat '%s''%s'", flextype.asString().c_str(), actual.asString().c_str());

	// prepend the missing return type
	result = newtype (flextype, actual);
	y2debug ("m_type '%s'", result.asString().c_str());

    } // parameter for 'A' found

    y2debug ("determineFlexType returns %s", result.asString().c_str());
    return result;
}


// if type == "", called from YESymFunc::toBuiltin, or stream creation
declaration_t *
YEBuiltin::attachBuiltinParameter (YCode *code, const TypeCode & type)
{
    extern StaticDeclaration static_declarations;

    y2debug ("YEBuiltin::attachBuiltinParameter (%s <:%s>)", code ? code->toString().c_str() : "<NULL>", type.asString().c_str());

    if ((code == 0)
	|| (code->isError()))
    {
	return 0;
    }

    declaration_t *decl = 0;

    if (!type.isUnspec ())
    {
	m_type = newtype (m_type, type);
	y2debug ("YEBuiltin::attachBuiltinParameter (%s:%s->'%s')", type.asString().c_str(), code->toString().c_str(), m_type.asString().c_str());

	decl = static_declarations.findDeclaration (m_decl, m_type, true);
	if (decl == 0)
	{
	    // FIXME: check for propagation
	    static_declarations.findDeclaration (m_decl, m_type, false);	// force error output
	    return decl;
	}

	m_type = determineFlexType (m_type, decl->type, "");
    }

    ycodelist_t *element = new ycodelist_t;
    element->code = code;
    element->next = 0;
    if (m_parameters == 0)
    {
	m_parameters = element;
    }
    else
    {
	m_last->next = element;
    }
    m_last = element;

    return decl;
}


string
YEBuiltin::toString() const
{
    string s = Decl2String (m_decl) + " (";

    ycodelist_t *parm = m_parameters;
    while (parm)
    {
	s += parm->code->toString();
	if (parm->next != 0)
	{
	    s += ", ";
	}
	parm = parm->next;
    }
    s += ")";
    return s;
}


YCPValue
YEBuiltin::evaluate (bool cse)
{
    y2debug ("YEBuiltin::evaluate [%s]", YCode::toString (code()).c_str());

    if (cse)
    {
	return YCPNull();
    }

    // init parameters

    ycodelist_t *actualp = m_parameters;
    const int maxargs = 10;
    YCPValue args[maxargs] = { YCPNull(), YCPNull(), YCPNull(), YCPNull(), YCPNull(), YCPNull(), YCPNull(), YCPNull(), YCPNull(), YCPNull() };
    YCPList list;
    TypeCode type; // looking for 'w'
    int typepos = 0; // position of w
    if ((m_decl->flags & DECL_WILD) != 0)
    {
	type = TypeCode (m_decl->type).args ();
    }

    // evaluate parameters

    int i = 0;

    while (i < maxargs)
    {
	if (actualp == 0)
	{
	    break;
	}

	if (actualp->code->isBlock())		// block as parameter to builtin function
	{
	    args[i] = YCPCode (actualp->code);	// pass as-is
	}
	else
	{
	    args[i] = actualp->code->evaluate ();
	}

	if ((args[i].isNull() || args[i]->isVoid())
	    && ((m_decl->flags & DECL_NIL) == 0))
	{
	    extern ExecutionEnvironment ee;
	    ycp2error (ee.filename().c_str(), ee.linenumber(), "Argument (%s) to %s(...) is nil", actualp->code->toString().c_str(), m_decl->name);
	    return YCPError("invalid argument");
	}
	y2debug ("actualp (%s) = (%s)", actualp->code->toString().c_str(), args[i]->toString().c_str());

	if (!type.isUnspec ())		// DECL_WILD !
	{
	    y2debug ("type(%s)@%d", type.asString().c_str(), typepos);
	    if (type.isWild ())	// at 'w' ?
	    {
		y2debug ("w! args[%d] = '%s'", i, args[i].isNull() ? "nil" : args[i]->toString().c_str());
		list->add (args[i]);	// Y: add value to list
	    }
	    else
	    {
		type = type.next ();
		++typepos;
	    }
	}
	i++;
	actualp = actualp->next;
    }

    // error checking

    if (actualp != 0)
    {
	extern ExecutionEnvironment ee;
	ycp2error (ee.filename().c_str(), ee.linenumber(), "More than %d arguments", maxargs);
	return YCPNull();
    }


    // wildcard checking

    if (type.isWild ())
    {
	y2debug ("w! pos %d '%s'", i, list->toString().c_str());
	i = typepos+1;
	args[i-1] = list;
    }


    // call builtin function

    y2debug ("YEBuiltin::evaluate [%s (%d args)]", Decl2String(m_decl, false).c_str(), i);
    YCPValue ret = YCPNull();
    if (m_decl->ptr == 0)
    {
	return ret;
    }
    switch (i)
    {
	case 0:
	    ret = (*(v2)m_decl->ptr) ();
	break;
	case 1:
	    ret = (*(v2v)m_decl->ptr) (args[0]);
	break;
	case 2:
	    ret = (*(v2vv)m_decl->ptr) (args[0], args[1]);
	break;
	case 3:
	    ret = (*(v2vvv)m_decl->ptr) (args[0], args[1], args[2]);
	break;
	case 4:
	    ret = (*(v2vvvv)m_decl->ptr) (args[0], args[1], args[2], args[3]);
	break;
	default:
	{
	    extern ExecutionEnvironment ee;

	    ycp2error (ee.filename().c_str(), ee.linenumber(), "Arg count %d", i);
	    ret = YCPError ("YEBuiltin bad");
	}
	break;
    }

#ifdef BUILTIN_STATISTICS
    if (!ret.isNull ())
    {
	FILE *fout = fopen ("/tmp/builtin-use.txt", "a");
	fprintf (fout, "%s %s\n", m_decl->name, m_decl->type);
	fclose (fout);
    }
#endif

    // error checking

    if (!ret.isNull()
	&& ret->isError())
    {
	// trigger error log
	ret = ret->asError()->evaluate();
    }

    y2debug ("YEBuiltin ret (%s)", ret.isNull() ? "NULL" : ret->toString().c_str());

    return ret;
}


std::ostream &
YEBuiltin::toStream (std::ostream & str) const
{
    YCode::toStream (str);
    extern StaticDeclaration static_declarations;

    static_declarations.writeDeclaration (str, m_decl);
    return Bytecode::writeYCodelist (str, m_parameters);
}

// ------------------------------------------------------------------
// builtin function with symbols ref (-> declaration_t, type, parameters)

YESymFunc::YESymFunc (declaration_t *decl, YBlock *parameterblock)
    : YCode (yeSymFunc)
    , m_decl (decl)
    , m_type ("|")
    , m_parameterblock (parameterblock)
{
}


YESymFunc::~YESymFunc ()
{
}

declaration_t *
YESymFunc::decl () const
{
    return m_decl;
}


declaration_t *
YESymFunc::setDecl (declaration_t *decl)
{
    extern StaticDeclaration static_declarations;

    y2debug ("YESymFunc::setDecl (%s), doing final type check", Decl2String(decl, true).c_str());

    // final type check for all parameters
    decl = static_declarations.findDeclaration (decl, m_type, false);
    if (decl != 0)
    {
	m_decl = decl;

	if (m_type.isArgs ())		// if not set by attachSymParameter ('A' type)
	{
	    m_type = decl->type;
	}
    }
    return decl;
}


TypeCode
YESymFunc::type () const
{
    return m_type;
}


YBlock *
YESymFunc::parameterBlock () const
{
    return m_parameterblock;
}


TypeCode
YESymFunc::returnType () const
{
    return m_type.returnType ();
}


// convert a YESymFunc without DECL_SYMBOL to YEBuiltin

YEBuiltin *
YESymFunc::toBuiltin (SymbolTable *table)
{
    y2debug ("YESymFunc::toBuiltin (%s)", toString().c_str());

    YEBuiltin *builtin = new YEBuiltin (m_decl, m_decl->type);
    for (unsigned int p = 0; p < m_parameterblock->symbolCount(); p++)
    {
	SymbolEntry *sentry = m_parameterblock->symbolEntry (p);
	y2debug ("%d: %s", p, sentry->toString(true).c_str());
	switch (sentry->category())
	{
	    case SymbolEntry::c_const:			// code
	    {
		builtin->attachBuiltinParameter (sentry->code(), TypeCode (""));
	    }
	    break;
	    case SymbolEntry::c_variable:		// symbol
	    {
		builtin->attachBuiltinParameter (new YConst (ycEntry, YCPEntry (sentry)), "");
		m_parameterblock->releaseSymbol (p);
	    }
	    break;
	    default:
	    {
		extern ExecutionEnvironment ee;

		ycp2error (ee.filename().c_str(), ee.linenumber(), "YESymFunc::toBuiltin with bad parameter");
		delete builtin;
		builtin = 0;
	    }
	    break;
	}
	if (builtin == 0)
	{
	    break;
	}
    }
    m_parameterblock->detachEnvironment (table);

    return builtin;
}


// attach value parameter to function
// returns 0 on error
declaration_t *
YESymFunc::attachSymValue (const TypeCode &type, unsigned int line, YCode *code)
{
    if ((code != 0)
	&& (code->isError()))
    {
	return 0;
    }

    extern StaticDeclaration static_declarations;

    y2debug ("YESymFunc::attachSymValue (%s:%s @%d", code->toString().c_str(), type.toString().c_str(), line);

    m_parameterblock->newValue (type, code);
    m_type = newtype (m_type, type);

    // attach new type to current type string and search matching decl
    //   FIXME: improve performance by checking if overload still possible (no full match found yet)

    declaration_t *old_decl = m_decl;
    m_decl = static_declarations.findDeclaration (m_decl, m_type, true);	// must do partial, incomplete parameters

    if (m_decl == 0)
    {
	// FIXME: check propagation
	static_declarations.findDeclaration (old_decl, m_type);		// trigger error report
	// Oops, no match found
	return m_decl;
    }

    m_type = determineFlexType (m_type, m_decl->type, type);

    return m_decl;
}


// attach symbolic variable parameter to function, return created TableEntry
//  for symbol parameters, if type is unspecified it's up to the declaration
//    (flags & DECL_SYMBOL) if a "...,`x,..." parameter gets converted to
//     a ycSymbol(x) or a ycEntry(any x)
// returns 0 on error

declaration_t *
YESymFunc::attachSymVariable (const char *name, const TypeCode &type, unsigned int line, TableEntry *&tentry)
{
    extern StaticDeclaration static_declarations;

    TypeCode matchedType;
    TypeCode addedType;

    declaration_t *decl = 0;

    y2debug ("YESymFunc::attachSymVariable (%s:%s @%d", name, type.toString().c_str(), line);

    if (type.isUnspec())
    {
	// try with symbol constant first
	addedType = TypeCode ("y");
	matchedType = newtype (m_type, addedType);
	decl = static_declarations.findDeclaration (m_decl, matchedType, true);
	if (decl != 0)
	{
	    m_parameterblock->newValue (addedType, new YConst (YCode::ycSymbol, YCPSymbol (name)));
	}
	else
	{
	    addedType = TypeCode ("Ya");
	    matchedType = newtype (m_type, addedType);
	    decl = static_declarations.findDeclaration (m_decl, matchedType, true);
	    if (decl != 0)
	    {
		extern ExecutionEnvironment ee;
		tentry = m_parameterblock->newEntry (name, SymbolEntry::c_variable, TypeCode ("a"), line);
		if (tentry == 0)
		{
		    ycp2error (ee.filename().c_str(), ee.linenumber(), "Duplicate symbol name %s", name);
		    return 0;
		}
		ycp2warning (ee.filename().c_str(), ee.linenumber(), "Parameter '%s' has unspecified type", name);
	    }
	}
    }
    else
    {
	addedType = newtype (TypeCode ("Y"), type);		// it's a typed symbolic variable
    }

    y2debug ("addedType %s", addedType.asString().c_str());

    if (decl == 0)	// if not set before
    {
	tentry = m_parameterblock->newEntry (name, SymbolEntry::c_variable, type, line);
	if (tentry == 0)
	{
	    extern ExecutionEnvironment ee;

	    ycp2error (ee.filename().c_str(), ee.linenumber(), "Duplicate symbol name %s", name);
	    return 0;
	}

	matchedType = newtype (m_type, addedType);

	// attach new type to current type string and search matching decl
	//   FIXME: improve performance by checking if overload still possible (no full match found yet)
	m_decl = static_declarations.findDeclaration (m_decl, matchedType, true);

	if (m_decl == 0)
	{
	    // Oops, no match found
	    return m_decl;
	}
    }
    else
    {
	m_decl = decl;
    }

    m_type = determineFlexType (matchedType, m_decl->type, addedType);

    return m_decl;
}


string
YESymFunc::toString() const
{
    string s = Decl2String (m_decl) + " (";
y2debug ("YESymFunc::toString (%s)", s.c_str());

    for (unsigned int p = 0; p < m_parameterblock->symbolCount(); p++)
    {
	if (p > 0)
	    s += ", ";

	SymbolEntry *sentry = m_parameterblock->symbolEntry (p);
y2debug ("%d: %s", p, sentry ? sentry->toString(true).c_str() : "NULL");
	switch (sentry->category())
	{
	    case SymbolEntry::c_const:
	    {
		s += sentry->code()->toString();
	    }
	    break;
	    case SymbolEntry::c_variable:
	    {
		s += sentry->toString (true);
	    }
	    break;
	    default:
	    {
		s += "???";
	    }
	}
    }
    s += ")";
y2debug ("YESymFunc::toString (%s)", s.c_str());
    return s;
}


YCPValue
YESymFunc::evaluate (bool cse)
{
    y2debug ("YESymFunc::evaluate [%d]", code());
    if (cse)
    {
	return YCPNull();
    }

    const unsigned int maxargs = 6;
    YCPValue args[maxargs] = { YCPNull(), YCPNull(), YCPNull(), YCPNull(), YCPNull(), YCPNull() };

    if (m_parameterblock->symbolCount() > maxargs)
    {
	return YCPError ("Too many arguments");
    }

    for (unsigned int p = 0; p < m_parameterblock->symbolCount(); p++)
    {
	SymbolEntry *sentry = m_parameterblock->symbolEntry (p);
	switch (sentry->category())
	{
	    case SymbolEntry::c_const:
	    {
		args[p] = sentry->code()->evaluate ();
		// YCPCode()
	    }
	    break;
	    case SymbolEntry::c_variable:
	    {
		args[p] = YCPEntry (sentry);
	    }
	    break;
	    default:
	    {
		return YCPError ("Bad parameter kind");
	    }
	}

	if ((args[p].isNull() || args[p]->isVoid())
	    && ((m_decl->flags & DECL_NIL) == 0))
	{
	    extern ExecutionEnvironment ee;
	    ycp2error (ee.filename().c_str(), ee.linenumber(), "Argument (%s) to %s(...) is nil", (sentry->category() == SymbolEntry::c_const) ? sentry->code()->toString().c_str() : sentry->toString(true).c_str(), m_decl->name);
	    return YCPError("invalid argument");
	}

    }

    y2debug ("YESymFunc::evaluate <%d>", m_parameterblock->symbolCount());
    YCPValue ret = YCPNull();
    switch (m_parameterblock->symbolCount())
    {
	case 0:
	    ret = (*(v2)m_decl->ptr) ();
	break;
	case 1:
	    ret = (*(v2v)m_decl->ptr) (args[0]);
	break;
	case 2:
	    ret = (*(v2vv)m_decl->ptr) (args[0], args[1]);
	break;
	case 3:
	    ret = (*(v2vvv)m_decl->ptr) (args[0], args[1], args[2]);
	break;
	case 4:
	    ret = (*(v2vvvv)m_decl->ptr) (args[0], args[1], args[2], args[3]);
	break;
	default:
	{
	    extern ExecutionEnvironment ee;
	    ycp2error (ee.filename().c_str(), ee.linenumber(), "Arg count %d", m_parameterblock->symbolCount());
	    ret = YCPError ("YESymFunc bad");
	}
	break;
    }

#ifdef BUILTIN_STATISTICS
    if (!ret.isNull ())
    {
	FILE *fout = fopen ("/tmp/builtin-use.txt", "a");
	fprintf (fout, "%s %s\n", m_decl->name, m_decl->type);
	fclose (fout);
    }
#endif

    y2debug ("YESymFunc ret (%s)", ret.isNull() ? "NULL" : ret->toString().c_str());
    return ret;
}


std::ostream &
YESymFunc::toStream (std::ostream & str) const
{
    YCode::toStream (str);
    return str;
}

// ------------------------------------------------------------------
// function ref (-> SymbolEntry + Parameters)

YEFunction::YEFunction (SymbolEntry *entry)
    : YCode (yeFunction)
    , m_entry (entry)
    , m_parameters (0)
{
}


YEFunction::YEFunction (std::istream & str)
    : YCode (yeFunction)
    , m_parameters (0)
{
    m_entry = Bytecode::readEntry (str);
    Bytecode::readYCodelist (str, &m_parameters, 0);
}


YEFunction::~YEFunction ()
{
    ycodelist_t *parm = m_parameters;
    while (parm)
    {
	ycodelist_t *next = parm->next;
	delete parm->code;
	delete parm;
	parm = next;
    }
}

SymbolEntry *
YEFunction::entry() const
{
    return m_entry;
}


TypeCode
YEFunction::attachFunctionParameter (YCode *code, const TypeCode & type)
{
    y2debug ("YEFunction::attachFunctionParameter (type '%s', code '%s')", type.asString().c_str(), code ? code->toString().c_str() : "(NULL)");

    if (code && code->isError())
    {
	return "";
    }

    // retrieve function pointer and formal parameter list
    YFunction *func = (YFunction *)(m_entry->code());

    y2debug ("func ([%d] %s)", m_entry->code()->code(), func->toString().c_str());

    unsigned int actual_count = 0;
    ycodelist_t *actual = m_parameters;

    // simulate a doubly linked list with a head
    /* always pfoo->next == foo */

    static ycodelist_t hactual;

    ycodelist_t *pactual = &hactual;
    pactual->next = actual;

    // move to the end of actual parameter list
    while (actual != 0)
    {
	actual_count++;
	pactual = actual;
	actual = actual->next;
    }

    TypeCode expected_type = "";

    // adding a normal parameter?
    if (code != 0)
    {
	// too many parameters? ('>=' since we're counting from 0..n-1)
	if (actual_count >= func->parameterCount())
	{
	    extern ExecutionEnvironment ee;
	    ycp2error (ee.filename().c_str(), ee.linenumber(), "excessive parameters in function call");
	    return "*";
	}
	// ok, check whether types match

	SymbolEntry *formal = func->parameter (actual_count);
	expected_type = formal->type ();
	y2debug ("expected_type '%s', type '%s'", expected_type.asString().c_str(), type.asString().c_str());

	int match = type.matchtype (expected_type);
	if (match < 0)
	{
	    // type mismatch
	    return expected_type;
	}
	if (match > 0)				// propagation possible
	{
	    code = new YEPropagate (code, type, expected_type);
	}

	// allocate new function parameter element
	ycodelist_t *element = new ycodelist_t;
	element->code = code;
	element->next = 0;

	if (pactual != &hactual)
	{
	    pactual->next = element;
	}
	else
	{
	    // the list head is not real
	    // so we must assign to the proper place
	    m_parameters = element;
	}
	return "";
    }
    // end of actual parameters
    else
    {
	// missing parameter
	if (actual_count < func->parameterCount())
	{
	    SymbolEntry *formal = func->parameter (actual_count);
	    expected_type = formal->type ();
	    y2debug ("Missing expected_type '%s'", expected_type.asString().c_str());
	    return expected_type;
	}

	// end of formal and actual params
	return "";
    }

    y2internal ("Unreachable code");
    return "*";
}


string
YEFunction::toString() const
{
    string s = m_entry->toString(false);

    s += " (";
    ycodelist_t *parm = m_parameters;
    while (parm)
    {
	s += parm->code->toString().c_str();
	if (parm->next != 0)
	{
	    s += ", ";
	}
	parm = parm->next;
    }
    s += ")";
    return s;
}


YCPValue
YEFunction::evaluate (bool cse)
{
    if (cse)
    {
	return YCPNull();
    }

    y2debug ("YEFunction::evaluate (%s)\n", toString().c_str());

    ycodelist_t *actualp = m_parameters;
    YFunction *func = (YFunction *)(m_entry->code());

    for (unsigned int p = 0; p < func->parameterCount(); p++)
    {
	if (actualp == 0)
	{
	    break;
	}

	// FIXME, check for symbol or block type and suppress evaluation

	YCPValue value = actualp->code->evaluate ();

	if (value.isNull())
	{
	    extern ExecutionEnvironment ee;
	    ycp2error (ee.filename().c_str(), ee.linenumber(), "Parameter eval failed (%s)", actualp->code->toString().c_str());
	    return value;
	}

	SymbolEntry *formalp = func->parameter (p);
	y2debug ("formalp (%s) = (%s)", formalp->toString().c_str(), actualp->code->toString().c_str());

	formalp->setValue (value);
	actualp = actualp->next;
    }

    YBlock *body = func->body();

    YCPValue value = body->evaluate ();

    y2debug("evaluate done (%s) = '%s'", body->toString().c_str(), value.isNull()?"NULL":value->toString().c_str());
    return value;
}


std::ostream &
YEFunction::toStream (std::ostream & str) const
{
    YCode::toStream (str);
    if (Bytecode::writeEntry (str, m_entry))
    {
	Bytecode::writeYCodelist (str, m_parameters);
    }
    return str;
}


// ------------------------------------------------------------------
// locale expression (-> singular, plural, count)

YELocale::YELocale (const char *singular, const char *plural, YCode *count, const char *textdomain)
    : YCode (yeLocale)
    , m_singular (singular)
    , m_plural (plural)
    , m_count (count)
    , m_domain (textdomain)
{
}


YELocale::YELocale (std::istream & str)
    : YCode (yeLocale)
{
    m_singular = Bytecode::readCharp (str);
    m_plural = Bytecode::readCharp (str);
    m_count = Bytecode::readCode (str);
    m_domain = Bytecode::readCharp (str);
}


YELocale::~YELocale ()
{
    free ((void *)m_singular);
    free ((void *)m_plural);
    delete (m_count);
}


string
YELocale::toString () const
{
    return "_(\"" + string (m_singular)
	   + "\", \"" + string (m_plural)
	   + "\", " + m_count->toString()
	   + ")";
}


YCPValue
YELocale::evaluate (bool cse)
{
    y2debug ("YELocale::evaluate(%s)\n", toString().c_str());
    if (cse)
    {
	return YCPNull();
    }

    YCPValue count = m_count->evaluate ();
    if (count.isNull())
    {
	return YCPError ("YELocale::evaluate invalid count");
    }
    if (!count->isInteger ())
    {
	return YCPError ("YELocale::evaluate count not integer");
    }

    const char *ret = dngettext (m_domain, m_singular, m_plural, count->asInteger()->value());

    y2debug ("localize <%s, %s, %d> to <%s>", m_singular, m_plural, (int)(count->asInteger()->value()), ret);

    return YCPString (ret);
}


std::ostream &
YELocale::toStream (std::ostream & str) const
{
    YCode::toStream (str);
    Bytecode::writeCharp (str, m_singular);
    Bytecode::writeCharp (str, m_plural);
    m_count->toStream (str);
    return Bytecode::writeCharp (str, m_domain);
}

// ------------------------------------------------------------------
// list expression (-> value, next list value)

YEList::YEList (YCode *code)
    : YCode (yeList)
{
    m_first = new ycodelist_t;
    m_first->code = code;
    m_first->next = 0;
    m_last = m_first;
}


YEList::YEList (std::istream & str)
    : YCode (yeList)
    , m_first (0)
    , m_last (0)
{
    Bytecode::readYCodelist (str, &m_first, &m_last);
}


YEList::~YEList ()
{
    ycodelist_t *element = m_first;
    ycodelist_t *next;
    while (element)
    {
	delete element->code;
	next = element->next;
	delete element;
	element = next;
    }
}


void
YEList::attach (YCode *code)
{
    ycodelist_t *element = new ycodelist_t;
    element->code = code;
    element->next = 0;
    m_last->next = element;
    m_last = element;
}


string
YEList::toString() const
{
    ycodelist_t *element = m_first;
    string s = "[";
    while (element)
    {
	if (element != m_first)
	    s += ", ";
	s += element->code->toString();
	element = element->next;
    }
    return s + "]";
}

YCPValue
YEList::evaluate (bool cse)
{
    y2debug ("YEList::evaluate(%s)\n", toString().c_str());
    YCPList list;
    ycodelist_t *element = m_first;
    while (element)
    {
	YCPValue value = element->code->evaluate (cse);
	if (value.isNull())
	{
	    return value;
	}
	else
	{
	    list->add (value);
	}
	element = element->next;
    }
    return list;
}

std::ostream &
YEList::toStream (std::ostream & str) const
{
    YCode::toStream (str);
    return Bytecode::writeYCodelist (str, m_first);
}

// ------------------------------------------------------------------
// map expression (-> key, value, next key/value pair)

YEMap::YEMap (YCode *key, YCode *value)
    : YCode (yeMap)
    , m_first (0)
    , m_last (0)
{
    attach (key, value);
}


YEMap::YEMap (std::istream & str)
    : YCode (yeMap)
    , m_first (0)
    , m_last (0)
{
    u_int32_t count = Bytecode::readInt32 (str);
    while (count-- > 0)
    {
	YCode *key = Bytecode::readCode (str);
	YCode *value = Bytecode::readCode (str);
	attach (key, value);
    }
}


YEMap::~YEMap ()
{
    mapval_t *element = m_first;
    mapval_t *next;
    while (element)
    {
	delete element->key;
	delete element->value;
	next = element->next;
	delete element;
	element = next;
    }
}


void
YEMap::attach (YCode *key, YCode *value)
{
    mapval_t *element = new mapval_t;
    element->key = key;
    element->value = value;
    element->next = 0;

    if (m_last == 0)
    {
	m_first = element;
    }
    else
    {
	m_last->next = element;
    }
    m_last = element;
}


string
YEMap::toString() const
{
    mapval_t *element = m_first;
    string s = "$[";
    while (element)
    {
	if (element != m_first)
	    s += ", ";
	s += element->key->toString();
	s += ":";
	s += element->value->toString();
	element = element->next;
    }
    return s + "]";
}


YCPValue
YEMap::evaluate (bool cse)
{
    y2debug ("YEMap::evaluate (%s)\n", toString().c_str());
    YCPMap map;
    mapval_t *element = m_first;
    while (element)
    {
	YCPValue key = element->key->evaluate (cse);
	if (key.isNull())
	{
	    if (element->key->isConstant())
	    {
		return YCPError ("Key evaluates to 'nil'");
	    }
	    if (cse)		// parse time checking, not a constant
	    {
		return key;
	    }
	}
	YCPValue value = element->value->evaluate (cse);
	if (value.isNull())
	{
	    if (cse)		// parse time checking, not a constant
		return value;
	}
	map->add (key, value);
	element = element->next;
    }
    return map;
}


std::ostream &
YEMap::toStream (std::ostream & str) const
{
    YCode::toStream (str);
    u_int32_t count = 0;
    mapval_t *mapp = m_first;
    while (mapp)
    {
	count++;
	mapp = mapp->next;
    }
    Bytecode::writeInt32 (str, count);

    mapp = m_first;
    while (mapp)
    {
	mapp->key->toStream (str);
	mapp->value->toStream (str);
	mapp = mapp->next;
    }
    return str;
}

// ------------------------------------------------------------------
// lookup expression (-> code, code, code, type)

YELookup::YELookup (YCode *map, YCode *key, YCode *dflt, const TypeCode & type)
    : YCode (yeLookup)
    , m_map (map)
    , m_key (key)
    , m_default (dflt)
    , m_type (type.valueType ())
{
}


YELookup::YELookup (std::istream & str)
    : YCode (yeLookup)
{
    m_map = Bytecode::readCode (str);
    m_key = Bytecode::readCode (str);
    m_default = Bytecode::readCode (str);
}


YELookup::~YELookup ()
{
    delete m_map;
    delete m_key;
    delete m_default;
}

string
YELookup::toString () const
{
    string s = "lookup (";
    s += m_map->toString();
    s += ", ";
    s += m_key->toString();
    s += ", ";
    s += m_default->toString();
    s += ")";
    return s;
}

YCPValue
YELookup::evaluate (bool cse)
{
    /**
     * @builtin lookup (map m, any k, any default) -> any
     * Looks up the value matching to given key <tt>k</tt>. Returns
     * <tt>default</tt> if the key was not found or has a different
     * type than <tt>default</tt>.
     *
     * Example: <pre>
     * lookup ($[1:"a", 2:"bc"], 371, "take this") -> "take this"
     *
     * Type mismatch, returns default:
     * lookup ($[1:"a", 2:"bc"], 1, true) -> true
     * </pre>
     */

    YCPValue v_map = m_map->evaluate (cse);
    YCPValue v_key = m_key->evaluate (cse);
    YCPValue v_default = m_default->evaluate (cse);

    y2debug ("YELookup::evaluate (%s,%s,%s)",
	v_map.isNull()?"nil":v_map->toString().c_str(),
	v_key.isNull()?"nil":v_key->toString().c_str(),
	v_default.isNull()?"nil":v_default->toString().c_str());

    if (v_map.isNull() || v_map->isVoid())
	return v_default;

    if (v_key.isNull() || v_key->isVoid())
	return v_default;

    YCPValue ret = v_map->asMap()->value (v_key);

    if (ret.isNull()				// not found
	|| ((m_type != YT_VOID)
	    && (ret->valuetype () != m_type)))	// or wrong type
	return v_default;			// -> return default

    return ret;
}


std::ostream &
YELookup::toStream (std::ostream & str) const
{
    YCode::toStream (str);
    m_map->toStream (str);
    m_key->toStream (str);
    return m_default->toStream (str);
}


// ------------------------------------------------------------------
// propagation expression (-> declaration_t for conversion, value)

YEPropagate::YEPropagate (YCode *value, const TypeCode & from, const TypeCode & to)
    : YCode (yePropagate)
    , m_from (from)
    , m_to (to)
    , m_value (value)
{
    //FIXME: save declaration/ptr to propagation function instead of from & to
    if (m_from.isFloat())
    {
	extern ExecutionEnvironment ee;
	ycp2warning (ee.filename().c_str(), ee.linenumber(), "Implicit float conversion will loose accuracy");
    }
}


YEPropagate::YEPropagate (std::istream & str)
    : YCode (yePropagate)
    , m_from (str)
    , m_to (str)
{
    m_value = Bytecode::readCode (str);
}


YEPropagate::~YEPropagate ()
{
    delete m_value;
}


string
YEPropagate::toString() const
{
    return string ("/* ") + m_from.asString().c_str() + " -> " + m_to.asString().c_str() + " */" + m_value->toString();
}


YCPValue
YEPropagate::evaluate (bool cse)
{
    y2debug ("YEPropagate::evaluate(%s)\n", toString().c_str());
    if (cse)
    {
	return YCPNull();
    }

    YCPValue v = m_value->evaluate ();
    if (v.isNull())
    {
	return v;
    }

    // If this proves too slow, maybe optimize it away completely
    // by inventing YEPropagateIntegerFloat
    if (m_to.isFloat()
	&& v->isInteger())
    {
	return YCPFloat (v->asInteger()->value());
    }
    else if (m_to.isInteger()
	     && v->isFloat())
    {
	return YCPInteger ((long long)(v->asFloat()->value()));
    }
    else if (m_to.isString()
	     && v->isString())	// Y(E)Locale evaluation already done
    {
	return v;
    }
    else
    {
	extern ExecutionEnvironment ee;
	ycp2error (ee.filename().c_str(), ee.linenumber(), "Can't convert '%s' to '%s': %s", m_from.toString().c_str(), m_to.toString().c_str(), v->toString().c_str());
    }
    return YCPError ("Propagation error");
}


std::ostream &
YEPropagate::toStream (std::ostream & str) const
{
    YCode::toStream (str);
    m_from.toStream (str);
    m_to.toStream (str);
    return m_value->toStream (str);
}

// ------------------------------------------------------------------
// unary expression (-> declaration_t, arg)

YEUnary::YEUnary (declaration_t *decl, YCode *arg)
    : YCode (yeUnary)
    , m_decl (decl)
    , m_arg (arg)
{
}


YEUnary::YEUnary (std::istream & str)
    : YCode (yeUnary)
{
    extern StaticDeclaration static_declarations;

    m_decl = static_declarations.readDeclaration (str);
    if (m_decl)
    {
	m_arg = Bytecode::readCode (str);
    }
}


YEUnary::~YEUnary ()
{
    delete m_arg;
}


declaration_t *
YEUnary::decl() const
{
    return m_decl;
}


string
YEUnary::toString() const
{
    return Decl2String (m_decl)
	   + " " + m_arg->toString();
}


YCPValue
YEUnary::evaluate (bool cse)
{
    y2debug ("YEUnary::evaluate(%s)\n", toString().c_str());
    if (cse)
    {
	return YCPNull();
    }

    YCPValue arg = m_arg->evaluate ();
    const declaration_t *decl = m_decl;

    y2debug ("func %s (%s)", decl->name, decl->type);

#ifdef BUILTIN_STATISTICS
    FILE *fout = fopen ("/tmp/builtin-use.txt", "a");
    fprintf (fout, "%s %s\n", decl->name, decl->type);
    fclose (fout);
#endif

    return (*(v2v)decl->ptr) (arg);
}


std::ostream &
YEUnary::toStream (std::ostream & str) const
{
    YCode::toStream (str);
    extern StaticDeclaration static_declarations;

    static_declarations.writeDeclaration (str, m_decl);
    return m_arg->toStream (str);
}

// ------------------------------------------------------------------
// binary expression (-> declaration_t, arg1, arg2)

YEBinary::YEBinary (declaration_t *decl, YCode *arg1, YCode *arg2)
    : YCode (yeBinary)
    , m_decl (decl)
    , m_arg1 (arg1)
    , m_arg2 (arg2)
{
}


YEBinary::YEBinary (std::istream & str)
    : YCode (yeBinary)
{
    extern StaticDeclaration static_declarations;

    m_decl = static_declarations.readDeclaration (str);
    if (m_decl)
    {
	m_arg1 = Bytecode::readCode (str);
	m_arg2 = Bytecode::readCode (str);
    }
}


declaration_t *
YEBinary::decl()
{
    return m_decl;
}


string
YEBinary::toString() const
{
    return "(" + m_arg1->toString()
	   + " " + Decl2String (m_decl)
	   + " " + m_arg2->toString() + ")";
}


YCPValue
YEBinary::evaluate (bool cse)
{
    if (cse) return YCPNull();

    y2debug ("YEBinary::evaluate(%s)\n", toString().c_str());

    const YCPValue arg1 = m_arg1->evaluate ();
    if ((arg1.isNull() || arg1->isVoid())
	&& ((m_decl->flags & DECL_NIL) == 0))
    {
	extern ExecutionEnvironment ee;
	ycp2error (ee.filename().c_str(), ee.linenumber(), "Argument (%s) to %s(...) evaluates to nil", m_arg1->toString().c_str(), m_decl->name);
	return YCPError("invalid argument");
    }
    const YCPValue arg2 = m_arg2->evaluate ();
    if ((arg2.isNull() || arg2->isVoid())
	&& ((m_decl->flags & DECL_NIL) == 0))
    {
	extern ExecutionEnvironment ee;
	ycp2error (ee.filename().c_str(), ee.linenumber(), "Argument (%s) to %s(...) evaluates to nil", m_arg2->toString().c_str(), m_decl->name);
	return YCPError("invalid argument");
    }
    const declaration_t *decl = m_decl;
    y2debug ("func %s (%s) [%s,%s]", decl->name, decl->type, arg1->toString().c_str(), arg2->toString().c_str());
    y2debug ("type1 %d, type2 %d", (int)arg1->valuetype(), (int)arg2->valuetype());

#ifdef BUILTIN_STATISTICS
    FILE *fout = fopen ("/tmp/builtin-use.txt", "a");
    fprintf (fout, "%s %s\n", decl->name, decl->type);
    fclose (fout);
#endif

    return (*(v2vv)decl->ptr) (arg1, arg2);
}


std::ostream &
YEBinary::toStream (std::ostream & str) const
{
    YCode::toStream (str);
    extern StaticDeclaration static_declarations;

    static_declarations.writeDeclaration (str, m_decl);
    m_arg1->toStream (str);
    return m_arg2->toStream (str);
}

// ------------------------------------------------------------------
// Triple (? :) expression (-> bool expr, true value, false value)

YETriple::YETriple (YCode *a_expr, YCode *a_true, YCode *a_false)
    : YCode (yeTriple)
    , m_expr (a_expr)
    , m_true (a_true)
    , m_false (a_false)
{
}


YETriple::YETriple (std::istream & str)
    : YCode (yeTriple)
{
    m_expr = Bytecode::readCode (str);
    m_true = Bytecode::readCode (str);
    m_false = Bytecode::readCode (str);
}


string
YETriple::toString() const
{
    return m_expr->toString()
	   + " ? " + m_true->toString()
	   + " : " + m_false->toString();
}


YCPValue
YETriple::evaluate (bool cse)
{
    y2debug ("YETriple::evaluate(%s)\n", toString().c_str());
    if (cse)
    {
	return YCPNull();
    }

    YCPValue expr = m_expr->evaluate ();
    if (expr->isBoolean())
    {
	if (expr->asBoolean()->value() == true)
	{
	    return m_true->evaluate ();
	}
	else
	{
	    return m_false->evaluate ();
	}
    }
    return YCPError ("Can't evaluate YETriple(" + toString() + ")");
}


std::ostream &
YETriple::toStream (std::ostream & str) const
{
    YCode::toStream (str);
    m_expr->toStream (str);
    m_true->toStream (str);
    return m_false->toStream (str);
}

// ------------------------------------------------------------------
// is (expression, type)

YEIs::YEIs (YCode *expr, const TypeCode & type)
    : YCode (yeIs)
    , m_expr (expr)
    , m_type (type)
{
}


YEIs::YEIs (std::istream & str)
    : YCode (yeIs)
    , m_type (str)
{
    m_expr = Bytecode::readCode (str);
}


YEIs::~YEIs ()
{
    delete m_expr;
}


string
YEIs::toString () const
{
    string s = "is ("
	+ m_expr->toString()
	+ ", " + m_type.toString()
	+ ")";
    return s;
}


YCPValue
YEIs::evaluate (bool cse)
{
    YCPValue value = m_expr->evaluate (cse);
    if (value.isNull())
    {
	return YCPError ("'is()' expression evaluates to 'NULL'.", YCPBoolean (false));
    }
    TypeCode expected_type = TypeCode::vt2type (value->valuetype());
    // FIXME, check "L", "M" better
    return YCPBoolean (equaltype (m_type, expected_type));
}


std::ostream &
YEIs::toStream (std::ostream & str) const
{
    YCode::toStream (str);
    m_type.toStream (str);
    return m_expr->toStream (str);
}

// ------------------------------------------------------------------
// Return (expression)

YEReturn::YEReturn (YCode *expr)
    : YCode (yeReturn)
    , m_expr (expr)
{
}


YEReturn::YEReturn (std::istream & str)
    : YCode (yeReturn)
{
    m_expr = Bytecode::readCode (str);
}


YEReturn::~YEReturn ()
{
    delete m_expr;
}


string
YEReturn::toString () const
{
    string s = "{ return "
	+ m_expr->toString()
	+ "; }";
    return s;
}


YCPValue
YEReturn::evaluate (bool cse)
{
    return YCPCode (m_expr);
}


std::ostream &
YEReturn::toStream (std::ostream & str) const
{
    YCode::toStream (str);
    return m_expr->toStream (str);
}

// ------------------------------------------------------------------
// bracket expression: identifier [ arg, arg, ...] : default

YEBracket::YEBracket (YCode *var, YCode *arg, YCode *def)
    : YCode (yeBracket)
    , m_var (var)
    , m_arg (arg)
    , m_def (def)
{
}


YEBracket::YEBracket (std::istream & str)
    : YCode (yeBracket)
{
    m_var = Bytecode::readCode (str);
    m_arg = Bytecode::readCode (str);
    m_def = Bytecode::readCode (str);
}


YEBracket::~YEBracket ()
{
    delete m_var;
    delete m_arg;
    delete m_def;
}


string
YEBracket::toString () const
{
    return m_var->toString()
	   + m_arg->toString()
	   + string (":")
	   + m_def->toString();
}


YCPValue
YEBracket::evaluate (bool cse)
{
    YCPValue var_value = m_var->evaluate (cse);
    YCPValue def_value = m_def->evaluate (cse);

    if (var_value->isVoid())
	return def_value;

    YCPValue arg_value = m_arg->evaluate (cse);

    if (arg_value->isVoid()
	|| !arg_value->isList())
    {
	return def_value;
    }

    YCPValue result = var_value;

    YCPList indices = arg_value->asList();
    for (int i = 0; i < indices->size(); ++i) // loop over all bracket indices
    {
	YCPValue v = indices->value(i);
	if (v.isNull())
	{
	    result = YCPError ("Invalid bracket parameter", def_value);
	    break;
	}
	else if (result->isList())
	{
	    YCPList l = result->asList();
	    if (!v->isInteger())
	    {
		result = YCPError ("Invalid bracket parameter for list", def_value);
		break;
	    }

	    long long idx = v->asInteger()->value();
	    if ((idx < 0)
		|| (idx >= l->size()))
	    {
		result = def_value;
		break;
	    }
	    result = l->value (idx);
	}
	else if (result->isTerm())
	{
	    YCPTerm t = result->asTerm();
	    if (!v->isInteger())
	    {
		result = YCPError ("Invalid bracket parameter for term", def_value);
		break;
	    }

	    long long idx = v->asInteger()->value();
	    if ((idx < 0)
		|| (idx >= t->size()))
	    {
		result = def_value;
		break;
	    }
	    result = t->value (idx);
	}
	else if (result->isMap())
	{
	    YCPMap m = result->asMap();
	    result = m->value (v);
	}
	else
	{
	    result = YCPError (string ("Bracket expression for '"
			     + result->toString()
			     + "' does not evaluate to a list or a map."), def_value);
	    break;
	}

	if (result.isNull())
	{
	    result = def_value;
	    break;
	}

    } // while bracket indices

    return result;
}

std::ostream &
YEBracket::toStream (std::ostream & str) const
{
    YCode::toStream (str);
    m_var->toStream (str);
    m_arg->toStream (str);
    return m_def->toStream (str);
}

// EOF
