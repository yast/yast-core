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

   File:       Bytecode.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/

#ifndef DO_DEBUG
#define DO_DEBUG 0
#endif

// MAJOR and MINOR number must the the same in header, RELEASE is assumed to
// provide a backward compatibility
#define YaST_BYTECODE_HEADER "YaST bytecode "
#define YaST_BYTECODE_MAJOR "1"
#define YaST_BYTECODE_MINOR "4"
#define YaST_BYTECODE_RELEASE "0"

#include "ycp/Bytecode.h"
#include "YCP.h"
#include "ycp/YCode.h"
#include "ycp/YExpression.h"
#include "ycp/YStatement.h"
#include "ycp/YBlock.h"

#include "y2/Y2Namespace.h"

#include "ycp/y2log.h"
#include "ycp/pathsearch.h"

#include <fstream>
#include <errno.h>
#include <string.h>
#include <ctype.h>

static int
readInt (bytecodeistream & str)
{
    int i = 0;

    char c;

    for (;;)
    {
	str.get (c);
	if (!isdigit (c))
	    break;
	i *= 10;
	i += (c - '0');
    }

    return i;
}


bytecodeistream::bytecodeistream (string filename)
    : std::ifstream (filename.c_str ())
    , m_major (-1)
    , m_minor (-1)
    , m_release (-1)
{
    if (!is_open ())
    {
	y2error ("Failed to open '%s': %s", filename.c_str(), strerror (errno));
	return;
    }
    // read YaST_BYTECODE_HEADER

    char header[sizeof(YaST_BYTECODE_HEADER)+1];
    int headerlen = strlen (YaST_BYTECODE_HEADER);
    read (header, headerlen);
    header[headerlen] = 0;
    if (strcmp (header, YaST_BYTECODE_HEADER) != 0)
    {
	y2error ("Not a bytecode file '%s'[%s]", filename.c_str(), header);
	return;
    }

    m_major = readInt (*this);
    m_minor = readInt (*this);
    m_release = readInt (*this);
}

bool bytecodeistream::isVersion (int major, int minor, int release)
{
    return (major == m_major) 
	&& (minor == m_minor) 
	&& (release == m_release);
}

bool bytecodeistream::isVersionAtMost (int major, int minor, int release)
{
    if (m_major > major)
    {
	return false;
    }
    
    if (m_major == major)
    {
	if (m_minor > minor)
	{
	    return false;
	}
	
	if ( (m_minor == minor) && (m_release > release))
	{
	    return false;
	}
    }

    return true;
}

int Bytecode::m_namespace_nesting_level = -1;
int Bytecode::m_namespace_nesting_array_size = 0;
int Bytecode::m_namespace_tare_level = 0;
Bytecode::namespaceentry_t *Bytecode::m_namespace_nesting_array = 0;

void
Bytecode::namespaceInit ()
{
#if DO_DEBUG
    y2debug ("Reinitialize namespaces");
#endif
    if (Bytecode::m_namespace_nesting_array)
    {
	free (Bytecode::m_namespace_nesting_array);
    }
    
    Bytecode::m_namespace_nesting_array = 0;
    Bytecode::m_namespace_nesting_level = -1;
    Bytecode::m_namespace_nesting_array_size = 0;
    Bytecode::m_namespace_tare_level = 0;
}

// ------------------------------------------------------------------
// bool I/O

std::ostream &
Bytecode::writeBool (std::ostream & str, const bool value)
{
    return str.put ((char)(value?'\x01':'\x00'));
}


bool
Bytecode::readBool (bytecodeistream & str)
{
    char c;
    str.get (c);
#if DO_DEBUG
//    y2debug ("Bytecode::readBool 0x%02x", (unsigned int)c);
#endif

    if (c != 0)
    {
	return true;
    }
    return false;
}


// ------------------------------------------------------------------
// u_int32_t I/O

std::ostream &
Bytecode::writeInt32 (std::ostream & str, const u_int32_t value)
{
    str.put ((char)4);
    str.put ((char)(value & 0xff));
    str.put ((char)(value>>8 & 0xff));
    str.put ((char)(value>>16 & 0xff));
    return str.put ((char)(value>>24 & 0xff));
}


