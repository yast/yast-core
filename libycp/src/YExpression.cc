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

/-*/
// -*- c++ -*-

#include <libintl.h>	// for dngettext

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
#include "ycp/SymbolTable.h"

#include "ycp/Bytecode.h"
#include "ycp/Xmlcode.h"

#ifndef DO_DEBUG
#define DO_DEBUG 0
#endif

typedef YCPValue (*v2) ();
typedef YCPValue (*v2v) (const YCPValue &);
typedef YCPValue (*v2vv) (const YCPValue &, const YCPValue &);
typedef YCPValue (*v2vvv) (const YCPValue &, const YCPValue &, const YCPValue &);
typedef YCPValue (*v2vvvv) (const YCPValue &, const YCPValue &, const YCPValue &, const YCPValue &);
typedef YCPValue (*v2vvvvv) (const YCPValue &, const YCPValue &, const YCPValue &, const YCPValue &, const YCPValue &);

// ------------------------------------------------------------------

IMPL_DERIVED_POINTER(YEVariable, YCode);
IMPL_DERIVED_POINTER(YEReference, YCode);
IMPL_DERIVED_POINTER(YETerm, YCode);
IMPL_DERIVED_POINTER(YECompare, YCode);
IMPL_DERIVED_POINTER(YELocale, YCode);
IMPL_DERIVED_POINTER(YEList, YCode);
IMPL_DERIVED_POINTER(YEMap, YCode);
IMPL_DERIVED_POINTER(YEPropagate, YCode);
IMPL_DERIVED_POINTER(YEUnary, YCode);
IMPL_DERIVED_POINTER(YEBinary, YCode);
IMPL_DERIVED_POINTER(YETriple, YCode);
IMPL_DERIVED_POINTER(YEIs, YCode);
IMPL_DERIVED_POINTER(YEReturn, YCode);
IMPL_DERIVED_POINTER(YEBracket, YCode);
IMPL_DERIVED_POINTER(YEBuiltin, YCode);
IMPL_DERIVED_POINTER(YECall, YCode);
IMPL_DERIVED_POINTER(YEFunction, YECall);
IMPL_DERIVED_POINTER(YEFunctionPointer, YECall);

// ------------------------------------------------------------------
// variable ref (-> SymbolEntry)

YEVariable::YEVariable (SymbolEntryPtr entry)
    : YCode ()
    , m_entry (entry)
{
}


YEVariable::YEVariable (bytecodeistream & str)
    : YCode ()
{
    m_entry = Bytecode::readEntry (str);
}


SymbolEntryPtr
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
	    ycp2error ("YEVariable::evaluate (%s) is not initialized", toString().c_str());
	    value = YCPVoid ();
	}
	else
	{
	    value = YCPReference (m_entry);
	}
    }
#if DO_DEBUG
    y2debug ("YEVariable::evaluate (%s) = %s", toString().c_str(), value.isNull() ? "NULL" : value->toString().c_str());
#endif
    return value;
}


std::ostream &
YEVariable::toStream (std::ostream & str) const
{
    YCode::toStream (str);
    Bytecode::writeEntry (str, m_entry);
    return str;
}

std::ostream &
YEVariable::toXml( std::ostream & str, int /*indent*/ ) const
{
    str << "<variable name=\"";
    str << m_entry->name() << "\"";
    commentToXml(str);
    string ns = m_entry->nameSpace()->name();
    if (!ns.empty())
      str << " ns=\"" << ns << "\"";
    str << " category=\"" << m_entry->catString();
    str << "\" type=\"" << m_entry->type()->toXmlString();

    return str << "\"/>";
}


// ------------------------------------------------------------------
// reference (-> SymbolEntry)

YEReference::YEReference (SymbolEntryPtr entry)
    : YCode ()
    , m_entry (entry)
{
#if DO_DEBUG
    y2debug ("YEReference::YEReference (%s)", entry->toString().c_str());
#endif
}


YEReference::YEReference (bytecodeistream & str)
    : YCode ()
{
    m_entry = Bytecode::readEntry (str);
}


SymbolEntryPtr 
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
#if DO_DEBUG
    y2debug ("YEReference::evaluate (%s) = %s", toString().c_str(), value.isNull() ? "NULL" : value->toString().c_str());
#endif
    return value;
}


std::ostream &
YEReference::toStream (std::ostream & str) const
{
    YCode::toStream (str);
    Bytecode::writeEntry (str, m_entry);
    return str;
}


