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

$Id$
/-*/

#include "ycp/Bytecode.h"
#include "YCP.h"
#include "ycp/YCode.h"
#include "ycp/YExpression.h"
#include "ycp/YStatement.h"
#include "ycp/YBlock.h"

#include "ycp/y2log.h"
#include "ycp/pathsearch.h"

#include <fstream>

int Bytecode::m_block_nesting_level = -1;
int Bytecode::m_block_nesting_array_size = 0;
int Bytecode::m_block_tare_level = 0;
const YBlock **Bytecode::m_block_nesting_array = 0;

// ------------------------------------------------------------------
// bool I/O

std::ostream & 
Bytecode::writeBool (std::ostream & str, const bool value)
{
    return str.put ((char)(value?'\x01':'\x00'));
}


bool
Bytecode::readBool (std::istream & str)
{
    char c;
    str.get (c);
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
Bytecode::readInt32 (std::istream & str)
{
    char c;
    str.get (c);
    if (c != 4)
    {
	return false;
    }
    u_int32_t cv, value = 0;
    str.get (c); cv = (unsigned char)c; value = cv;
    str.get (c); cv = (unsigned char)c; cv <<= 8; value |= cv;
    str.get (c); cv = (unsigned char)c; cv <<= 16; value |= cv;
    str.get (c); cv = (unsigned char)c; cv <<= 24; value |= cv;
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
Bytecode::readString (std::istream & streamref, string & stringref)
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
// char * I/O

std::ostream & 
Bytecode::writeCharp (std::ostream & str, const char * charp)
{
    u_int32_t len = strlen (charp);
    writeInt32 (str, len);
    return str.write (charp, len);
}


char *
Bytecode::readCharp (std::istream & str)
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
Bytecode::readBytep (std::istream & str)
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
    else if (str.put ((char)value->valuetype()))
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
Bytecode::readValue (std::istream & str)
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
	    default:
	    {
		y2error ("readValue stream code %d", vt);
		break;
	    }
	}
    }
    else
    {
	y2warning ("readValue(%d:%s) NIL", (int)vt, YCode::toString((YCode::ycode)vt).c_str());
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

    y2debug ("Bytecode::writeYCodelist %d entries", count);

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
Bytecode::readYCodelist (std::istream & str, ycodelist_t **anchor, ycodelist_t **last)
{
    u_int32_t count = readInt32 (str);

    y2debug ("Bytecode::readYCodelist %d entries", count);

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
	else if (last != 0)		// last given
	{
	    (*last)->next = element;
	}

	if (last != 0)
	{
	    *last = element;
	}
    }

    return str.good();
}


// ------------------------------------------------------------------
// block stack handling

// find Id matching block
int
Bytecode::blockId (const YBlock *block)
{
    for (int i = m_block_tare_level; i <= m_block_nesting_level; i++)
    {
	if (m_block_nesting_array[i] == block)
	{
	    return i - m_block_tare_level;
	}
    }
    y2debug ("No ID for %p, level %d", block, m_block_nesting_level);
    return -1;
}


// retrieve block for ID
const YBlock *
Bytecode::blockPtr (int block_id)
{
    block_id += m_block_tare_level;
    if (block_id <= m_block_nesting_level)	// local block
    {
	return m_block_nesting_array[block_id];
    }
    y2error ("Block id %d > nesting_level %d", block_id-m_block_tare_level, m_block_nesting_level-m_block_tare_level);
    return 0;
}


// push block to stack
//  the stack resembles the nesting of blocks
int
Bytecode::pushBlock (const YBlock *block)
{
    m_block_nesting_level++;
    if (m_block_nesting_array_size <= m_block_nesting_level)
    {
	m_block_nesting_array_size++;
	m_block_nesting_array = (const YBlock **)realloc (m_block_nesting_array, sizeof (YBlock *) * m_block_nesting_array_size);
    }
    y2debug ("Bytecode::pushBlock (%p), level %d, size %d, tare %d", block, m_block_nesting_level, m_block_nesting_array_size, m_block_tare_level);
    m_block_nesting_array[m_block_nesting_level] = block;
    return m_block_nesting_level-m_block_tare_level;
}