u_int32_t
Bytecode::readInt32 (bytecodeistream & str)
{
//    char c;
//    str.get (c);
//    if (c != 4)
//    {
//	return false;
//    }

    char v[5];

    str.read (v, 5);
    if (v[0] != 4)
    {
	return false;
    }

    u_int32_t cv, value = 0;
    cv = (unsigned char)v[1]; value = cv;
    cv = (unsigned char)v[2]; cv <<= 8; value |= cv;
    cv = (unsigned char)v[3]; cv <<= 16; value |= cv;
    cv = (unsigned char)v[4]; cv <<= 24; value |= cv;
    return value;
}


// ------------------------------------------------------------------
// string I/O

std::ostream &
Bytecode::writeString (std::ostream & streamref, const string & stringref)
{
    u_int32_t len = stringref.size();

    writeInt32 (streamref, len);
    return streamref.write (stringref.c_str(), len);
}


bool
Bytecode::readString (bytecodeistream & streamref, string & stringref)
{
    bool ret = false;
    stringref.erase();
    u_int32_t len = readInt32 (streamref);
    if (len > 0)
    {
	char *buf = new char [len+1];
	if (streamref.read (buf, len))
	{
	    buf[len] = 0;
	    stringref = string (buf);
	    ret = true;
	}
	delete [] buf;
    }
    return ret;
}


// ------------------------------------------------------------------
// Ustring I/O

std::ostream &
Bytecode::writeUstring (std::ostream & streamref, const Ustring ustringref)
{
    u_int32_t len = ustringref->size();

    writeInt32 (streamref, len);
    return streamref.write (ustringref->c_str(), len);
}


Ustring
Bytecode::readUstring (bytecodeistream & streamref)
{
    u_int32_t len = readInt32 (streamref);
    Ustring ret = Ustring (*SymbolEntry::_nameHash, "");
    if (len > 0)
    {
	char *buf = new char [len+1];
	if (streamref.read (buf, len))
	{
	    buf[len] = 0;
	    ret = Ustring (*SymbolEntry::_nameHash, buf);
	}
	delete [] buf;
    }
    return ret;
}

// ------------------------------------------------------------------
// char * I/O

std::ostream &
Bytecode::writeCharp (std::ostream & str, const char * charp)
{
    u_int32_t len = strlen (charp);
    writeInt32 (str, len);
    return str.write (charp, len);
}


char *
Bytecode::readCharp (bytecodeistream & str)
{
    u_int32_t len = readInt32 (str);
    if (str.good())
    {
	char *buf = new char [len+1];
	if (str.read (buf, len))
	{
	    buf[len] = 0;
	    return buf;
	}
	delete [] buf;
    }
    return 0;
}


// ------------------------------------------------------------------
// bytecode I/O

std::ostream &
Bytecode::writeBytep (std::ostream & str, const unsigned char * bytep, unsigned int len)
{
    writeInt32 (str, len);
    return str.write ((char *)bytep, len);
}


unsigned char *
Bytecode::readBytep (bytecodeistream & str)
{
    u_int32_t len = readInt32 (str);
    if (str.good())
    {
	unsigned char *buf = new unsigned char [len];
	if (str.read ((char *)buf, len))
	{
	    return buf;
	}
	delete [] buf;
    }
    return 0;
}


// ------------------------------------------------------------------
// Type I/O

std::ostream &
Bytecode::writeType (std::ostream & str, constTypePtr type)
{
    return type->toStream (str);
}


