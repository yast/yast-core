/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                    (C) SuSE Linux AG |
\----------------------------------------------------------------------/

   File:	Type.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/

#include "ycp/y2log.h"
#include "ycp/Type.h"
#include "ycp/Bytecode.h"
#include "ycp/Xmlcode.h"
#include "ycp/YCPMap.h"
#include "ycp/YCPCode.h"	// for YT_Code in matchvalue()

#ifndef DO_DEBUG
#define DO_DEBUG 0
#endif

#include <ctype.h>

//----------------------------------------------------------------...
// pointers

IMPL_BASE_POINTER(Type);
IMPL_DERIVED_POINTER(FlexType, Type);
IMPL_DERIVED_POINTER(NFlexType, Type);
IMPL_DERIVED_POINTER(VariableType, Type);
IMPL_DERIVED_POINTER(ListType, Type);
IMPL_DERIVED_POINTER(MapType, Type);
IMPL_DERIVED_POINTER(BlockType, Type);
IMPL_DERIVED_POINTER(TupleType, Type);
IMPL_DERIVED_POINTER(FunctionType, Type);


//----------------------------------------------------------------...
// constants

const constTypePtr Type::Unspec		= TypePtr ( new Type (UnspecT));
const constTypePtr Type::Any		= TypePtr ( new Type (AnyT));

const constTypePtr Type::Void		= TypePtr ( new Type (VoidT));
const constTypePtr Type::Boolean	= TypePtr ( new Type (BooleanT));
const constTypePtr Type::Byteblock	= TypePtr ( new Type (ByteblockT));
const constTypePtr Type::Integer	= TypePtr ( new Type (IntegerT));
const constTypePtr Type::Float		= TypePtr ( new Type (FloatT));
const constTypePtr Type::String		= TypePtr ( new Type (StringT));
const constTypePtr Type::Symbol		= TypePtr ( new Type (SymbolT));
const constTypePtr Type::Term		= TypePtr ( new Type (TermT));
const constTypePtr Type::Locale		= TypePtr ( new Type (LocaleT));
const constTypePtr Type::Path		= TypePtr ( new Type (PathT));
const constTypePtr Type::Wildcard	= TypePtr ( new Type (WildcardT));

const constTypePtr Type::Flex		= TypePtr ( new FlexType());
const constTypePtr Type::NFlex1		= TypePtr ( new NFlexType(1));
const constTypePtr Type::NFlex2		= TypePtr ( new NFlexType(2));
const constTypePtr Type::NFlex3		= TypePtr ( new NFlexType(3));
const constTypePtr Type::NFlex4		= TypePtr ( new NFlexType(4));

const constTypePtr Type::ConstAny	= TypePtr ( new Type (AnyT, true));
const constTypePtr Type::ConstVoid	= TypePtr ( new Type (VoidT, true));
const constTypePtr Type::ConstBoolean	= TypePtr ( new Type (BooleanT, true));
const constTypePtr Type::ConstByteblock	= TypePtr ( new Type (ByteblockT, true));
const constTypePtr Type::ConstInteger	= TypePtr ( new Type (IntegerT, true));
const constTypePtr Type::ConstFloat	= TypePtr ( new Type (FloatT, true));
const constTypePtr Type::ConstString	= TypePtr ( new Type (StringT, true));
const constTypePtr Type::ConstSymbol	= TypePtr ( new Type (SymbolT, true));
const constTypePtr Type::ConstTerm	= TypePtr ( new Type (TermT, true));
const constTypePtr Type::ConstList	= TypePtr ( new Type (ListT, true));
const constTypePtr Type::ConstMap	= TypePtr ( new Type (MapT, true));
const constTypePtr Type::ConstLocale	= TypePtr ( new Type (LocaleT, true));
const constTypePtr Type::ConstPath	= TypePtr ( new Type (PathT, true));

const constTypePtr Type::ConstFlex	= TypePtr ( new FlexType (true));
const constTypePtr Type::ConstNFlex1	= TypePtr ( new NFlexType (1, true));
const constTypePtr Type::ConstNFlex2	= TypePtr ( new NFlexType (2, true));
const constTypePtr Type::ConstNFlex3	= TypePtr ( new NFlexType (3, true));
const constTypePtr Type::ConstNFlex4	= TypePtr ( new NFlexType (4, true));

const constTypePtr Type::Error		= TypePtr ( new Type (ErrorT));

const constTypePtr Type::Nil		= TypePtr ( new Type (NilT));

const constTypePtr Type::Variable	= new VariableType (Type::Any);
const constTypePtr Type::Block 		= new BlockType (Type::Any);
const constTypePtr Type::ListUnspec	= new ListType (Type::Unspec);
const constTypePtr Type::List		= new ListType (Type::Any);
const constTypePtr Type::MapUnspec	= new MapType (Type::Unspec, Type::Unspec);
const constTypePtr Type::Map		= new MapType (Type::Any, Type::Any);

FunctionTypePtr
Type::Function(constTypePtr return_type)
{
    return FunctionTypePtr ( new FunctionType (return_type) );
}

//----------------------------------------------------------------
// constructor / destructor

Type::Type ()
    : m_kind (UnspecT)
{
}

Type::Type (const Type &type)
    : Rep (type), m_kind (type.m_kind)
{
}


Type::~Type()
{
}

//----------------------------------------------------------------
// stream I/O

Type::Type (tkind kind, bytecodeistream & str)
    : m_kind (kind)
    , m_const (Bytecode::readBool (str))
    , m_reference (Bytecode::readBool (str))
{
#if DO_DEBUG
    y2debug ("Type::fromStream (kind %d, const %d, ref %d)", m_kind, m_const, m_reference);
#endif
}


/**
 * write out to bytecode stream
 */

std::ostream &
Type::toStream (std::ostream & str) const
{
#if DO_DEBUG
    y2debug ("Type::toStream ([%d]%s)", m_kind, toString().c_str());
#endif
    Bytecode::writeInt32 (str, m_kind);
    Bytecode::writeBool (str, m_const);
    Bytecode::writeBool (str, m_reference);
    return str;
}


std::ostream &
Type::toXml( std::ostream & str, int /*indent*/ ) const
{
    str << " type=\"" << toXmlString() << "\"";
    if (m_const) str << " const=\"1\"";
    if (m_reference) str << " reference=\"1\"";
    return str;
}

