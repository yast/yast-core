/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                        (C) SuSE GmbH |
\----------------------------------------------------------------------/

   File:       YCPPath.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPPath_h
#define YCPPath_h


#include "YCPValue.h"
#include <y2util/Ustring.h>

#include "ycp/SymbolEntry.h"

/**
 * @short YCPValueRep representing a data path
 * A YCP path describes a subtree in a YCP data structure. Non-leaf-nodes
 * are values of type list and term. A path is a list of path
 * components. A path component is a symbol or a number.
 *
 * YCPSyntax: A single dot or any sequence of dot-symbol pairs. The symbols
 * may consist of digits, letters and underscores.
 * <pre>.  .etc  .1.2.127  .etc.fstab.7</pre>
 */
class YCPPathRep : public YCPValueRep
{
    struct Component
    {
        Ustring component; // component string
        bool complex;     // true if component is quoted by " in source. false otherwise (component contains only a-zA-Z0-9-_)
        Component() : component (SymbolEntry::emptyUstring), complex (false) {}
        Component(string s);  // for initial creation. Unquotes and unescapes
        Component(bytecodeistream & str);  // for initial creation
        int compare(const Component&to) const {
            return component.asString().compare(to.component.asString());
        }
        string toString() const;
	std::ostream & toStream (std::ostream & str) const;
    };

    vector<Component> components;

protected:
    friend class YCPPath;

    /**
     * Creates a new root path. The ASCII representation of
     * the root path is a single dot.
     */
    YCPPathRep();

    /**
     * Creates a new path from its ASCII representation.
     */
    YCPPathRep(const char *r);

    /**
     * Appends component to path.
     */
    void append(const Component&c);
public:

    /**
     * Returns true, if this is a root path.
     */
    bool isRoot() const;

    /**
     * Selects the subtree of a YCPValueRep, that is denoted by
     * this path. If this is the root path, select(v) simply
     * returns v. If v is a list or a term, the first component
     * of the path selects an element of the list and evalutes
     * recursivly remainingpath->select(list_element).
     * Returns a 0 pointer, if the path was incorrect.
     */
    YCPValue select(const YCPValue& val);

    /**
     * Append path to this
     */
    void append(const YCPPath&p);

    /**
     * Appends a component to the path.
     */
    void append(string c);

    /**
     * Returns the length of the path, i.e. the number of
     * components. The root path has length 0.
     */
    long length() const;

    /**
     * Checks if this path is a prefix of path p. This holds, if
     * this path has size n and the first n components of p are
     * exactly those of this path. The root path is prefix of
     * any path. That path .a.b is a prefix of .a.b.c but not
     * of the path .a.bc
     */
    bool isPrefixOf(const YCPPath& p) const;

    /**
     * Returns a postfix of the path. You must check, that the
     * index you give is 0 <= i < @ref #length.
     * @return Postfix path. destroy it after use.
     */
    YCPPath at(long index) const;

    /**
     * Returns one component of the path as string. No error check
     * is done for index. You must check yourself that 0 < index < @ref #length.
     */
    string component_str(long index) const;

    /**
     * Compares two YCPPaths for equality, greaterness or smallerness.
     * @param v value to compare against
     * @return YO_LESS, if this is smaller than v,
     *         YO_EQUAL,   if this is equal to v,
     *         YO_GREATER, if this is greater to v
     */
    YCPOrder compare(const YCPPath &v) const;

    /**
     * Returns a string representation of this object, that may
     * be parsed by the YCP parser. A path is denoted by a
     * list symbols or number separated by dots, e.g.
     * . or .12 or .eth0.12
     */
    string toString() const;

    /**
     * Output value as bytecode to stream
     */
    std::ostream & toStream (std::ostream & str) const;

    /**
     * Returns YT_PATH. See @ref YCPValueRep#valuetype.
     */
    YCPValueType valuetype() const;
};

/**
 * @short Wrapper for YCPPathRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPPathRep
 * with the arrow operator. See @ref YCPPathRep.
 */
class YCPPath : public YCPValue
{
    DEF_COMMON(Path, Value);
public:
    YCPPath() : YCPValue(new YCPPathRep()) {}
    YCPPath(const char *r) : YCPValue(new YCPPathRep(r)) {}
    YCPPath(string s) : YCPValue(new YCPPathRep(s.c_str())) {}
    YCPPath(bytecodeistream & str);
};

#endif   // YCPPath_h