TypePtr
Bytecode::readType (bytecodeistream & str)
{
    int kind = readInt32 (str);
#if DO_DEBUG
y2debug ("Bytecode::readType(%d)", kind);
#endif
    switch ((Type::tkind)kind)
    {
	case Type::UnspecT:
	case Type::ErrorT:
	case Type::AnyT:
	case Type::BooleanT:
	case Type::ByteblockT:
	case Type::FloatT:
	case Type::IntegerT:
	case Type::LocaleT:
	case Type::PathT:
	case Type::StringT:
	case Type::SymbolT:
	case Type::TermT:
	case Type::VoidT:
	case Type::WildcardT:
	case Type::FlexT:
	case Type::NilT:
	    return TypePtr ( new Type ((Type::tkind)kind, str) );
	break;

	case Type::NFlexT:	return TypePtr ( new NFlexType (str) ); break;
	case Type::VariableT:	return TypePtr ( new VariableType (str) ); break;
	case Type::BlockT:	return TypePtr ( new BlockType (str) ); break;
	case Type::ListT:	return TypePtr ( new ListType (str) ); break;
	case Type::MapT:	return TypePtr ( new MapType (str) ); break;
	case Type::TupleT:	return TypePtr ( new TupleType (str) ); break;
	case Type::FunctionT:	return TypePtr ( new FunctionType (str) ); break;
    }
    y2error ("Unhandled type kind %d", kind);
    return Type::Error->clone();
}

// ------------------------------------------------------------------
// YCPValue I/O


/*
 * write value to stream
 *
 */

std::ostream &
Bytecode::writeValue (std::ostream & str, const YCPValue value)
{
    if (value.isNull())
    {
	y2error ("writeValue (NULL)");
	str.setstate (std::ostream::failbit);
    }
    else if (str.put ((char)(value->valuetype())))
    {
	return value->toStream (str);
    }
    return str;
}


/*
 * read value from stream
 *
 */

YCPValue
Bytecode::readValue (bytecodeistream & str)
{
    char vt;
    if (str.get (vt))
    {
	switch (vt)
	{
	    case YT_VOID:
	    {
		return YCPVoid (str);
	    }
	    break;
	    case YT_BOOLEAN:
	    {
		return YCPBoolean (str);
	    }
	    break;
	    case YT_INTEGER:
	    {
		return YCPInteger (str);
	    }
	    break;
	    case YT_FLOAT:
	    {
		return YCPFloat (str);
	    }
	    break;
	    case YT_STRING:
	    {
		return YCPString (str);
	    }
	    break;
	    case YT_BYTEBLOCK:
	    {
		return YCPByteblock (str);
	    }
	    break;
	    case YT_PATH:
	    {
		return YCPPath (str);
	    }
	    break;
	    case YT_SYMBOL:
	    {
		return YCPSymbol (str);
	    }
	    break;
	    case YT_LIST:
	    {
		return YCPList (str);
	    }
	    break;
	    case YT_MAP:
	    {
		return YCPMap (str);
	    }
	    break;
	    case YT_TERM:
	    {
		return YCPTerm (str);
	    }
	    break;
	    case YT_CODE:
	    {
		return YCPCode (str);
	    }
	    break;
	    default:
	    {
		y2error ("readValue stream code %d", vt);
		break;
	    }
	}
    }
    else
    {
	y2warning ("readValue(%d:%s) NIL", (int)vt, YCode::toString((YCode::ykind)vt).c_str());
    }

    return YCPNull();
}


// ------------------------------------------------------------------
// ycodelist_t * I/O

std::ostream &
Bytecode::writeYCodelist (std::ostream & str, const ycodelist_t *codelist)
{
    u_int32_t count = 0;
    const ycodelist_t *codep = codelist;

    while (codep)
    {
	count++;
	codep = codep->next;
    }

#if DO_DEBUG
    y2debug ("Bytecode::writeYCodelist %d entries", count);
#endif

    if (Bytecode::writeInt32 (str, count))
    {
	codep = codelist;
	while (codep)
	{
	    if (!codep->code->toStream (str))
	    {
		y2error ("Error writing codelist");
		break;
	    }
	    codep = codep->next;
	}
    }

    return str;
}


bool
Bytecode::readYCodelist (bytecodeistream & str, ycodelist_t **anchor)
{
    u_int32_t count = readInt32 (str);

#if DO_DEBUG
    y2debug ("Bytecode::readYCodelist %d entries", count);
#endif

    ycodelist_t *last = 0;

    while (count-- > 0)
    {
	ycodelist_t *element = new ycodelist_t;

	element->code = Bytecode::readCode (str);
	element->next = 0;

	if (element->code == 0)
	{
	    y2error ("Bytecode::readYCodelist failed");
	    delete element;
	    return false;
	}

	if (*anchor == 0)		// anchor undefined
	{
	    *anchor = element;
	}
	else
	{
	    last->next = element;
	}

	last = element;
    }

    return str.good();
}


