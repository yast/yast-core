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

#include <y2util/PathInfo.h>
#include <ycp/y2log.h>
#include <vector>
#include <set>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <glob.h>
#include <cassert>

#include "IniParser.h"
#include "IniFile.h"

IMPL_BASE_POINTER (Regex_t);

IniParser::~IniParser ()
{
    // regex deallocation used to be here
}
/**
 * Debugging.
 */
void printPath(const vector<string>&p, const char*c = "")
{
    int i = 0;
    int len = p.size();
    printf("%s:", c);
    for (;i<len;i++)
	printf("%s ", p[i].c_str());
    printf("\n");
}
void y2errPath (const vector<string>&p, const char*c = "")
{
    string out = c;
    int i = 0;
    int len = p.size();
    for (;i<len;i++)
	out = out + p[i] + " ";
    y2error (out.c_str());
}

bool onlySpaces (const char*str)
{
    while (*str)
    {
	if (' ' != *str && '\t' != *str && '\r' != *str && '\n' != *str)
	    return false;
	str++;
    }
    return true;
}

bool isYCPStringPair (const YCPValue &v)
{
    if (!v->isList ())
	return false;
    YCPList l = v->asList ();
    if (l->size () != 2)
	return false;
    return
	l->value (0)->isString () &&
	l->value (1)->isString ();
}

/**
 * Return 0 if there is:
 * $[ "begin" : [ "...", "...", ],
 *    "end"   : [ "...", "...", ],]
 *        1 if there is:
 * $[ "begin" : [ "...", "...", ],]
 *       -1 if format is totaly broken
 */
int getBeginEndType (const YCPMap&m)
{
    if (m->value(YCPString("begin")).isNull())
	return -1;
    if (!isYCPStringPair (m->value (YCPString ("begin"))))
	return -1;
    
    if (m->value(YCPString("end")).isNull())
	return 1;
    if (!isYCPStringPair (m->value (YCPString ("end"))))
	return 1;

    return 0;
}

/**
 * Returns 0 if there is:
 * $[ "match"     : [ "...", "...", ],
 *    "multiline" : [ "...", "...", ],
 * ]
 *         1 if there is:
 * $[ "match"     : [ "...", "...", ],]
 *        -1 otherwise
 */
int getParamsType (const YCPMap&m)
{
    if (m->value(YCPString("match")).isNull())
	return -1;
    if (!isYCPStringPair (m->value (YCPString ("match"))))
	return -1;
    
    if (m->value(YCPString("multiline")).isNull())
	return 1;
    if (!isYCPStringPair (m->value (YCPString ("multiline"))))
	return 1;

    return 0;
}

