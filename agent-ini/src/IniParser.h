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

#ifndef __IniParser_h__
#define __IniParser_h__

#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <regex.h>
#include <locale.h>

#include <y2util/RepDef.h>
#include <YCP.h>

#include <iosfwd>
#include <fstream>
#include <string>
#include <vector>
#include <set>

#include "IniFile.h"

using std::string;
using std::vector;
using std::ifstream;
using std::ofstream;
using std::set;

DEFINE_BASE_POINTER (Regex_t);

#pragma GCC visibility push(hidden)

//! Set and later restore a locale category.
// It is restored when we go out of scope.
class TemporaryLocale
{
public:
    TemporaryLocale (int category, const char * locale);
    ~TemporaryLocale ();
private:
    //! call setlocale but log errors
    char *my_setlocale(int category, const char *locale);

    int _category;
    char * _oldlocale;
};
#pragma GCC visibility pop

/**
 * Wrapper to manage regex_t *
 * Must not be copied because regex_t is opaque
 */
class Regex_t : virtual public Rep
{
    REP_BODY (Regex_t); //! prohibits copying and assignment just like we want
private:
    friend class Regex;

    regex_t regex;		//! glibc regex buffer
    bool live; //! has regex been regcomp'd and should it be regfree'd?

public:
    Regex_t ():
	live (false) {}
    ~Regex_t () {
	if (live)
	{
	    regfree (&regex);
	}
    }
    /**
     * Initialize the regex
     * Can be called only once
     * @param pattern a pattern (REG_EXTENDED)
     * @bool ignore_case REG_ICASE?
     * @return 0 for success
     */
    int compile (const string& pattern, bool ignore_case) {
	int ret = -1;
	if (live)
	{
	    y2error ("Regex_t @%p already compiled", this);
	}
	else
	{
	    // #177560: [A-Za-z] excludes some ASCII letters in Estonian
	    TemporaryLocale tl (LC_ALL, "C");

	    ret = regcomp (&regex, pattern.c_str (),
			   REG_EXTENDED | (ignore_case ? REG_ICASE : 0));
	    if (ret)
	    {
		char error[256];
		regerror (ret, &regex, error, 256);
		y2error ("Regex_t %s error: %s", pattern.c_str (), error);
	    }
	    else
	    {
		live = true;
	    }
	}
	return ret;
    }
};

/**
 * Manages references to a regex buffer
 */
class Regex
{
    Regex_tPtr rxtp;
public:
    Regex (): rxtp (0) {}
    /**
     * Initialize the regex
     * @param pattern a pattern (REG_EXTENDED)
     * @bool ignore_case REG_ICASE?
     * @return 0 for success
     */
    int compile (const string& pattern, bool ignore_case) {
	if (rxtp)
	{
	    y2error ("Regex_t @%p already compiled", this);
	    return -1;
	}
	else
	{
	    rxtp = new Regex_t;
	    return rxtp->compile (pattern, ignore_case);
	}
    }
    const regex_t * regex () const { return & rxtp->regex; }
};

/**
 * Tries to match a string against a regex and holds results
 */
class RegexMatch
{
public:
    /** Matched subexpressions (0 - the whole regex) */
    vector<string> matches;
    /** The unmatched part of the string */
    string rest;

    /** @return i-th match */
    const string& operator[] (size_t i) { return matches[i]; }
    /** did the string match */
    operator bool () { return matches.size () > 0; }

    /**
     * @param rx a compiled regex
     * @param s  a string to match
     * @param nmatch how many rm_matches to reserve
     */
    RegexMatch (const Regex& rx, const string& s, size_t nmatch = 20) {
	// allocate at least for the whole match
	if (nmatch == 0)
	{
	    nmatch = 1;
	}
	regmatch_t rm_matches[nmatch];
	if (0 == regexec (rx.regex (), s.c_str (), nmatch, rm_matches, 0))
	{
	    // match
	    matches.reserve (nmatch);
	    rest = s.substr (0, rm_matches[0].rm_so) +
		s.substr (rm_matches[0].rm_eo);
	}
	else
	{
	    // no match
	    rm_matches[0].rm_so = -1;
	    rest = s;
	}

	size_t i;
	for (i = 0; i < nmatch && rm_matches[i].rm_so != -1; ++i)
	{
	    matches.push_back (s.substr (rm_matches[i].rm_so,
					 rm_matches[i].rm_eo - rm_matches[i].rm_so));
	}
    }
    
};

/**
 * Eg.\ rx: "^ *Section +(.*)$", out: "Section %s" (ini-agent)
 */
struct IoPattern
{
    Regex rx;
    string out;
};

/**
 * section description (ini-agent)
 */
struct section
{
    IoPattern begin;
    IoPattern end;
    bool end_valid;
};

/**
 * Parametr description (ini-agent)
 */
struct param
{
    /** single-line values, the normal case */
    IoPattern line;
    /** multiline begin */
    Regex begin;
    /** multiline end */
    Regex end;
    /** was multiline specified*/
    bool multiline_valid;
};

/// File description (name, section name, mtime); ini-agent
struct FileDescr
{
    /**
     * File name
     */
    string fn;
    /**
     * Section name
     */
    string sn;
    /**
     * Time of the last modification
     */
    time_t timestamp;
    FileDescr (char*fn);
    bool changed ();
    FileDescr () {}
};

/**
 * Contains info from scrconf file and ini file read routines.
 */