// ------------------------------------------------------------------
// namespace stack handling

// find Id matching namespace
int
Bytecode::namespaceId (const Y2Namespace *name_space)
{
    for (int i = m_namespace_tare_level; i <= m_namespace_nesting_level; i++)
    {
	if (m_namespace_nesting_array[i].name_space == name_space)
	{
	    return i - m_namespace_tare_level;
	}
    }
    y2error ("No ID for %p, level %d", name_space, m_namespace_nesting_level);
    return -1;
}


// retrieve namespace for ID
const Y2Namespace *
Bytecode::namespacePtr (int namespace_id)
{
    // for entries without a name_space (foreach)
    if (namespace_id < 0) return 0;

    namespace_id += m_namespace_tare_level;
    if (namespace_id <= m_namespace_nesting_level)	// local namespace
    {
	return m_namespace_nesting_array[namespace_id].name_space;
    }
    y2error ("Block id %d > nesting_level %d", namespace_id - m_namespace_tare_level, m_namespace_nesting_level - m_namespace_tare_level);
    return 0;
}


// push namespace to stack
//  the stack resembles the nesting of namespaces
int
Bytecode::pushNamespace (const Y2Namespace *name_space, bool with_xrefs)
{
    if (name_space == 0)
    {
	y2error ("Bytecode::pushNamespace (%p) NULL", name_space);
	return -1;
    }

    m_namespace_nesting_level++;
    if (m_namespace_nesting_array_size <= m_namespace_nesting_level)
    {
	m_namespace_nesting_array_size += 16;
	m_namespace_nesting_array = (namespaceentry_t *)realloc (m_namespace_nesting_array, sizeof (namespaceentry_t) * m_namespace_nesting_array_size);
    }
#if DO_DEBUG
    y2debug ("Bytecode::pushNamespace (%p), level %d, size %d, tare %d", name_space, m_namespace_nesting_level, m_namespace_nesting_array_size, m_namespace_tare_level);
#endif
    m_namespace_nesting_array[m_namespace_nesting_level].name_space = name_space;
    m_namespace_nesting_array[m_namespace_nesting_level].with_xrefs = with_xrefs;
    if (with_xrefs)
    {
	name_space->table()->openXRefs();
    }

    return m_namespace_nesting_level-m_namespace_tare_level;
}


// pop namespace from stack
//  the stack resembles the nesting of namespaces
int
Bytecode::popNamespace (const Y2Namespace *name_space)
{
#if DO_DEBUG
    y2debug ("Bytecode::popNamespace (%p), level %d, size %d, tare %d", name_space, m_namespace_nesting_level, m_namespace_nesting_array_size, m_namespace_tare_level);
#endif
    if (name_space == 0)
    {
	y2error ("Bytecode::popNamespace (%p) NULL", name_space);
	return -1;
    }

    if (m_namespace_nesting_level < m_namespace_tare_level)
    {
	y2error ("Bytecode::popNamespace (%p) empty stack", name_space);
    }
    else if (m_namespace_nesting_array[m_namespace_nesting_level].name_space != name_space)
    {
	y2error ("Bytecode::popNamespace (%p) not top of stack [%d]%p", name_space, m_namespace_nesting_level, m_namespace_nesting_array[m_namespace_nesting_level].name_space);
    }
    else
    {
	if (m_namespace_nesting_array[m_namespace_nesting_level].with_xrefs)
	{
	    name_space->table()->closeXRefs();
	}
	m_namespace_nesting_level--;
    }
    return 0;
}


// pop all from id stack until given namespace is reached and popped too
void
Bytecode::popUptoNamespace (const Y2Namespace *name_space)
{
#if DO_DEBUG
    y2debug ("Bytecode::popUptoNamespace (%p), level %d, size %d, tare %d", name_space, m_namespace_nesting_level, m_namespace_nesting_array_size, m_namespace_tare_level);
#endif
    if (name_space == 0)
    {
	y2error ("Bytecode::popUptoNamespace (%p) NULL", name_space);
	return;
    }

    while (m_namespace_nesting_level >= m_namespace_tare_level)
    {
	const Y2Namespace *top_space = m_namespace_nesting_array[m_namespace_nesting_level].name_space;
	if (m_namespace_nesting_array[m_namespace_nesting_level].with_xrefs)
	{
	    top_space->table()->closeXRefs();
	}
	m_namespace_nesting_level--;
	if (top_space == name_space)
	{
	    return;
	}
    }
    y2error ("Bytecode::popUptoNamespace (%p) empty stack", name_space);
    return;
}