std::ostream &
YEReference::toXml (std::ostream & str, int /*indent*/ ) const
{
    str << "<yereference";
    commentToXml(str);
    str << ">";
    Xmlcode::writeEntry (str, m_entry);
    return str << "</yereference>";
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
    : YCode ()
    , m_name (name)
    , m_parameters (0)
{
}


YETerm::YETerm (bytecodeistream & str)
    : YCode ()
    , m_parameters (0)
{
    m_name = Bytecode::readCharp (str);
    if (m_name)
    {
	if (!Bytecode::readYCodelist (str, &m_parameters))
	{
	    delete [] m_name;
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
	delete parm;
	parm = next;
    }
    delete [] m_name;
}


const char *
YETerm::name () const
{
    return m_name;
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
YETerm::attachParameter (YCodePtr code, constTypePtr /*dummy*/)
{
    if ((code == 0)
	|| (code->isError()))
    {
#if DO_DEBUG
	y2debug ("YETerm::attachParameter (Error)");
#endif
	return Type::Unspec;
    }

#if DO_DEBUG
    y2debug ("YETerm::attachParameter (%s)", code->toString().c_str());
#endif

    ycodelist_t *element = new ycodelist_t;
    element->code = code;
    element->next = 0;

    if (m_parameters == 0)
    {
	m_parameters = element;
    }
    else
    {
	ycodelist_t *ptr = m_parameters;
	while (ptr->next) ptr = ptr->next;
	ptr->next = element;
    }

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
	    ycp2error( "parameter without code");
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

#if DO_DEBUG
    y2debug ("YETerm::evaluate (%s) = '%s'", toString().c_str(), term->toString().c_str());
#endif

    return term;
}


std::ostream &
YETerm::toStream (std::ostream & str) const
{
    YCode::toStream (str);
    Bytecode::writeCharp (str, m_name);
    return Bytecode::writeYCodelist (str, m_parameters);
}

std::ostream &
YETerm::toXml (std::ostream & str, int /*indent*/ ) const
{
    u_int32_t count = 0;
    const ycodelist_t *codep = m_parameters;

    while( codep )
    {
	count++;
	codep = codep->next;
    }
    str << "<yeterm name=\"" << m_name << "\" args=\"" << count << "\"";
    commentToXml(str);
    str << ">";
    Xmlcode::writeYCodelist (str, m_parameters);
    return str << "</yeterm>";
}



// ------------------------------------------------------------------
// Compare (-> left, right, type)

YECompare::YECompare (YCodePtr left, c_op op, YCodePtr right)
    : YCode ()
    , m_left (left)
    , m_op (op)
    , m_right (right)
{
}


YECompare::YECompare (bytecodeistream & str)
    : YCode ()
{
    m_left = Bytecode::readCode (str);
    char c;
    str.get (c);
    m_op = (c_op) c;
    m_right = Bytecode::readCode (str);
}


YECompare::~YECompare ()
{
}

static string
compare_op_string( YECompare::c_op op )
{
    switch (op)
    {
	case YECompare::C_EQ:  return "=="; break;
	case YECompare::C_NEQ: return "!="; break;
	case YECompare::C_LT:  return "<";  break;
	case YECompare::C_GE:  return ">="; break;
	case YECompare::C_LE:  return "<="; break;
	case YECompare::C_GT:  return ">";  break;
	default:
		break;
    }
    return "?compare?";
}


string
YECompare::toString () const
{
    string s = "(" + m_left->toString();
    s += " ";
    s += compare_op_string( m_op );
    s += " ";
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
#if DO_DEBUG
    y2debug ("YECompare::evaluate (%s, '%d', %s)", vl.isNull() ? "NULL" : vl->toString().c_str(), m_op, vr.isNull() ? "NULL" : vr->toString().c_str());
#endif

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
    ycp2error ("YECompare unknown type");
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


std::ostream &
YECompare::toXml (std::ostream & str, int /*indent*/ ) const
{
    str << "<compare op=\"" << Xmlcode::xmlify( compare_op_string( m_op ) ) << "\"";
    commentToXml(str);
    str << ">";
    str << "<lhs>"; m_left->toXml( str, 0 ); str << "</lhs>";
    str << "<rhs>"; m_right->toXml( str, 0 ); str << "</rhs>";
    return str << "</compare>";
}


// ------------------------------------------------------------------
// locale expression (-> singular, plural, count)

YELocale::YELocale (const char *singular, const char *plural, YCodePtr count, const char *textdomain)
    : YCode ()
    , m_singular (singular)
    , m_plural (plural)
    , m_count (count)
{
    if (YLocale::domains.find (textdomain) == YLocale::domains.end ())
    {
	YLocale::domains.insert (std::make_pair(textdomain,false));
    }
    m_domain = YLocale::domains.find (textdomain);
}


YELocale::YELocale (bytecodeistream & str)
    : YCode ()
{
    m_singular = Bytecode::readCharp (str);		// text for singular
    m_plural = Bytecode::readCharp (str);		// text for plural
    m_count = Bytecode::readCode (str);
    const char * dom = Bytecode::readCharp (str);

    if (YLocale::domains.find (dom) == YLocale::domains.end ())
    {
	YLocale::domains.insert (std::make_pair(dom,false));
	m_domain = YLocale::domains.find (dom);
    }
    else
    // the textdomain was already there, we can free the memory allocated in readCharp
    {
	m_domain = YLocale::domains.find (dom);
        delete[] dom;
    }

}


YELocale::~YELocale ()
{
    delete[] m_singular;
    delete[] m_plural;
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
#if DO_DEBUG
    y2debug ("YELocale::evaluate(%s)\n", toString().c_str());
#endif

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

    const char *ret = dngettext (m_domain->first, m_singular, m_plural, count->asInteger()->value());

#if DO_DEBUG
    y2debug ("localize <%s, %s, %d> to <%s>", m_singular, m_plural, (int)(count->asInteger()->value()), ret);
#endif

    return YCPString (ret);
}


std::ostream &
YELocale::toStream (std::ostream & str) const
{
    YCode::toStream (str);
    Bytecode::writeCharp (str, m_singular);
    Bytecode::writeCharp (str, m_plural);
    m_count->toStream (str);
    return Bytecode::writeCharp (str, m_domain->first);
}


// see also YLocale::toXml
std::ostream &
YELocale::toXml (std::ostream & str, int /*indent*/ ) const
{
    str << "<locale domain=\"" << m_domain->first << "\" text=\"" << Xmlcode::xmlify( m_singular )
	<< "\" plural=\"" << Xmlcode::xmlify( m_plural ) << "\"";
    commentToXml(str);
    str << ">";
    m_count->toXml( str, 0 );
    str << "</locale>";
    return str;
}


// ------------------------------------------------------------------
// list expression (-> value, next list value)

YEList::YEList (YCodePtr code)
    : YCode ()
{
    m_first = new ycodelist_t;
    m_first->code = code;
    m_first->next = 0;
}


YEList::YEList (bytecodeistream & str)
    : YCode ()
    , m_first (0)
{
    Bytecode::readYCodelist (str, &m_first);
}


YEList::~YEList ()
{
    ycodelist_t *element = m_first;
    ycodelist_t *next;
    while (element)
    {
	next = element->next;
	delete element;
	element = next;
    }
}


void
YEList::attach (YCodePtr code)
{
    ycodelist_t *element = new ycodelist_t;
    element->code = code;
    element->next = 0;
    if (m_first == 0)
    {
	m_first = element;
    }
    else
    {
	ycodelist_t *ptr = m_first;
	while (ptr->next) ptr = ptr->next;
	ptr->next = element;
    }
}


string
YEList::toString() const
{
    ycodelist_t *element = m_first;
    string s = "[";
    while (element)
    {
	if (element != m_first)
	{
	    s += ", ";
	}
	s += element->code->toString();
	element = element->next;
    }
    return s + "]";
}


YCPValue
YEList::evaluate (bool cse)
{
#if DO_DEBUG
    y2debug ("YEList::evaluate(%s)\n", toString().c_str());
#endif

    YCPList list;
    ycodelist_t *element = m_first;

    while (element)
    {
	YCPValue value = element->code->evaluate (cse);

	if (value.isNull())
        {
	    if (cse) {
	        return value;   // expression is not a constant
	    }
	    else {
	        value = YCPVoid();
	    }
	}
	list->add (value);
	element = element->next;
    }
    return list;
}


int
YEList::count () const
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


YCodePtr
YEList::value (int index) const
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


std::ostream &
YEList::toXml (std::ostream & str, int /*indent*/ ) const
{
    u_int32_t count = 0;
    const ycodelist_t *codep = m_first;

    while( codep )
    {
	count++;
	codep = codep->next;
    }
    str << "<list size=\"" << count << "\"";
    commentToXml(str);
    str << ">";
    Xmlcode::writeYCodelist( str, m_first );
    return str << "</list>";
}


constTypePtr
YEList::type () const
{
    ycodelist_t *element = m_first;
    
    constTypePtr res = m_first->code->type ();
    element = element->next;

    while (element)
    {
	res = res->commontype (element->code->type ());
	element = element->next;
    }
    
    return new ListType (res);
}


// ------------------------------------------------------------------
// map expression (-> key, value, next key/value pair)

YEMap::YEMap (YCodePtr key, YCodePtr value)
    : YCode ()
    , m_first (0)
{
    attach (key, value);
}


YEMap::YEMap (bytecodeistream & str)
    : YCode ()
    , m_first (0)
{
    u_int32_t count = Bytecode::readInt32 (str);
    while (count-- > 0)
    {
	YCodePtr key = Bytecode::readCode (str);
	YCodePtr value = Bytecode::readCode (str);
	attach (key, value);
    }
}


YEMap::~YEMap ()
{
    mapval_t *element = m_first;
    mapval_t *next;
    while (element)
    {
	next = element->next;
	delete element;
	element = next;
    }
}


void
YEMap::attach (YCodePtr key, YCodePtr value)
{
    mapval_t *element = new mapval_t;
    element->key = key;
    element->value = value;
    element->next = 0;

    if (m_first == 0)
    {
	m_first = element;
    }
    else
    {
	mapval_t *ptr = m_first;
	while (ptr->next) ptr = ptr->next;
	ptr->next = element;
    }
}


string
YEMap::toString() const
{
#if DO_DEBUG
    y2debug ("YEMap::toString()");
#endif
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
#if DO_DEBUG
    y2debug ("YEMap::evaluate (%s)\n", toString().c_str());
#endif
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


std::ostream &
YEMap::toXml( std::ostream & str, int /*indent*/ ) const
{
    mapval_t *mapp = m_first;
    u_int32_t count = 0;
    while( mapp )
    {
	count++;
	mapp = mapp->next;
    }
    str << "<map size=\"" << count << "\"";
    commentToXml(str);
    str << ">";

    mapp = m_first;
    while (mapp)
    {
	str << "<element>";
	str << "<key>"; mapp->key->toXml( str, 0 ); str << "</key>";
	str << "<value>"; mapp->value->toXml( str, 0 ); str << "</value>";
	str << "</element>";
	mapp = mapp->next;
    }
    return str << "</map>";
}


constTypePtr
YEMap::type () const
{

    mapval_t *element = m_first;

    constTypePtr res_key = m_first->key->type ();
    constTypePtr res_value = m_first->value->type ();
    element = element->next;

    while (element)
    {
	res_key = res_key->commontype (element->key->type ());
	res_value = res_value->commontype (element->value->type ());
	element = element->next;
    }
    
    return new MapType (res_key, res_value);
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

YEPropagate::YEPropagate (YCodePtr value, constTypePtr from, constTypePtr to)
    : YCode ()
    , m_from (from)
    , m_to (to)
    , m_value (value)
{
    //FIXME: save declaration/ptr to propagation function instead of from & to
    if (m_from->isFloat())
    {
	ycp2warning(YaST::ee.filename().c_str(), YaST::ee.linenumber(), "Implicit float conversion will loose accuracy");
    }
}


YEPropagate::YEPropagate (bytecodeistream & str)
    : YCode ()
    , m_from (Bytecode::readType (str))
    , m_to (Bytecode::readType (str))
{
    m_value = Bytecode::readCode (str);
}


YEPropagate::~YEPropagate ()
{
}


string
YEPropagate::toString() const
{
    return string ("/* ") + m_from->toString().c_str() + " -> " + m_to->toString().c_str() + " */" + m_value->toString();
}


bool
YEPropagate::canPropagate(const YCPValue& value, constTypePtr to_type) const
{
    if (value.isNull()
	|| value->isVoid()					// value is nil, this is allowed everywhere
	|| to_type->isAny ()					// casting to any
	|| to_type->isUnspec ()					// casting to unspec
	|| ( to_type->isBasetype ()				// casting to equivalent base type
	     && value->valuetype () == to_type->valueType ()))
    {
	return true;						// this is all ok
    }

#if DO_DEBUG
    y2debug ("to type: %s", to_type->toString ().c_str () );
#endif
    if (to_type->isList ()					// casting to a list
	&& value->isList ())
    {
	// check types of all elements
	constTypePtr elem = ((constListTypePtr)to_type)->type ();
	if (elem->isAny ()) return true;
	if (elem->isUnspec ()) return true;			// untyped list

	YCPList v = value->asList ();

	for (int i=0; i < v->size (); i++ )
	{
#if DO_DEBUG
	    y2debug ("testing %s", v->value (i)->toString ().c_str ());
#endif
	    if (! canPropagate (v->value (i), elem) )
	    {
		return false;
	    }
	}
	return true;
    }

    if (to_type->isMap ()					// casting to a map
	&& value->isMap ())
    {
	// check types of all elements
	constTypePtr key = ((constMapTypePtr)to_type)->keytype ();
	constTypePtr elem = ((constMapTypePtr)to_type)->valuetype ();

	if (elem->isAny () && key->isAny ()) return true;	// map<any,any>

	// not typed maps
	if (elem->isUnspec () && key->isUnspec ()) return true;

	YCPMap map = value->asMap ();

	for (YCPMap::const_iterator pos = map->begin(); pos != map->end(); ++pos)
	{
	    if (! canPropagate (pos->first, key) )
	    {
		return false;
	    }
	    if (! canPropagate (pos->second, elem) )
	    {
		return false;
	    }
	}
	return true;
    }

    if (to_type->isFunction ()
	&& value->isCode ())
    {
	YCodePtr c = value->asCode ()->code ();
	constFunctionTypePtr t = (constFunctionTypePtr)to_type;
	return c->kind () == ycFunction;
    }

    return false;
}


YCPValue
YEPropagate::evaluate (bool cse)
{
#if DO_DEBUG
    y2debug ("YEPropagate::evaluate(%s)\n", toString().c_str());
#endif
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
    else if (v->isReference () 
	&& m_to->match (v->asReference ()->entry ()->type ()) == 0)
    {
	return v;
    }
    else if (canPropagate (v, m_to))
    {
	return v;
    }

    ycp2error ("Can't convert value '%s' to type '%s'", v->toString().c_str(), m_to->toString().c_str());

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

std::ostream &
YEPropagate::toXml (std::ostream & str, int /*indent*/ ) const
{
    str << "<yepropagate from=\"" << Xmlcode::xmlify( m_from->toString() ) << "\" to=\"" << Xmlcode::xmlify( m_to->toString() ) << "\"";
    commentToXml(str);
    str << ">";
    m_value->toXml( str, 0 );
    return str << "</yepropagate>";
}



// ------------------------------------------------------------------
// unary expression (-> declaration_t, arg)

YEUnary::YEUnary (declaration_t *decl, YCodePtr arg)
    : YCode ()
    , m_decl (decl)
    , m_arg (arg)
{
}


YEUnary::YEUnary (bytecodeistream & str)
    : YCode ()
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
#if DO_DEBUG
    y2debug ("YEUnary::evaluate(%s)\n", toString().c_str());
#endif
    if (cse)
    {
	return YCPNull();
    }

    YCPValue arg = m_arg->evaluate ();
    const declaration_t *decl = m_decl;

    if (arg.isNull()
	&& ((decl->flags & DECL_NIL) == 0))
    {
	ycp2error ("Argument (%s) to %s(...) is nil", m_arg->toString().c_str(), m_decl->name);
	return YCPNull ();
    }

#if DO_DEBUG
    y2debug ("func %s (%s)", decl->name, decl->type->toString().c_str());
#endif

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


std::ostream &
YEUnary::toXml (std::ostream & str, int /*indent*/ ) const
{
    str << "<yeunary";
    commentToXml(str);
    extern StaticDeclaration static_declarations;
    static_declarations.writeXmlDeclaration( str, m_decl );
    str << ">";
    m_arg->toXml( str, 0 );
    return str << "</yeunary>";
}


// ------------------------------------------------------------------
// binary expression (-> declaration_t, arg1, arg2)

YEBinary::YEBinary (declaration_t *decl, YCodePtr arg1, YCodePtr arg2)
    : YCode ()
    , m_decl (decl)
    , m_arg1 (arg1)
    , m_arg2 (arg2)
{
}


YEBinary::YEBinary (bytecodeistream & str)
    : YCode ()
{
    extern StaticDeclaration static_declarations;

    m_decl = static_declarations.readDeclaration (str);
    if (m_decl)
    {
	m_arg1 = Bytecode::readCode (str);
	m_arg2 = Bytecode::readCode (str);
    }
}


YEBinary::~YEBinary ()
{
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

#if DO_DEBUG
    y2debug ("YEBinary::evaluate(%s)\n", toString().c_str());
#endif

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
#if DO_DEBUG
    y2debug ("func %s (%s) [%s,%s]", decl->name, decl->type->toString().c_str(), arg1->toString().c_str(), arg2->toString().c_str());
    y2debug ("type1 %s, type2 %s", arg1->valuetype_str(), arg2->valuetype_str());
#endif

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


std::ostream &
YEBinary::toXml (std::ostream & str, int /*indent*/ ) const
{
    str << "<yebinary";
    commentToXml(str);
    extern StaticDeclaration static_declarations;
    static_declarations.writeXmlDeclaration (str, m_decl);
    str << ">";
    m_arg1->toXml( str, 0 );
    m_arg2->toXml( str, 0 );
    return str << "</yebinary>";
}


constTypePtr
YEBinary::type () const
{
    if (m_decl->flags && DECL_FLEX)
    {
	// reconstruct type
	FunctionTypePtr ft = new FunctionType (Type::Unspec);
	ft->concat (m_arg1->type ());
	ft->concat (m_arg2->type ());
	
	constTypePtr cft = Type::determineFlexType (ft, m_decl->type);
        if (cft == 0)                                       // failed
        {
	    y2internal ("Cannot determine type of the binary operator: %s", toString ().c_str ());
            return Type::Unspec;
        }
	
	return ((constFunctionTypePtr)cft)->returnType ();

    }
    else
    {
	return ((constFunctionTypePtr)m_decl->type)->returnType (); 
    }
}

// ------------------------------------------------------------------
// Triple (? :) expression (-> bool expr, true value, false value)

YETriple::YETriple (YCodePtr a_expr, YCodePtr a_true, YCodePtr a_false)
    : YCode ()
    , m_expr (a_expr)
    , m_true (a_true)
    , m_false (a_false)
{
}


YETriple::YETriple (bytecodeistream & str)
    : YCode ()
{
    m_expr = Bytecode::readCode (str);
    m_true = Bytecode::readCode (str);
    m_false = Bytecode::readCode (str);
}


YETriple::~YETriple ()
{
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
#if DO_DEBUG
    y2debug ("YETriple::evaluate(%s)\n", toString().c_str());
#endif
    if (cse)
    {
	return YCPNull();
    }

    YCPValue expr = m_expr->evaluate ();

    if (expr.isNull () || expr->isVoid())
    {
	ycp2warning(YaST::ee.filename().c_str(), YaST::ee.linenumber(), "Condition expression evaluates to nil in ?: expression, using false instead.");
	return m_false->evaluate ();
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


std::ostream &
YETriple::toXml (std::ostream & str, int /*indent*/ ) const
{
    str << "<yetriple";
    commentToXml(str);
    str << ">";
    str << "<cond>"; m_expr->toXml( str, 0); str << "</cond>";
    str << "<true>"; m_true->toXml( str, 0 ); str << "</true>";
    str << "<false>"; m_false->toXml( str, 0); str << "</false>";
    return str << "</yetriple>";
}


// ------------------------------------------------------------------
// is (expression, type)

// it is documented like a *builtin* in YCPBuiltinMisc.cc !

YEIs::YEIs (YCodePtr expr, constTypePtr type)
    : YCode ()
    , m_expr (expr)
    , m_type (type)
{
}


YEIs::YEIs (bytecodeistream & str)
    : YCode ()
    , m_type (Bytecode::readType (str))
{
    m_expr = Bytecode::readCode (str);
}


YEIs::~YEIs ()
{
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
#if DO_DEBUG
    y2debug ("YEIs::evaluate (%s)%s", toString().c_str(), cse ? "<cse>" : "");
#endif

    YCPValue value = m_expr->evaluate (cse);		// evaluate the value

    if (value.isNull())
    {
	if (!cse)					// thats an error at runtime
	{
	    ycp2error ("'is()' expression evaluates to nil.");
	}
	return value;
    }

    constTypePtr value_type;				// now determine the values type

    if (value->isCode())				// value is YCode
    {
	YCodePtr code = value->asCode()->code();
	
	// is it a function? => function pointer
	if (code->kind () != YCode::ycFunction)
	{
	    // assume the code was passed via doublequotes
	    value_type = new BlockType (code->type ());
	}
	else
	{
	    value_type = code->type ();
	}
    }
    else if (value->isReference ())			// value is Reference
    {
	value_type = value->asReference ()->entry ()->type ();
    }
    else						// value is constant (YCPValue)
    {
        return YCPBoolean (m_type->matchvalue (value) >= 0);
    }

    // allow full or propagated match

    return YCPBoolean (value_type->match (m_type) >= 0);
}


std::ostream &
YEIs::toStream (std::ostream & str) const
{
    YCode::toStream (str);
    m_type->toStream (str);
    return m_expr->toStream (str);
}


std::ostream &
YEIs::toXml (std::ostream & str, int /*indent*/ ) const
{
    str << "<yeis "; m_type->toXml( str, 0); str;
    commentToXml(str);
    str << ">";
    str << "<expr>"; m_expr->toXml( str, 0); str << "</expr>";
    return str << "</yeis>";
}


// ------------------------------------------------------------------
// Return (expression)

YEReturn::YEReturn (YCodePtr expr)
    : YCode ()
    , m_expr (expr)
{
}


YEReturn::YEReturn (bytecodeistream & str)
    : YCode ()
{
    m_expr = Bytecode::readCode (str);
}


YEReturn::~YEReturn ()
{
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
YEReturn::evaluate (bool /*cse*/)
{
    return YCPCode (m_expr);
}


std::ostream &
YEReturn::toStream (std::ostream & str) const
{
    YCode::toStream (str);
    return m_expr->toStream (str);
}

std::ostream &
YEReturn::toXml (std::ostream & str, int /*indent*/ ) const
{
    str << "<yereturn";
    commentToXml(str);
    str << ">";
    m_expr->toXml( str, 0 );
    return str << "</yereturn>";
}


// ------------------------------------------------------------------
// bracket expression: identifier [ arg, arg, ...] : default

YEBracket::YEBracket (YCodePtr var, YCodePtr arg, YCodePtr def, constTypePtr resultType)
    : YCode ()
    , m_var (var)
    , m_arg (arg)
    , m_def (def)
    , m_resultType (resultType)
{
}


YEBracket::YEBracket (bytecodeistream & str)
    : YCode ()
{
    m_var = Bytecode::readCode (str);
    m_arg = Bytecode::readCode (str);
    m_def = Bytecode::readCode (str);
    // throw away the type info
    Bytecode::readType (str);
    m_resultType = Type::Void;
}


YEBracket::~YEBracket ()
{
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

    // parse time?
    if (cse
	&& var_value.isNull ())
    {
	return YCPNull ();
    }

    if (var_value.isNull()
	|| var_value->isVoid())
    {
	return m_def->evaluate (cse);
    }

    YCPValue arg_value = m_arg->evaluate (cse);

    // parse time?
    if (cse
	&& arg_value.isNull () )
    {
	return YCPNull ();
    }

    if (arg_value.isNull()
	|| arg_value->isVoid()
	|| !arg_value->isList())
    {
	return m_def->evaluate (cse);
    }

    YCPValue result = var_value;

    YCPList indices = arg_value->asList();
    for (int i = 0; i < indices->size(); ++i) // loop over all bracket indices
    {
	YCPValue v = indices->value(i);
	if (v.isNull())
	{
	    result = YCPNull();
	    ycp2error ("Invalid bracket parameter nil");
	    break;
	}
	else if (result->isList())
	{
	    YCPList l = result->asList();
	    if (!v->isInteger())
	    {
		result = YCPNull();
		ycp2error ("Invalid bracket parameter for list");
		break;
	    }

	    long long idx = v->asInteger()->value();
	    if ((idx < 0)
		|| (idx >= l->size()))
	    {
		result = YCPNull();
		break;
	    }
	    result = l->value (idx);
	}
	else if (result->isTerm())
	{
	    YCPTerm t = result->asTerm();
	    if (!v->isInteger())
	    {
		result = YCPNull();
		ycp2error ("Invalid bracket parameter for term");
		break;
	    }

	    long long idx = v->asInteger()->value();
	    if ((idx < 0)
		|| (idx >= t->size()))
	    {
		result = YCPNull();
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
	    result = YCPNull();
	    break;
	}

	if (result.isNull())
	{
	    break;
	}

    } // while bracket indices

    if (result.isNull())
    {
	result = m_def->evaluate (cse);
    }

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


std::ostream &
YEBracket::toXml (std::ostream & str, int /*indent*/ ) const
{
    str << "<yebracket ";
    commentToXml(str);
    m_resultType->toXml( str, 0);
    str << ">";
    m_var->toXml( str, 0 );		// variable
    m_arg->toXml( str, 0 );		// list
    m_def->toXml( str, 0 );		// default
    return str << "</yebracket>";
}


// ------------------------------------------------------------------
// builtin function ref (-> declaration_t, type, parameters)

YEBuiltin::YEBuiltin (declaration_t *decl, YBlockPtr parameterblock, constTypePtr type)
    : YCode ()
    , m_decl (decl)
    , m_type (type==0 ? Type::Function(Type::Unspec) : (constFunctionTypePtr)type)
    , m_parameterblock (parameterblock)
    , m_parameters (0)
{
}


YEBuiltin::YEBuiltin (bytecodeistream & str)
    : YCode ()
    , m_parameterblock (0)
    , m_parameters (0)
{
    m_type = FunctionTypePtr (Bytecode::readType (str));
    extern StaticDeclaration static_declarations;
    m_decl = static_declarations.readDeclaration (str);
#if DO_DEBUG
    y2debug ("YEBuiltin::YEBuiltin(type '%s', decl '%s:%s')", (m_type == 0) ? "<NULL>" : m_type->toString().c_str(), (m_decl == 0) ? "<NULL>" : m_decl->name, (m_decl && m_decl->type) ? m_decl->type->toString().c_str() : "<NULL>");
#endif
    if (Bytecode::readBool (str))
    {
	m_parameterblock = (YBlockPtr)Bytecode::readCode (str);
	Bytecode::pushNamespace (m_parameterblock->nameSpace());
    }
    Bytecode::readYCodelist (str, &m_parameters);
    if (m_parameterblock != 0)
    {
	Bytecode::popNamespace (m_parameterblock->nameSpace());
    }
    // throw away type info
    m_type = Type::Void;
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
	Bytecode::pushNamespace (m_parameterblock->nameSpace());
    }
    Bytecode::writeYCodelist (str, m_parameters);
    if (m_parameterblock != 0)
    {
	Bytecode::popNamespace (m_parameterblock->nameSpace());
    }
    return str;
}


std::ostream &
YEBuiltin::toXml( std::ostream & str, int indent ) const
{
    str << "<builtin name=\"" << m_decl->name << "\"";

    if (m_decl->name_space)
      str << " ns=\"" << m_decl->name_space->name << "\"";

    commentToXml(str);

    if (m_parameterblock != 0)
    {
	Xmlcode::pushNamespace( m_parameterblock->nameSpace() );
    }

    // check DECL_SYMBOL functions, these are e.g.
    // list: find, filter, maplist, listmap, sort, foreach
    // map: mapmap, maplist, filter, foreach

    ycodelist_t *block = NULL;
    std::vector<ycodelist_t *> values;
    ycodelist_t *p = m_parameters;
    u_int32_t count = 0;
    while (p) {

	if (m_decl->flags & DECL_SYMBOL)
	{
	    // we have three types of parameters
	    //  1. symbols (used to assign values for the block)
	    //     - symbols always come first
	    //  2. expression (must eval to a enumerable type, list or map
	    //	the builtin operates on and assigns values by iterating over the value)
	    //  3. block to call with symbols assigned
	    //     - the block always is last
	    //
	    // find symbols (entries into m_parameterblock) first
	    //

	    if (p->code->kind() == ycEntry) {
		str << " sym" << count << "=\"" << Xmlcode::xmlify( p->code->toString() ) << "\"";
	    }
	    else if (p->next) {
		    values.push_back(p);
	    }
	    else {
		if (block) cerr << "Block already set!" << endl;
		block = p;
	    }
	}
	p = p->next;
	++count;
    }

    if ((m_decl->flags & DECL_SYMBOL) == 0)
    {
	str << " args=\"" << count << "\"";
    }
    str << ">";

    if (m_decl->flags & DECL_SYMBOL)
    {
      for(std::vector<ycodelist_t*>::iterator i = values.begin(); i != values.end(); ++i)
      {
        str << "<expr>";
        (*i)->code->toXml( str, indent+2 );
        str << "</expr>";
      }
	block->code->toXml( str, indent );
    }
    else {
	Xmlcode::writeYCodelist( str, m_parameters );
    }
    
    if (m_parameterblock != 0)
    {
	Xmlcode::popNamespace( m_parameterblock->nameSpace() );
    }
    return str << "</builtin>";
}


YEBuiltin::~YEBuiltin ()
{
    ycodelist_t *parm = m_parameters;
    while (parm)
    {
	ycodelist_t *next = parm->next;
	delete parm;
	parm = next;
    }
}


declaration_t *
YEBuiltin::decl () const
{
    return m_decl;
}


YBlockPtr
YEBuiltin::parameterBlock () const
{
    return m_parameterblock;
}


/**
 * 'close' function, perform final parameter check
 * if ok, return 0
 * if bad signature, return expected signature
 *   (i.e. if non-matching template)
 * if undefined, return Type::Error
 *   (wrong type was already reported in attachParameter())
 */

constTypePtr
YEBuiltin::finalize (Logger* problem_logger)
{
    extern StaticDeclaration static_declarations;
    
#if DO_DEBUG
    y2debug ("YEBuiltin::finalize (%s)", StaticDeclaration::Decl2String (m_decl, true).c_str());
    y2debug ("m_type: %s", m_type->toString ().c_str());
#endif

    // final type check for all parameters
    declaration_t *decl = static_declarations.findDeclaration (m_decl, m_type, false);
    if (decl == 0)
    {
	StaticDeclaration::errorNoMatch (problem_logger, m_type, m_decl);
	return Type::Error;
    }
    m_decl = decl;					// remember matching declaration

#if DO_DEBUG
    y2debug ("YEBuiltin::finalize found (%s : %s)", StaticDeclaration::Decl2String (m_decl, true).c_str(), m_type->toString().c_str());
#endif

    if (m_decl->flags & DECL_FLEX)
    {
	// expand template type to real type

	constTypePtr rtype = Type::determineFlexType (m_type, m_decl->type);
	if (rtype == 0)					// error
	{
	    return m_decl->type;
	}
	else if (m_type->isFunction()
		 && rtype->isFunction())
	{
	    constFunctionTypePtr mft = m_type;
	    constFunctionTypePtr rft = rtype;
	    if (mft->parameters()->match (rft->parameters()) != 0)		//  or realtype does not match actual type
	    {
#if DO_DEBUG
    y2debug ("YEBuiltin::finalize (ERR : %s)", rtype ? rtype->toString().c_str() : "NULL");
#endif
		return rtype;
	    }
	}
	m_type = rtype;
    }
    else
    {
	m_type = m_decl->type;
    }

    // function has format string "%1 ..." as first arg, check number of %n against number of parameters

    if (m_decl->flags & DECL_FORMATTED)
    {
	YCodePtr formatcode = m_parameters ? m_parameters->code : 0;
	if (formatcode == 0)
	{
	    problem_logger->error ("First parameter should be format string");
	    return Type::Error;
	}

	// get the argument string

	const char *cptr = NULL;
	YCPString ystring("");			// temporary string, just to keep the reference from going out of scope

	if (formatcode->kind() == YCode::ycString)			// it might be a normal string constant
	{
	    // keep the reference in ystring
	    ystring = formatcode->evaluate()->asString();
	    cptr = ystring->value_cstr();
	}
	else if (formatcode->kind() == YCode::ycLocale)			// or a translatable string
	{
	    // use the untranslated locale string (avoids warning for sformat)
	    cptr = ((YLocalePtr)formatcode)->value();
	}
	else								// any other value we can't check here (defer to runtime checking)
	{
	    // otherwise accept only strings
	    problem_logger->warning ("Format string is not constant, no parameter checking possible");
	    return 0;
	}

	// save start of cptr, for error message
	const char *cptr_start = cptr;

	// check %n values and set bits in 'mask' for every n
	// dont simply count the number of %n occurences, since they might be duplicate

	unsigned long long mask = 0;
	int bits = sizeof (unsigned long long) * 8;
	while (*cptr != 0)
	{
	    if (*cptr == '%')
	    {
		cptr++;
		
		int number = 0;
		
		if ( (*cptr) > '0' && (*cptr) <= '9' )
		{
		    number = (*cptr)-'0'; // for more digits use atoi (cptr);
		}
		if (number > 0)
		{
		    if (number >= bits)
		    {
			problem_logger->warning (string ("Numeric value after % too large (")
			    + *cptr + "), cant check validity");
		    }
		    else
		    {
			mask |= 1LL << (number - 1);
		    }
		}
		else if ((number == 0)			// no digit following %
			 && (*cptr != '%'))		// %% is allowed
		{
		    problem_logger->error (string("Bad '%' selector in format string at '")
			+ (cptr-1) + "', use '%n' (n=1,2,...) instead.");
		    problem_logger->error (string("Full string: '") + cptr_start + "'.");
		    return Type::Error;
		}
	    }
	    cptr++;
	}

	// now count the number of different %n's in the format string

	int count = 0;
	while (mask != 0)
	{
	    if (mask & 1) count++;
	    mask >>= 1;
	}

	// now check the number of actual parameters

	ycodelist_t *paramptr = m_parameters->next;	// we already know that at least one parameter (the format string) exists.
	while (paramptr != 0)
	{
	    count--;
	    paramptr = paramptr->next;
	}
	if (count != 0)
	{
	    problem_logger->error ("Format string doesn't match number of parameters");
	    return Type::Error;
	}
    } // if DECL_FORMATTED

#if DO_DEBUG
    y2debug ("YEBuiltin::finalize (%s : %s)", StaticDeclaration::Decl2String (m_decl, true).c_str(), m_type->toString().c_str());
#endif

    return 0;
}


// check if m_parameterblock is really needed, drop if not
//  the m_parameterblock is of course needed for DECL_SYMBOL but
//  parser.yy will also open one for overloaded builtins.
void
YEBuiltin::closeParameters ()
{
#if DO_DEBUG
    y2debug ("YEBuiltin::closeParameters (m_parameterblock %s)", m_parameterblock ? "SET" : "CLEAR");
#endif
    if (m_parameterblock != 0)
    {
	if (m_parameterblock->symbolCount() == 0)
	{
#if DO_DEBUG
	    y2debug ("YEBuiltin has no symbolic parameters");
#endif
	    m_parameterblock = 0;
	}
    }

}


constTypePtr
YEBuiltin::type () const
{
    constTypePtr ret = m_type->returnType ();
#if DO_DEBUG
    y2debug ("ret '%s' -> %s", m_type->toString().c_str(), ret->toString().c_str());
#endif

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
#if DO_DEBUG
    y2debug ("ret %s", ft->returnType()->toString().c_str());
#endif

    return ft->returnType();
}

constTypePtr
YEBuiltin::completeType () const
{
    return m_type;
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
YEBuiltin::attachParameter (YCodePtr code, constTypePtr type)
{
#if DO_DEBUG
    y2debug ("YEBuiltin::attachParameter (%s:%s)", code ? code->toString().c_str() : "<NULL>", type->toString().c_str());
#endif

    if ((code == 0)
	|| (code->isError()))
    {
	y2debug ("Bad code");
	return Type::Unspec;
    }


    if (!type->isUnspec ())
    {
	FunctionTypePtr ntype = m_type->clone();
	ntype->concat (type);

#if DO_DEBUG
	y2debug ("YEBuiltin::attachParameter (%s:%s -> '%s')", type->toString().c_str(), code->toString().c_str(), ntype->toString().c_str());
#endif
	m_type = ntype;
    }
#if DO_DEBUG
    else
    {
	y2debug ("type '%s' isUnspec", type->toString().c_str());
    }
#endif

    ycodelist_t *element = new ycodelist_t;
    element->code = code;
    element->next = 0;
    if (m_parameters == 0)
    {
	m_parameters = element;
    }
    else
    {
	ycodelist_t *ptr = m_parameters;
	while (ptr->next != 0) ptr = ptr->next;
	ptr->next = element;
    }

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

#if DO_DEBUG
    y2debug ("YEBuiltin::attachSymVariable (%s:%s @%d, to %s:%s", name, type->toString().c_str(), line, m_decl->name, m_type->toString().c_str());
#endif

    if (type->isUnspec())							// no type given, might be symbol or untyped variable
    {
	// try with symbol constant first
	addedType = Type::Symbol;

	matchedType = m_type->clone();
	matchedType->concat (addedType);

	declaration_t *decl = static_declarations.findDeclaration (m_decl, matchedType, true);
	if (decl != 0)
	{
#if DO_DEBUG
	    y2debug ("YEBuiltin::attachSymVariable() symbol constant matched");
#endif
	    return attachParameter (new YConst (YCode::ycSymbol, YCPSymbol (name)), addedType);
	}

	// no match, try with untyped variable
	type = Type::Any;

	ycp2warning(YaST::ee.filename().c_str(), YaST::ee.linenumber(), "Parameter '%s' has unspecified type", name);
    }

    addedType = VariableTypePtr (new VariableType (type));	// it's a typed symbolic variable

#if DO_DEBUG
    y2debug ("addedType %s", addedType->toString().c_str());
#endif

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
#if DO_DEBUG
    y2debug ("YEBuiltin::evaluate [%s:%s]", YCode::toString (kind()).c_str(), m_decl->name);
#endif

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

#if DO_DEBUG
	y2debug ("actualp ([%d]%s)", actualp->code->kind(), actualp->code->toString().c_str());
#endif

	if (actualp->code->isBlock() || ( (m_decl->flags & DECL_NOEVAL) == DECL_NOEVAL))
	    // block as parameter to builtin function or builtin will eval on its own
	{
	    args[i] = YCPCode (actualp->code);	// pass as-is
	}
	else if (actualp->code->kind() == ycEntry)
	{
	    args[i] = ((YConstPtr)(actualp->code))->value();
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

#if DO_DEBUG
	y2debug ("==> (%s)", args[i].isNull() ? "NULL" : args[i]->toString().c_str());
#endif

	if ((typepos < 0)					// not at wildcard yet
	    && type->parameterType(i)->isWildcard ())		// at '...' now ?
	{
	    typepos = i;
#if DO_DEBUG
	    y2debug ("type '...' %d", i);
#endif
	}

	if (typepos >= 0)					// at or beyond '...'
	{
#if DO_DEBUG
	    y2debug ("w! args[%d] = '%s'", i, args[i].isNull() ? "nil" : args[i]->toString().c_str());
#endif
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
#if DO_DEBUG
	y2debug ("w! pos %d '%s'", i, list->toString().c_str());
#endif
	i = typepos+1;
	args[i-1] = list;
    }


    // call builtin function

#if DO_DEBUG
    y2debug ("YEBuiltin::evaluate [%s (%d args)]", StaticDeclaration::Decl2String (m_decl, false).c_str(), i);
    y2debug ("parameter 1: %s", i > 0 ? (args[0].isNull() ? "NULL" : args[0]->toString().c_str()) : "nil" );
#endif
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
	    ycp2error("YEBuiltin::evaluate [%s (%d args)]: Call handler declared, but not present",
		    StaticDeclaration::Decl2String (m_decl, false).c_str(), i);
	    return YCPNull();
	}
    }
    else
    {
	if (m_parameterblock) m_parameterblock->pushToStack ();
	
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
	if (m_parameterblock) m_parameterblock->popFromStack ();
    }

#ifdef BUILTIN_STATISTICS
    if (!ret.isNull ())
    {
	FILE *fout = fopen ("/tmp/builtin-use.txt", "a");
	fprintf (fout, "%s %s\n", m_decl->name, m_decl->type->toString().c_str());
	fclose (fout);
    }
#endif

#if DO_DEBUG
    y2debug ("YEBuiltin ret (%s)", ret.isNull() ? "NULL" : ret->toString().c_str());
#endif

    return ret;
}


// ------------------------------------------------------------------
// function call parameter handling (-> SymbolEntry + Parameters)

YECall::YECall (TableEntry* entry)
    : YCode ()
    , m_entry (entry)
    , m_sentry ( entry ? entry->sentry () : 0)
    , m_parameters (0)
    , m_parameter_types (0)
    , m_functioncall (0)
{
#if DO_DEBUG
    y2debug ("YECall[%p] (%s)", this, entry ? entry->sentry()->toString().c_str() : 0);
#endif
    // lookup the maximal number of parameters for this entry (and overloads)
    // retrieve function type for formal parameter list
    int max = 0;
    
    TableEntry* t = entry;
    while (t)
    {
	int curr = ((constFunctionTypePtr)t->sentry ()->type ())->parameterCount ();
	if (curr > max)
	{
	    max = curr;
	}
	
	t = t->next_overloaded ();
    }
    
    if (max>0)
    {
	m_parameters = new YCodePtr[max];
	m_parameter_types = new constTypePtr[max];
    }
    m_next_param_id = 0;
}


YECall::YECall (bytecodeistream & str)
    : YCode ()
    , m_entry (0)
    , m_sentry (Bytecode::readEntry (str))
    , m_parameters (0)
    , m_parameter_types (0)
    , m_functioncall (0)
    , m_next_param_id (0)
{
    u_int32_t count = Bytecode::readInt32 (str);
    if (count>0)
    {
	m_parameters = new YCodePtr[count];
	
	for (uint i = 0 ; i < count; i++)
	{
	    m_parameters[i] = Bytecode::readCode (str);
	    if (m_parameters[i] == 0)
	    {
		y2error ("parameter code read failed for %d", i);
		throw Bytecode::Invalid();
	    }
	}
    }
    
    m_next_param_id = count;

#if DO_DEBUG
    y2debug ("YECall (fromStream): %s", toString().c_str());
#endif
}


YECall::~YECall ()
{
    if (m_parameters)
    {
	delete[] m_parameters;
    }
    
    if (m_parameter_types)
    {
	delete[] m_parameter_types;
    }

    if (m_functioncall)
    {
	delete m_functioncall;
	m_functioncall = 0;
    }
}


/**
 * A static method to read YEFunction for function pointers and
 * convert it to YEFunctionPointer implementation. This is
 * needed for backward compatibility with SLES9/9.1
 *
 * @param str	the input bytecode stream
 * @return 	read YCode or 0 on errors
 */
YECallPtr YECall::readCall (bytecodeistream & str)
{
    YECallPtr res = 0;

    SymbolEntryPtr sentry = Bytecode::readEntry (str);
    
    if (!sentry)
    {
	return 0;
    }
    
    if (str.isVersion (1,3,2) && sentry->isVariable ())
    {
	// it is a function pointer from SLES9/9.1
	res = new YEFunctionPointer (0);
    }
    else
    {
	// it is direct function call
	res = new YEFunction (0);
    }
    
    res->m_sentry = sentry;
    
    // read the parameters
    u_int32_t count = Bytecode::readInt32 (str);
    
    if (count>0)
    {
	res->m_parameters = new YCodePtr[count];

	for (uint i = 0 ; i < count; i++)
	{
	    res->m_parameters[i] = Bytecode::readCode (str);
	    if (res->m_parameters[i] == 0)
	    {
		y2error ("parameter code read failed for %d", i);
		return 0;
	    }
	}
    }

    res->m_next_param_id = count;

    return res;
}



const SymbolEntryPtr 
YECall::entry() const
{
    return m_sentry;
}


    /**
     * Attach parameter to external function call
     *
     * This function doesn't really check parameter types,
     * this is done in YECall::finalize() below.
     *
     * @param code: parameter code
     * @param type: parameter type
     * @return NULL if success,
     *    != NULL (expected type) if wrong parameter type
     *    Type::Unspec if bad code (NULL or isError)
     *    Type::Error if excessive parameter
     */

constTypePtr
YECall::attachParameter (YCodePtr code, constTypePtr type)
{
#if DO_DEBUG
    y2debug ("YECall::attachParameter (%s:%s)", code ? code->toString().c_str() : "(NULL)", type->toString().c_str());
#endif

    if (code == 0 || code->isError())
    {
	return Type::Unspec;
    }
    
    // check, if there is not too many of params
    if ((m_parameters == 0)
	|| (m_parameter_types == 0))
    {
	// excessive parameter - we don't expect any
	return Type::Error;
    }
    
    // find next parameter slot to be filled, loop through chain
    //  of overloaded functions

    uint max = 0;
    TableEntry* t = m_entry;
    while (t)
    {
        uint curr = ((constFunctionTypePtr)t->sentry ()->type ())->parameterCount ();
        if (curr > max)
        {
            max = curr;
        }

        t = t->next_overloaded ();
    }
    
    if (m_next_param_id >= max)
    {
	return Type::Error;
    }

    m_parameters[m_next_param_id] = code;
    m_parameter_types[m_next_param_id] = type;
    m_next_param_id++;

#if DO_DEBUG
    y2debug ("done");
#endif
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
YECall::finalize()
{
    TableEntry* entry = m_entry;

    while (entry)
    {
	// now check the parameters really. if they don't match,
	// lookup also the overloaded ones    
	SymbolEntryPtr sentry = entry->sentry ();

	// prepare the overloaded one, if exists
	TableEntry* next_overloaded = entry->next_overloaded ();

	// retrieve function type for formal parameter list
	constFunctionTypePtr ftype = sentry->type();
	
	// not the correct number of parameters?
	if ((int)m_next_param_id != ftype->parameterCount())
	{
	    // try to continue with the next one
	    entry = next_overloaded;
	    continue;
	}
    
	bool accept = true;
	// ok, check whether types match
	for (uint check_count = 0; check_count < m_next_param_id ; check_count++)
	{
	    constTypePtr expected_type = ftype->parameterType (check_count);
	    YCodePtr code = m_parameters[check_count];
	    constTypePtr type = m_parameter_types[check_count];
	    
	    // if the parameter type is block, find out the type for the actual param
	    if (expected_type->isBlock ())
	    {
		if (code->isBlock ())
		{
		    type = new BlockType(type->isUnspec () ? Type::Void : type);
		}
	    }

#if DO_DEBUG
	    y2debug ("checking parameter %d type: expected '%s', given '%s'", check_count, expected_type->toString().c_str(), type->toString().c_str());
#endif

	    int match = type->match (expected_type);
	    if (match < 0)
	    {
#if DO_DEBUG
		y2debug ("type mismatch");
#endif
		// type mismatch
		entry = next_overloaded;
		accept = false;
		break;
	    }

	    if (expected_type->isReference())	// function expects a reference
	    {
		if (! code->isReferenceable())	// but the actual parameter isn't referenceable
		{
		    y2debug ("Can't take reference of '%s'", code->toString().c_str());
		    entry = next_overloaded;
		    accept = false;
		    break;
		}
		else if (match > 0)
		{
#if DO_DEBUG
		    y2debug ("Can't reference to type propagation '%s' -> '%s'", type->toString().c_str(), expected_type->toString().c_str());
#endif		    
		    entry = next_overloaded;
		    accept = false;
		    break;
		}
	    }
	}
	
	if (accept)
	{
#if DO_DEBUG
	    y2debug ("Accepting for: %s", entry->sentry()->toString().c_str());
#endif
	    // adapt the code (add propagation/references if needed)
	    for (uint check_count = 0 ; check_count < m_next_param_id; check_count++ )
	    {
		constTypePtr expected_type = ftype->parameterType (check_count);
		constTypePtr type = m_parameters[check_count]->type ();
	    
		// if the parameter type is block, find out the type for the actual param
		if (expected_type->isBlock ())
		{
		    if (m_parameters[check_count]->isBlock ())
		    {
			type = new BlockType(type->isUnspec () ? Type::Void : type);
		    }
		}

		int match = type->match (expected_type);

		if (expected_type->isReference())	// function expects a reference
		{
		    YEVariablePtr var = (YEVariablePtr)(m_parameters[check_count]);
		    m_parameters[check_count] = new YEReference (var->entry());
		}

		if (match > 0)				// propagation
		{
		    m_parameters[check_count] = new YEPropagate (m_parameters[check_count], type, expected_type);
		}
	    }
	    
	    // update the sentry
	    m_entry = entry;
	    m_sentry = m_entry->sentry ();

	    return 0;
	}
    }

    // none was accepted;
    return Type::Error;
}


string
YECall::toString() const
{
#if DO_DEBUG
    y2debug ("YECall::toString [%p]", this);
#endif
    string s = m_sentry->toString(false);

    s += " (";
    
    for (uint i = 0 ; i < m_next_param_id ; i++)
    {
	s += m_parameters[i]->toString().c_str();
	if (i + 1 < m_next_param_id)
	{
	    s += ", ";
	}
    }
    s += ")";
    return s;
}


std::ostream &
YECall::toStream (std::ostream & str) const
{
    YCode::toStream (str);
    if (Bytecode::writeEntry (str, m_sentry))
    {
	Bytecode::writeInt32 (str, m_next_param_id);

	for (uint i = 0 ; i < m_next_param_id; i++)
	{
	    m_parameters[i]->toStream (str);
	}
    }
    return str;
}


std::ostream &
YECall::toXml (std::ostream & str, int /*indent*/ ) const
{
    str << "<call";
    str << " category=\"" << m_sentry->catString() << "\"";
    commentToXml(str);
    if (!m_sentry->nameSpace()->name().empty()) {
	str << " ns=\"" << m_sentry->nameSpace()->name() << "\"";
    }
    str << " type=\"" << m_sentry->type()->toXmlString() << "\"";
    str << " name=\"" << m_sentry->name() << "\">";
    if (m_next_param_id > 0) {
	str << "<args>";

	for (uint i = 0 ; i < m_next_param_id; i++)
	{
	    m_parameters[i]->toXml( str, 0 );
	}
	str << "</args>";
    }
    return str << "</call>";
}


constTypePtr
YECall::type () const
{
    return ((constFunctionTypePtr)(m_sentry->type ()))->returnType ();
}


string
YECall::qualifiedName () const
{
    string n = m_sentry->nameSpace () && ! m_sentry->nameSpace ()->name ().empty ()?
	(m_sentry->nameSpace ()->name () + string ("::")) :
	"";
    return n + m_sentry->name ();
}

// ------------------------------------------------------------------
// function ref (-> SymbolEntry + Parameters)

YEFunction::YEFunction (TableEntry* entry)
    : YECall (entry)
{
#if DO_DEBUG
    y2debug ("YEFunction[%p] (%s)", this, entry ? entry->sentry()->toString().c_str() : "nil");
#endif
}


YEFunction::YEFunction (bytecodeistream & str)
    : YECall (str)
{
    m_functioncall = const_cast<Y2Namespace*>(m_sentry->nameSpace())->createFunctionCall (m_sentry->name (), m_sentry->type ());
    if (m_functioncall == 0)
    {
	y2error ("Cannot create a function call for %s", m_sentry->toString ().c_str ());
	throw Bytecode::Invalid();
    }
}


YCPValue
YEFunction::evaluate (bool cse)
{
    if (cse)
    {
	return YCPNull();
    }

#if DO_DEBUG
    y2debug ("YEFunction::evaluate (%s)\n", toString().c_str());
#endif

    
    if (!m_functioncall)
    {
	m_functioncall = const_cast<Y2Namespace*>(m_sentry->nameSpace())->createFunctionCall (m_sentry->name (), m_sentry->type ());
	if (m_functioncall == 0)
	{
	    y2error ("Cannot create a function call for %s", m_sentry->toString ().c_str ());
	    return YCPVoid ();
        }
    }
    else
    {
	if (! m_functioncall->reset ())
	{
	    y2error ("failed to reset function call parameters for %s", m_sentry->toString ().c_str ());
	    return YCPVoid ();
	}
    }
    
    YCPValue evaluated_params [m_next_param_id];

    for (unsigned int p = 0; p < m_next_param_id ; p++)
    {
	// FIXME, check for symbol or block type and suppress evaluation

	YCPValue value = m_parameters[p]->evaluate ();

	if (value.isNull())
	{
	    ycp2error ("Parameter eval failed (%s)", m_parameters[p]->toString().c_str());
	    return value;
	}
	
#if DO_DEBUG
	y2debug ("parameter %d = (%s)", p, value->toString().c_str());
#endif

	evaluated_params [p] = value;
    }

    // set the parameters for Y2Function
    for (unsigned int p = 0; p < m_next_param_id ; p++)
    {
	m_functioncall->attachParameter (evaluated_params[p], p);
    }
    
    // save the context info
    int linenumber = YaST::ee.linenumber();
    string filename = YaST::ee.filename();

    if (YaST::ee.endlessRecursion())
    {
	ycp2error ("Returning nil instead of calling the function.");
	return YCPVoid ();
    }

    YaST::ee.pushframe((YECallPtr)this, evaluated_params);

    YCPValue value = m_functioncall->evaluateCall ();

    // restore the context info
    YaST::ee.setLinenumber(linenumber);
    YaST::ee.setFilename(filename);
    
    YaST::ee.popframe();

#if DO_DEBUG
    y2debug("evaluate done (%s) = '%s'", qualifiedName ().c_str(), value.isNull() ? "NULL" : value->toString().c_str());
#endif
    return value;
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
    constTypePtr res = YECall::finalize ();
    
    if (res != 0)
    {
	return res;
    }
    
    m_functioncall = const_cast<Y2Namespace*>(m_sentry->nameSpace())->createFunctionCall (m_sentry->name (), m_sentry->type ());
    if (m_functioncall == 0)
    {
	y2error ("Cannot create a function call for %s", m_sentry->toString ().c_str ());
	return Type::Error;
    }

    return 0;
}


// ------------------------------------------------------------------
// function ref (-> SymbolEntry + Parameters)

YEFunctionPointer::YEFunctionPointer (TableEntry* entry)
    : YECall (entry)
{
#if DO_DEBUG
    y2debug ("YEFunctionPointer[%p] (%s)", this, entry ? entry->sentry()->toString().c_str() : "nil");
#endif
}


YEFunctionPointer::YEFunctionPointer (bytecodeistream & str)
    : YECall (str)
{
}


YCPValue
YEFunctionPointer::evaluate (bool cse)
{
    if (cse)
    {
	return YCPNull();
    }

#if DO_DEBUG
    y2debug ("YEFunctionPointer::evaluate (%s)\n", toString().c_str());
#endif

    YCPValue ptr = m_sentry->value ();
    if (ptr.isNull () || ! ptr->isReference ())
    {
	ycp2error ("Function pointer (%s) is %s"
	    , m_sentry->toString().c_str()
	    , ptr.isNull () ? "NULL" : ptr->toString ().c_str ());
	return YCPVoid ();
    }
    
    SymbolEntryPtr ptr_sentry = ptr->asReference ()->entry ();

    Y2Namespace* ns = const_cast<Y2Namespace*> (ptr_sentry->nameSpace ());

    m_functioncall = ns->createFunctionCall (
	ptr_sentry->name (),
	ptr_sentry->type ()
    );
    
    if (!m_functioncall)
    {
	y2internal ("Cannot get function call object for %s", m_sentry->toString().c_str());
	return YCPVoid ();
    }

    // FIXME: this could fail    
    m_functioncall->reset ();
    
    YCPValue m_params [m_next_param_id];

    for (unsigned int p = 0; p < m_next_param_id ; p++)
    {
	// FIXME, check for symbol or block type and suppress evaluation

	YCPValue value = m_parameters[p]->evaluate ();

	if (value.isNull())
	{
	    ycp2error ("Parameter eval failed (%s)", m_parameters[p]->toString().c_str());
	    return value;
	}
	
#if DO_DEBUG
	y2debug ("parameter %d = (%s)", p, value->toString().c_str());
#endif

	m_params [p] = value;
    }

    // set the parameters for Y2Function
    for (unsigned int p = 0; p < m_next_param_id ; p++)
    {
	m_functioncall->attachParameter (m_params[p], p);
    }
    
    // save the context info
    int linenumber = YaST::ee.linenumber();
    string filename = YaST::ee.filename();

    YCPValue value = m_functioncall->evaluateCall ();

    // restore the context info
    YaST::ee.setLinenumber(linenumber);
    YaST::ee.setFilename(filename);

#if DO_DEBUG
    y2debug("evaluate done (%s) = '%s'", qualifiedName ().c_str(), value.isNull() ? "NULL" : value->toString().c_str());
#endif
    return value;
}


// ------------------------------------------------------------------
// function call for outside of YCP (similar to YEFunction) (-> SymbolEntry + Parameters)

Y2YCPFunction::Y2YCPFunction (YSymbolEntryPtr entry)
    : Y2Function ()
    , m_sentry (entry)
    , m_parameters (NULL)
{
#if DO_DEBUG
    y2debug ("Y2YCPFunction[%p] (%s)", this, entry->toString().c_str());
#endif
    // cleanup an array for the parameters
    
    uint count = ((constFunctionTypePtr)(m_sentry->type ()))->parameterCount ();
    
    m_parameters = new YCPValue[count];
    
    for (uint i=0; i < count; i++)
    {
	m_parameters[i] = YCPNull ();
    }
}


Y2YCPFunction::~Y2YCPFunction ()
{
    delete[] m_parameters;
}


YCPValue
Y2YCPFunction::evaluateCall ()
{
#if DO_DEBUG
//    y2debug ("Y2YCPFunction::evaluateCall (%s)\n", toString().c_str());
#endif

    YFunctionPtr func = (YFunctionPtr)(m_sentry->code());

    // push parameter values for recursion
    for (unsigned int p = 0; p < func->parameterCount(); p++)
    {
	func->parameter (p)->push ();
    }

    // push also local parameters
    YCodePtr definition = func->definition ();

    if (definition == 0)
    {
	ycp2error ("Function '%s' is only declared, but not defined yet.", m_sentry->toString().c_str());
	return YCPNull();
    }

    if (definition->isBlock())
    {
//       ((YBlockPtr)definition)->pushToStack ();
    }

    for (unsigned int p = 0; p < func->parameterCount(); p++)
    {
	YCPValue value = m_parameters[p];

	if (value.isNull())
	{
	    ycp2error ("Parameter not specified (%d)", p);

	    // cleanup: pop parameter values for recursion
	    for (unsigned int p = 0; p < func->parameterCount(); p++)
	    {
		func->parameter (p)->pop ();
	    }

	    return value;
	}
	
	SymbolEntryPtr formalp = func->parameter (p);
#if DO_DEBUG
	y2debug ("formalp (%s) = (%s)", formalp->toString().c_str(), value->toString().c_str());
#endif

	formalp->setValue (value);
    }
    
    // save the context info
    int linenumber = YaST::ee.linenumber();
    string filename = YaST::ee.filename();

    YCPValue value = definition->evaluate ();

    if (definition->isBlock())
    {
       // pop also local parameters
//       ((YBlockPtr)definition)->popFromStack ();
    }

    // restore the context info
    YaST::ee.setLinenumber(linenumber);
    YaST::ee.setFilename(filename);

    // pop parameter values for recursion
    for (unsigned int p = 0; p < func->parameterCount(); p++)
    {
	func->parameter (p)->pop ();
    }

#if DO_DEBUG
    y2debug("evaluate done (%s) = '%s'", definition->toString().c_str(), value.isNull() ? "NULL" : value->toString().c_str());
#endif
    return value;
}


constTypePtr
Y2YCPFunction::wantedParameterType () const
{
    if (m_sentry->code ()->kind() != YCode::ycFunction)
    {
	return Type::Unspec;
    }
    YFunctionPtr func_f = m_sentry->code ();

    // find out number of already done parameters
    for (int i = 0 ; i < ((constFunctionTypePtr)(m_sentry->type ()))->parameterCount (); i++)
    {
	if (m_parameters[i].isNull ())
	{
	    // returns NULL if actual_count is out of bounds
	    SymbolEntryPtr param_se = func_f->parameter (i);

	    // this is for value conversion purposes. if this is an excess
	    // parameter, we don't care about the type now since
	    // attachParameter will complain anyway
    	    constTypePtr param_tp = param_se ? param_se->type () : Type::Any;
	    return param_tp;
	}
    }
    
    // all parameters done
    return Type::Unspec;
}


bool
Y2YCPFunction::attachParameter (const YCPValue& arg, int pos)
{
    if (pos < 0 || pos > ((constFunctionTypePtr)(m_sentry->type ()))->parameterCount () )
    {
	y2error ("Attaching parameter to function '%s' at incorrect position: %d", m_sentry->toString().c_str(), pos );
	return false;
    }
    m_parameters[pos] = arg;
    return true;
}


bool
Y2YCPFunction::appendParameter (const YCPValue& arg)
{
    if (arg.isNull())
    {
	ycp2error ("NULL parameter to %s", qualifiedName ().c_str ());
	return false;
    }

    // FIXME: check the type
    
    // lookup the first non-set parameter
    for (int i = 0 ; i < ((constFunctionTypePtr)(m_sentry->type ()))->parameterCount (); i++)
    {
	if (m_parameters[i].isNull ())
	{
#if DO_DEBUG
	    y2debug ("Assigning parameter %d: %s", i, arg->toString ().c_str ());
#endif	    
	    m_parameters[i] = arg;
	    return true;
	}
    }
    
    // Our caller should report the place
    // in a script where this happened
    ycp2error ("Excessive parameter to %s", qualifiedName ().c_str ());

    return false;
}


bool
Y2YCPFunction::finishParameters ()
{
    for (int i = 0 ; i < ((constFunctionTypePtr)(m_sentry->type ()))->parameterCount (); i++)
    {
	if (m_parameters [i].isNull ())
	{
	    y2error ("Missing parameter %d to %s",
		 i, qualifiedName ().c_str ());
	    return false;
	}
    }
    return true;
}


string
Y2YCPFunction::qualifiedName () const
{
    string n = m_sentry->nameSpace () && ! m_sentry->nameSpace ()->name ().empty ()?
	(m_sentry->nameSpace ()->name () + string ("::")) :
	"";
    return n + m_sentry->name ();
}

bool
Y2YCPFunction::reset ()
{
    for (int i=0; i<((constFunctionTypePtr)(m_sentry->type ()))->parameterCount (); i++)
    {
	m_parameters[i] = YCPNull ();
    }

    return true;
}


string
Y2YCPFunction::name () const
{
    return m_sentry->name ();
}
// EOF