// pop block from stack
//  the stack resembles the nesting of blocks
int
Bytecode::popBlock (const YBlock *block)
{
    y2debug ("Bytecode::popBlock (%p), level %d, size %d, tare %d", block, m_block_nesting_level, m_block_nesting_array_size, m_block_tare_level);
    if (m_block_nesting_level-m_block_tare_level < 0)
    {
	y2error ("Bytecode::popBlock(%p) empty stack", block);
    }
    else if (m_block_nesting_array[m_block_nesting_level] != block)
    {
	y2error ("Bytecode::popBlock(%p) not top of stack [%d]%p", block, m_block_nesting_level, m_block_nesting_array[m_block_nesting_level]);
    }
    else
    {
	m_block_nesting_level--;
    }
    return 0;
}


// reset current block stack to 'empty' for module loading
//   returns a tare id needed later
int
Bytecode::tareStack ()
{
    int tare = m_block_nesting_level - m_block_tare_level + 1;
    y2debug ("Bytecode::tareStack() level %d, size %d, current tare %d, tare_id %d", m_block_nesting_level, m_block_nesting_array_size, m_block_tare_level, tare);
    m_block_tare_level = m_block_nesting_level + 1;
    return tare;
}


void
Bytecode::untareStack (int tare_id)
{
    y2debug ("Bytecode::untareStack() level %d, size %d, current tare %d, tare_id %d", m_block_nesting_level, m_block_nesting_array_size, m_block_tare_level, tare_id);
    m_block_tare_level -= tare_id;
    return;
}

// ------------------------------------------------------------------
// SymbolEntry pointer (!) handling
//   the SymbolEntries itself are 'owned' by YBlock and referenced via pointers
//   to SymbolEntry. These functions handle stream I/O for SymbolEntry pointers.

std::ostream &
Bytecode::writeEntry (std::ostream & str, const SymbolEntry *entry)
{
    y2debug ("Bytecode::writeEntry (%s: blk %d, pos %d)", entry->toString().c_str(), Bytecode::blockId (entry->block()), entry->position());
    Bytecode::writeInt32 (str, Bytecode::blockId (entry->block()));
    return Bytecode::writeInt32 (str, entry->position());
}


SymbolEntry *
Bytecode::readEntry (std::istream & str)
{
    int block_id = Bytecode::readInt32 (str);
    int position = Bytecode::readInt32 (str);
    y2debug ("Bytecode::readEntry (blk %d, pos %d)", block_id, position);

    const YBlock *block = Bytecode::blockPtr (block_id);
    if (block == 0)
    {
	y2error ("invalid block %d for entry", block_id);
	return 0;
    }
    SymbolEntry *entry = block->symbolEntry (position);
    if (entry == 0)
    {
	y2error ("invalid entry %d for block (%s)", position, block->toString().c_str());
	return 0;
    }
    y2debug ("entry <blk %p[%s]> (%s)", block, block->name().c_str(), entry->toString().c_str());
    return entry;
}

// ------------------------------------------------------------------
// YCode read

// read module block
YBlock *
Bytecode::readModule (const string & mname)
{
    // TODO better error reporting?
    // like: could not find foo.ycp in /modules, /a/modules.
    // It will return an empty string on failure
    string filename = YCPPathSearch::findModule (mname);
    if (filename.empty())
    {
	y2error ("Module '%s' not found", mname.c_str());
	return 0;
    }
    std::ifstream instream (filename.c_str());
    if (!instream.is_open ())
    {
	y2error ("Failed to import '%s'", filename.c_str());
	return 0;
    }
    int tare_id = Bytecode::tareStack ();
    YBlock *block = (YBlock *)Bytecode::readCode (instream);
    Bytecode::untareStack (tare_id);
    return block;
}