//----------------------------------------------------------------
// Type

string
Type::toString () const
{
    string ret;
    switch (m_kind)
    {
	case UnspecT:	ret = "<unspec>"; break;
	case ErrorT:	ret = "<*ERR*>"; break;
	case AnyT:	ret = "any"; break;
	case BooleanT:	ret = "boolean"; break;
	case ByteblockT: ret = "byteblock"; break;
	case FloatT:	ret = "float"; break;
	case IntegerT:	ret = "integer"; break;
	case LocaleT:	ret = "locale"; break;
	case PathT:	ret = "path"; break;
	case StringT:	ret = "string"; break;
	case SymbolT:	ret = "symbol"; break;
	case TermT:	ret = "term"; break;
	case VoidT:	ret = "void"; break;
	case WildcardT: ret = "..."; break;

	case FlexT:	ret = "<T>"; break;
	case VariableT:	ret = "<VARIABLE>"; break;
	case BlockT:	ret = "<BLOCK>"; break;
	case ListT:	ret = "<LIST>"; break;
	case MapT:	ret = "<MAP>"; break;
	case TupleT:	ret = "<TUPLE>"; break;
	case FunctionT:	ret = "<FUNCTION>"; break;

	case NilT:	ret = "<nil>"; break;
	case NFlexT:	ret = "<Tx>"; break;
	// no default:, let gcc warn
    }
    if (ret.empty()) ret = "<UNHANDLED>";
    return preToString() + ret + postToString();
}


string
Type::toXmlString () const
{
    return Xmlcode::xmlify(toString());
}


// match basic (non-constructed) types
//
// returns 0 if match
//	   -1 if no match
//	   +1 if 'free' (any, wildcard, ...) match

int
Type::basematch (constTypePtr expected) const
{
    tkind ek = expected->m_kind;

#if DO_DEBUG
//    y2debug ("basematch '%s', expected '%s'[%d]", toString().c_str(), expected->toString().c_str(), ek);
#endif

    if (isError()
	|| ek == ErrorT)
    {
#if DO_DEBUG
	y2debug ("Error !");
#endif
	return -1;
    }

    if (expected->isReference()
	&& (!expected->isConst()
	    && isConst()))
    {
#if DO_DEBUG
	y2debug ("doesn't expect const");
#endif
	return -1;
    }

    if (ek == AnyT
	|| ek == FlexT
	|| ek == NFlexT
	|| ek == WildcardT
	|| ek == UnspecT)			// list == list<unspec>
    {
#if DO_DEBUG
//	y2debug ("free match");
#endif
	return 1;
    }

    if (isVoid())				// nil matches everywhere
    {
	return 0;
    }

    if (isBasetype() == expected->isBasetype())
    {
	return 0;
    }

    return -1;
}


int
Type::match (constTypePtr expected) const
{
#if DO_DEBUG
    y2debug ("match '%s', expected '%s'", toString().c_str(), expected->toString().c_str());
#endif
    if (basematch (expected) < 0)
    {
#if DO_DEBUG
	y2debug ("basematch failed");
#endif
	return -1;
    }

    tkind ek = expected->m_kind;

    if (ek == AnyT
	|| ek == FlexT
	|| ek == NFlexT
	|| ek == WildcardT
	|| ek == UnspecT)			// list == list<unspec>
    {
#if DO_DEBUG
	y2debug ("free match");
#endif
	return 0;
    }

    if (m_kind == ek)
    {
#if DO_DEBUG
	y2debug ("m_kind (%d) == expected m_kind (%d)", m_kind, ek);
#endif
	return 0;
    }

    if (isAny()) return -1;		// any does not propagate
    if (isUnspec()) return 0;		// unspecified matches all
    if (isVoid()) return 0;		// nil matches all
    if (isInteger())			// int -> float
    {
	return (ek == FloatT) ? 2 : -1;
    }
    if (isFloat())			// float -> int
    {
	return (ek == IntegerT) ? 1 : -1;
    }
    if (isLocale())			// locale -> string
    {
	return (ek == StringT) ? 0 : -1;
    }
    if (isString())			// string -> locale
    {
	return (ek == LocaleT) ? 0 : -1;
    }
    return -1;
}


int
Type::matchvalue (YCPValue value) const
{
#if DO_DEBUG
    y2debug ("matchvalue type '%s', value '%s'", toString().c_str(), value.isNull()?"NULL":value->toString().c_str());
#endif
    y2debug ("matchvalue type '%s'[%d], value '%s'[%s]",
	     toString().c_str(), m_kind,
	     value.isNull()?"NULL":value->toString().c_str(),
	     value.isNull()?"":value->valuetype_str());

    if (value.isNull()) return -1;			// error value

    if (isAny()) return 0;				// type 'any' matches any value

    int m = -1;

    switch (value->valuetype())
    {
	case YT_VOID:
	    m = 0;					// value 'nil' matches any type
	break;
	case YT_BOOLEAN:
	    if (isBoolean ()) m = 0;
	break;
	case YT_INTEGER:
	    if (isInteger ()) m = 0;
	break;
	case YT_FLOAT:
	    if (isFloat ()) m = 0;
	break;
	case YT_STRING:
	    if (isString ()) m = 0;
	break;
	case YT_BYTEBLOCK:
	    if (isByteblock ()) m = 0;
	break;
	case YT_PATH:
	    if (isPath ()) m = 0;
	break;
	case YT_SYMBOL:
	    if (isSymbol ()) m = 0;
	break;
	case YT_TERM:
	    if (isTerm ()) m = 0;
	break;
	case YT_LIST:
	{
	    if (!isList()) break;

	    m = 0;

	    constTypePtr type = this;
	    constTypePtr element_type = ((constListTypePtr)type)->type();
	    y2debug ("isList, element_type '%s'", element_type->toString().c_str());
	    if (element_type->isAny()			// list<any>
		|| element_type->isUnspec())		// list
	    {
		break;					// -> match
	    }

	    // check every list element
	    YCPList lvalue = value->asList();
	    for (int i = 0; i < lvalue->size(); i++)
	    {
		YCPValue evalue = lvalue->value (i);	// get list element value
		y2debug ("evalue '%s'", evalue->toString().c_str());
		if (element_type->matchvalue (evalue) < 0)
		{
		    y2debug ("Foul");
		    m = -1;
		    break;
		}
	    }
	}
	break;
	case YT_MAP:
	{
	    if (!isMap()) break;

	    m = 0;

	    constTypePtr type = this;
	    constTypePtr key_type = ((constMapTypePtr)type)->keytype();
	    constTypePtr element_type = ((constMapTypePtr)type)->valuetype();
	    y2debug ("isMap, key_type '%s', element_type '%s'", key_type->toString().c_str(), element_type->toString().c_str());
	    if (element_type->isAny()			// map<keytype, any>
		|| element_type->isUnspec())		// map
	    {
		break;					// -> match
	    }

	    // check every map element
	    YCPMap mvalue = value->asMap();
	    for (YCPMap::const_iterator i = mvalue->begin(); i != mvalue->end(); ++i)
	    {
		YCPValue kvalue = i->first;			// get map key value
		YCPValue evalue = i->second;			// get map element value
		y2debug ("kvalue '%s', evalue '%s'", kvalue->toString().c_str(), evalue->toString().c_str());
		if (key_type->matchvalue (kvalue) < 0)
		{
		    y2debug ("Key has bad type");
		    m = -1;
		    break;
		}
		if (element_type->matchvalue (evalue) < 0)
		{
		    y2debug ("Value has bad type");
		    m = -1;
		    break;
		}
	    }
	}
	break;
	case YT_CODE:					// ``{ ... }
	{
	    YCPCode ycpcode = value->asCode();
	    y2debug ("Code (%s)", ycpcode->toString().c_str());
	}
	break;
	case YT_EXTERNAL:				// external entity
	{
	    y2debug ("External payload (%s)", value->toString().c_str());
	}
	break;
	case YT_RETURN:					// { return; }
	{
	    m = 0;					// -> evaluates to 'nil' -> matches every type
	}
	break;
	default:
	break;
    }
    y2debug ("matchvalue -> %d", m);
    return m;
}


