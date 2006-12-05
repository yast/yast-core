/**							-*- c++ -*-
 * YaST2: Core system
 *
 * Description:
 *   YaST2 SCR: Ini file agent.
 *
 * Authors:
 *   Petr Blahos <pblahos@suse.cz>
 *   Martin Vidner <mvidner@suse.cz>
 *
 * $Id$
 */

#ifndef __IniFile_h__
#define __IniFile_h__

#include <string>
#include <map>
#include <list>
#include <vector>

#include <YCP.h>

using std::map;
using std::multimap;
using std::list;
using std::vector;
using std::string;

/**
 * Base class of IniEntry and IniSection.
 * This keeps name, its comment and index of rule it was read by.
 * set* functions are used from ycp code to change values.
 * init* functions are set when reading file from disk
 */
class IniBase
{
protected:
    /** name */
    string name;
    /** comment */
    string comment;
    /** index to params/sections in IniParser using which this item was read */
    int read_by;
    /** changed? */
    bool dirty;

    /** IniSection default ctor sets to -1. Why?*/
    IniBase (int rb)
	: name (), comment (), read_by (rb), dirty (false) {}
    /** Used by another IniSection ctor */
    IniBase (const string &n)
	: name (n), comment (), read_by (0), dirty (true) {}
public:
    virtual ~IniBase () {}

    const char* getName()    const { return name.c_str();    }
    const char* getComment() const { return comment.c_str(); }
    int getReadBy()          const { return read_by;   }

    /** set dirty flag to false */
    virtual void clean() { dirty = false; }

    /** changes and sets dirty flag */
    void setName(const string&c)    { dirty = true; name = c;    }
    /** changes and sets dirty flag */
    void setComment(const string&c) { dirty = true; comment = c; }
    /** changes and sets dirty flag */
    void setReadBy(int r)	    { dirty = true; read_by = r; }
    /** sets dirty flag */
    void setDirty()		    { dirty = true; }

    /** changes value only if not dirty */
    void initName(const string&c)    { if (!dirty) name = c;    }
    /** changes value only if not dirty */
    void initComment(const string&c) { if (!dirty) comment = c; }
    /** changes value only if not dirty */
    void initReadBy(const int r)     { if (!dirty) read_by = r; }

    /** changes values only if not dirty */
    void init(const string &n,const string&c, int rb)
	    {
		if (!dirty)
		    {
			name = n;
			comment = c;
			read_by = rb;
		    }
	    }

protected:
    /**
     * Constructs a map of the fields, for Read (.all...)
     */
    virtual YCPMap getAllDoIt () {
	YCPMap m;

	m->add (YCPString ("name"), YCPString (name));
	m->add (YCPString ("type"), YCPInteger (read_by));
	m->add (YCPString ("comment"), YCPString (comment));
	return m;
    }

    //! helper for setAllDoIt
    bool getMapString (const YCPMap &in, const string &k, string &s) {
	YCPValue v = in->value (YCPString (k));
	if (v.isNull () || !v->isString ())
	{
	    y2error ("Missing in Write (.all): %s", k.c_str ());
	    return false;
	}
	s = v->asString ()->value ();
	return true;
    }

    //! helper for setAllDoIt
    bool getMapInteger (const YCPMap &in, const string &k, int &i) {
	YCPValue v = in->value (YCPString (k));
	if (v.isNull () || !v->isInteger ())
	{
	    y2error ("Missing in Write (.all): %s", k.c_str ());
	    return false;
	}
	i = v->asInteger ()->value ();
	return true;
    }

    virtual int setAllDoIt (const YCPMap &in) {
	dirty = true;

	bool ok = true;
	ok = ok && getMapString (in, "name", name);
	ok = ok && getMapInteger (in, "type", read_by);
	ok = ok && getMapString (in, "comment", comment);
	return ok? 0: -1;
    }
};

/**
 * Only the string value in addition to IniBase
 */
class IniEntry : public IniBase
{
private:
    /** value */
    string val;
public:
    IniEntry ()
	: IniBase (0), val () {}
    /** explicit uninitialized constructor */
    IniEntry (const char *u): IniBase (u) {}

    const char* getValue()   const { return val.c_str();     }

    void setValue(const string&c)   { dirty = true; val = c;     }

    /** changes value only if not dirty */
    void initValue(const string&c)   { if (!dirty) val = c;     }
    /** changes value only if not dirty */
    void initReadBy(const int r)     { if (!dirty) read_by = r; }

    /** changes values only if not dirty */
    void init(const string &n, const string &c, int rb, const string &v)
	    {
		if (!dirty)
		    {
			val = v;
			IniBase::init (n, c, rb);
		    }
	    }

