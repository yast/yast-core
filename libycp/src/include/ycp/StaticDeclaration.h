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
#include <list>
using namespace std;

#include "ycp/YCPValue.h"
#include "ycp/YCPList.h"
#include "ycp/Type.h"
#include "y2/Y2Namespace.h"

class SymbolEntry;
class SymbolTable;
class TableEntry;
class bytecodeistream;

// Only use BUILTIN_STATISTICS for testing. It will create three files
// /tmp/builtin-X.txt which list all builtins registered, looked up
// and used.
// #define BUILTIN_STATISTICS


// structure for static declarations
enum DeclFlags
{
    DECL_NIL =		0x00000001,	// function accepts nil
    DECL_WILD =		0x00000002,	// function expects wildcard
    DECL_SYMBOL =	0x00000004,	// function expects a symbol as parameter (local environment)
    DECL_CODE =		0x00000008,	// function expects code as parameter (local evaluation)
    DECL_LOOP =		0x00000010,	// function implements a loop, allows break statement
    DECL_TYPEDEF =	0x00000020,	// name declares a typedef
    DECL_CONSTANT =	0x00000040,	// name declares a constant
    DECL_NAMESPACE =	0x00000080,	// name declares a namespace (switches registerDeclarations !)
    DECL_FLEX =		0x00000100,	// function signature include 'flex' type
    DECL_NOEVAL =	0x00000200,	// function will evaluate its parameters on its own (boolean functions for shortcut eval)
    DECL_CALL_HANDLER =	0x00000400,	// ptr is a call handler (only together with DECL_NAMESPACE)
    DECL_DEPRECATED =	0x00000800,	// deprecated function
    DECL_FORMATTED =	0x00001000	// has format string with "%1" as first arg
};

// declaration::ptr is a function pointer of this type if the first entry of a StaticDeclaration
// is declared with flags DECL_NAMESPACE | DECL_CALL_HANDLER :
typedef YCPValue (*call_handler_t)(void * function, int argc, YCPValue args[] );

/**
 * A declaration of a builtin function
 */

struct declaration {
    const char *name;			// name of variable/function/typedef
    const char *signature;		// signature of variable/function/typedef (before registration)
    void *ptr;				// pointer to builtin value/function
    int flags;				// parameter acceptance, @ref DeclFlags
    struct declaration *name_space;	// table of the namespace (internal use only)
    constTypePtr type;
    TableEntry* tentry;
};
typedef struct declaration declaration_t;

class StaticDeclaration {
private:
    // toplevel table for all static declaration
    SymbolTable *m_declTable;
    // list of predefined namespaces which are already active, Y2Namespace is non-const since it might get evaluated
    std::list<std::pair<std::string, Y2Namespace *> > m_active_predefined;
public:
    // constructor
    StaticDeclaration ();
    ~StaticDeclaration ();

    SymbolTable *symbolTable() { return m_declTable; };

    // list of registered namespaces which were predefined
    const std::list<std::pair<std::string, Y2Namespace *> > & active_predefined() const { return m_active_predefined; };

    // register declarations
    void registerDeclarations (const char *filename, declaration_t *declarations);

    // find a declaration
    declaration_t *findDeclaration (const char *name) const;
    declaration_t *findDeclaration (const char *name, constTypePtr type, bool partial = false) const;
    declaration_t *findDeclaration (declaration_t *decl, constTypePtr type, bool partial = false) const;

    // dump all registered builtins
    void dumpDeclarations () const;

    // write declaration to stream (name and type)
    std::ostream & writeDeclaration (std::ostream & str, const declaration_t *decl) const;

    // read declaration from stream (return declaration matching name and type _exactly_)
    declaration_t *readDeclaration (bytecodeistream & str) const;

    // show a declaration
    // @param full if false, just show the name; if true, show name and signatur
    static string Decl2String (const declaration_t *declaration, bool full = false);
};

#endif // StaticDeclaration_h