bool
Type::canCast (constTypePtr to) const
{
#if DO_DEBUG
    y2debug ("Type::canCast [%s] -> [%s]", toString().c_str(), to->toString().c_str());
#endif
    if (isAny())
    {
	return true;
    }
    return (match (to) >= 0);
}


bool
Type::equals (constTypePtr expected) const
{
#if DO_DEBUG
//    y2debug ("equals '%s', expected '%s'", toString().c_str(), expected->toString().c_str());
#endif

    return (isBasetype() == expected->isBasetype()) && (expected->m_kind == m_kind);
}


TypePtr
Type::clone () const
{
    TypePtr tp = TypePtr (new Type (m_kind));
    return tp;
}


constTypePtr
Type::unflex (constTypePtr type, unsigned int number) const
{
    return clone();
}

//----------------------------------------------------------------
// FlexType

FlexType::FlexType (bool as_const)
    : Type (FlexT, as_const)
{
}


FlexType::FlexType (bytecodeistream & str)
    : Type (FlexT, str)
{
#if DO_DEBUG
    y2debug ("FlexType::FlexType");
#endif
}


FlexType::~FlexType ()
{
}


std::ostream &
FlexType::toStream (std::ostream & str) const
{
    Type::toStream (str);
    return str;
}


string
FlexType::toString () const
{
    return preToString() + "<T>" + postToString();
}


constTypePtr
FlexType::matchFlex (constTypePtr type, unsigned int number) const
{
#if DO_DEBUG
    y2debug ("matchFlex '%s' (%s)", toString().c_str(), type->toString().c_str());
#endif
    if (number != 0)
    {
	return 0;
    }
    if (type->isUnspec ())
    {
	return Type::Any;
    }
    return type;
}


int
FlexType::match (constTypePtr expected) const
{
#if DO_DEBUG
    y2debug ("FlexType::match '%s', expected '%s'", toString().c_str(), expected->toString().c_str());
#endif
    return (expected->isFlex() ? 0 : -1);
}


TypePtr
FlexType::clone () const
{
    FlexTypePtr tp = FlexTypePtr (new FlexType ());
    return tp;
}


constTypePtr
FlexType::unflex (constTypePtr type, unsigned int number) const
{
    TypePtr tp;
    if (isFlex()
	&& number == 0)
    {
	tp = type->clone();
	if (isConst()) tp->asConst();		// keep qualifiers from flex
	if (isReference()) tp->asReference();
    }
    else
    {
	tp = clone();
    }
#if DO_DEBUG
    y2debug ("unflex '%s'%d -%s-> '%s'", toString().c_str(), number, type->toString().c_str(), tp->toString().c_str());
#endif
    return tp;
}

//----------------------------------------------------------------
// NFlexType

NFlexType::NFlexType (unsigned int number, bool as_const)
    : Type (NFlexT, as_const)
    , m_number (number)
{
    if (number == 0) ycp2error ("NFlexType::NFlexType (0)");
}


NFlexType::NFlexType (bytecodeistream & str)
    : Type (NFlexT, str)
    , m_number (Bytecode::readInt32 (str))
{
#if DO_DEBUG
    y2debug ("NFlexType::NFlexType(stream: %d)", m_number);
#endif
}


NFlexType::~NFlexType ()
{
}


std::ostream &
NFlexType::toStream (std::ostream & str) const
{
#if DO_DEBUG
    y2debug ("NFlexType::toStream (%d)", m_number);
#endif
    Type::toStream (str);
    Bytecode::writeInt32 (str, m_number);
    return str;
}


string
NFlexType::toString () const
{
    static char numbuf[8];
    snprintf (numbuf, 8, "%d", m_number);

    return preToString() + "<T" + string (numbuf) + ">" + postToString();
}


constTypePtr
NFlexType::matchFlex (constTypePtr type, unsigned int number) const
{
#if DO_DEBUG
    y2debug ("matchFlex '%s' (%s)", toString().c_str(), type->toString().c_str());
#endif
    if (number == 0
	|| number != m_number)
    {
	return 0;
    }
    if (type->isUnspec ())
    {
	return Type::Any;
    }
    return type;
}


int
NFlexType::match (constTypePtr expected) const
{
#if DO_DEBUG
    y2debug ("NFlexType::match '%s', expected '%s'", toString().c_str(), expected->toString().c_str());
#endif
    return (expected->isFlex() ? 0 : -1);
}


