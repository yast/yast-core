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

   File:	StaticDeclaration.h

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/
// -*- c++ -*-

#ifndef StaticDeclaration_h
#define StaticDeclaration_h

#include <string>
using namespace std;

#include "ycp/YCPValue.h"
#include "ycp/YCPList.h"
#include "ycp/TypeCode.h"

class SymbolTable;

// Only use BUILTIN_STATISTICS for testing. It will create three files
// /tmp/builtin-X.txt which list all builtins registered, looked up
// and used.
#define BUILTIN_STATISTICS


// structure for static declarations
enum DeclFlags
{
    DECL_NIL =		0x00000001,	// accepts nil
    DECL_WILD =		0x00000002,	// wildcard
    DECL_SYMBOL =	0x00000004,	// symbol as parameter
    DECL_CODE =		0x00000008,	// code as parameter
    DECL_LOOP =		0x00000010,	// implements a loop, allows break statement
    DECL_TYPEDEF =	0x00000020,	// name is a typedef
    DECL_CONSTANT =	0x00000040,	// name is a constant
    DECL_NAMESPACE =	0x00000080,	// name is a namespace (switches registerDeclarations !)
};

/**
 * A declaration of a (builtin?) function
 */

struct declaration {
  const char *name;		// name of variable/function/typedef
  const char *type;		// type of variable/function/typedef
  int flags;			// parameter acceptance, @ref DeclFlags
  void *ptr;			// pointer to builtin value/function
  struct declaration *next;	// link to next overloaded declaration (internal use only)
};
typedef struct declaration declaration_t;

/**
 * show a declaration
 * @param full if false, just show the name; if true, show name and type
 */
string Decl2String (const declaration_t *declaration, bool full = false);

class StaticDeclaration {
private:
    SymbolTable *declTable;

public:
    // constructor
    StaticDeclaration ();
    ~StaticDeclaration ();

    SymbolTable *symbolTable() { return declTable; };

    // register declarations
    void registerDeclarations (const char *filename, declaration_t *declarations);

    // find a declaration
    declaration_t *findDeclaration (const char *name) const;
    declaration_t *findDeclaration (const char *name, const TypeCode &type, bool partial = false) const;
    declaration_t *findDeclaration (declaration_t *decl, const TypeCode &type, bool partial = false) const;

    // give return type
    TypeCode returnType (const declaration_t *declaration) const;

    // dump all registered builtins
    void dumpDeclarations () const;

    // write declaration to stream (name and type)
    std::ostream & writeDeclaration (std::ostream & str, const declaration_t *decl) const;

    // read declaration from stream (return declaration matching name and type _exactly_)
    declaration_t *readDeclaration (std::istream & str) const;
};

#endif // StaticDeclaration_h
