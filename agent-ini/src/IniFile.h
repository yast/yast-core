/**							-*- c++ -*-
 * YaST2: Core system
 *
 * Description:
 *   YaST2 SCR: Ini file agent.
 *
 * Authors:
 *   Petr Blahos <pblahos@suse.cz>
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
};

/**
 * Used in list of entris. Each entry may be 
 * key/value pair or section.
 */
struct IniName
{
    /**
     * name of key or section?
     */
    string name;
    /**
     * is it a section or key?
     */
    enum IniType { VALUE, SECTION,} type;
    IniName (const string&s,const IniType t) : name (s), type (t) {}
    bool operator ==(const IniName& x) { return name == x.name && type == x.type; }
    bool isSection() {  return type == SECTION; }
};

typedef list<IniName> IniFileIndex;

class IniSection;
typedef map<const string,IniSection> IniSectionMap;
typedef map<const string,IniSection>::iterator IniSectionMapIterator;

typedef map<const string,IniEntry> IniEntryMap;
typedef map<const string,IniEntry>::iterator IniEntryMapIterator;

/**
 * Section definition.
 */
class IniSection : public IniBase
{
private:
    /** ignore case (in key names and section names) */
    bool ignore_case;
    /** 
     * style of case ignoration: 0 prefere lower case, 2: prefere upper
     * case, 2: first upper, other lower
     */
    int ignore_style;
    /** section may contain values */
    bool allow_values;
    /** section may contain sections */
    bool allow_sections;
    /* may sub-sections of this section have subsections? */
    bool allow_subsub;
    /* special flat mode for RcFile mode */
    bool flat;

    /**
     * Index of all values and sections. 
     * Values and sections appear in file in the 
     * same order as are in this file
     */
    IniFileIndex index;

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
     * values contained by this section
     */
    IniEntryMap values;
    /**
     * Sections contained by this section
     */
    IniSectionMap sections;

    /**
     * Get a value on a path
     * @param p path to value
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
     * @param p path to value
     * @param out output is placed here as YCPString or YCPInteger
     * @param what 0 - comment, 1 - rewrite_by, other - read-by
     * @param depth Index of current path component. This function is
     * recursive and depth marks the depth of recursion. We look for
     * path[depth] in current "scope"
     * @return 0 in case of success
     */
    int getSectionProp (const YCPPath&p, YCPValue&out,int what, int depth = 0);
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
    int setValueFlat (const YCPPath&p, const YCPValue&out);
    /**
     * Delete value in flat mode
     */
    int delValueFlat (const YCPPath&p);
    /**
     * Get list of values in flat mode.
     */
    int dirValueFlat (const YCPPath&p, YCPList&l);
public:
    IniSection ()
	: IniBase (-1),
	  ignore_case (false), ignore_style (0), allow_values (true), 
	  allow_sections (true), allow_subsub (true), flat (false),
	  index (), end_comment (), 
	  rewrite_by(-1), values(), sections()
	    {}
    virtual ~IniSection () {}

    /** 
     * this is a constructor for newly added sections --> sets dirty 
     * @param ic ignore case
     * @param is ignore style
     * @param allow_ss allow sub sections
     * @param n name of section
     */
    IniSection (bool ic, int is, bool allow_ss, string n)
	: IniBase (n),
	  ignore_case (ic), ignore_style (is), allow_values (true), 
	  allow_sections (allow_ss), allow_subsub (allow_ss), flat (false),
	  index (), end_comment (),
	  rewrite_by(0), values(), sections()
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
     */
    void initSection (const string&name,const string&comment,int rb, int wb=-2);
    /**
     * This function has very special purpose, it ensures that top-section
     * delimiter is not written when saving multiple files.
     */
    void initReadBy () { read_by = -1; }

    /** sets dirty flag also */
    void setRewriteBy (int c) 	     	{ dirty = true; rewrite_by = c; }
    /** 
     * If there is no comment at the beginning and no values and no
     * sections, it is better to set is as comment at the beginning.
     * Sets also dirty flag.
     * @param c comment
     */
    void setEndComment (const char*c);

    /**
     * Set ignore case and style.
     */
    void setIgnoreCase (bool ic, int style);
    /**
     * Set nesting style.
     */
    void setNesting (bool no_sub_sec, bool global_val);
    
    int getRewriteBy() { return rewrite_by; }
    /**
     * Gets section on a path. Recursive. Attention! This function
     * aborts when it doesn't find the section! Use with care!
     * @param path path to the section
     * @param from recursion depth
     * @return Found ini section iterator
     */
    IniSectionMapIterator findSection(const vector<string>&path, int from = 0);
    /**
     * If currently parsed end-section-tag hasn't matched currently
     * processed section, we need to find the best possible match. Hence we
     * look for a section on current path which can be closed by found
     * end-section-tag. We look from end (the most recently open section)
     * up (to the tree root). Note: this function can abort if the path
     * passed in invalid.
     * @param path stack of the sections
     * @param wanted read-by we want to find
     * @param found let unset
     * @param from let unset
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

    /**
     * return index of all subsections and keys
     */
    IniFileIndex& getIndex () { return index; }
    /**
     * Aborts if entry doesn't exist!
     * @param name name of the entry to get
     * @return entry
     */
    IniEntry& getEntry (const char*name);
    /**
     * Aborts if section doesn't exist!
     * @param name name of the section to get
     * @return section
     */
    IniSection& getSection (const char*name);
    /**
     * @param name name of a section
     * @return rewrite-by of section or -1 if the section wasn't found
     */
    int getSubSectionRewriteBy (const char*name);

    bool isDirty ();
    /** set all subsection and values to clean */
    virtual void clean();
    void setFlat ()  { flat  = true; }

    const char* getEndComment() const { return end_comment.c_str(); }
};

#endif//__IniFile_h__