    YCPMap getAllDoIt () {
	YCPMap m = IniBase::getAllDoIt ();
	m->add (YCPString ("kind"), YCPString ("value"));
	m->add (YCPString ("value"), YCPString (val));
	return m;
    }

    int setAllDoIt (const YCPMap &in) {
	int ret = IniBase::setAllDoIt (in);
	if (ret == 0)
	{
	    string kind;
	    if (!getMapString (in, "kind", kind) || kind != "value")
	    {
		y2error ("Kind should be 'value'");
		return -1;
	    }

	    if (!getMapString (in, "value", val))
	    {
		return -1;
	    }
	}
	return ret;
    }
};

class IniSection;


enum IniType { VALUE, SECTION,};
struct IniContainerElement;

typedef list<IniContainerElement> IniContainer;
typedef IniContainer::iterator IniIterator;

/** indices */
typedef multimap<string, IniIterator> IniEntryIndex;
typedef multimap<string, IniIterator> IniSectionIndex;
/**
  Watch it, "find" finds an iterator pointing at an iterator (which is
  in a pair with the key, uninteresting).
  Usage:
    IniSectionIdxIterator sxi = isections.find (key);
    if (sxi != isections.end ())
      IniSectionIterator si = sxi->second;
 */
typedef IniEntryIndex::iterator IniEntryIdxIterator;
typedef IniSectionIndex::iterator IniSectionIdxIterator;

class IniParser;

/**
 * Section definition.
 */
class IniSection : public IniBase
{
private:
    // huh??? allow_values, allow_sections and allow_subsub
    // were never actuially used

    /** The parser, queried about global settings
     * But once the const is discarded to add to deleted_sections
     */
    const IniParser *ip;

    /**
     * if this is global section, there may be comment at the end
     * this is quite special case, it is impossible to change it
     */
    string end_comment;

    /** index to IniParser::rewrites for filename - section name mapping
     * It appears that read_by was used for both purposes,
     * causing bug (#19066).
     */
    int rewrite_by;

    /**
     * What entries of cvalues and csections are valid
     * Values contained by this section
     * Sections contained by this section
     */
    IniContainer container;
    // these must be kept up to date!
    /**
     * Index of values
     */
    IniEntryIndex ivalues;
    /**
     * Index of sections
     */
    IniSectionIndex isections;

    /** build ivalues and isections */
    void reindex ();

    /**
     * Get a value (or list of them if repeat_names) in this section
     * It would be enough to pass only k instead of p and depth,
     * but then the error messages would not know the whole path
     * @param p path
     * @param out output is placed here as YCPString or YCPInteger
     * @param what 0 - value, 1 - comment, other - read-by
     * @param depth path index
     * @return 0 in case of success
     */
    int getMyValue (const YCPPath &p, YCPValue &out, int what, int depth);
    /**
     * Get a value on a path
     * @param p path to value: .value.sec1.sec2.key
     * @param out output is placed here as YCPString or YCPInteger
     * @param what 0 - value, 1 - comment, other - read-by
     * @param depth Index of current path component. This function is
     * recursive and depth marks the depth of recursion. We look for
     * path[depth] in current "scope"
     * @return 0 in case of success
     */
    int getValue (const YCPPath&p, YCPValue&out,int what, int depth = 0);
    /**
     * Get section property -- comment or read-by
     * @param p path to value: .section_commment.sec1.sec2.sec3
     * @param out output is placed here as YCPString or YCPInteger
     * @param what 0 - comment, 1 - rewrite_by, other - read-by
     * @param depth Index of current path component. This function is
     * recursive and depth marks the depth of recursion. We look for
     * path[depth] in current "scope"
     * @return 0 in case of success
     */
    int getSectionProp (const YCPPath&p, YCPValue&out,int what, int depth = 0);
    /**
     * Get a complete subtree
     * @param p path : .all or .all.sec1.sec2
     * @param out output is placed here
     * @param depth Index of current path component. This function is
     * recursive and depth marks the depth of recursion. We look for
     * path[depth] in current "scope"
     * @return 0 in case of success
     */
    int getAll (const YCPPath&p, YCPValue&out, int depth);
    /**
     * Gets data for this section and all its values and subsections
     */
    YCPMap getAllDoIt ();

    /**
     * Get directory of this section
     * @param l result list
     * @param what VALUE or SECTION
     * @return 0 in case of success
     */
    int myDir (YCPList& l, IniType what);