void IniParser::initFiles (const YCPList&f)
{
    multiple_files = true;
    files.clear ();

    int len = f->size();
    for (int i = 0;i<len;i++)
	if (f->value (i)->isString ())
	    files.push_back (f->value (i)->asString()->value());
	else
	    y2error ("Bad file specification: %s", f->value (i)->toString ().c_str());
}
void IniParser::initFiles (const char*fn)
{
    file = fn;
    multiple_files = false;
}
int IniParser::initMachine (const YCPMap&scr)
{
    started = true;

    //
    // now process the script
    //
    ignore_case_regexps = ignore_case = prefer_uppercase = first_upper = line_can_continue = no_nested_sections =
	global_values = repeat_names = comments_last = join_multiline =
	no_finalcomment_kill = read_only = flat = false;

    // read the options
    YCPValue v = scr->value(YCPString("options"));
    if (!v.isNull() && v->isList ())
	{
#define COMPARE_OPTION(X) if (v->asList()->value(i)->asString()->value() == #X) X = true; else 
	    int len = v->asList()->size();
	    for (int i = 0;i<len;i++)
		{
		    COMPARE_OPTION (ignore_case_regexps)
		    COMPARE_OPTION (ignore_case)
		    COMPARE_OPTION (prefer_uppercase)
		    COMPARE_OPTION (first_upper)
		    COMPARE_OPTION (line_can_continue)
		    COMPARE_OPTION (no_nested_sections)
		    COMPARE_OPTION (global_values)
		    COMPARE_OPTION (repeat_names)
		    COMPARE_OPTION (comments_last)
		    COMPARE_OPTION (join_multiline)
		    COMPARE_OPTION (no_finalcomment_kill)
		    COMPARE_OPTION (read_only)
		    COMPARE_OPTION (flat)
			y2error ("Option not implemented yet: %s", v->asList()->value(i)->toString().c_str());
		}
#undef  COMPARE_OPTION
	}

    v = scr->value(YCPString("rewrite"));
    if (!v.isNull() && v->isList())
    {
	int len = v->asList()->size();
	rewrites.clear ();
	rewrites.reserve (len);
	for (int i = 0; i<len;i++)
	{
	    YCPValue ival = v->asList()->value(i);
	    if (ival->isList() && 
		2 == ival->asList()->size() && 
		ival->asList()->value(0)->isString() &&
		ival->asList()->value(1)->isString())
	    {
		IoPattern p;
		if (!p.rx.compile (ival->asList()->value(0)->asString()->value(), ignore_case_regexps))
		{
		    p.out = ival->asList()->value(1)->asString()->value();
		    rewrites.push_back (p);
		}
	    }
	}
    }

    v = scr->value(YCPString("subindent"));
    if (!v.isNull() && v->isString())
	subindent = v->asString()->value();

    // read comments
    v = scr->value(YCPString("comments"));
    if (!v.isNull() && v->isList ())
	{
	    int len = v->asList()->size();
	    linecomments.clear ();
	    comments.clear ();
	    linecomments.reserve (len);
	    comments.reserve (len);
	    for (int i = 0;  i < len; i++)
	    {
		YCPString s = v->asList()->value(i)->asString();
		vector <Regex> & regexes = ('^' == s->value_cstr()[0]) ?
		    linecomments : comments;
		Regex r;
		if (!r.compile (s->value (), ignore_case_regexps))
		{
		    regexes.push_back (r);
		}
	    }
	}

    // read sections
    v = scr->value(YCPString("sections"));
    if (!v.isNull() && v->isList ())
	{
	    int len = v->asList()->size();
	    // compile them to regex_t
	    sections.clear ();
	    sections.reserve (len);
	    for (int i = 0;  i < len; i++)
		{
		    if (v->asList()->value(i)->isMap())
			{
			    section s;
			    YCPMap m = v->asList()->value(i)->asMap ();
			    s.end_valid = false;
			    YCPList p;
			    switch (getBeginEndType (m))
				{
				case 0:
				    p = m->value(YCPString("end"))->asList();
				    if (s.end.rx.compile (
					    p->value(0)->asString()->value (),
					    ignore_case_regexps))
					break;
				    s.end.out =
					p->value(1)->asString()->value ();
				    s.end_valid = true;
				    // Fall through
				case 1:
				    p = m->value(YCPString("begin"))->asList();
				    if (s.begin.rx.compile (
					    p->value(0)->asString()->value (),
					    ignore_case_regexps))
					{
					    // compile failed
					    break;
					}
				    s.begin.out =
					p->value(1)->asString()->value ();
				    sections.push_back (s);
				    break;
				case -1:
				default:
				    y2error ("Bad format of %dth section map", i);
				}
			}
		}
	}

    // read parameters
    v = scr->value(YCPString("params"));
    if (!v.isNull() && v->isList ())
	{
	    int len = v->asList()->size();
	    // compile them to regex_t
	    params.clear ();
	    params.reserve (len);
	    for (int i = 0; i < len; i++)
		{
		    if (v->asList()->value(i)->isMap())
			{
			    YCPMap m = v->asList()->value(i)->asMap ();
			    param pa;
			    pa.multiline_valid = false;
			    YCPList p;
			    switch (getParamsType (m))
				{
				case 0:
				    p = m->value(YCPString("multiline"))->asList();
				    if (!pa.begin.compile (
					    p->value(0)->asString()->value (),
					    ignore_case_regexps))
					if (!pa.end.compile (
						p->value(1)->asString()->value (),
						ignore_case_regexps))
					    {
						pa.multiline_valid = true;
					    }
					else
					{
					    y2error ("Bad regexp(multiline): %s",
						p->value(1)->asString()->value_cstr());
					}
				    else
					  y2error ("Bad regexp(multiline): %s",
					      p->value(0)->asString()->value_cstr());
				case 1:
				    p = m->value(YCPString("match"))->asList();
				    if (pa.line.rx.compile (
					    p->value(0)->asString()->value (),
					    ignore_case_regexps))
				    {
					if (pa.multiline_valid)
					{
					    y2error ("Bad regexp(match): %s",
						p->value(0)->asString()->value_cstr());
					}
					break;
				    }
				    pa.line.out =
					p->value(1)->asString()->value ();
				    params.push_back (pa);
				    break;
				case -1:
				default:
				    y2error ("Bad format of %dth param map", i);
				}
			}
		}
	}
    return 0;
}