TypePtr
NFlexType::clone () const
{
    NFlexTypePtr tp = NFlexTypePtr (new NFlexType (m_number));
    return tp;
}


unsigned int
NFlexType::number () const
{
    return m_number;
}


constTypePtr
NFlexType::unflex (constTypePtr type, unsigned int number) const
{
    TypePtr tp;
    if (isNFlex()
	&& m_number == number)
    {
	tp = type->clone();
	if (isConst()) tp->asConst();		// keep qualifiers from flex
	if (isReference()) tp->asReference();
    }
    else
    {
	tp = clone();
    }
#if DO_DEBUG
    y2debug ("unflex '%s'%d -%s-> '%s'", toString().c_str(), number, type->toString().c_str(), tp->toString().c_str());
#endif
    return tp;
}

//----------------------------------------------------------------
// VariableType

VariableType::VariableType (constTypePtr type, bool as_const)
    : Type (VariableT, as_const)
    , m_type (type)
{
}


VariableType::VariableType (bytecodeistream & str)
    : Type (VariableT, str)
    , m_type (Bytecode::readType (str))
{
}


VariableType::~VariableType ()
{
}


std::ostream &
VariableType::toStream (std::ostream & str) const
{
    Type::toStream (str);
    Bytecode::writeType (str, m_type);
    return str;
}


string
VariableType::toString () const
{
    return "variable <" + m_type->toString() + ">";
}


constTypePtr
VariableType::matchFlex (constTypePtr type, unsigned int number) const
{
#if DO_DEBUG
    y2debug ("matchFlex '%s' (%s)", toString().c_str(), type->toString().c_str());
#endif
    if (type->isVariable())
    {
	return m_type->matchFlex (((constVariableTypePtr)type)->m_type, number);
    }
    return 0;
}


int
VariableType::match (constTypePtr expected) const
{
#if DO_DEBUG
    y2debug ("match '%s', expected '%s'", toString().c_str(), expected->toString().c_str());
#endif
    int bm = basematch (expected);
    if (bm == 1
	|| (bm == 0
	    && expected->isVariable()
	    && m_type->match (((constVariableTypePtr)expected)->m_type) >= 0))
    {
	return 0;
    }
    return -1;
}


bool
VariableType::equals (constTypePtr expected) const
{
#if DO_DEBUG
//    y2debug ("equals '%s', expected '%s'", toString().c_str(), expected->toString().c_str());
#endif
    if (expected->isVariable()
	    && m_type->equals (((constVariableTypePtr)expected)->m_type))
    {
	return true;
    }
    return false;
}


TypePtr
VariableType::clone () const
{
    VariableTypePtr tp = VariableTypePtr (new VariableType (m_type));
    return tp;
}


constTypePtr
VariableType::unflex (constTypePtr type, unsigned int number) const
{
    TypePtr tp = VariableTypePtr (new VariableType (m_type->unflex (type, number)));
#if DO_DEBUG
    y2debug ("unflex '%s'%d -%s-> '%s'", toString().c_str(), number, type->toString().c_str(), tp->toString().c_str());
#endif
    return tp;
}

//----------------------------------------------------------------
// ListType


ListType::ListType (constTypePtr type, bool as_const)
    : Type (ListT, as_const)
    , m_type (type)
{
}


ListType::ListType (bytecodeistream & str)
    : Type (ListT, str)
    , m_type (Bytecode::readType (str))
{
}


ListType::~ListType ()
{
}


std::ostream &
ListType::toStream (std::ostream & str) const
{
    Type::toStream (str);
    Bytecode::writeType (str, m_type);
    return str;
}


string
ListType::toString () const
{
    string ret = preToString() + "list";

    if (!m_type->isUnspec()
	&& !m_type->isAny())		// list <any> -> list
    {
	ret += " <";
	ret += m_type->toString();
	ret += ">";
    }
    ret += postToString();
    return ret;
}


constTypePtr
ListType::matchFlex (constTypePtr type, unsigned int number) const
{
#if DO_DEBUG
    y2debug ("matchFlex '%s' (%s)", toString().c_str(), type->toString().c_str());
#endif
    if (type->isList())
    {
	return m_type->matchFlex (((constListTypePtr)type)->m_type, number);
    }
    return 0;
}


int
ListType::match (constTypePtr expected) const
{
    int bm = basematch (expected);
#if DO_DEBUG
    y2debug ("match '%s', expected '%s', basematch %d", toString().c_str(), expected->toString().c_str(), bm);
#endif
    if (bm == 1)
    {
	return 0;
    }
    if (bm == 0
	&& expected->isList())
    {
	constListTypePtr lexpected = expected;
	if (lexpected->m_type->isUnspec()				// empty list
	    || m_type->isUnspec()
	    || m_type->match (lexpected->m_type) >= 0)
    {
	return 0;
    }
    }
    return -1;
}

bool
ListType::equals (constTypePtr expected) const
{
#if DO_DEBUG
//    y2debug ("equals '%s', expected '%s'", toString().c_str(), expected->toString().c_str());
#endif
    if (expected->isList()
	    && m_type->equals (((constListTypePtr)expected)->m_type))
    {
	return true;
    }
    return false;
}


/**
 * Finds a type that can hold both given types
 * This should be the narrowest such type - TODO
 */

constTypePtr
ListType::commontype (constTypePtr type) const
{
#if DO_DEBUG
    y2debug ("commontype '%s', '%s'", toString().c_str(), type->toString().c_str());
#endif
    if (type->isVoid()
	|| type->isUnspec())
    {
	return constListTypePtr (this);
    }
    else if (type->isList())
    {
	constListTypePtr listtype = type;
	return ListTypePtr (new ListType (m_type->commontype (listtype->m_type)));
    }

    return Type::Any;
}


/**
 * Finds a type that contains most information
 * This should be the widest such type - TODO
 */

constTypePtr
ListType::detailedtype (constTypePtr type) const
{
#if DO_DEBUG
    y2debug ("ListType::detailedtype '%s', '%s'", toString().c_str(), type->toString().c_str());
#endif
#warning unfinished
    if (type->isVoid()
	|| type->isUnspec())
    {
	return constListTypePtr (this);
    }
    else if (type->isList())
    {
	constListTypePtr listtype = type;
	return ListTypePtr (new ListType (m_type->detailedtype (listtype->m_type)));
    }

    return Type::Error;
}