    /**
     * Recursive function to find the one section we want to dir
     * and at last to do dir.
     * @param p path
     * @param out list of sections/keys
     * @param sections get sections (0) or values (!0)?
     * @param depth see getSectionProp
     * @return 0 in case of success
     */
    int dirHelper (const YCPPath&p, YCPList&out,int sections,int depth = 0);
    /**
     * Set a value (or list of them if repeat_names) in this section
     * It would be enough to pass only k instead of p and depth,
     * but then the error messages would not know the whole path
     * @param p path
     * @param in value to set (YCPString or YCPInteger) (or YCPList)
     * @param what 0 -- value, 1 -- comment, other -- read-by.
     * @param depth path index
     * @return 0 on success
     */
    int setMyValue (const YCPPath &p, const YCPValue&in, int what, int depth);
    /**
     * Set value on path. Creates recursively all non-existing subsections.
     * @param p path to set value on
     * @param in value to set (YCPString or YCPInteger)
     * @param what 0 -- value, 1 -- comment, other -- read-by.
     * @param depth see getSectionProp
     * @return 0
     */
    int setValue (const YCPPath&p,const YCPValue&in,int what, int depth = 0);
    /**
     * Set section comment or read-by. Creates recursively all non-existing subsections.
     * @param p path to set value on
     * @param in value to set (YCPString or YCPInteger)
     * @param what 0 -- comment, 1 - rewrite_by, other -- read-by.
     * @param depth see getSectionProp
     * @return 0
     */
    int setSectionProp (const YCPPath&p,const YCPValue&in, int what, int depth);
    /**
     * Set all properties and values for a section.
     * No recursive creation of the specified path.
     * @param p path where to start
     * @param in value to set
     * @param depth see getSectionProp
     * @return 0 in case of success
     */
    int setAll (const YCPPath&p, const YCPValue& in, int depth);
    /**
     * Set all properties and values for a section.
     * @param in value to set
     * @return 0 in case of success
     */
    int setAllDoIt (const YCPMap &in);
    /**
     * Delete value on path
     * @param p path to delete value at
     * @param depth see getSectionProp
     * @return 0 in case of success
     */
    int delValue (const YCPPath&p, int depth);
    /**
     * Delete section on path. Deletes also all its subsections.
     * @param p path to delete value at
     * @param depth see getSectionProp
     * @return 0 in case of success
     */
    int delSection (const YCPPath&p, int depth);

    /**
     * deletes all values of this name we own
     * @param k normalized key
     */
    void delMyValue (const string &k);
    /**
     * deletes a section we own
     */
    void delValue1 (IniEntryIdxIterator exi);
    /**
     * deletes a section we own
     */
    void delSection1 (IniSectionIdxIterator sxi);

    /**
     * Get value in flat mode.
     * @param p path to value
     * @param out output
     * @return 0 in case of success
     */
    int getValueFlat (const YCPPath&p, YCPValue&out);
    /**
     * Set value in flat mode.
     * @param p path to value
     * @param out input
     * @return 0 in case of success
     */
    int setValueFlat (const YCPPath&p, const YCPValue& in);
    /**
     * Delete value in flat mode
     */
    int delValueFlat (const YCPPath&p);
    /**
     * Get list of values in flat mode.
     */
    int dirValueFlat (const YCPPath&p, YCPList&l);
//    IniSection ();
public:
    /** explicit uninitialized constructor */
    IniSection (const char *u): IniBase (u) {}

    IniSection (const IniParser *p)
	: IniBase (-1),
	  ip (p),
	  end_comment (), rewrite_by(-1),
	  container (), ivalues (), isections ()
	    {}

    /**
     * Must define an own copy constructor
     * so that the indices point to the copy, not the original
     */
    IniSection (const IniSection &s) :
	IniBase (s),
	  ip (s.ip),
	  end_comment (s.end_comment), rewrite_by (s.rewrite_by),
	  container (s.container)
	{ reindex (); }

    void operator = (const IniSection &s)
	{
	    if (&s == this)
	    {
		return;
	    } 
	    IniBase::operator = (s);
	    ip = s.ip;
	    end_comment = s.end_comment; rewrite_by = s.rewrite_by;
	    container = s.container;

	    reindex ();
	}

    virtual ~IniSection () {}