int IniParser::scanner_start(const char*fn)
{
    scanner.open(fn);
    scanner_line = 0;
    if (!scanner.is_open())
        return -1;
    return 0;
}
void IniParser::scanner_stop()
{
    scanner.close();
    scanner.clear();
}
int IniParser::scanner_get(string&s)
{
    if (!scanner)
	return -1;
    getline (scanner, s);
    scanner_line++;
    if (line_can_continue && s.length ())
    {
	string tmp;
	while (s[s.length()-1] == '\\')
	{
	    getline (scanner, tmp);
	    scanner_line++;
	    s = s + "\n" + tmp;
	}
    }
    return 0;
}

void StripLine (string&l, regmatch_t&r)
{
    string out;
    if (r.rm_so>1)
	out = l.substr (0,r.rm_so);
    out = out + l.substr(r.rm_eo);
    l = out;
}

bool FileDescr::changed ()
{
    struct stat st;
    if (stat(fn.c_str(), &st))
    {
	y2error("Unable to stat '%s': %s", fn.c_str(), strerror(errno));
	return false;
    }
    if (timestamp != st.st_mtime)
    {
	timestamp = st.st_mtime;
	return true;
    }
    return false;
}

FileDescr::FileDescr (char*fn_)
{
    fn = fn_;
    sn = fn_;
    struct stat st;
    if (stat(fn_, &st))
    {
	y2error("Unable to stat '%s': %s", fn_, strerror(errno));
	timestamp = 0;
    }
    timestamp = st.st_mtime;
}

int IniParser::parse()
{
    if (multiple_files)
    {
	glob_t do_files;
	int len = files.size ();
	int flags = 0;
	for (int i = 0;i<len;i++)
	{
	    glob (files[i].c_str (),flags, NULL, &do_files);
	    flags = GLOB_APPEND;
	}
	char**f = do_files.gl_pathv;
	for (unsigned int i = 0;i<do_files.gl_pathc;i++, f++)
	{
	    int section_index = -1;
	    string section_name = *f;
	    //FIXME: create function out of it.
	    // do we have name rewrite rules?
	    for (size_t j = 0; j < rewrites.size (); j++)
		{
		    RegexMatch m (rewrites[j].rx, section_name);
		    if (m)
		    {
			section_index = j;
			section_name = m[1];
			y2debug ("Rewriting %s to %s", *f, section_name.c_str());
			break;
		    }
		}

	    // do we know about the file?
	    map<string,FileDescr>::iterator ff = multi_files.find (*f);
	    if (ff == multi_files.end())
	    {
		// new file
		if (scanner_start (*f))
		    y2error ("Cannot open %s.", *f);
		else
		{
		    FileDescr fdsc (*f);
		    multi_files[*f] = fdsc;
		    inifile.initSection (section_name, "", -1, section_index);
		    parse_helper(inifile.getSection(section_name.c_str()));
		    scanner_stop();
		}
	    }
	    else
	    {
		if ((*ff).second.changed ())
		{
		    if (scanner_start (*f))
			y2error ("Cannot open %s.", *f);
		    else
		    {
			y2debug ("File %s changed. Reloading.", *f);
			FileDescr fdsc (*f);
			multi_files [*f] = fdsc;
			inifile.initSection (section_name, "", -1, section_index);
			parse_helper(inifile.getSection(section_name.c_str()));
			scanner_stop();
		    }
		}
	    }
	}
    }
    else
    {
	if (scanner_start (file.c_str()))
	    {
		y2error ("Can not open %s.", file.c_str());
		return -1;
	    }
	parse_helper(inifile);
	scanner_stop();
	timestamp = getTimeStamp ();
    }
    return 0;
}

