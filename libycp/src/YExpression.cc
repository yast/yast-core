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

#include "ycp/y2log.h"
#include "ycp/StaticDeclaration.h"

#include "ycp/YStatement.h"
#include "ycp/YBlock.h"
#include "ycp/YCPCode.h"
#include "ycp/YCPTerm.h"
#include "ycp/YCPBoolean.h"
#include "ycp/YCPInteger.h"
#include "ycp/YCPFloat.h"
#include "ycp/YCPList.h"
#include "ycp/YCPMap.h"
#include "ycp/YCPVoid.h"
#include "ycp/YExpression.h"

#include "ycp/Bytecode.h"

#include "ycp/y2log.h"
#include "ycp/ExecutionEnvironment.h"

typedef YCPValue (*v2) ();
typedef YCPValue (*v2v) (const YCPValue &);
typedef YCPValue (*v2vv) (const YCPValue &, const YCPValue &);
typedef YCPValue (*v2vvv) (const YCPValue &, const YCPValue &, const YCPValue &);
typedef YCPValue (*v2vvvv) (const YCPValue &, const YCPValue &, const YCPValue &, const YCPValue &);
typedef YCPValue (*v2vvvvv) (const YCPValue &, const YCPValue &, const YCPValue &, const YCPValue &, const YCPValue &);

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
	// it's OK for the functions (somebody wants our code (function pointers)), but not others
	if (! m_entry->isFunction ())
	{
	    y2error ("YEVariable::evaluate (%s) has no value", toString().c_str());
	}
	value = YCPCode (m_entry->code());
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
// reference (-> SymbolEntry)

YEReference::YEReference (SymbolEntry *entry)
    : YCode (yeReference)
    , m_entry (entry)
{
    y2debug ("YEReference::YEReference (%s)", entry->toString().c_str());
}


YEReference::YEReference (std::istream & str)
    : YCode (yeReference)
{
    m_entry = Bytecode::readEntry (str);
}


SymbolEntry *
YEReference::entry() const
{
    return m_entry;
}


const char *
YEReference::name() const
{
    return m_entry->name();
}


string
YEReference::toString() const
{
    return m_entry->toString(false);
}


YCPValue
YEReference::evaluate (bool cse)
{
    if (cse) return YCPNull();

    YCPValue value = YCPReference (m_entry);

    y2debug ("YEReference::evaluate (%s) = %s", toString().c_str(), value.isNull() ? "NULL" : value->toString().c_str());
    return value;
}


std::ostream &
YEReference::toStream (std::ostream & str) const
{
    YCode::toStream (str);
    Bytecode::writeEntry (str, m_entry);
    return str;
}