// read code from stream
YCode *
Bytecode::readCode (std::istream & str)
{
    char code;
    if (!str.get (code))
    {
	y2error ("Can't read from stream");
	return 0;
    }
    y2debug ("Bytecode::readCode (%d)", code);
    if (code < YCode::ycConstant)
    {
	return new YConst ((YCode::ycode)code, str);
    }

    switch (code)
    {
	case YCode::ycLocale:
	{
	    return new YLocale (str);
	}
	case YCode::ycFunction:
	{
	    return new YFunction (str);
	}
	case YCode::yePropagate:
	{
	    return new YEPropagate (str);
	}
	break;
	case YCode::yeUnary:
	{
	    return new YEUnary (str);
	}
	break;
	case YCode::yeBinary:
	{
	    return new YEBinary (str);
	}
	break;
	case YCode::yeTriple:
	{
	    return new YETriple (str);
	}
	break;
	case YCode::yeCompare:
	{
	    return new YECompare (str);
	}
	break;
	case YCode::yeLocale:
	{
	    return new YELocale (str);
	}
	break;
	case YCode::yeList:
	{
	    return new YEList (str);
	}
	break;
	case YCode::yeMap:
	{
	    return new YEMap (str);
	}
	break;
	case YCode::yeTerm:
	{
	    return new YETerm (str);
	}
	break;
	case YCode::yeLookup:
	{
	    return new YELookup (str);
	}
	break;
	case YCode::yeIs:
	{
	    return new YEIs (str);
	}
	break;
	case YCode::yeBracket:
	{
	    return new YEBracket (str);
	}
	break;
	case YCode::yeBlock:
	{
	    return new YBlock (str);
	}
	break;
	case YCode::yeReturn:
	{
	    return new YEReturn (str);
	}
	break;
	case YCode::yeVariable:
	{
	    return new YEVariable (str);
	}
	break;
	case YCode::yeBuiltin:
	{
	    return new YEBuiltin (str);
	}
	break;
	case YCode::yeSymFunc:
	{
	    // ERROR
	}
	break;
	case YCode::yeFunction:
	{
	    return new YEFunction (str);
	}
	break;
	case YCode::ysTypedef:
	{
	    return new YSTypedef (str);
	}
	break;
	case YCode::ysVariable:
	{
	    return new YSVariable (str);
	}
	break;
	case YCode::ysFunction:
	{
	    return new YSFunction (str);
	}
	break;
	case YCode::ysAssign:
	{
	    return new YSAssign (str);
	}
	break;
	case YCode::ysBracket:
	{
	    return new YSBracket (str);
	}
	break;
	case YCode::ysIf:
	{
	    return new YSIf (str);
	}
	break;
	case YCode::ysWhile:
	{
	    return new YSWhile (str);
	}
	break;
	case YCode::ysDo:
	{
	    return new YSDo (str);
	}
	break;
	case YCode::ysRepeat:
	{
	    return new YSRepeat (str);
	}
	break;
	case YCode::ysExpression:
	{
	    return new YSExpression (str);
	}
	break;
	case YCode::ysReturn:
	{
	    return new YSReturn (str);
	}
	break;
	case YCode::ysBreak:
	{
	    return new YStatement ((YCode::ycode)code, str);
	}
	break;
	case YCode::ysContinue:
	{
	    return new YStatement ((YCode::ycode)code, str);
	}
	break;
	case YCode::ysTextdomain:
	{
	    return new YSTextdomain (str);
	}
	break;
	case YCode::ysFilename:
	{
	    return new YSFilename (str);
	}
	break;
	default:
	{
	    y2error ("Unknown code %d", code);
	}
	break;
    }
    return 0;
}


YStatement *
Bytecode::readStatement (std::istream & str)
{
    YCode *code = readCode (str);
    if (code == 0)
	return 0;

    if (code->isStatement())
    {
	return (YStatement *)code;
    }
    y2error ("bad Bytecode::readStatement %d", code->code());
    return 0;
}
