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
#include "ycp/YBlock.h"
#include "ycp/Type.h"

class Y2Namespace;

#include <iosfwd>
#include <string>
#include <map>

class Bytecode {
    static int m_namespace_nesting_level;
    static int m_namespace_nesting_array_size;
    static int m_namespace_tare_level;
    typedef struct { const Y2Namespace *name_space; bool with_xrefs; } namespaceentry_t;
    static namespaceentry_t *m_namespace_nesting_array;
    static map<string, YBlockPtr> m_bytecodeCache;

    public:
	// bool I/O
	static std::ostream & writeBool (std::ostream & streamref, bool value);
	static bool readBool (std::istream & streamref);

	// string I/O
	static std::ostream & writeString (std::ostream & streamref, const std::string & stringref);
	static bool readString (std::istream & streamref, std::string & stringref);

	// Ustring I/O
	static std::ostream & writeUstring (std::ostream & streamref, const Ustring ustringref);
	static Ustring readUstring (std::istream & streamref);

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
	static bool readYCodelist (std::istream & str, ycodelist_t **anchor);

	//-----------------------------------------------------------
	// block nesting handling
	//
	static void namespaceInit ();
	// retrieve ID (nesting level) for namespace
	static int namespaceId (const Y2Namespace *name_space);
	// retrieve namespace for ID
	static const Y2Namespace *namespacePtr (int namespace_id);

	// push given namespace to id stack, return new id, -1 on error
	static int pushNamespace (const Y2Namespace *name_space, bool with_xrefs = false);

	// pop given namespace from id stack, return namespace id, -1 on error
	static int popNamespace (const Y2Namespace *name_space);

	// pop all from id stack until given namespace is reached and popped too
	static void popUptoNamespace (const Y2Namespace *name_space);

	// reset current namespace stack to 'empty' for module loading
	//   returns a tare id needed later
	static int tareStack ();
	static void untareStack (int tare_id);

	//-----------------------------------------------------------
	// SymbolEntry pointer (!) handling
	//   the SymbolEntries itself are 'owned' by Y2Namespace (YBlock in YCP) and referenced via pointers
	//   to SymbolEntry. These functions handle stream I/O for SymbolEntry pointers.
	static std::ostream &writeEntry (std::ostream & str, const SymbolEntryPtr entry);
	static SymbolEntryPtr readEntry (std::istream & str);

	//-----------------------------------------------------------
	// YCode read.
	// Must be implemented outside of YCode since we have derived classes to allocate...
	// see YCode for write
	static YCodePtr readCode (std::istream & str);

	// File I/O
	// reading and writing complete files is done via separate functions
	// which add/check a 'magic value' header denoting "YCode" and its version.

	// read YCode from file in Module path, return YBlock (NULL in case of error)
	static YBlockPtr readModule (const string & mname);

	// read YCode from file, return YCode (YError in case of failure)
	static YCodePtr readFile (const string & filename);

	// write YCode to file, return true on success (check errno for errors)
	static bool writeFile (const YCodePtr code, const string & filename);
};

#endif // Bytecode_h
