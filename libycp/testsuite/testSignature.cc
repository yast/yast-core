/*
    testSignature.cc

    test program for TypeCode::fromSignature()
*/

#include "ycp/y2log.h"
#include "ycp/ExecutionEnvironment.h"
#include "ycp/Type.h"
#include "ycp/Bytecode.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

ExecutionEnvironment ee;

void
fromSig (const char *s, constTypePtr t, string ts)
{
    TypePtr i = Type::fromSignature (s);

    if (ts.empty()) ts = t->toString();

    if (i == 0)
    {
	fprintf (stderr, "Failed (\"%s\" != \"%s\")\n", s, ts.c_str()); fflush (stderr);
	return;
    }

    string is = i->toString();
    if (is != ts)
    {
	printf ("Expect: \"%s\" -> '%s'\n", s, ts.c_str());
	printf ("Got   : \"%s\" -> '%s'\n", s, is.c_str());
    }
    else
    {
	printf ("Ok: %s\n", is.c_str());
    }
    return;
}


int
main ()
{
#if 1
    fromSig ("any", Type::Any, "");			// any
    fromSig ("boolean", Type::Boolean, "");		// boolean
    fromSig ("byteblock", Type::Byteblock, "");	// byteblock
    fromSig ("float", Type::Float, "");		// float
    fromSig ("integer", Type::Integer, "");		// integer
    fromSig ("locale", Type::Locale,"");		// locale
    fromSig ("path", Type::Path, "");		// path
    fromSig ("string", Type::String, "");		// string
    fromSig ("symbol", Type::Symbol, "");		// symbol
    fromSig ("term", Type::Term, "");		// term
    fromSig ("void", Type::Void, "");		// void

    fromSig ("const integer", Type::Unspec, "const integer");
    fromSig ("integer &", Type::Unspec, "integer &");
    fromSig ("variable <integer >", Type::Unspec, "variable <integer>");

    fromSig ("const integer &", Type::Unspec, "const integer &");
    fromSig ("const integer &", Type::Unspec, "const integer &");

    fromSig ("block <integer >", Type::Unspec, "block <integer>");
    fromSig ("list <string >", Type::Unspec, "list <string>");
    fromSig ("map <symbol, float >", Type::Unspec, "map <symbol, float>");

    fromSig ("tuple <path, byteblock, term >", Type::Unspec, "tuple <path, byteblock, term>");
#endif

    fromSig ("list <list <integer >>", Type::Unspec, "list <list <integer>>");
#if 1
    fromSig ("block <list <integer >>", Type::Unspec, "block <list <integer>>");
    fromSig ("list <block <integer >>", Type::Unspec, "list <block <integer>>");

    fromSig ("integer ()", Type::Unspec, "integer ()");

    fromSig ("integer (boolean )", Type::Unspec, "integer (boolean)");
    fromSig ("integer (float, string )", Type::Unspec, "integer (float, string)");

    fromSig ("integer & (float, string )", Type::Unspec, "integer & (float, string)");

    fromSig ("integer (float &, string )", Type::Unspec, "integer (float &, string)");
    fromSig ("integer (float, string &)", Type::Unspec, "integer (float, string &)");

    fromSig ("integer (block <list <integer >>)", Type::Unspec, "integer (block <list <integer>>)");
    fromSig ("list <integer >(list <integer >)", Type::Unspec, "list <integer> (list <integer>)");

    fromSig ("const block <integer (float )> &", Type::Unspec, "const block <integer (float)> &");

    fromSig ("boolean (list <flex> , flex)", Type::Unspec, "boolean (list <<flex>>, <flex>)");
    fromSig ("list <flex> (variable <flex>, list <flex>, block <boolean >)", Type::Unspec, "list <<flex>> (variable <<flex>>, list <<flex>>, block <boolean>)");
    fromSig ("list <flex> (list <flex >)", Type::Unspec, "list <<flex>> (list <<flex>>)");
    fromSig ("void (string,  ...)", Type::Unspec, "void (string, ...)");
    fromSig ("list <flex> (list <list <flex>>)", Type::Unspec, "list <<flex>> (list <list <<flex>>>)");
#endif
    // list <list <string> > BLAH (map <string, string>)
    // The return type is constructed, so "(" can't be used

    // Even worse, a function returning a function pointer
    // integer (string) BLAH (boolean)
}