    /** 
     * this is a constructor for newly added sections --> sets dirty 
     * @param ip parser to take options from
     * @param n name of section
     */
    IniSection (const IniParser *p, string n)
	: IniBase (n),
	  ip (p),
	  end_comment (), rewrite_by(0),
	  container(), ivalues (), isections ()
	    {}
    /**
     * If value doesn't exist, creates new, otherwise calls method init of
     * the existing one.
     * @param key key
     * @param val value
     * @param comment comment
     * @param rb read-by
     */
    void initValue (const string&key,const string&val,const string&comment,int rb);
    /**
     * If section already exist, it is updated only in case, that it isn't
     * dirty.
     * @param name section name
     * @param comment comment
     * @param rb read-by
     * @param wb rewrite-by. if -2 (default), it is not changed
     * @return a reference to the section
     */
    IniSection& initSection (const string&name,const string&comment,int rb, int wb=-2);
    /**
     * This function has very special purpose, it ensures that top-section
     * delimiter is not written when saving multiple files.
     */
    void initReadBy () { read_by = -1; }

    /** sets dirty flag also */
    void setRewriteBy (int c) 	     	{ dirty = true; rewrite_by = c; }
    int getRewriteBy() { return rewrite_by; }
    /**
     * @param name name of a section
     * @return rewrite-by of section or -1 if the section wasn't found
     */
    int getSubSectionRewriteBy (const char*name);

    /** 
     * If there is no comment at the beginning and no values and no
     * sections, it is better to set is as comment at the beginning.
     * Sets also dirty flag.
     * @param c comment
     */
    void setEndComment (const char*c);
    const char* getEndComment() const { return end_comment.c_str(); }

    bool isDirty ();
    /** set all subsection and values to clean */
    virtual void clean();

    /**
     * OBSOLETE, to be removed.
     * Gets section on a path. Recursive. Attention! This function
     * aborts when it doesn't find the section! Use with care!
     * (Used only by IniParser::parse_helper)
     * @param path path to the section
     * @param from recursion depth
     * @return Found ini section iterator
     */
    IniSection& findSection(const vector<string>&path, int from = 0);
    /**
     * OBSOLETE, to be removed.
     * If currently parsed end-section-tag hasn't matched currently
     * processed section by name, we need to find the best possible match
     * by type (read_by). Hence we
     * look for a section on current path which can be closed by found
     * end-section-tag. Note: this function can abort if the path
     * passed in invalid.
     * @param path stack of the sections
     * @param wanted read-by we want to find
     * @param found let unset, last path index that matched
     * @param from let unset, current path index
     * @return index to path
     */
    int findEndFromUp(const vector<string>&path, int wanted, int found = -1, int from = 0);

    /**
     * Dump a section with subsections and subvalues to stdout.
     */
    void Dump ();

    /**
     * Generic interface to SCR::Read
     * @param rewrite a #19066 hack - if rewriting is active, .st accesses rewrite_by
     */
    int Read (const YCPPath&p, YCPValue&out, bool rewrite);
    /**
     * Generic interface to SCR::Dir
     */
    int Dir (const YCPPath&p, YCPList&out);
    /**
     * Generic interface to SCR::Write
     * @param rewrite a #19066 hack - if rewriting is active, .st accesses rewrite_by
     */
    int Write (const YCPPath&p, const YCPValue&v, bool rewrite);
    /**
     * Generic delete for values, sections.
     * @param p path to delete
     * 	@return 0: success
     */
    int Delete (const YCPPath&p);

    // used by IniParser::write
    IniIterator getContainerBegin ();
    IniIterator getContainerEnd ();

    /**
     * Aborts if entry doesn't exist!
     * @param name name of the entry to get
     * @return entry
     */
//unused
//    IniEntry& getEntry (const char*name);
    /**
     * Aborts if section doesn't exist!
     * TODO gets any of multiple sections
     * @param name name of the section to get
     * @return section
     */
    IniSection& getSection (const char*name);
};

/**
 * A single container 
 */
class IniContainerElement
{
    IniType	_t;
    IniEntry	_e;
    IniSection	_s;

public:
    /// accessors
    IniType t () const { return _t; }
    const IniEntry& e () const { return _e; }
          IniEntry& e ()       { return _e; }
    const IniSection& s () const { return _s; }
          IniSection& s ()       { return _s; }

    /// construct from a value
    IniContainerElement (const IniEntry& e) :
	_t (VALUE),
	_e (e),
//	_s (IniSection ("uninitialized"))
	_s (IniSection ((const IniParser *)NULL))
	{}

    /// construct from a section
    IniContainerElement (const IniSection& s) :
	_t (SECTION),
//	_e (IniEntry ("uninitialized")),
	_e (IniEntry ()),
	_s (s)
	{}

    IniContainerElement (const IniContainerElement& c) :
	_t (c._t),
	_e (c._e),
	_s (c._s)
	{}
};


#endif//__IniFile_h__