// reset current namespace stack to 'empty' for module loading
//   returns a tare id needed later
int
Bytecode::tareStack ()
{
    int tare = m_namespace_nesting_level - m_namespace_tare_level + 1;
#if DO_DEBUG
//    y2debug ("Bytecode::tareStack() level %d, size %d, current tare %d, tare_id %d", m_namespace_nesting_level, m_namespace_nesting_array_size, m_namespace_tare_level, tare);
#endif
    m_namespace_tare_level = m_namespace_nesting_level + 1;
    return tare;
}


void
Bytecode::untareStack (int tare_id)
{
#if DO_DEBUG
//    y2debug ("Bytecode::untareStack() level %d, size %d, current tare %d, tare_id %d", m_namespace_nesting_level, m_namespace_nesting_array_size, m_namespace_tare_level, tare_id);
#endif
    m_namespace_tare_level -= tare_id;
    return;
}

// ------------------------------------------------------------------
// SymbolEntry pointer (!) handling
//   the SymbolEntries itself are 'owned' by YBlock and referenced via pointers
//   to SymbolEntry. These functions handle stream I/O for SymbolEntry pointers.
//
//  position is the index in namespace's m_senvironment[] for _local_ symbols
//  position is the index in module table's m_xrefs[] for _external_ symbols, see YSImport
//
std::ostream &
Bytecode::writeEntry (std::ostream & str, const SymbolEntryPtr sentry)
{
    int id = Bytecode::namespaceId (sentry->nameSpace());
    if (id < 0)
    {
	y2error ("No id for entry (%s)", sentry->toString().c_str());
	abort ();
    }
#if DO_DEBUG
//    y2debug ("Bytecode::writeEntry (%p:%s: id %d, pos %d)", sentry, sentry->toString().c_str(), id, sentry->position());
#endif
    Bytecode::writeInt32 (str, id);
    return Bytecode::writeInt32 (str, sentry->position());
}


SymbolEntryPtr 
Bytecode::readEntry (bytecodeistream & str)
{
    // read reference to namespaces (namespace_id) symbol table (position)
    int namespace_id = Bytecode::readInt32 (str);
    int position = Bytecode::readInt32 (str);

    if (namespace_id == -1)
    {
#if DO_DEBUG
	y2debug( "Special entry without namespace" );
#endif
	// FIXME: this may be wrong
	return 0;
    }

    // get namespace pointer and SymbolEntry within namespace
    const Y2Namespace *name_space = Bytecode::namespacePtr (namespace_id);
    if (name_space == 0)
    {
	y2error ("invalid namespace %d for entry", namespace_id);
	return 0;
    }

    SymbolEntryPtr sentry;
    if (position < 0)				// it's an Xref !
    {
	SymbolTable *table = name_space->table();
	if (table == 0)
	{
	    ycp2error ("No table associated to xref namespace\n");
	    exit (1);
	}
	position = -position - 1;		// -1 .. -n  --> 0 .. n-1
#if DO_DEBUG
	y2debug ("get reference %d from table %p", position, table);
#endif
	sentry = table->getXRef(position);
    }
    else
    {
	sentry = name_space->symbolEntry (position);
    }

    if (sentry == 0)
    {
	y2error ("invalid entry %d for namespace (%s)", position, name_space->name().c_str());
	return 0;
    }
#if DO_DEBUG
    y2debug ("entry <namespace id %d @ %p[%s]> pos %d = (%s)", namespace_id, name_space, name_space->name().c_str(), position, sentry->toString().c_str());
#endif
    return sentry;
}

// ------------------------------------------------------------------
// YCode read


