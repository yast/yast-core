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

$Id$
/-*/

#include "ycp/y2log.h"
#include "ycp/Type.h"
#include "ycp/Bytecode.h"

#ifndef DO_DEBUG
#define DO_DEBUG 1
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


VariableTypePtr
Type::Variable()
{
    return VariableTypePtr ( new VariableType (Type::Unspec) );
}


ListTypePtr
Type::List()
{
    return ListTypePtr ( new ListType (Type::Unspec) );
}


MapTypePtr
Type::Map()
{
    return MapTypePtr ( new MapType (Type::Any, Type::Unspec) );
}


BlockTypePtr
Type::Block()
{
    return BlockTypePtr ( new BlockType (Type::Unspec) );
}


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
    : m_kind (type.m_kind)
{
}


Type::~Type()
{
}

//----------------------------------------------------------------
// stream I/O

Type::Type (tkind kind, std::istream & str)
    : m_kind (kind)
    , m_const (Bytecode::readBool (str))
    , m_reference (Bytecode::readBool (str))
{
    y2debug ("Type::fromStream (kind %d, const %d, ref %d)", m_kind, m_const, m_reference);
}


/**
 * write out to stream
 */

std::ostream &
Type::toStream (std::ostream & str) const
{
    y2debug ("Type::toStream ([%d]%s)", m_kind, toString().c_str());
    Bytecode::writeInt32 (str, m_kind);
    Bytecode::writeBool (str, m_const);
    Bytecode::writeBool (str, m_reference);
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


// match basic (non-constructed) types
//
// returns 0 if match
//	   -1 if no match
//	   +1 if 'free' (any, wildcard, ...) match

int
Type::basematch (constTypePtr expected) const
{
    tkind ek = expected->m_kind;

//    y2debug ("basematch '%s', expected '%s'[%d]", toString().c_str(), expected->toString().c_str(), ek);

    if (m_kind == ErrorT
	|| ek == ErrorT)
    {
	y2debug ("Error !");
	return -1;
    }

    if (expected->isReference()
	&& (!expected->isConst()
	    && isConst()))
    {
	y2debug ("doesn't expect const");
	return -1;
    }

    if (ek == AnyT
	|| ek == FlexT
	|| ek == NFlexT
	|| ek == WildcardT
	|| ek == UnspecT)			// list == list<unspec>
    {
//	y2debug ("free match");
	return 1;
    }

    if (m_kind == VoidT)			// nil matches everywhere
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
    if (m_nocheck)
    {
	return 0;
    }

//    y2debug ("match '%s', expected '%s'", toString().c_str(), expected->toString().c_str());
    if (basematch (expected) < 0)
    {
//	y2debug ("basematch failed");
	return -1;
    }

    tkind ek = expected->m_kind;

    if (ek == AnyT
	|| ek == FlexT
	|| ek == NFlexT
	|| ek == WildcardT
	|| ek == UnspecT)			// list == list<unspec>
    {
//	y2debug ("free match");
	return 0;
    }

    if (m_kind == ek)
    {
	return 0;
    }

    if (m_kind == AnyT) return -1;		// any does not propagate
    if (m_kind == VoidT) return 0;		// nil matches all
    if (m_kind == IntegerT)			// int -> float
    {
	return (ek == FloatT) ? 2 : -1;
    }
    if (m_kind == FloatT)			// float -> int
    {
	return (ek == IntegerT) ? 1 : -1;
    }
    if (m_kind == LocaleT)			// locale -> string
    {
	return (ek == StringT) ? 0 : -1;
    }
    if (m_kind== StringT)			// string -> locale
    {
	return (ek == LocaleT) ? 0 : -1;
    }
    return -1;
}


bool
Type::equals (constTypePtr expected) const
{
    if (m_nocheck)
	return true;

//    y2debug ("equals '%s', expected '%s'", toString().c_str(), expected->toString().c_str());

    return (isBasetype() == expected->isBasetype()) && (expected->m_kind == m_kind);
}


TypePtr
Type::clone () const
{
    TypePtr tp = TypePtr (new Type (m_kind));
    return tp;
}


TypePtr
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


FlexType::FlexType (std::istream & str)
    : Type (FlexT, str)
{
    y2debug ("FlexType::FlexType");
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
    y2debug ("FlexType::match '%s', expected '%s'", toString().c_str(), expected->toString().c_str());

    return (expected->isFlex() ? 0 : -1);
}


TypePtr
FlexType::clone () const
{
    FlexTypePtr tp = FlexTypePtr (new FlexType ());
    return tp;
}


TypePtr
FlexType::unflex (constTypePtr type, unsigned int number) const
{
    TypePtr tp;
    if (m_kind == FlexT
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


NFlexType::NFlexType (std::istream & str)
    : Type (NFlexT, str)
    , m_number (Bytecode::readInt32 (str))
{
    y2debug ("NFlexType::NFlexType(stream: %d)", m_number);
}


NFlexType::~NFlexType ()
{
}


std::ostream &
NFlexType::toStream (std::ostream & str) const
{
    y2debug ("NFlexType::toStream (%d)", m_number);
    Type::toStream (str);
    Bytecode::writeInt32 (str, m_number);
    return str;
}


string
NFlexType::toString () const
{
    static char numbuf[8];
    sprintf (numbuf, "%d", m_number);

    return preToString() + "<T" + string (numbuf) + ">" + postToString();
}


constTypePtr
NFlexType::matchFlex (constTypePtr type, unsigned int number) const
{
#if DO_DEBUG
    y2debug ("matchFlex '%s' (%s)", toString().c_str(), type->toString().c_str());
#endif
    if (number != 0
	&& number != m_number)
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
    y2debug ("NFlexType::match '%s', expected '%s'", toString().c_str(), expected->toString().c_str());

    return (expected->isFlex() ? 0 : -1);
}


TypePtr
NFlexType::clone () const
{
    NFlexTypePtr tp = NFlexTypePtr (new NFlexType (m_number));
    return tp;
}


TypePtr
NFlexType::unflex (constTypePtr type, unsigned int number) const
{
    TypePtr tp;
    if (m_kind == NFlexT
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


VariableType::VariableType (std::istream & str)
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
    if (m_kind == FlexT)
    {
	return type;
    }
    return 0;
}


int
VariableType::match (constTypePtr expected) const
{
//    y2debug ("match '%s', expected '%s'", toString().c_str(), expected->toString().c_str());

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
//    y2debug ("equals '%s', expected '%s'", toString().c_str(), expected->toString().c_str());

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


TypePtr
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


ListType::ListType (std::istream & str)
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
//    y2debug ("match '%s', expected '%s'", toString().c_str(), expected->toString().c_str());

    int bm = basematch (expected);
    if (bm == 1
	|| (bm == 0
	    && expected->isList()
	    && (m_type->isUnspec()				// empty list
		|| m_type->match (((constListTypePtr)expected)->m_type) >= 0)))
    {
	return 0;
    }
    return -1;
}

bool
ListType::equals (constTypePtr expected) const
{
//    y2debug ("equals '%s', expected '%s'", toString().c_str(), expected->toString().c_str());

    if (expected->isList()
	    && m_type->equals (((constListTypePtr)expected)->m_type))
    {
	return true;
    }
    return false;
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


TypePtr
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


MapType::MapType (std::istream & str)
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

    // map <any, <unspec>> -> map

    if (!m_keytype->isAny()
	&& !m_valuetype->isUnspec())
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
//    y2debug ("match '%s', expected '%s'", toString().c_str(), expected->toString().c_str());

    int bm = basematch (expected);
    if (bm == 1
	|| (bm == 0
	    && expected->isMap()
	    && (m_keytype->isUnspec() || m_keytype->match (((constMapTypePtr)expected)->m_keytype) >= 0)
	    && (m_valuetype->isUnspec()					// empty map
		|| m_valuetype->match (((constMapTypePtr)expected)->m_valuetype) >= 0)))
    {
	return 0;
    }
    return -1;
}

bool
MapType::equals (constTypePtr expected) const
{
//    y2debug ("equals '%s', expected '%s'", toString().c_str(), expected->toString().c_str());

    if (expected->isMap()
	    && m_valuetype->equals (((constMapTypePtr)expected)->m_valuetype)
	    && m_keytype->equals (((constMapTypePtr)expected)->m_keytype) )
    {
	return true;
    }
    return false;
}


bool
MapType::canCast (constTypePtr to) const
{
//    y2debug ("canCast '%s' to '%s'", toString().c_str(), to->toString().c_str());

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


TypePtr
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


BlockType::BlockType (std::istream & str)
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
//    y2debug ("match '%s', expected '%s'", toString().c_str(), expected->toString().c_str());

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
//    y2debug ("equals '%s', expected '%s'", toString().c_str(), expected->toString().c_str());

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


TypePtr
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


TupleType::TupleType (std::istream & str)
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
    if ((parameter_number < 0)
	|| (parameter_number >= m_types.size()))
    {
	return Type::Error;
    }
    return m_types[parameter_number];
}


string
TupleType::toString () const
{
    string ret = preToString() + "tuple <";

    for (unsigned index = 0; index < m_types.size(); index++)
    {
	if (index != 0) ret += ", ";
	ret += m_types[index]->toString();
    }
    ret += ">";
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
//    y2debug ("match '%s', expected '%s'", toString().c_str(), expected->toString().c_str());

    int bm = basematch (expected);

    if (bm == 1)
	return 0;

    if (bm == 0
	    && expected->isTuple())
    {
	const TupleType & tt = (const TupleType)expected;
	unsigned int esize = tt.m_types.size();
	bool wildcard = tt.m_types[esize-1]->isWildcard();
	for (unsigned index = 0; index < m_types.size(); index++)
	{
	    if (index > esize
		&& wildcard)
	    {
		break;
	    }
	    if (m_types[index]->match (tt.m_types[index]) < 0)
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
//    y2debug ("equals '%s', expected '%s'", toString().c_str(), expected->toString().c_str());

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


TypePtr
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

FunctionType::FunctionType (std::istream & str)
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
//    y2debug ("FunctionType::match '%s', expected '%s'", toString().c_str(), expected->toString().c_str());

    int bm = basematch (expected);

    if (bm == 1) return 0;		// any or wildcard match

    if ((bm == 0
	    && expected->isFunction()))
    {
	constFunctionTypePtr ft (expected);
	if (m_returntype->match (ft->m_returntype) < 0)
	{
//	    y2debug ("return type mismatch");
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
//		y2debug ("parameter count mismatch");
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
//    y2debug ("FunctionType::equals '%s', expected '%s'", toString().c_str(), expected->toString().c_str());

    if (expected->isFunction())
    {
	constFunctionTypePtr ft (expected);
	if (! m_returntype->equals (ft->m_returntype))
	{
//	    y2debug ("return type mismatch");
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
//		y2debug ("parameter %d type mismatch", index);
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


TypePtr
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
    if (match (type) >= 0)
    {
#if DO_DEBUG
//y2debug ("commontype '%s'* ('%s')", toString().c_str(), type->toString().c_str());
#endif
	return this;
    }
    else if (!isAny()
	     && type->match (this) >= 0)
    {
#if DO_DEBUG
//y2debug ("commontype '%s' ('%s')*", toString().c_str(), type->toString().c_str());
#endif
	return type;
    }
#if DO_DEBUG
//y2debug ("commontype '%s' ('%s') any", toString().c_str(), type->toString().c_str());
#endif
    return Type::Any;
}

// EOF