class IniParser
{
private:
    /**
     * Time of last modification of file, used in single file mode.
     */
    time_t timestamp;
    /**
     * Times of last modification of read files, used in multiple files
     * mode.
     */
    map<string,FileDescr> multi_files;
    /**
     * File name of the ini file -- single file mode only.
     */
    string file;
    /**
     * Get time stamp of file in sinble file mode.
     */
    time_t getTimeStamp();
    /** if there is \ at the end of line, next line is appended to the current one */
    bool line_can_continue;
    /** ignore case in regexps */
    bool ignore_case_regexps;
    /** ignore case in keys and section names */
    bool ignore_case;
    /** if ignore case, prefer upper case when saving */
    bool prefer_uppercase;
    /** if ignore case, outputs first upper and other lower
     * If not first_upper, nor prefer_uppercase is set, keys and values are
     * saved in lower case.
     */
    bool first_upper;
    /** nested sections are not allowed */
    bool no_nested_sections;
    /** values at the top level(not in section) are allowed */
    bool global_values;
    /** more values or sections of the same name are allowed */
    bool repeat_names;
    /** lines are parsed for comments after they are parsed for values */
    bool comments_last;
    /** multiline values are connected into one */
    bool join_multiline;
    /** do not kill empty lines at final comment at the end of top-level section */
    bool no_finalcomment_kill;
    /** read-only */
    bool read_only;
    /** ini file sections are created in flat-mode */
    bool flat;

    /** this string is printed before each line in subsections */
    string subindent;
    /**
     * Regular expression for comments over whole line.
     */
    vector<Regex> linecomments;
    /**
     * Regular expressions for comments over part of the line.
     */
    vector<Regex> comments;
    /**
     * Regular expressions for sections.
     */
    vector<section> sections;
    /**
     * Regular expressions for parameters (keys/values).
     */
    vector<param> params;
    /**
     * Regular expressions for rewrite rules.
     */
    vector<IoPattern> rewrites;

    /**
     * opened file for scanner
     */
    ifstream scanner;
    /**
     * name of scanned file
     */
    string scanner_file;
    /**
     * line number of scanned file
     */
    int scanner_line;

    /**
     * set to true in initMachine (after internal parsing structures are
     * initialized, when IniParser is ready to work).
     */
    bool started;

    /**
     * Multiple files mode or single file mode?
     */
    bool multiple_files;
    /**
     * Vector of globe-expressions.
     */
    vector<string> files;

    /**
     * Open ini file.
     */
    int scanner_start(const char*fn);
    /**
     * Close ini file.
     */
    void scanner_stop();
    /**
     * get line from ini file.
     */
    int scanner_get(string&s);

    /**
     * Parse one ini file and build a structure of IniSection.
     */
    int parse_helper(IniSection&ini);
    /**
     * Write one ini file.
     */
    int write_helper(IniSection&ini, ofstream&of,int depth);
public:
    /**
     * If Write (.s.section_name, nil) was called in multiple files mode,
     * than the file section_name has to be removed at the end. But as we
     * have file name rewrite rules, section_name needn't be file name.
     * Hence it is necessary to convert section_name to file name before
     * inserting to deleted_sections. <br>
     * Note: <tt>Write (.s.section_name, nil); Write (.v.section_name.k, "v");</tt>
     * means that section is deleted at first and created again later. In
     * this case file isn't removed!
     */
    set<string> deleted_sections;
    /**
     * Toplevel ini section.
     */
    IniSection inifile;
    // apparently the uninitialized members are filled in
    // by the grammar definition
    IniParser () :
	timestamp (0),
	linecomments (), comments (),
	sections (), params (), rewrites (),
	started (false), multiple_files (false),
//	inifile ("toplevel")
	inifile (this)
	    {}
    ~IniParser ();
    /**
     * Sets parser to single file mode and sets the file name to read.
     * @param fn file name of ini file
     */
    void initFiles (const char*fn);
    /**
     * Sets parser to multiple file mode and sets the glob-expressions.
     * @param f list of glob-expressions
     */
    void initFiles (const YCPList&f);
    /**
     * Sets flags and regular expressions.
     * @param scr control script
     * @return 0 if successful
     */
    int initMachine (const YCPMap&scr);
    bool isStarted() { return started; }

    /**
     * Parse the ini files. Parser must be started before this function
     * is called.
     */
    int parse();
    /**
     * Check the ini files and in case some of them changed externally,
     * reload it.
     */
    void UpdateIfModif ();

    /**
     * Write changed ini files on disk
     */
    int write ();

    /**
     * Does a section have end-mark defined?
     * @param i index of section rule
     * @return true if yes
     */
    bool sectionNeedsEnd (int i) { return sections[i].end_valid; }

    /**
     * Get the file name of section. If there is a rewrite rule rb,
     * rewrites section name to file name using the rule rb.
     * @param sec section name
     * @param rb index of rewrite rule
     * @return rewritten file name
     */
    string getFileName (const string&sec, int rb) const;
    /**
     * Using file name rewriting?
     */
    bool HaveRewrites () const { return rewrites.size () > 0; }

    //! accessor method
    bool repeatNames () const { return repeat_names; }
    //! accessor method
    bool isFlat () const { return flat; }

    /**
     * change case of string
     * @param str string to change
     * @return changed string
     */
    string changeCase (const string&str) const;
};

#endif//__IniParser_h__