bool
ListType::canCast (constTypePtr to) const
{
    if (to->isList ()) return m_type->canCast (((constListTypePtr)to)->m_type);
    return false;
}

TypePtr
ListType::clone () const
{
    ListTypePtr tp = ListTypePtr (new ListType (m_type));
    return tp;
}


constTypePtr
ListType::unflex (constTypePtr type, unsigned int number) const
{
    TypePtr tp = ListTypePtr (new ListType (m_type->unflex (type, number)));
#if DO_DEBUG
    y2debug ("unflex '%s'%d -%s-> '%s'", toString().c_str(), number, type->toString().c_str(), tp->toString().c_str());
#endif
    return tp;
}
//----------------------------------------------------------------
// MapType

MapType::MapType (constTypePtr key, constTypePtr value, bool as_const)
    : Type (MapT, as_const)
    , m_keytype (key)
    , m_valuetype (value)
{
}


MapType::MapType (bytecodeistream & str)
    : Type (MapT, str)
    , m_keytype (Bytecode::readType (str))
    , m_valuetype (Bytecode::readType (str))
{
}


MapType::~MapType ()
{
}


std::ostream &
MapType::toStream (std::ostream & str) const
{
    Type::toStream (str);
    Bytecode::writeType (str, m_keytype);
    Bytecode::writeType (str, m_valuetype);
    return str;
}


string
MapType::toString () const
{
    string ret = preToString() + "map";

    // map <<unspec>, <unspec>> -> map
    // map <any, <unspec>> -> map
    // map <any, any> -> map
    if (! (   (m_keytype->isUnspec() || m_keytype->isAny())
	   && (m_valuetype->isUnspec() || m_valuetype->isAny())))
    {
	ret += " <";
	ret += m_keytype->toString();
	ret += ", ";
	ret += m_valuetype->toString();
	ret += ">";
    }
    ret += postToString();
    return ret;
}


constTypePtr
MapType::matchFlex (constTypePtr type, unsigned int number) const
{
#if DO_DEBUG
    y2debug ("matchFlex '%s' (%s)", toString().c_str(), type->toString().c_str());
#endif
    if (!type->isMap())
    {
	return 0;
    }
    constTypePtr flex = m_valuetype->matchFlex (((constMapTypePtr)type)->m_valuetype, number);
    if (flex)
    {
	return flex;
    }
    return m_keytype->matchFlex (((constMapTypePtr)type)->m_keytype, number);
}


int
MapType::match (constTypePtr expected) const
{
    int bm = basematch (expected);
#if DO_DEBUG
    y2debug ("match '%s', expected '%s', basematch %d", toString().c_str(), expected->toString().c_str(), bm);
#endif
    if (bm == 1)
    {
	return 0;
    }

    if (bm == 0
	&& expected->isMap())
    {
	// check if the expected map is more detailed

	constMapTypePtr mexpected = (constMapTypePtr)expected;
	if (   (m_keytype->isUnspec()					// this is a constant $[]
		|| m_keytype->match (mexpected->m_keytype) >= 0)
	    && (m_valuetype->isUnspec()					// this is a constant $[]
		|| m_valuetype->match (mexpected->m_valuetype) >= 0))
	{
	    return 0;
	}
    }
    return -1;
}

bool
MapType::equals (constTypePtr expected) const
{
#if DO_DEBUG
//    y2debug ("equals '%s', expected '%s'", toString().c_str(), expected->toString().c_str());
#endif
    if (expected->isMap()
	    && m_valuetype->equals (((constMapTypePtr)expected)->m_valuetype)
	    && m_keytype->equals (((constMapTypePtr)expected)->m_keytype) )
    {
	return true;
    }
    return false;
}


/**
 * Finds a type that can hold both given types
 * This should be the narrowest such type - TODO
 */

constTypePtr
MapType::commontype (constTypePtr type) const
{
#if DO_DEBUG
    y2debug ("commontype '%s', '%s'", toString().c_str(), type->toString().c_str());
#endif
    if (type->isVoid()
	|| type->isUnspec())
    {
	return constMapTypePtr (this);
    }
    else if (type->isMap())
    {
	constMapTypePtr maptype = type;
	return MapTypePtr (new MapType (m_keytype->commontype (maptype->m_keytype), m_valuetype->commontype (maptype->m_valuetype)));
    }

    return Type::Any;
}


/**
 * Finds a type which contains most information
 */

constTypePtr
MapType::detailedtype (constTypePtr type) const
{
#if DO_DEBUG
    y2debug ("MapType::detailedtype '%s', '%s'", toString().c_str(), type->toString().c_str());
#endif
#warning not implemented
    if (type->isVoid()
	|| type->isUnspec())
    {
	return constMapTypePtr (this);
    }
    else if (type->isMap())
    {
	constMapTypePtr maptype = type;
	return MapTypePtr (new MapType (m_keytype->detailedtype (maptype->m_keytype), m_valuetype->detailedtype (maptype->m_valuetype)));
    }

    return Type::Error;
}


bool
MapType::canCast (constTypePtr to) const
{
#if DO_DEBUG
//    y2debug ("canCast '%s' to '%s'", toString().c_str(), to->toString().c_str());
#endif
    if (to->isMap ())
	return (m_keytype->canCast (((constMapTypePtr)to)->m_keytype))
	    && (m_valuetype->canCast (((constMapTypePtr)to)->m_valuetype));
    return false;
}


TypePtr
MapType::clone () const
{
    MapTypePtr tp = MapTypePtr (new MapType (m_keytype, m_valuetype));
    return tp;
}


constTypePtr
MapType::unflex (constTypePtr type, unsigned int number) const
{
    TypePtr tp;
    if (m_keytype->isFlex())
    {
	tp = MapTypePtr (new MapType (m_keytype->unflex (type, number), m_valuetype));
    }
    else if (m_valuetype->isFlex())
    {
	tp = MapTypePtr (new MapType (m_keytype, m_valuetype->unflex (type, number)));
    }
    else
    {
	tp = clone();
    }

#if DO_DEBUG
    y2debug ("unflex '%s'%d -%s-> '%s'", toString().c_str(), number, type->toString().c_str(), tp->toString().c_str());
#endif
    return tp;
}

