/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|						     (C) SuSE Linux AG |
\----------------------------------------------------------------------/

   File:	YCPBuiltinPath.cc

   Authors:	Klaus Kaempf <kkaempf@suse.de>
		Arvin Schnell <arvin@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

$Id$
/-*/

#include "ycp/YCPBuiltinPath.h"
#include "ycp/YCPPath.h"
#include "ycp/YCPString.h"
#include "ycp/YCPInteger.h"
#include "ycp/StaticDeclaration.h"


extern StaticDeclaration static_declarations;


static YCPValue
p_size (const YCPPath &path)
{
    /**
     * @builtin size (path p) -> integer
     * Returns the number of path elements of the path p, i.e. the
     * length of <tt>p</tt>.
     *
     * Examples: <pre>
     * size (.hello.world) -> 2
     * size (.) -> 0
     * </pre>
     */
     
    if (path.isNull ())
	return YCPNull ();

    return YCPInteger (path->length ());
}


static YCPValue
p_add (const YCPPath &path, const YCPString &s)
{
    /**
     * @builtin add (path p, string s) -> path
     * Returns <tt>p</tt> with added path element created from string
     * <tt>s</tt>.
     *
     * Example: <pre>
     * add (.aaa, "anypath...\n\"") -> .aaa."anypath...\n\""
     * </pre>
     */

    if (path.isNull () || s.isNull ())
	return YCPNull ();

    YCPPath ret;
    ret->append (path);
    ret->append (s->value ());
    return ret;
}


static YCPValue
p_plus (const YCPPath &path1, const YCPPath &path2)
{
    /**
     * @builtin path p1 + path p2 -> path
     * Returns <tt>p1</tt> with added <tt>p2</tt> element created from
     * string <tt>s</tt>.
     *
     * Example: <pre>
     * .aaa + "anypath...\n\"" -> .aaa."anypath...\n\""
     * </pre>
     */

    if (path1.isNull () || path2.isNull ())
	return YCPNull ();

    YCPPath ret;
    ret->append (path1);
    ret->append (path2);
    return ret;
}


static YCPValue
p_topath (const YCPValue &v)
{
    /**
     * @builtin topath (string s) -> path
     * Converts a value to a path.
     * If the value can't be converted to a path, nilpath is returned.
     *
     * Example: <pre>
     * topath ("path") -> .path
     * topath (".some.path") -> .some.path
     * </pre>
     */

    if (v.isNull())
    {
	return v;
    }
    else if (v->valuetype() == YT_PATH)
    {
	return v->asPath();
    }
    else if (v->valuetype() == YT_STRING)
    {
	string s = v->asString()->value();
	if (s[0] != '.')
	{
	    s = "." + s;
	}
	return YCPPath (s);
    }
    return YCPNull();
}


YCPBuiltinPath::YCPBuiltinPath ()
{
    // must be static, registerDeclarations saves a pointer to it!
    static declaration_t declarations[] = {
	{ "+",	    "path (path, path)",	(void *)p_plus },
	{ "+",	    "path (path, string)",	(void *)p_add },
	{ "size",   "integer (path)",		(void *)p_size },
	{ "add",    "path (path, string)",	(void *)p_add },
	{ "add",    "path (path, path)",	(void *)p_plus },
	{ "topath", "path (any)",		(void *)p_topath },
	{ 0 }
    };

    static_declarations.registerDeclarations ("YCPBuiltinMap", declarations);
}