// read code from stream
YCodePtr
Bytecode::readCode (bytecodeistream & str)
{
    char code;
    if (!str.get (code))
    {
	y2error ("Can't read from stream");
	return 0;
    }
#if DO_DEBUG
//    y2debug ("Bytecode::readCode (%d:%s)", code, YCode::toString ((YCode::ykind)code).c_str());
#endif
    if (code < YCode::ycConstant)
    {
	return new YConst ((YCode::ykind)code, str);
    }
    
    // compatibility with 9.1/SLES
    if (str.isVersion (1,3,2) && code > YCode::yeExpression)
    {
	// yeFunctionPointer did not exist then
	code++;
    }
    
    YCodePtr res = 0;

    // there used to be a try/catch here but it was moved to readFile

    switch (code)
    {
	case YCode::ycConstant:
	{
	    // this constant is a placeholder, typically used by
	    // language bindings that cannot provide type information
	    y2error ("Unable to read constant, check the compilation of the module");
	    return 0;
	}
	case YCode::ycLocale:
	{
	    res = new YLocale (str);
	}
	break;
	case YCode::ycFunction:
	{
	    res = new YFunction (str);
	}
	break;
	case YCode::yePropagate:
	{
	    res = new YEPropagate (str);
	}
	break;
	case YCode::yeUnary:
	{
	    res = new YEUnary (str);
	}
	break;
	case YCode::yeBinary:
	{
	    res = new YEBinary (str);
	}
	break;
	case YCode::yeTriple:
	{
	    res = new YETriple (str);
	}
	break;
	case YCode::yeCompare:
	{
	    res = new YECompare (str);
	}
	break;
	case YCode::yeLocale:
	{
	    res = new YELocale (str);
	}
	break;
	case YCode::yeList:
	{
	    res = new YEList (str);
	}
	break;
	case YCode::yeMap:
	{
	    res = new YEMap (str);
	}
	break;
	case YCode::yeTerm:
	{
	    res = new YETerm (str);
	}
	break;
	case YCode::yeIs:
	{
	    res = new YEIs (str);
	}
	break;
	case YCode::yeBracket:
	{
	    res = new YEBracket (str);
	}
	break;
	case YCode::yeBlock:
	{
	    res = new YBlock (str);
	}
	break;
	case YCode::yeReturn:
	{
	    res = new YEReturn (str);
	}
	break;
	case YCode::yeVariable:
	{
	    res = new YEVariable (str);
	}
	break;
	case YCode::yeReference:
	{
	    res = new YEReference (str);
	}
	break;
	case YCode::yeBuiltin:
	{
	    res = new YEBuiltin (str);
	}
	break;
	case YCode::yeFunction:
	{
	    res = YECall::readCall (str);
	}
	break;
	case YCode::yeFunctionPointer:
	{
	    res = new YEFunctionPointer (str);
	}
	break;
	case YCode::ysTypedef:
	{
	    res = new YSTypedef (str);
	}
	break;
	case YCode::ysVariable:
	{
	    res = new YSVariable (str);
	}
	break;
	case YCode::ysFunction:
	{
	    res = new YSFunction (str);
	}
	break;
	case YCode::ysAssign:
	{
	    res = new YSAssign (str);
	}
	break;
	case YCode::ysBracket:
	{
	    res = new YSBracket (str);
	}
	break;
	case YCode::ysIf:
	{
	    res = new YSIf (str);
	}
	break;
	case YCode::ysWhile:
	{
	    res = new YSWhile (str);
	}
	break;
	case YCode::ysDo:
	{
	    res = new YSDo (str);
	}
	break;
	case YCode::ysRepeat:
	{
	    res = new YSRepeat (str);
	}
	break;
	case YCode::ysExpression:
	{
	    res = new YSExpression (str);
	}
	break;
	case YCode::ysReturn:
	{
	    res = new YSReturn (str);
	}
	break;
	case YCode::ysBreak:
	{
	    res = new YSBreak (str);
	}
	break;
	case YCode::ysContinue:
	{
	    res = new YSContinue (str);
	}
	break;
	case YCode::ysTextdomain:
	{
	    res = new YSTextdomain (str);
	}
	break;
	case YCode::ysInclude:
	{
	    res = new YSInclude (str);
	}
	break;
	case YCode::ysFilename:
	{
	    res = new YSFilename (str);
	}
	break;
	case YCode::ysImport:
	{
	    res = new YSImport (str);
	}
	break;
	case YCode::ysBlock:
	{
	    res = new YSBlock (str);
	}
	break;
	case YCode::ysSwitch:
	{
	    res = new YSSwitch (str);
	}
	break;
	default:
	{
	    y2error ("Unknown code %d", code);
	}
	break;
    }

    return res;
}