//----------------------------------------------------------------
// BlockType

BlockType::BlockType (constTypePtr type, bool as_const)
    : Type (BlockT, as_const)
    , m_type (type)
{
}


BlockType::BlockType (bytecodeistream & str)
    : Type (BlockT, str)
    , m_type (Bytecode::readType (str))
{
}


BlockType::~BlockType ()
{
}


std::ostream &
BlockType::toStream (std::ostream & str) const
{
    Type::toStream (str);
    Bytecode::writeType (str, m_type);
    return str;
}


string
BlockType::toString () const
{
    return preToString()
	   + "block <" + m_type->toString() + ">"
	   + postToString();
}


constTypePtr
BlockType::matchFlex (constTypePtr type, unsigned int number) const
{
#if DO_DEBUG
    y2debug ("matchFlex '%s' (%s)", toString().c_str(), type->toString().c_str());
#endif
    if (type->isBlock())
    {
	return m_type->matchFlex (((constBlockTypePtr)type)->m_type, number);
    }
    return 0;
}


int
BlockType::match (constTypePtr expected) const
{
#if DO_DEBUG
    y2debug ("match '%s', expected '%s'", toString().c_str(), expected->toString().c_str());
#endif
    int bm = basematch (expected);
    if (bm == 1
	|| (bm == 0
	    && expected->isBlock()
	    && m_type->match (((constBlockTypePtr)expected)->m_type) >= 0))
    {
	return 0;
    }
    return -1;
}


bool
BlockType::equals (constTypePtr expected) const
{
#if DO_DEBUG
//    y2debug ("equals '%s', expected '%s'", toString().c_str(), expected->toString().c_str());
#endif
    if (expected->isBlock()
	    && m_type->equals (((constBlockTypePtr)expected)->m_type) )
    {
	return true;
    }
    return false;
}


bool
BlockType::canCast (constTypePtr to) const
{
    if (to->isBlock ()) return m_type->canCast (((constBlockTypePtr)to)->m_type);
    return false;
}

TypePtr
BlockType::clone () const
{
    BlockTypePtr tp = BlockTypePtr (new BlockType (m_type));
    return tp;
}


constTypePtr
BlockType::unflex (constTypePtr type, unsigned int number) const
{
    TypePtr tp = BlockTypePtr (new BlockType (m_type->unflex (type, number)));
#if DO_DEBUG
    y2debug ("unflex '%s'%d -%s-> '%s'", toString().c_str(), number, type->toString().c_str(), tp->toString().c_str());
#endif
    return tp;
}

//----------------------------------------------------------------
// TupleType

TupleType::TupleType (constTypePtr type, bool as_const)
    : Type (TupleT, as_const)
{
    concat (type);
}


TupleType::TupleType (bytecodeistream & str)
    : Type (TupleT, str)
{
    u_int32_t count = Bytecode::readInt32 (str);
#if DO_DEBUG
    y2debug ("TupleType::TupleType %d elements", count);
#endif
    for (unsigned index = 0; index < count; index++)
    {
	m_types.push_back (Bytecode::readType (str));
    }
#if DO_DEBUG
    y2debug ("TupleType::TupleType done");
#endif
}


TupleType::~TupleType()
{
}


std::ostream &
TupleType::toStream (std::ostream & str) const
{
    Type::toStream (str);
    Bytecode::writeInt32 (str, m_types.size());
    for (unsigned index = 0; index < m_types.size(); index++)
    {
	Bytecode::writeType (str, m_types[index]);
    }
    return str;
}


void
TupleType::concat (constTypePtr t)
{
    m_types.push_back (t);
    return;
}


constTypePtr
TupleType::parameterType (unsigned int parameter_number) const
{
    if (parameter_number >= m_types.size())
    {
	return Type::Error;
    }
    return m_types[parameter_number];
}


string
TupleType::toString () const
{
    string ret = preToString() + "(";

    for (unsigned index = 0; index < m_types.size(); index++)
    {
	if (index != 0) ret += ", ";
	ret += m_types[index]->toString();
    }
    ret += ")";
    ret += postToString();
    return ret;
}


constTypePtr
TupleType::matchFlex (constTypePtr type, unsigned int number) const
{
#if DO_DEBUG
    y2debug ("matchFlex '%s' (%s)", toString().c_str(), type->toString().c_str());
#endif
    if (!type->isTuple())
    {
	return 0;
    }

    constTupleTypePtr tt = (constTupleTypePtr)type;
    for (unsigned index = 0; index < m_types.size(); index++)
    {
	constTypePtr t = tt->parameterType (index);
	if (t->isError())
	{
	    break;
	}
	t = m_types[index]->matchFlex (t, number);
	if (t != 0)
	{
	    return t;
	}
    }
    return 0;
}


int
TupleType::match (constTypePtr expected) const
{
#if DO_DEBUG
    y2debug ("match '%s', expected '%s'", toString().c_str(), expected->toString().c_str());
#endif
    int bm = basematch (expected);

    if (bm == 1)
    {
	return 0;
    }

    if (bm == 0
	    && expected->isTuple())
    {
	constTupleTypePtr tt = expected;
	unsigned int esize = tt->m_types.size();
	bool wildcard = tt->m_types[esize-1]->isWildcard();
	for (unsigned index = 0; index < m_types.size(); index++)
	{
	    if (index > esize
		&& wildcard)
	    {
		break;
	    }
	    if (m_types[index]->match (tt->m_types[index]) < 0)
	    {
		return -1;
	    }
	}
	return 0;
    }
    return -1;
}


bool
TupleType::equals (constTypePtr expected) const
{
#if DO_DEBUG
//    y2debug ("equals '%s', expected '%s'", toString().c_str(), expected->toString().c_str());
#endif
    if (expected->isTuple())
    {
	const TupleType & tt = (const TupleType)expected;
	unsigned int esize = tt.m_types.size();

	if (esize != m_types.size ())
	{
	    return false;
	}

	for (unsigned index = 0; index < m_types.size(); index++)
	{
	    if (m_types[index]->equals (tt.m_types[index]))
	    {
		return false;
	    }
	}
	return true;
    }
    return false;
}