int IniParser::parse_helper(IniSection&ini)
{
    string comment = "";
    string key = "";
    string val = "";
    int state = 0;		// 1: precessing a multiline value
    int matched_by = -1;

    string line;
    size_t i;

    // stack of section names
    vector<string>path;

    //
    // read line
    //
    while (!scanner_get (line))
	{
	    //
	    // check for whole-line comment (always as the first stage)
	    //
	    for (i = 0;i<linecomments.size (); i++)
		{
		    if (RegexMatch (linecomments[i], line, 0))
			{
			    // we have it !!!
			    comment = comment + line + "\n";
			    break;
			}
		}
	    if (i<linecomments.size ()) // found? -> next line
		continue;

	    //
	    // check for comments on line
	    //
	    if (!comments_last)
		{
		    for (i = 0;i<comments.size (); i++)
			{
			    RegexMatch m (comments[i], line);
			    if (m)
			    {
				// we have it !!!
				comment = comment + m[0] + "\n";
				line = m.rest;
				break;
			    }
			}
		}

	    //
	    // are we in broken line?
	    //
	    if (state)
		{
		    RegexMatch m (params[matched_by].end, line);
		    if (m)
		    {    
			// it is the end of broken line
			state = 0;
			val = val + (join_multiline ? "" : "\n") + m[1];
			line = m.rest;
			if (!path.size())
			    {   // we are in toplevel section, going deeper
				// check for toplevel values allowance
				if (!global_values)
				    y2error ("%d: %s: values at the top level not allowed.", scanner_line, key.c_str ());
				else
				    ini.initValue (key, val, comment, matched_by);
			    }
			else
			    {
				ini.findSection(path).initValue(key, val, comment, matched_by);
			    }
			comment = "";
		    }
		    else
			val = val + (join_multiline ? "" : "\n") + line;
		}
	    if (!state)
		{
		    //
		    // check for section begin
		    //
		    {
			string found;

			for (i = 0; i < sections.size (); i++)
			    {
				RegexMatch m (sections[i].begin.rx, line);
				if (m)
				{
				    found = m[1];
				    line = m.rest;
				    break;
				}
			    }
			if (i < sections.size ())
			    {
				// section begin found !!! check conditions
				if (path.size())
				    {   // there were some sections
					// is there need to close previous section?
					if (sectionNeedsEnd(ini.findSection(path).getReadBy()))
					    {
						if(no_nested_sections)
						    {
							y2error ("%d: Section %s started but section %s is not finished",
								 scanner_line, 
								 found.c_str(),
								 path[path.size()-1].c_str());
							path.pop_back();
						    }
					    }
					else
					    path.pop_back();
				    }
				if (!path.size())
				    {   // we are in toplevel section, going deeper
					ini.initSection (found, comment, i);
				    }
				else
				    {
					if (no_nested_sections)
					    y2error ("%d: Attempt to create nested section %s.", scanner_line, found.c_str ());
					else
					{
					    ini.findSection(path).initSection(found, comment, i);
					}
				    }
				comment = "";
				path.push_back(found);
			    }
		    } // check for section begin

		    //
		    // check for section end
		    //
		    {
			string found;

			for (i = 0; i < sections.size (); i++)
			    {
				if (!sections[i].end_valid)
				    continue;
				RegexMatch m (sections[i].end.rx, line);
				if (m)
				{
				    found = 1 < m.matches.size () ? m[1]: "";
				    line = m.rest;
				    break;
				}
			    }
			if (i < sections.size ())
			    {
				// we found new section enclosing which
				// means that we can save possible trailing
				// comment
				if (!comment.empty ())
				    {
					if (!path.size())
					    ini.setEndComment (comment.c_str ());
					else
					    {
						ini.findSection(path).setEndComment (comment.c_str ());
					    }
					comment = "";
				    }
				if (!path.size ())
				    y2error ("%d: Nothing to close.", scanner_line);
				else if (!found.empty ())
				    {   // there is a subexpression
					int len = path.size ();
					int j;

					for (j = len - 1; j >= 0;j--)
					    if (path[j] == found)
					    {
						path.resize (j);
						break;
					    }
				    	if (j == -1)
					{   // we did not find, so close the first that needs it
					    int m = ini.findEndFromUp (path, i);
					    int toclose = len - 1;
					    if (-1 != m)
					    {
						toclose = m - 1;
					    }
					    y2error ("%d: Unexpected closing %s. Closing section %s.", scanner_line, found.c_str(), path[toclose].c_str());
					    path.resize (toclose);
					}
				    }
				else
				    {   // there is no subexpression
					int m = ini.findEndFromUp (path, i);
					if (-1 != m) // we have perfect match
					{
					    path.resize(m - 1);
					}
					else if ((i = path.size ()) > 0)
					{
					    path.resize (i - 1);
					}
				    }
			    }
		    }

		    //
		    // check for line
		    //
		    {
			string key,val;
			for (i = 0; i < params.size (); i++)
			{
			    RegexMatch m (params[i].line.rx, line);
			    if (m)
			    {
				key = m[1];
				val = m[2];
				line = m.rest;
				break;
			    }				
			}
			if (i != params.size ())
			    {
				if (!path.size())
				    {   // we are in toplevel section, going deeper
					// check for toplevel values allowance
					if (!global_values)
					    y2error ("%d: %s: values at the top level not allowed.", scanner_line, key.c_str ());
					else
					    ini.initValue (key, val, comment, i);
				    }
				else
				    {
					ini.findSection(path).initValue(key, val, comment, i);
				    }
				comment = "";
			    }
		    }

		    //
		    // check for broken line
		    //
		    {
			for (i = 0; i < params.size (); i++)
			{
			    if (!params[i].multiline_valid)
				continue;
			    RegexMatch m (params[i].begin, line);
			    if (m)
			    {
				// broken line
				key = m[1];
				val = m[2];
				line = m.rest;
				matched_by = i;
				state = 1;
				break;
			    }
			}
		    }

		    //
		    // check for comments on line
		    //
		    if (comments_last && !comments.empty ())
		    {
			for (i = 0; i < comments.size (); i++)
			{
			    RegexMatch m (comments[i], line);
			    if (m)
			    {
				// we have it !!!
				comment = comment + m[0] + "\n";
				line = m.rest;
				break;
			    }
			}
		    }
		    //
		    // if line is not empty, report it
		    //
		    {
			if (!onlySpaces (line.c_str()))
			    y2error ("%d: Extra characters: %s", scanner_line, line.c_str ());
		    }
		}
	}
    if (!comment.empty ())
    {
	if (!no_finalcomment_kill)
	{
	    // kill empty lines at the end of comment
	    int i = comment.length ();
	    const char*p = comment.c_str () + i - 1;
	    while (i)
	    {
		if ('\n' != *p)
		    break;
		i --;
		p --;
	    }
	    if (i > 0)
		i++;
	    comment.resize (i);
	}
	ini.setEndComment (comment.c_str ());
    }

    return 0;
}

