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

   File:       Bytecode.h

   Primitive bytecode I/O functions.
   Acts as a namespace wrapper.

   Author:     Klaus Kaempf <kkaempf@suse.de>
   Maintainer: Klaus Kaempf <kkaempf@suse.de>

/-*/
// -*- c++ -*-

#ifndef Bytecode_h
#define Bytecode_h

#include "ycp/YCPValue.h"
#include "ycp/YCode.h"
#include "ycp/YStatement.h"
#include "ycp/Type.h"

class Y2Namespace;
class YBlock;

#include <iosfwd>
#include <string>
#include <map>

class Bytecode {
    static int m_block_nesting_level;
    static int m_block_nesting_array_size;
    static int m_block_tare_level;
    static const Y2Namespace **m_block_nesting_array;
    static map<string, YBlock*> bytecodeCache;

    public:
	// bool I/O
	static std::ostream & writeBool (std::ostream & streamref, bool value);
	static bool readBool (std::istream & streamref);

	// string I/O
	static std::ostream & writeString (std::ostream & streamref, const std::string & stringref);
	static bool readString (std::istream & streamref, std::string & stringref);

	// char * I/O
	static std::ostream & writeCharp (std::ostream & streamref, const char * charp);
	static char * readCharp (std::istream & streamref);

	// bytepointer I/O
	static std::ostream & writeBytep (std::ostream & streamref, const unsigned char * bytep, unsigned int len);
	static unsigned char * readBytep (std::istream & streamref);

	// u_int32_t I/O
	static std::ostream & writeInt32 (std::ostream & str, const u_int32_t value);
	static u_int32_t readInt32 (std::istream & str);

	// Type I/O
	static std::ostream & writeType (std::ostream & str, constTypePtr type);
	static TypePtr readType (std::istream & str);

	// YCPValue I/O
	static std::ostream & writeValue (std::ostream & str, const YCPValue value);
	static YCPValue readValue (std::istream & str);

	// ycodelist_t * I/O
	static std::ostream & writeYCodelist (std::ostream & str, const ycodelist_t *codelist);
	static bool readYCodelist (std::istream & str, ycodelist_t **anchor, ycodelist_t **last);

	// block nesting handling
	//
	// retrieve ID (nesting level) for block
	static int blockId (const Y2Namespace *block);
	// retrieve block for ID
	static const Y2Namespace *blockPtr (int block_id);
	// push given block to id stack, return new id, -1 on error
	static int pushBlock (const Y2Namespace *block);
	// pop given block from id stack, return block id, -1 on error
	static int popBlock (const Y2Namespace *block);
	// reset current block stack to 'empty' for module loading
	//   returns a tare id needed later
	static int tareStack ();
	static void untareStack (int tare_id);

	// SymbolEntry pointer (!) handling
	//   the SymbolEntries itself are 'owned' by Y2Namespace (YBlock in YCP) and referenced via pointers
	//   to SymbolEntry. These functions handle stream I/O for SymbolEntry pointers.
	static std::ostream &writeEntry (std::ostream & str, const SymbolEntry *entry);
	static SymbolEntry *readEntry (std::istream & str);

	// YCode read.
	// Must be implemented outside of YCode since we have derived classes to allocate...
	// see YCode for write
	static YCode *readCode (std::istream & str);

	// File I/O
	// reading and writing complete files is done via separate functions
	// which add/check a 'magic value' header denoting "YCode" and its version.

	// read YCode from file in Module path, return YBlock (NULL in case of error)
	static YBlock *readModule (const string & mname, const string & timestamp);

	// read YCode from file, return YCode (YError in case of failure)
	static YCode *readFile (const string & filename);

	// write YCode to file, return errno (i.e. file not existing)
	static int writeFile (const YCode *code, const string & filename);
	
	// compute a timestamp for a given file name
	static string fileTimestamp (const string & filename);
};

#endif // Bytecode_h