bool
TupleType::canCast (constTypePtr to) const
{
    if (to->isTuple ())
    {
	const TupleType & tt = (const TupleType)to;
	unsigned int esize = tt.m_types.size();
	bool wildcard = tt.m_types[esize-1]->isWildcard();
	for (unsigned index = 0; index < m_types.size(); index++)
	{
	    if (index > esize
		&& wildcard)
	    {
		break;
	    }
	    if (!m_types[index]->canCast (tt.m_types[index]))
	    {
		return false;
	    }
	}
	return true;
    }
    return false;
}


TypePtr
TupleType::clone () const
{
    TupleTypePtr tp = TupleTypePtr (new TupleType (m_types[0]));
    for (unsigned index = 1; index < m_types.size(); index++)
    {
	tp->concat (m_types[index]);
    }
    return tp;
}


constTypePtr
TupleType::unflex (constTypePtr type, unsigned int number) const
{
    TupleTypePtr tp = TupleTypePtr (new TupleType (m_types[0]->unflex (type, number)));
    for (unsigned index = 1; index < m_types.size(); index++)
    {
	tp->concat (m_types[index]->unflex (type, number));
    }
#if DO_DEBUG
    y2debug ("unflex '%s'%d -%s-> '%s'", toString().c_str(), number, type->toString().c_str(), tp->toString().c_str());
#endif
    return tp;
}

//----------------------------------------------------------------
// Function <returntype, arg1type, arg2type, ...>

FunctionType::FunctionType (constTypePtr returntype, bool as_const)
    : Type (FunctionT, as_const)
    , m_returntype (returntype)
{
}

FunctionType::FunctionType (constTypePtr return_type, constFunctionTypePtr arguments)
    : Type (FunctionT)
    , m_returntype (return_type)
{
    for (int i = 0; i < arguments->parameterCount (); i++)
    {
	concat (arguments->parameterType (i));
    }
}

FunctionType::FunctionType (bytecodeistream & str)
    : Type (FunctionT, str)
    , m_returntype (Bytecode::readType (str))
{
#if DO_DEBUG
    y2debug ("FunctionType::FunctionType (stream)");
#endif
    if (Bytecode::readBool (str))
    {
	m_arguments = Bytecode::readType (str);
    }
#if DO_DEBUG
    y2debug ("FunctionType::fromStream (m_returntype %p, m_arguments %p)", (const void *)m_returntype, (const void *)m_arguments);
#endif
}


std::ostream &
FunctionType::toStream (std::ostream & str) const
{
    Type::toStream (str);
#if DO_DEBUG
    y2debug ("FunctionType::toStream (m_returntype %p, m_arguments %p", (const void *)m_returntype, (const void *)m_arguments);
#endif
    Bytecode::writeType (str, m_returntype);
    if (m_arguments)
    {
	Bytecode::writeBool (str, true);
	Bytecode::writeType (str, m_arguments);
    }
    else
    {
	Bytecode::writeBool (str, false);
    }
    return str;
}


FunctionType::~FunctionType ()
{
}


void
FunctionType::concat (constTypePtr t)
{
    if (!m_arguments)
    {
	m_arguments = TupleTypePtr (new TupleType (t));
    }
    else
    {
	m_arguments->concat (t);
    }
    return;
}


int
FunctionType::parameterCount () const
{
    if (!m_arguments)
    {
	return 0;
    }
    return m_arguments->parameterCount();
}


constTypePtr
FunctionType::parameterType (unsigned int parameter_number) const
{
    if (!m_arguments)
    {
	return Type::Error;
    }
    return m_arguments->parameterType (parameter_number);
}


constTupleTypePtr
FunctionType::parameters () const
{
    return m_arguments;
}


string
FunctionType::toString () const
{
    string ret = preToString() + m_returntype->toString () + " (";

    if (m_arguments)
    {
	for (unsigned int index = 0; index < m_arguments->parameterCount(); index++)
	{
	    if (index > 0)
	    {
		ret += ", ";
	    }
	    ret += m_arguments->parameterType(index)->toString();
	}
    }

    ret += ")";
    ret += postToString();
    return ret;
}


constTypePtr
FunctionType::matchFlex (constTypePtr type, unsigned int number) const
{
#if DO_DEBUG
    y2debug ("matchFlex '%s' (%s)", toString().c_str(), type->toString().c_str());
#endif
    if (!type->isFunction())
    {
	return 0;
    }
    if (!m_arguments)
    {
	return 0;
    }
    constFunctionTypePtr ft = (constFunctionTypePtr)type;
    if (!ft->m_arguments)
    {
	return 0;
    }
    return m_arguments->matchFlex (ft->m_arguments, number);
}


int
FunctionType::match (constTypePtr expected) const
{
#if DO_DEBUG
    y2debug ("FunctionType::match '%s', expected '%s'", toString().c_str(), expected->toString().c_str());
#endif
    int bm = basematch (expected);

    if (bm == 1) return 0;		// any or wildcard match

    if ((bm == 0
	    && expected->isFunction()))
    {
	constFunctionTypePtr ft (expected);
	if (ft->m_returntype->match (m_returntype) < 0)
	{
#if DO_DEBUG
//	    y2debug ("return type mismatch");
#endif
	    return -1;
	}

	// expected parameter size
	unsigned int esize = ft->parameterCount();

	if ((esize == 0)						// both (void) ?
	    && (m_arguments == 0))
	{
	    return 0;
	}

	bool wildcard = ft->parameterType(esize-1)->isWildcard();	// expect ... as last arg ?
	if (!wildcard)
	{
	    if ((esize != 0 && m_arguments == NULL) || (esize != m_arguments->parameterCount()))
	    {
#if DO_DEBUG
//		y2debug ("parameter count mismatch");
#endif
		return -1;
	    }
	}

	for (unsigned index = 0; index < m_arguments->parameterCount(); index++)
	{
	    if (index > esize			// don't check wildcard parameters
		&& wildcard)
	    {
		break;
	    }
	    if (m_arguments->parameterType(index)->match (ft->parameterType(index)) < 0)
	    {
		return -1;
	    }
	}
	return 0;
    }
    return -1;
}


