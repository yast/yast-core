/**
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

#include <YCP.h>

#include <iostream>
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

/**
 * section description
 */
struct section
{
    section ()
	: begin (0), end (0), begin_out(0), end_out(0)
	    {
	    }
    ~section ()
	    {
		if (begin)
		    {
			regfree (begin);
			delete begin;
		    }
		if (end)
		    {
			regfree (end);
			delete end;
		    }
		if (begin_out)
		    delete [] begin_out;
		if (end_out)
		    delete [] end_out;
	    }
    regex_t* begin;
    bool end_valid;
    regex_t* end;
    char* begin_out;
    char* end_out;
};

/**
 * Parametr description
 */
struct param
{
    param ()
	: line (0), begin (0), end (0), out (0) 
	    {
	    }
    ~param ()
	    {
		if (line)
		    {
			regfree (line);
			delete line;
		    }
		if (begin)
		    {
			regfree (begin);
			delete begin;
		    }
		if (end)
		    {
			regfree (end);
			delete end;
		    }
		if (out)
		    delete [] out;
	    }
    regex_t* line;
    bool multiline_valid;
    regex_t* begin;
    regex_t* end;
    char* out;
};

struct rewrite
{
    rewrite () : in (0), out (0) {}
    ~rewrite ()
    {
	if (in)
	{
	    regfree (in);
	    delete in;
	}
	if (out)
	    delete [] out;
    }
    regex_t* in;
    char* out;
};

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
     * Number of regular expressions for comments over whole line.
     */
    int linecomment_len;
    /**
     * Regular expression for comments over whole line.
     */
    regex_t** linecomments;

    /**
     * Number of regular expressions for comments over part of the line.
     */
    int comment_len;
    /**
     * Regular expressions for comments over part of the line.
     */
    regex_t** comments;

    /**
     * Number of regular expressions for sections.
     */
    int section_len;
    /**
     * Regular expressions for sections.
     */
    section* sections;

    /**
     * Number of regular expressions for parameters (keys/values).
     */
    int param_len;
    /**
     * Regular expressions for parameters (keys/values).
     */
    param* params;

    /**
     * Number of regular expressions for rewrite rules.
     */
    int rewrite_len;
    /**
     * Regular expressions for rewrite rules.
     */
    rewrite* rewrites;

    /**
     * opened file for scanner
     */
    ifstream scanner;
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
    /**
     * Helper function to compile a regular expression.
     */
    int CompileRegex (regex_t**comp,const char*pattern);
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
    IniParser () :
	linecomment_len (0), linecomments (0), comment_len (0), comments (0),
	section_len (0), sections (0),
	param_len (0), params (0), rewrite_len (0), rewrites (0),
	started (false), multiple_files (false)
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
    string getFileName (const string&sec, int rb);
    /**
     * Using file name rewriting?
     */
    bool HaveRewrites () { return rewrite_len > 0; }
};

#endif//__IniParser_h__