// ------------------------------------------------------------------
// File I/O

// static member
map <string, YBlockPtr>* Bytecode::m_bytecodeCache = NULL;

// read file from module path

YBlockPtr 
Bytecode::readModule (const string & mname)
{
#if DO_DEBUG
//    y2debug ("Bytecode::readModule (%s) ", mname.c_str ());
#endif

    // TODO better error reporting?
    // like: could not find foo.ycp in /modules, /a/modules.
    // It will return an empty string on failure

    string filename = YCPPathSearch::findModule (mname);
    if (filename.empty())
    {
	ycperror ("Module '%s' not found", mname.c_str());
	return 0;
    }
    
    if (! m_bytecodeCache)
    {
	m_bytecodeCache = new map <string, YBlockPtr>;
    }

    // check the cache
    if (m_bytecodeCache->find (mname) != m_bytecodeCache->end ())
    {
#if DO_DEBUG
//	y2debug ("Bytecode cache hit: %s", mname.c_str ());
#endif

	return m_bytecodeCache->find (mname)->second;
    }
    
    int tare_id = Bytecode::tareStack ();			// current nesting level is 0 for this module
    YBlockPtr block = (YBlockPtr)Bytecode::readFile (filename);

    if (block == NULL)
    {
	return NULL;
    }

    Bytecode::untareStack (tare_id);

    if (!block->isModule())
    {
	y2error ("'%s' is no module", filename.c_str());
	return NULL;
    }

    m_bytecodeCache->insert (std::make_pair (mname, block));

    return block;
}


// read YCode from file, return YCode (0 in case of failure)
YCodePtr
Bytecode::readFile (const string & filename)
{
#if DO_DEBUG
//    y2debug ("Bytecode::readFile (%s)", filename.c_str());
#endif
    bytecodeistream instream (filename);
    if (!instream.is_open ())
    {
	y2error ("Failed to open '%s': %s", filename.c_str(), strerror (errno));
	return 0;
    }
    // check YaST_BYTECODE_HEADER
    if ( 
	instream.isVersion (
	    atoi (YaST_BYTECODE_MAJOR)
	    , atoi (YaST_BYTECODE_MINOR)
	    , atoi (YaST_BYTECODE_RELEASE))
	||
	instream.isVersion (1,3,2) )	// 9.1/SLES9
    {
#if DO_DEBUG
//	y2debug ("Header accepted");
#endif

	try
	{
	    return readCode (instream);
	}
	catch (const Bytecode::Invalid&)
	{
	    // there are memory leaks all over the place now
	    y2error ("Caught invalid bytecode in '%s'", filename.c_str());
	    return 0;
	}
    }

    y2error ("Unsupported version %d.%d.%d"
	, instream.major ()
	, instream.minor ()
	, instream.release ());
    return 0;
}


// write YCode to file, return false on error (i.e. file not existing - see errno)
bool
Bytecode::writeFile (const YCodePtr code, const string & filename)
{
    // clear errno first
    errno = 0;

#if DO_DEBUG
//    y2debug ("Bytecode::writeFile (%s)", filename.c_str());
#endif
    std::ofstream outstream (filename.c_str());
    if (!outstream.is_open ())
    {
	y2error ("Failed to write '%s': %s", filename.c_str(), strerror (errno));
	return false;
    }

    // write Bytecode without any localization
    outstream.imbue(std::locale("C"));

    string header =  string (YaST_BYTECODE_HEADER YaST_BYTECODE_MAJOR "." YaST_BYTECODE_MINOR "." YaST_BYTECODE_RELEASE);
    outstream.write (header.c_str(), header.size() + 1);	// including trailing \0

    code->toStream (outstream);

    return ! outstream.fail ();
}