bool
FunctionType::equals (constTypePtr expected) const
{
#if DO_DEBUG
//    y2debug ("FunctionType::equals '%s', expected '%s'", toString().c_str(), expected->toString().c_str());
#endif
    if (expected->isFunction())
    {
	constFunctionTypePtr ft (expected);
	if (! m_returntype->equals (ft->m_returntype))
	{
#if DO_DEBUG
//	    y2debug ("return type mismatch");
#endif
	    return false;
	}

	// expected parameter size
	unsigned int esize = ft->parameterCount();

	if ((esize == 0)						// both (void) ?
	    && (m_arguments == 0))
	{
	    return true;
	}

	for (unsigned index = 0; index < m_arguments->parameterCount(); index++)
	{
	    if (! m_arguments->parameterType(index)->equals (ft->parameterType(index)))
	    {
#if DO_DEBUG
//		y2debug ("parameter %d type mismatch", index);
#endif
		return false;
	    }
	}
	return true;
    }
    return false;
}


TypePtr
FunctionType::clone () const
{
    FunctionTypePtr tp = FunctionTypePtr (new FunctionType (m_returntype));
    if (m_arguments != 0)
    {
	for (unsigned index = 0; index < m_arguments->parameterCount(); index++)
	{
	    tp->concat (m_arguments->parameterType (index));
	}
    }
    return tp;
}


constTypePtr
FunctionType::unflex (constTypePtr type, unsigned int number) const
{
    FunctionTypePtr tp = FunctionTypePtr (new FunctionType (m_returntype->unflex (type, number)));
    if (m_arguments != 0)
    {
	for (unsigned index = 0; index < m_arguments->parameterCount (); index++)
	{
	    tp->concat (m_arguments->parameterType(index)->unflex (type, number));
	}
    }
#if DO_DEBUG
    y2debug ("unflex '%s'%d -%s-> '%s'", toString().c_str(), number, type->toString().c_str(), tp->toString().c_str());
#endif
    return tp;
}

//----------------------------------------------------------------

YCPValueType
Type::valueType () const
{
    switch (m_kind)
    {
	case UnspecT:	return YT_ERROR;
	case VoidT:	return YT_VOID;
	case AnyT:	return YT_VOID;
	case BooleanT:	return YT_BOOLEAN;
	case IntegerT:	return YT_INTEGER;
	case FloatT:	return YT_FLOAT;
	case StringT:	return YT_STRING ;
	case ByteblockT:return YT_BYTEBLOCK;
	case PathT:	return YT_PATH;
	case TermT:	return YT_TERM;
	case SymbolT:	return YT_SYMBOL;
	case ErrorT:	return YT_ERROR;
	case LocaleT:	return YT_STRING;
	case WildcardT: return YT_ERROR;
	case FlexT:	return YT_ERROR;
	case NFlexT:	return YT_ERROR;

	case VariableT: return YT_ENTRY;
	case BlockT:	return YT_CODE;
	case ListT:	return YT_LIST;
	case MapT:	return YT_MAP;
	case TupleT:	return YT_ERROR;	// not expressable
	case FunctionT:	return YT_ERROR;	// not expressable

	case NilT:	return YT_VOID;
    }

    return YT_ERROR;
}


/**
 * Finds a type that can hold both given types
 * This should be the narrowest such type - TODO
 */

constTypePtr
Type::commontype (constTypePtr type) const
{
    if (isVoid())		// 'nil' does not influence the type
    {
	return type;
    }

    if (match (type) >= 0)
    {
#if DO_DEBUG
	y2debug ("commontype '%s'* ('%s')", toString().c_str(), type->toString().c_str());
#endif
	return this;
    }
    else if (!isAny()
	     && type->match (this) >= 0)
    {
#if DO_DEBUG
    y2debug ("commontype '%s' ('%s')*", toString().c_str(), type->toString().c_str());
#endif
	return type;
    }
#if DO_DEBUG
    y2debug ("commontype '%s' ('%s') -> any", toString().c_str(), type->toString().c_str());
#endif
    return Type::Any;
}


/**
 * Finds a type which contains most information
 * This should be the narrowest such type - TODO
 */

constTypePtr
Type::detailedtype (constTypePtr type) const
{
#if DO_DEBUG
    y2debug ("Type::detailedtype '%s' ('%s')", toString().c_str(), type->toString().c_str());
#endif

    if (isVoid())			// 'nil' does not contain type information
    {
#if DO_DEBUG
	y2debug ("Type::detailedtype '%s' ('%s')*", toString().c_str(), type->toString().c_str());
#endif
	return type;
    }
    else if (type->isVoid())		// 'nil' does not contain type information
    {
#if DO_DEBUG
	y2debug ("Type::detailedtype '%s'* ('%s')", toString().c_str(), type->toString().c_str());
#endif
	return this;
    }
    else if (type->isUnspec())		// <unspec> does not contain type information
    {
#if DO_DEBUG
	y2debug ("Type::detailedtype '%s'* ('%s')", toString().c_str(), type->toString().c_str());
#endif
	return this;
    }
    else if (type->isAny())		// 'any' does not contain type information
    {
#if DO_DEBUG
	y2debug ("Type::detailedtype '%s'* ('%s')", toString().c_str(), type->toString().c_str());
#endif
	return this;
    }
    else if (isUnspec())		// <unspec> does not contain type information
    {
#if DO_DEBUG
	y2debug ("Type::detailedtype '%s' ('%s')*", toString().c_str(), type->toString().c_str());
#endif
	return type;
    }
    else if (isAny())			// 'any' does not contain type information
    {
#if DO_DEBUG
	y2debug ("Type::detailedtype '%s' ('%s')*", toString().c_str(), type->toString().c_str());
#endif
	return type;
    }
    else if (match (type) >= 0)		// if this matches the expected type, the latter is more detailed
    {
#if DO_DEBUG
	y2debug ("Type::detailedtype '%s' ('%s')*", toString().c_str(), type->toString().c_str());
#endif
	return type;
    }
    else if (type->match (this) >= 0)	// if type matches the expected this, the latter is more detailed
    {
#if DO_DEBUG
	y2debug ("Type::detailedtype '%s'* ('%s')", toString().c_str(), type->toString().c_str());
#endif
	return this;
    }

#if DO_DEBUG
    y2debug ("Type::detailedtype '%s' ('%s') -> error", toString().c_str(), type->toString().c_str());
#endif
    return Type::Error;
}

// EOF