constTypePtr
YEReference::type () const
{
    TypePtr t = m_entry->type()->clone();
    t->asReference();
    return t;
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


    /**
     * Attach parameter to external function call
     * @param code: parameter code
     * @param type: parameter type
     * @return NULL if success,
     *    != NULL (expected type) if wrong parameter type
     *    Type::Unspec if bad code (NULL or isError)
     *    Type::Error if excessive parameter
     */

constTypePtr
YETerm::attachParameter (YCode *code, constTypePtr dummy)
{
    if ((code == 0)
	|| (code->isError()))
    {
	y2debug ("YETerm::attachParameter (Error)");
	return Type::Unspec;
    }

    y2debug ("YETerm::attachParameter (%s)", code->toString().c_str());

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

    return 0;
}


string
YETerm::toString () const
{
    string s = "`" + string (m_name) + " (";

    ycodelist_t *parm = m_parameters;
    while (parm)
    {
	if (parm->code == 0)
	{
	    y2error( "parameter without code");
	    parm = parm->next;
	    continue;
	}
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
	    if (!cse)		// not parse time
	    {
		ycp2error ("Term parameter evaluates to 'NULL'");
	    }
	    return YCPNull ();
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

    if ( (vl.isNull () || vl->isVoid () || vr.isNull () || vr->isVoid ()) && (m_op != C_EQ && m_op != C_NEQ) )	// nil can be compared only for (n)equality
    {
	ycp2error ("Nil can be compared only for equality and non-equality");
	return YCPNull ();
    }

    // left value is nil
    if (vl.isNull() || vl->isVoid ())
    {
	if (m_op == C_NEQ)
	{
	    return YCPBoolean (! (vr.isNull () || vr->isVoid ()));
	}
	else
	{
	    // only C_EQ, others are not permitted above
	    return YCPBoolean (vr.isNull() || vr->isVoid ());	// 'nil == nil'
	}
    }

    // left != nil, right == nil

    if (vr.isNull() || vr->isVoid ())
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
    y2error ("YECompare unknown type");
    return YCPBoolean (false);
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
	ycp2error ("YELocale::evaluate invalid count");
	return YCPNull ();
    }
    if (!count->isInteger ())
    {
	ycp2error ("YELocale::evaluate count not integer");
	return YCPNull ();
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


int YEList::count () const
{
    int res = 0;
    ycodelist_t *element = m_first;
    while (element)
    {
	element = element->next;
	res ++;
    }
    return res;
}


YCode* YEList::value (int index) const
{
    ycodelist_t *element = m_first;
    while (element && index)
    {
	element = element->next;
	index--;
    }
    return element != NULL ? element->code : NULL;
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
		ycp2error ("Key evaluates to 'nil'");
		return YCPNull ();
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


// I will let this comment here for the moment
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

// ------------------------------------------------------------------
// propagation expression (-> declaration_t for conversion, value)

YEPropagate::YEPropagate (YCode *value, constTypePtr from, constTypePtr to)
    : YCode (yePropagate)
    , m_from (from)
    , m_to (to)
    , m_value (value)
{
    //FIXME: save declaration/ptr to propagation function instead of from & to
    if (m_from->isFloat())
    {
	extern ExecutionEnvironment ee;
	ycp2warning (ee.filename().c_str(), ee.linenumber(), "Implicit float conversion will loose accuracy");
    }
}


YEPropagate::YEPropagate (std::istream & str)
    : YCode (yePropagate)
    , m_from (Bytecode::readType (str))
    , m_to (Bytecode::readType (str))
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
    return string ("/* ") + m_from->toString().c_str() + " -> " + m_to->toString().c_str() + " */" + m_value->toString();
}

bool
YEPropagate::canPropagate(const YCPValue& value, constTypePtr to_type) const
{
    if (to_type->isAny () || to_type->isUnspec () || ( to_type->isBasetype () && value->valuetype () == to_type->valueType ()))
	return true;

    y2debug ("to type: %s", to_type->toString ().c_str () );
    if (to_type->isList () && value->isList ())
    {
	// check types of all elements
	constTypePtr elem = ((constListTypePtr)to_type)->type ();
	if (elem->isAny ()) return true;
	if (elem->isUnspec ()) return true;

	YCPList v = value->asList ();

	for (int i=0; i < v->size (); i++ )
	{
	    y2debug ("testing %s", v->value (i)->toString ().c_str ());
	    if (! canPropagate (v->value (i), elem) )
		return false;
	}
	return true;
    }

    if (to_type->isMap () && value->isMap ())
    {
	// check types of all elements
	constTypePtr key = ((constMapTypePtr)to_type)->keytype ();
	constTypePtr elem = ((constMapTypePtr)to_type)->valuetype ();

	if (elem->isAny () && key->isAny ()) return true;

	// not typed maps
	if (elem->isUnspec () && key->isUnspec ()) return true;

	YCPMap map = value->asMap ();

	for (YCPMapIterator pos = map->begin (); pos != map->end (); pos++)
	{
	    if (! canPropagate (pos.key (), key) )
		return false;
	    if (! canPropagate (pos.value (), elem) )
		return false;
	}
	return true;
    }

    if (to_type->isFunction () && value->isCode ())
    {
	YCode* c = value->asCode ()->code ();
	constFunctionTypePtr t = (constFunctionTypePtr)to_type;
#warning Function type cannot be checked at runtime, because of the return type
	return c->kind () == ycFunction;
    }

    return false;
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
    if (v.isNull() || v->isVoid ())
    {
	return v;
    }

    // If this proves too slow, maybe optimize it away completely
    // by inventing YEPropagateIntegerFloat
    if (m_to->isFloat()
	&& v->isInteger())
    {
	return YCPFloat (v->asInteger()->value());
    }
    else if (m_to->isInteger()
	     && v->isFloat())
    {
	return YCPInteger ((long long)(v->asFloat()->value()));
    }
    else if (canPropagate (v, m_to))
    {
	return v;
    }

    ycp2error ("Can't convert '%s' to '%s': %s", m_from->toString().c_str(), m_to->toString().c_str(), v->toString().c_str());

    return YCPNull ();
}


std::ostream &
YEPropagate::toStream (std::ostream & str) const
{
    YCode::toStream (str);
    m_from->toStream (str);
    m_to->toStream (str);
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
    return StaticDeclaration::Decl2String (m_decl)
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

    y2debug ("func %s (%s)", decl->name, decl->type->toString().c_str());

#ifdef BUILTIN_STATISTICS
    FILE *fout = fopen ("/tmp/builtin-use.txt", "a");
    fprintf (fout, "%s %s\n", decl->name, decl->type->toString().c_str());
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
	+ " " + StaticDeclaration::Decl2String (m_decl)
	+ " " + m_arg2->toString() + ")";
}


YCPValue
YEBinary::evaluate (bool cse)
{
    if (cse) return YCPNull();

    y2debug ("YEBinary::evaluate(%s)\n", toString().c_str());

    if ( (m_decl->flags & DECL_NOEVAL) == DECL_NOEVAL)
    {
	return (*(v2vv)m_decl->ptr) (YCPCode(m_arg1), YCPCode (m_arg2));
    }

    const YCPValue arg1 = m_arg1->evaluate ();
    if ((arg1.isNull() || arg1->isVoid())
	&& ((m_decl->flags & DECL_NIL) == 0))
    {
	ycp2error ("Argument (%s) to %s(...) evaluates to nil", m_arg1->toString().c_str(), m_decl->name);
	return YCPNull ();
    }
    const YCPValue arg2 = m_arg2->evaluate ();
    if ((arg2.isNull() || arg2->isVoid())
	&& ((m_decl->flags & DECL_NIL) == 0))
    {
	ycp2error ("Argument (%s) to %s(...) evaluates to nil", m_arg2->toString().c_str(), m_decl->name);
	return YCPNull ();
    }
    const declaration_t *decl = m_decl;
    y2debug ("func %s (%s) [%s,%s]", decl->name, decl->type->toString().c_str(), arg1->toString().c_str(), arg2->toString().c_str());
    y2debug ("type1 %d, type2 %d", (int)arg1->valuetype(), (int)arg2->valuetype());

#ifdef BUILTIN_STATISTICS
    FILE *fout = fopen ("/tmp/builtin-use.txt", "a");
    fprintf (fout, "%s %s\n", decl->name, decl->type->toString().c_str());
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

    if (expr.isNull ())
    {
	ycp2error ("Condition expression evaluates to nil in ?: expression");
	return YCPNull ();
    }

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
    ycp2error ("Condition expression evaluates to a non boolean value %s in ?: expression", expr->toString ().c_str ());
    return YCPNull ();
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

YEIs::YEIs (YCode *expr, constTypePtr type)
    : YCode (yeIs)
    , m_expr (expr)
    , m_type (type)
{
}


YEIs::YEIs (std::istream & str)
    : YCode (yeIs)
    , m_type (Bytecode::readType (str))
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
	+ ", " + m_type->toString()
	+ ")";
    return s;
}


YCPValue
YEIs::evaluate (bool cse)
{
    YCPValue value = m_expr->evaluate (cse);
    if (value.isNull())
    {
	ycp2error ("'is()' expression evaluates to nil.");
	return YCPBoolean (false);
    }
    constTypePtr expected_type = Type::vt2type (value->valuetype());
    return YCPBoolean (m_type->match (expected_type) >= 0);
}


std::ostream &
YEIs::toStream (std::ostream & str) const
{
    YCode::toStream (str);
    m_type->toStream (str);
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

YEBracket::YEBracket (YCode *var, YCode *arg, YCode *def, constTypePtr resultType)
    : YCode (yeBracket)
    , m_var (var)
    , m_arg (arg)
    , m_def (def)
    , m_resultType (resultType)
{
}


YEBracket::YEBracket (std::istream & str)
    : YCode (yeBracket)
{
    m_var = Bytecode::readCode (str);
    m_arg = Bytecode::readCode (str);
    m_def = Bytecode::readCode (str);
    m_resultType = Bytecode::readType (str);
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

    // parse time?
    if (cse && (var_value.isNull () || def_value.isNull ()) )
	return YCPNull ();

    if (var_value.isNull() || var_value->isVoid())
	return def_value;

    YCPValue arg_value = m_arg->evaluate (cse);

    // parse time?
    if (cse && arg_value.isNull () )
	return YCPNull ();

    if (arg_value.isNull() || arg_value->isVoid()
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
	    result = def_value;
	    ycp2error ("Invalid bracket parameter nil");
	    break;
	}
	else if (result->isList())
	{
	    YCPList l = result->asList();
	    if (!v->isInteger())
	    {
		result = def_value;
		ycp2error ("Invalid bracket parameter for list");
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
		result = def_value;
		ycp2error ("Invalid bracket parameter for term");
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
	    ycp2error ("Bracket expression for '%s' does not evaluate to a list or a map.", result->toString ().c_str ());
	    result = def_value;
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
    m_def->toStream (str);
    return m_resultType->toStream (str);
}


// ------------------------------------------------------------------
// builtin function ref (-> declaration_t, type, parameters)

YEBuiltin::YEBuiltin (declaration_t *decl, YBlock *parameterblock, constTypePtr type)
    : YCode (yeBuiltin)
    , m_decl (decl)
    , m_type (type==0 ? Type::Function(Type::Unspec) : (FunctionTypePtr)(type->clone()))
    , m_parameterblock (parameterblock)
    , m_parameters (0)
    , m_last (0)
{
}


YEBuiltin::YEBuiltin (std::istream & str)
    : YCode (yeBuiltin)
    , m_parameterblock (0)
    , m_parameters (0)
    , m_last (0)
{
    m_type = FunctionTypePtr (Bytecode::readType (str));
    extern StaticDeclaration static_declarations;
    m_decl = static_declarations.readDeclaration (str);
    if (Bytecode::readBool (str))
    {
	m_parameterblock = (YBlock *)Bytecode::readCode (str);
	Bytecode::pushBlock (m_parameterblock);
    }
    Bytecode::readYCodelist (str, &m_parameters, &m_last);
    if (m_parameterblock != 0)
    {
	Bytecode::popBlock (m_parameterblock);
    }
    if (!m_decl
	|| m_type->isError ())
    {
	y2error ("Can't bind '%s'", StaticDeclaration::Decl2String (m_decl, true).c_str());
    }
}


std::ostream &
YEBuiltin::toStream (std::ostream & str) const
{
    YCode::toStream (str);
    extern StaticDeclaration static_declarations;

    Bytecode::writeType (str, m_type);
    static_declarations.writeDeclaration (str, m_decl);
    if (m_parameterblock == 0)
    {
	Bytecode::writeBool (str, false);
    }
    else
    {
	Bytecode::writeBool (str, true);
	m_parameterblock->toStream (str);
	Bytecode::pushBlock (m_parameterblock);
    }
    Bytecode::writeYCodelist (str, m_parameters);
    if (m_parameterblock != 0)
    {
	Bytecode::popBlock (m_parameterblock);
    }
    return str;
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


YBlock *
YEBuiltin::parameterBlock () const
{
    return m_parameterblock;
}


/**
 * 'close' function, perform final parameter check
 * if ok, return 0
 * if missing parameter, return its type
 * if undefined, return Type::Error (wrong type was already
 *   reported in attachParameter()
 */

constTypePtr
YEBuiltin::finalize ()
{
    extern StaticDeclaration static_declarations;

    y2debug ("YEBuiltin::finalize (%s)", StaticDeclaration::Decl2String (m_decl, true).c_str());

    // final type check for all parameters
    declaration_t *decl = static_declarations.findDeclaration (m_decl, m_type, false);
    if (decl == 0)
    {
	y2error ("YEBuiltin::finalize() FAILED");
	return Type::Error;
    }

    if (decl->flags & DECL_FLEX)
    {
	m_type = Type::determineFlexType (m_type, decl->type, Type::Unspec);
    }
    else
    {
	m_type = decl->type->clone();
    }
    m_decl = decl;
    y2debug ("YEBuiltin::finalize (%s : %s)", StaticDeclaration::Decl2String (m_decl, true).c_str(), m_type->toString().c_str());

    return 0;
}


constTypePtr
YEBuiltin::returnType () const
{
    constTypePtr ret = m_type->returnType ();
y2debug ("ret '%s' -> %s", m_type->toString().c_str(), ret->toString().c_str());

    if (!ret->isUnspec())
    {
	return ret;
    }

    // unfinished YEBuiltin (finalize() not yet called)

    constFunctionTypePtr ft = m_decl->type;
    if (!ft->isFunction())
    {
	return Type::Error;
    }
y2debug ("ret %s", ft->returnType()->toString().c_str());

    return ft->returnType();
}


    /**
     * Attach parameter to external function call
     * @param code: parameter code
     * @param type: parameter type
     * @return NULL if success,
     *    != NULL (expected type) if wrong parameter type
     *    Type::Unspec if bad code (NULL or isError)
     *    Type::Error if excessive parameter
     */
// if type == Unspec (default !), called from stream creation

constTypePtr
YEBuiltin::attachParameter (YCode *code, constTypePtr type)
{
    extern StaticDeclaration static_declarations;

    y2debug ("YEBuiltin::attachParameter (%s:%s)", code ? code->toString().c_str() : "<NULL>", type->toString().c_str());

    if ((code == 0)
	|| (code->isError()))
    {
	y2debug ("Bad code");
	return Type::Unspec;
    }

    declaration_t *decl = 0;

    if (!type->isUnspec ())
    {
	m_type->concat (type);
	y2debug ("YEBuiltin::attachParameter (%s:%s -> '%s')", type->toString().c_str(), code->toString().c_str(), m_type->toString().c_str());

	decl = static_declarations.findDeclaration (m_decl, m_type, true);
	if (decl == 0)
	{
	    // FIXME: check for propagation
	    static_declarations.findDeclaration (m_decl, m_type, false);	// force error output
	    // FIXME: return expected type
	    return Type::Unspec;
	}
    }
    else
    {
	y2debug ("type '%s' isUnspec", type->toString().c_str());
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

    return 0;
}


// attach symbolic variable parameter to function, return created TableEntry
//  for symbol parameters, if type is unspecified it's up to the declaration
//    (flags & DECL_SYMBOL) if a "...,`x,..." parameter gets converted to
//     a ycSymbol(x) or a ycEntry(any x)
// returns 0 if ok
//	Type::Error if duplicate parameter, Type::Unspec on error

constTypePtr
YEBuiltin::attachSymVariable (const char *name, constTypePtr type, unsigned int line, TableEntry *&tentry)
{
    extern StaticDeclaration static_declarations;

    FunctionTypePtr matchedType;
    constTypePtr addedType;

    y2debug ("YEBuiltin::attachSymVariable (%s:%s @%d, to %s:%s", name, type->toString().c_str(), line, m_decl->name, m_type->toString().c_str());

    if (type->isUnspec())							// no type given, might be symbol or untyped variable
    {
	// try with symbol constant first
	addedType = Type::Symbol;
	matchedType = m_type->clone();
	matchedType->concat (addedType);
	declaration_t *decl = static_declarations.findDeclaration (m_decl, matchedType, true);
	if (decl != 0)
	{
	    y2debug ("YEBuiltin::attachSymVariable() symbol constant matched");
	    return attachParameter (new YConst (YCode::ycSymbol, YCPSymbol (name)), addedType);
	}

	// no match, try with untyped variable
	type = Type::Any;

	extern ExecutionEnvironment ee;
	ycp2warning (ee.filename().c_str(), ee.linenumber(), "Parameter '%s' has unspecified type", name);
    }

    addedType = VariableTypePtr (new VariableType (type));	// it's a typed symbolic variable

    y2debug ("addedType %s", addedType->toString().c_str());

    tentry = m_parameterblock->newEntry (name, SymbolEntry::c_variable, type, line);
    if (tentry == 0)
    {
	return Type::Error;
    }

    return attachParameter (new YConst (ycEntry, YCPEntry (tentry->sentry())), addedType);
}


string
YEBuiltin::toString() const
{
    string s = StaticDeclaration::Decl2String (m_decl) + " (";

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
    y2debug ("YEBuiltin::evaluate [%s]", YCode::toString (kind()).c_str());

    if (cse)
    {
	return YCPNull();
    }

    // init parameters

    ycodelist_t *actualp = m_parameters;
    const int maxargs = 10;
    YCPValue args[maxargs] = { YCPNull(), YCPNull(), YCPNull(), YCPNull(), YCPNull(), YCPNull(), YCPNull(), YCPNull(), YCPNull(), YCPNull() };
    YCPList list;

    constFunctionTypePtr type = m_decl->type;
    int typepos = -1; // position of w

    // evaluate parameters

    int i = 0;

    while (i < maxargs)
    {
	if (actualp == 0)
	{
	    break;
	}

	y2debug ("actualp ([%d]%s)", actualp->code->kind(), actualp->code->toString().c_str());

	if (actualp->code->isBlock() || ( (m_decl->flags & DECL_NOEVAL) == DECL_NOEVAL))
	    // block as parameter to builtin function or builtin will eval on its own
	{
	    args[i] = YCPCode (actualp->code);	// pass as-is
	}
	else if (actualp->code->kind() == ycEntry)
	{
	    args[i] = ((YConst *)(actualp->code))->value();
	}
	else
	{
	    args[i] = actualp->code->evaluate ();
	}

	if ((args[i].isNull() || args[i]->isVoid())
	    && ((m_decl->flags & DECL_NIL) == 0))
	{
	    ycp2error ("Argument (%s) to %s(...) is nil", actualp->code->toString().c_str(), m_decl->name);
	    return YCPNull ();
	}

	if (args[i].isNull ())
	{
	    args[i] = YCPVoid ();
	}

	y2debug ("==> (%s)", args[i]->toString().c_str());

	if ((typepos < 0)					// not at wildcard yet
	    && type->parameterType(i)->isWildcard ())		// at '...' now ?
	{
	    typepos = i;
	    y2debug ("type '...' %d", i);
	}

	if (typepos >= 0)					// at or beyond '...'
	{
	    y2debug ("w! args[%d] = '%s'", i, args[i].isNull() ? "nil" : args[i]->toString().c_str());
	    list->add (args[i]);	// Y: add value to list
	}
	i++;
	actualp = actualp->next;
    }

    // error checking

    if (actualp != 0)
    {
	ycp2error ("More than %d arguments", maxargs);
	return YCPNull();
    }


    // wildcard checking

    if (typepos >= 0)
    {
	y2debug ("w! pos %d '%s'", i, list->toString().c_str());
	i = typepos+1;
	args[i-1] = list;
    }


    // call builtin function

    y2debug ("YEBuiltin::evaluate [%s (%d args)]", StaticDeclaration::Decl2String (m_decl, false).c_str(), i);
    y2debug ("parameter 1: %s", i > 0 ? args[0]->toString ().c_str (): "nil" );
    YCPValue ret = YCPNull();
    if (m_decl->ptr == 0)
    {
	return ret;
    }

    if (m_decl->name_space && ( m_decl->name_space->flags & DECL_CALL_HANDLER ) )
    {
	// The bultin belongs to a name space with a special call handler -
	// don't simply call the builtin function via m_decl->ptr(),
	// call the call handler and pass the builtin function pointer and
	// arguments to the call handler
	call_handler_t call_handler = (call_handler_t) m_decl->name_space->ptr;
	if (call_handler)
	{
	    return call_handler (m_decl->ptr, type->parameterCount (), args);
	}
	else
	{
	    y2error("YEBuiltin::evaluate [%s (%d args)]: Call handler declared, but not present",
		    StaticDeclaration::Decl2String (m_decl, false).c_str(), i);
	    return YCPNull();
	}
    }
    else
    {
	switch (type->parameterCount ())
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
	    case 5:
		ret = (*(v2vvvvv)m_decl->ptr) (args[0], args[1], args[2], args[3], args[4]);
	    break;
	    default:
	    {
		ycp2error ("Bad builtin: Arg count %d", i);
		ret = YCPNull ();
	    }
	    break;
	}
    }

#ifdef BUILTIN_STATISTICS
    if (!ret.isNull ())
    {
	FILE *fout = fopen ("/tmp/builtin-use.txt", "a");
	fprintf (fout, "%s %s\n", m_decl->name, m_decl->type->toString().c_str());
	fclose (fout);
    }
#endif

    y2debug ("YEBuiltin ret (%s)", ret.isNull() ? "NULL" : ret->toString().c_str());

    return ret;
}


constTypePtr
YEBuiltin::type () const
{
    constTypePtr return_type = returnType();
    if (return_type->isError())
    {
	y2debug ("YEBuiltin::type() error !");
	return return_type;
    }
    FunctionTypePtr f = Type::Function(return_type)->clone();
    ycodelist_t *parm = m_parameters;
    while (parm)
    {
	f->concat(parm->code->type());
	parm = parm->next;
    }
    return f;
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
    ycodelist_t *last = 0;
    Bytecode::readYCodelist (str, &m_parameters, &last);
    y2debug ("YEFunction (fromStream): %s", toString().c_str());
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


    /**
     * Attach parameter to external function call
     * @param code: parameter code
     * @param type: parameter type
     * @return NULL if success,
     *    != NULL (expected type) if wrong parameter type
     *    Type::Unspec if bad code (NULL or isError)
     *    Type::Error if excessive parameter
     */

constTypePtr
YEFunction::attachParameter (YCode *code, constTypePtr type)
{
    y2debug ("YEFunction::attachParameter (%s:%s)", code ? code->toString().c_str() : "(NULL)", type->toString().c_str());

    if (code == 0 || code->isError())
    {
	return Type::Unspec;
    }

    // retrieve function type for formal parameter list
    constFunctionTypePtr ftype = m_entry->type();

    y2debug ("ftype (%s)", ftype->toString().c_str());

    // count the actual parameters (so far) for checking against func->parameterCount()
    int actual_count = 0;
    ycodelist_t *actual = m_parameters;

    constTypePtr expected_type = Type::Unspec;

    ycodelist_t *plast_actual = actual;

    // move to the end of actual parameter list,
    //   the parameter will be added to the end

    while (actual != 0)
    {
	actual_count++;
	plast_actual = actual;
	actual = actual->next;
    }

    // too many parameters? ('>=' since we're counting from 0..n-1)
    if (actual_count >= ftype->parameterCount())
    {
	return Type::Error;
    }

    // ok, check whether types match

    expected_type = ftype->parameterType (actual_count);
    y2debug ("checking parameter type: expected '%s', given '%s'", expected_type->toString().c_str(), type->toString().c_str());

    int match = type->match (expected_type);
    if (match < 0)
    {
	y2debug ("type mismatch");
	// type mismatch
	return expected_type;
    }

    if (expected_type->isReference())	// function expects a reference
    {
	if (!code->isReferenceable())	// but the actual parameter isn't referenceable
	{
	    y2error ("Can't take reference of '%s'", code->toString().c_str());
	    return expected_type;
	}
	else if (match > 0)
	{
	    y2error ("Can't reference to type propagation '%s' -> '%s'", type->toString().c_str(), expected_type->toString().c_str());
	    return expected_type;
	}
	else
	{
	    YEVariable *var = (YEVariable *)code;
	    code = new YEReference (var->entry());
	}
    }

    if (match > 0)				// propagation possible
    {
	y2debug ("type propagation");
	code = new YEPropagate (code, type, expected_type);
    }

    y2debug ("add parameter to end of actual list");

    // allocate new function parameter element
    ycodelist_t *element = new ycodelist_t;

    element->code = code;
    element->next = 0;

    if (plast_actual != 0)		// we already have a last actual parameter
    {
	plast_actual->next = element;
    }
    else				// this is the first parameter
    {
	m_parameters = element;
    }
    y2debug ("done");
    return 0;
}


/**
 * 'close' function, perform final parameter check
 * if ok, return 0
 * if missing parameter, return its type
 * if undefined, return Type::Error (wrong type was already
 *   reported in attachParameter()
 */

constTypePtr
YEFunction::finalize()
{
    // count the actual parameters (so far) for checking against func->parameterCount()
    int actual_count = 0;
    ycodelist_t *actual = m_parameters;

    // count actual parameter list,

    while (actual != 0)
    {
	actual_count++;
	actual = actual->next;
    }

    // retrieve function type for formal parameter list
    constFunctionTypePtr ftype = m_entry->type();

    // a parameter missing ?
    if (actual_count < ftype->parameterCount())
    {
	y2debug ("Missing expected_type '%s'", ftype->parameterType (actual_count)->toString().c_str());
	return ftype->parameterType (actual_count);
    }
    y2debug ("parameter count matches");

    return 0;
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
    y2debug ("func kind ([%d] %s)\n", func->kind(), func->toString().c_str());

    // check for function or function pointer
    if (func->kind() != YCode::ycFunction)			// must be pointer
    {
	// It's a YEVariable

	YCPValue value = m_entry->value ();
	if (value.isNull())
	    y2debug ("m_entry value NULL");
	else
	    y2debug ("m_entry value ([%d] %s)\n", m_entry->value()->valuetype(), m_entry->value()->toString().c_str());
	if (!value->isCode())
	{
	    ycp2error ("Not a function pointer (%s)", m_entry->toString().c_str());
	    return YCPNull();
	}

	func = (YFunction *)(value->asCode()->code());
	y2debug ("func kind ([%d] %s)\n", func->kind(), func->toString().c_str());
    }

    // push parameter values for recursion
    for (unsigned int p = 0; p < func->parameterCount(); p++)
    {
	func->parameter (p)->push ();
    }
    // push also local parameters
    YBlock *definition = func->definition ();
    definition->push_to_stack ();

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
	    ycp2error ("Parameter eval failed (%s)", actualp->code->toString().c_str());

	    // cleanup: pop parameter values for recursion
	    for (unsigned int p = 0; p < func->parameterCount(); p++)
	    {
		func->parameter (p)->pop ();
	    }

	    return value;
	}

	SymbolEntry *formalp = func->parameter (p);
	y2debug ("formalp (%s) = (%s)", formalp->toString().c_str(), value->toString().c_str());

	formalp->setValue (value);
	actualp = actualp->next;
    }

    YCPValue value = definition->evaluate ();

    // pop also local parameters
    definition->pop_from_stack ();

    // pop parameter values for recursion
    for (unsigned int p = 0; p < func->parameterCount(); p++)
    {
	func->parameter (p)->pop ();
    }

    y2debug("evaluate done (%s) = '%s'", definition->toString().c_str(), value.isNull() ? "NULL" : value->toString().c_str());
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


constTypePtr
YEFunction::type () const
{
    return m_entry->type ();
}


constTypePtr
YEFunction::wantedParameterType () const
{

    YFunction *func_f = dynamic_cast<YFunction *> (m_entry->code ());

    // find out number of already done parameters
    ycodelist_t *actual = m_parameters;
    int actual_count = 0;

    // move to the end of actual parameter list,
    //   the parameter will be added to the end

    while (actual != 0)
    {
        actual_count++;
        actual = actual->next;
    }

    // returns NULL if actual_count is out of bounds
    SymbolEntry *param_se = func_f->parameter (actual_count);

    // this is for value conversion purposes. if this is an excess
    // parameter, we don't care about the type now since
    // attachParameter will complain anyway
    constTypePtr param_tp = param_se ? param_se->type () : Type::Any;
    return param_tp;
}

bool YEFunction::attachParameter (const YCPValue& arg, int pos)
{
    y2error ("Attaching constant parameter not possible at the given position yet");
    return false;
}

bool YEFunction::appendParameter (const YCPValue& arg)
{
    // evil hack - pretend that the actual type is the expected type
    // FIXME
    constTypePtr param_tp = wantedParameterType ();

    // Such YConsts without a specific type produce invalid
    // bytecode. (Which is OK if only Y2Function::evaluate is used)
    // The actual parameter's YCode becomes owned by the function call?
    YConst *param_c = new YConst (YCode::ycConstant, arg);

    // Attach the parameter
    // Returns NULL if OK, Type::Error if excessive argument
    // Other errors (bad code, bad type) shouldn't happen
    constTypePtr err_tp = attachParameter (param_c, param_tp);
    if (err_tp != NULL)
    {
	if (err_tp->isError ())
	{
	    // Our caller should report the place
	    // in a script where this happened
	    y2error ("Excessive parameter to %s",
		     qualifiedName ().c_str ());
	}
	else
	{
	    y2internal ("attachParameter returned %s",
			err_tp->toString ().c_str ());
	}
	return false;
    }
    return true;
}

bool YEFunction::finishParameters ()
{
    // now must check if we got fewer parameters than needed
    constTypePtr err_tp = finalize ();
    if (err_tp != NULL)
    {
	y2error ("Missing %s parameter to %s",
		 err_tp->toString ().c_str (), qualifiedName ().c_str ());
	return false;
    }
    return true;
}

string YEFunction::qualifiedName () const
{
    string n = m_entry->block () ?
	(m_entry->block ()->name () + string ("::")) :
	"";
    return n + m_entry->name ();
}

// EOF