void IniParser::UpdateIfModif ()
{
    if (read_only)
        return;
    if (multiple_files)
	parse ();
    else
    {
	if (timestamp != getTimeStamp())
	{
	    y2warning("Data file '%s' was changed externaly!", file.c_str());
	    parse ();
	}
    }
    return ;
}

time_t IniParser::getTimeStamp()
{
    struct stat st;
    if (multiple_files)
    {
	printf ("bad call of getTimeStamp aborting. FIXME\n");//FIXME
	abort ();
    }
    if (stat(file.c_str(), &st))
    {
	y2error("Unable to stat '%s': %s", file.c_str(), strerror(errno));
	return 0;
    }
    return st.st_mtime;
}
int IniParser::write()
{
    int bugs = 0;
    if (!inifile.isDirty())
    {
        y2debug ("File %s did not change. Not saving.", multiple_files ? files[0].c_str () : file.c_str ());
	return 0;
    }
    if (read_only)
    {
        y2debug ("Attempt to write file %s that was mounted read-only. Not saving.", multiple_files ? files[0].c_str () : file.c_str ());
	return 0;
    }
    UpdateIfModif ();

    if (multiple_files)
    {
	IniIterator
	    ci = inifile.getContainerBegin (),
	    ce = inifile.getContainerEnd ();

	for (;ci != ce; ++ci)
	    {
		if (ci->t () == SECTION)
		    {
			IniSection&s = ci->s ();
			int wb = s.getRewriteBy (); // bug #19066 
			string filename = getFileName (s.getName (), wb);

			deleted_sections.erase (filename);
			if (!s.isDirty ()) {
			    y2debug ("Skipping file %s that was not changed.", filename.c_str());
			    continue;
			}
			s.initReadBy ();
			// ensure that the directories exist
			Pathname pn (filename);
			PathInfo::assert_dir (pn.dirname ());
			ofstream of(filename.c_str());
			if (!of.good())
			{
			    bugs++;
			    y2error ("Can not open file %s for write", filename.c_str());
			    continue;
			}
			write_helper (s, of, 0);
			s.clean();
			of.close ();
		    }
		else
		    {
			y2error ("Value %s encountered at multifile top level",
				 ci->e ().getName ());
		    }
	    }

	// FIXME: update time stamps of files...

	// erase removed files...
	for (set<string>::iterator i = deleted_sections.begin (); i!=deleted_sections.end();i++)
	    if (multi_files.find (*i) != multi_files.end ()) {
		y2debug ("Removing file %s\n", (*i).c_str());
		unlink ((*i).c_str());
	    }
    }
    else
    {
	// ensure that the directories exist
	Pathname pn (file);
	PathInfo::assert_dir (pn.dirname ());
	ofstream of(file.c_str());
	if (!of.good())
	{
	    y2error ("Can not open file %s for write", file.c_str());
	    return -1;
	}

	write_helper (inifile, of, 0);

	of.close();
	timestamp = getTimeStamp ();
    }
    inifile.clean ();
    return bugs ? -1 : 0;
}
int IniParser::write_helper(IniSection&ini, ofstream&of, int depth)
{
    char out_buffer[2048];
    string indent;
    string indent2;
    int readby = ini.getReadBy ();
    if (!subindent.empty ())
    {
	for (int ii = 0; ii<depth - 1;ii++)
	    indent = indent + subindent;
	if (depth)
	    indent2 = indent + subindent;
    }

    if (ini.getComment ()[0])
        of << indent << ini.getComment();
    if (readby>=0 && readby < (int)sections.size ())
	{
	    snprintf (out_buffer, 2048, sections[readby].begin.out.c_str (), ini.getName());
	    of << indent << out_buffer << "\n";
	}
    
    IniIterator
	ci = ini.getContainerBegin (),
	ce = ini.getContainerEnd ();

    for (;ci != ce; ++ci)
	{
	    if (ci->t () == SECTION)
		{
		    write_helper (ci->s (), of, depth + 1);
		    ci->s ().clean();
		}
	    else
		{
		    IniEntry&e = ci->e ();
		    if (e.getComment ()[0])
			of << indent2 << e.getComment();
		    if (e.getReadBy()>=0 && e.getReadBy() < (int)params.size ())
			snprintf (out_buffer, 2048, params[e.getReadBy ()].line.out.c_str (), e.getName(), e.getValue());
		    of << indent2 << out_buffer << "\n";
		    e.clean();
		}
	}

    if (ini.getEndComment ()[0])
        of << indent << ini.getEndComment();
    if (readby>=0 && readby < (int) sections.size () && sections[readby].end_valid)
	{
	    snprintf (out_buffer, 2048, sections[readby].end.out.c_str (), ini.getName());
	    of << indent << out_buffer << "\n";
	}
    ini.clean();
    return 0;
}
string IniParser::getFileName (const string&sec, int rb)
{
    string file = sec;
    if (-1 != rb && (int) rewrites.size () > rb)
    {
	int max = rewrites[rb].out.length () + sec.length () + 1;
	char*buf = new char[max + 1];
	snprintf (buf, max, rewrites[rb].out.c_str (), sec.c_str());
	y2debug ("Rewriting %s to %s", sec.c_str(), buf);
	file = buf;
	delete [] buf;
    }
    return file;
}

/**
 * change case of string
 * @param str string to change
 * @return changed string
 */
string IniParser::changeCase (const string&str) const
{
    string tmp = str;
    if (!ignore_case)
      return tmp;
    if (prefer_uppercase)
    {
	for (string::iterator it = tmp.begin(); it != tmp.end(); ++it)
	    *it = toupper (*it);
    }
    else
    {
	for (string::iterator it = tmp.begin(); it != tmp.end(); ++it)
	    *it = tolower (*it);
	if (first_upper)
	{
	string::iterator it = tmp.begin ();
	if (it != tmp.end ())
	    *it = toupper (*it);
	}	    
    }
    return tmp;
}

