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

#include <ycp/y2log.h>
#include <vector>
#include <set>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <glob.h>

#include "IniParser.h"
#include "IniFile.h"

IniParser::~IniParser ()
{
    int i;
    if (linecomments)
    {
	for (i = 0;i<linecomment_len;i++)
	{
	    regfree (linecomments[i]);
	    delete linecomments[i];
	}
        delete [] linecomments;
    }

    if (comments)
    {
	for (i = 0;i<comment_len;i++)
	{
	    regfree (comments[i]);
	    delete comments[i];
	}
	delete [] comments;
    }
    if (sections)
       delete [] sections;
    if (params)
	delete [] params;
    if (rewrites)
	delete [] rewrites;
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
    if (!m->value(YCPString("begin"))->isList())
	// must have begin
	return -1;
    // we know that begin list is present
    if (2!=m->value(YCPString("begin"))->asList()->size())
	// there must be 2 string arguments in begin list
	return -1;
    if (!m->value(YCPString("begin"))->asList()->value(0)->isString() ||
	!m->value(YCPString("begin"))->asList()->value(1)->isString())
	// there must be 2 strings
	return -1;
    // Begin is OK. Now end.
    
    if (m->value(YCPString("end")).isNull())
	return 1;
    if (!m->value(YCPString("end"))->isList())
	return 1;
    // we know that end list is present
    if (2!=m->value(YCPString("end"))->asList()->size())
	// there must be 2 string arguments in end list
	return 1;
    if (!m->value(YCPString("end"))->asList()->value(0)->isString() ||
	!m->value(YCPString("end"))->asList()->value(1)->isString())
	// there must be 2 strings
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
    if (!m->value(YCPString("match"))->isList())
	return -1;
    if (2!=m->value(YCPString("match"))->asList()->size())
	return -1;
    if (!m->value(YCPString("match"))->asList()->value(0)->isString() ||
	!m->value(YCPString("match"))->asList()->value(1)->isString())
	return -1;

    if (m->value(YCPString("multiline")).isNull())
	return 1;
    if (!m->value(YCPString("multiline"))->isList())
	return 1;
    if (2!=m->value(YCPString("multiline"))->asList()->size())
	return 1;
    if (!m->value(YCPString("multiline"))->asList()->value(0)->isString() ||
	!m->value(YCPString("multiline"))->asList()->value(1)->isString())
	return 1;
    return 0;
}

/**
 * just calls regcomp but if it returns error, it logs the
 * error
 */
int IniParser::CompileRegex (regex_t**comp,const char*pattern)
{
    *comp = new regex_t;
    int ret = regcomp (*comp, pattern, REG_EXTENDED | (ignore_case_regexps ? REG_ICASE : 0));
    if (ret)
	{
	    char error[256];
	    regerror (ret, *comp, error, 256);
	    y2error ("Regex %s error: %s", pattern, error);
	    regfree (*comp);
	    delete *comp;
	    *comp = 0;
	}
    return ret;
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
	global_values = comments_last = join_multiline = no_finalcomment_kill = read_only = flat = false;
    linecomment_len = comment_len = section_len = param_len = 0;

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
		    COMPARE_OPTION (comments_last)
		    COMPARE_OPTION (join_multiline)
		    COMPARE_OPTION (no_finalcomment_kill)
		    COMPARE_OPTION (read_only)
		    COMPARE_OPTION (flat)
			y2error ("Option not implemented yet: %s", v->asList()->value(i)->toString().c_str());
		}
#undef  COMPARE_OPTION
	    inifile.setIgnoreCase (ignore_case, prefer_uppercase ? 1 : first_upper ? 2 : 0);
	    inifile.setNesting (no_nested_sections, global_values);
	    if (flat)	inifile.setFlat ();
	}
    v = scr->value(YCPString("rewrite"));
    if (!v.isNull() && v->isList())
    {
	int len = v->asList()->size();
	rewrite_len = 0;
	rewrites = new rewrite[len];
	for (int i = 0;i<len;i++)
	    if (v->asList()->value(i)->isList() && 
		2 == v->asList()->value(i)->asList()->size() && 
		v->asList()->value(i)->asList()->value(0)->isString() &&
		v->asList()->value(i)->asList()->value(1)->isString())
	    {
		if (!CompileRegex (&rewrites[rewrite_len].in,v->asList()->value(i)->asList()->value(0)->asString()->value_cstr()))
		{
		    const char*out = v->asList()->value(i)->asList()->value(1)->asString()->value_cstr();
		    rewrites[rewrite_len].out = new char[strlen(out)+1];
		    strcpy(rewrites[rewrite_len].out, out);
		    rewrite_len++;
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
	    comment_len = 0;
	    comments = new regex_t*[len];
	    linecomment_len = 0;
	    linecomments = new regex_t*[len];
	    for (int i = 0;i<len;i++)
		{
		    if ('^' == v->asList()->value(i)->asString()->value_cstr()[0])
			{
			    if (!CompileRegex (&linecomments[linecomment_len], v->asList()->value(i)->asString()->value_cstr()))
				linecomment_len++;
			}
		    else
			{
			    if (!CompileRegex (&comments[comment_len], v->asList()->value(i)->asString()->value_cstr()))
				comment_len++;
			}
		}
	}
    // read sections
    v = scr->value(YCPString("sections"));
    if (!v.isNull() && v->isList ())
	{
	    int len = v->asList()->size();
	    // compile them to regex_t
	    sections = new section[len];
	    section_len = 0;
	    for (int i = 0;i<len;i++)
		{
		    if (v->asList()->value(i)->isMap())
			{
			    YCPMap m = v->asList()->value(i)->asMap ();
			    sections[section_len].end_valid = false;
			    switch (getBeginEndType (m))
				{
				case 0:
				    if (CompileRegex (&sections[section_len].end,
						      m->value(YCPString("end"))->asList()->value(0)->asString()->value_cstr()))
					break;
				    sections[section_len].end_out = new char [
					m->value(YCPString("end"))->asList()->value(1)->asString()->value().length()+1];
				    strcpy (sections[section_len].end_out,
					    m->value(YCPString("end"))->asList()->value(1)->asString()->value_cstr());
				    sections[section_len].end_valid = true;
				case 1:
				    if (CompileRegex (&sections[section_len].begin,
						      m->value(YCPString("begin"))->asList()->value(0)->asString()->value_cstr()))
					{
					    if (sections[section_len].end_out)
						{
						    regfree (sections[section_len].end);
						    delete sections[section_len].end;
						    delete sections[section_len].end_out;
						}
					    break;
					}
				    sections[section_len].begin_out = new char [
					m->value(YCPString("begin"))->asList()->value(1)->asString()->value().length()+1];
				    strcpy (sections[section_len].begin_out,
					    m->value(YCPString("begin"))->asList()->value(1)->asString()->value_cstr());
				    section_len++;
				    break;
				case -1:
				default:
				    y2error ("Bad format of %dth section map", i);
				}
			}
		}
	}
    // read sections
    v = scr->value(YCPString("params"));
    if (!v.isNull() && v->isList ())
	{
	    int len = v->asList()->size();
	    // compile them to regex_t
	    params = new param[len];
	    param_len = 0;
	    for (int i = 0;i<len;i++)
		{
		    if (v->asList()->value(i)->isMap())
			{
			    YCPMap m = v->asList()->value(i)->asMap ();
			    params[param_len].multiline_valid = false;
			    switch (getParamsType (m))
				{
				case 0:
				    if (!CompileRegex (&params[param_len].begin,
					  m->value(YCPString("multiline"))->asList()->value(0)->asString()->value_cstr()))
					if (!CompileRegex (&params[param_len].end,
					      m->value(YCPString("multiline"))->asList()->value(1)->asString()->value_cstr()))
					{
					    params[param_len].multiline_valid = true;
					}
					else
					{
					    y2error ("Bad regexp(multiline): %s",
						m->value(YCPString("multiline"))->asList()->value(1)->asString()->value_cstr());
					    regfree (params[param_len].begin);
					}
				    else
					  y2error ("Bad regexp(multiline): %s",
					      m->value(YCPString("multiline"))->asList()->value(0)->asString()->value_cstr());
				case 1:
				    if (CompileRegex (&params[param_len].line,
					  m->value(YCPString("match"))->asList()->value(0)->asString()->value_cstr()))
				    {
					if (params[param_len].multiline_valid)
					{
					    y2error ("Bad regexp(match): %s",
						m->value(YCPString("match"))->asList()->value(0)->asString()->value_cstr());
					    regfree (params[param_len].begin);
					    regfree (params[param_len].end);
					}
					break;
				    }
				    params[param_len].out = new char [
					m->value(YCPString("match"))->asList()->value(1)->asString()->value().length()+1];
				    strcpy (params[param_len].out,
					    m->value(YCPString("match"))->asList()->value(1)->asString()->value_cstr());
				    param_len++;
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
	    if (rewrite_len)
	    {
		for (int j = 0;j<rewrite_len;j++)
		{
		    regmatch_t matched[5];
		    if (!regexec (rewrites[j].in, section_name.c_str(), 5, matched, 0))
		    {
			section_index = j;
			section_name = section_name.substr(matched[1].rm_so,matched[1].rm_eo-matched[1].rm_so);
			y2debug ("Rewriting %s to %s", *f, section_name.c_str());
			break;
		    }
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
    int state = 0;
    int matched_by = -1;

    regmatch_t matched[20];
    string line;
    int i;

    // stack of section names
    vector<string>path;
    regex_t**walk;

    //
    // read line
    //
    while (!scanner_get (line))
	{
	    //
	    // check for whole-line comment (always as the first stage)
	    //
	    for (walk = linecomments, i = 0;i<linecomment_len;i++,walk++)
		{
		    if (!regexec (*walk, line.c_str (), 0, NULL, 0))
			{
			    // we have it !!!
			    comment = comment + line + "\n";
			    break;
			}
		}
	    if (i<linecomment_len) // found? -> next line
		continue;

	    //
	    // check for comments on line
	    //
	    if (!comments_last && comments)
		{
		    for (walk = comments, i = 0;i<comment_len;i++,walk++)
			{
			    if (!regexec (*walk, line.c_str (), 20, matched, 0))
				{
				    
				    // we have it !!!
				    comment = comment + line.substr(matched[0].rm_so,matched[0].rm_eo-matched[0].rm_so) + "\n";
				    StripLine(line, matched[0]);
				    break;
				}
			}
		}

	    //
	    // are we in broken line?
	    //
	    if (state)
		{
		    if (!regexec (params[matched_by].end, line.c_str(), 20, matched, 0))
		    {    
			// it is the end of broken line
			state = 0;
			val = val + (join_multiline ? "" : "\n") + line.substr (matched[1].rm_so,matched[1].rm_eo-matched[1].rm_so);
			StripLine (line, matched[0]);
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
				IniSectionMapIterator sec = ini.findSection(path);
				(*sec).second.initValue(key, val, comment, matched_by);
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
			section*walk;
			for (walk = sections, i = 0;i<section_len;i++,walk++)
			    {
				if (!regexec (walk->begin, line.c_str (), 20, matched, 0))
				    {
					found = line.substr(matched[1].rm_so,matched[1].rm_eo-matched[1].rm_so);
					StripLine(line, matched[0]);
					break;
				    }
			    }
			if (i<section_len)
			    {
				// section begin found !!! check conditions
				if (path.size())
				    {   // there were some sections
					// is there need to close previous section?
					IniSectionMapIterator sec = ini.findSection(path);
					if (sectionNeedsEnd((*sec).second.getReadBy()))
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
					    IniSectionMapIterator sec = ini.findSection(path);
					    (*sec).second.initSection(found, comment, i);
					}
				    }
				comment = "";
				path.push_back(found);
			    }
		    }

		    //
		    // check for section end
		    //
		    {
			string found;
			section*walk;
			for (walk = sections, i = 0;i<section_len;i++,walk++)
			    {
				if (!walk->end_valid)
				    continue;
				if (!regexec (walk->end, line.c_str (), 20, matched, 0))
				    {
					found = walk->end->re_nsub ? line.substr(matched[1].rm_so,matched[1].rm_eo-matched[1].rm_so) : "";
					StripLine(line, matched[0]);
					break;
				    }
			    }
			if (i<section_len)
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
						IniSectionMapIterator sec = ini.findSection(path);
						(*sec).second.setEndComment (comment.c_str ());
					    }
					comment = "";
				    }
				if (!path.size ())
				    y2error ("%d: Nothing to close.", scanner_line);
				else if (walk->end->re_nsub && found.length())
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
					    if (-1 != m)
					    {
						y2error ("%d: Unexpected closing %s. Closing section %s.", scanner_line, found.c_str(), path[m-1].c_str());
						path.resize (m - 1);
					    }
					    else
					    {
						y2error ("%d: Unexpected closing %s. Closing section %s.", scanner_line, found.c_str(), path[len - 1].c_str());
						path.resize (len - 1);
					    }
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
			param*walk;
			for (walk = params, i = 0;i<param_len;i++,walk++)
			    {
				if (!regexec (walk->line, line.c_str (), 20, matched, 0))
				    {
					key = line.substr(matched[1].rm_so,matched[1].rm_eo-matched[1].rm_so);
					val = line.substr(matched[2].rm_so,matched[2].rm_eo-matched[2].rm_so);
					StripLine(line, matched[0]);
					break;
				    }				
			    }
			if (i!=param_len)
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
					IniSectionMapIterator sec = ini.findSection(path);
					(*sec).second.initValue(key, val, comment, i);
				    }
				comment = "";
			    }
		    }

		    //
		    // check for broken line
		    //
		    {
			param*walk;
			for (walk = params, i = 0;i<param_len;i++, walk++)
			{
			    if (!walk->multiline_valid)
				continue;
			    if (!regexec (walk->begin, line.c_str (), 20, matched, 0))
			    {
				// broken line
				key = line.substr(matched[1].rm_so,matched[1].rm_eo-matched[1].rm_so);
				val = line.substr(matched[2].rm_so,matched[2].rm_eo-matched[2].rm_so);
				StripLine(line, matched[0]);
				matched_by = i;
				state = 1;
				break;
			    }
			}
		    }

		    //
		    // check for comments on line
		    //
		    if (comments_last && comments)
			{
			    for (walk = comments, i = 0;i<comment_len;i++,walk++)
				{
				    if (!regexec (*walk, line.c_str (), 20, matched, 0))
					{
					    
					    // we have it !!!
					    comment = comment + line.substr(matched[0].rm_so,matched[0].rm_eo-matched[0].rm_so) + "\n";
					    StripLine(line, matched[0]);
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
	IniFileIndex&index = inifile.getIndex();
	IniFileIndex::iterator i = index.begin();
	for (;i!=index.end();i++)
	    {
		const char * iname =  (*i).name.c_str();
		if ((*i).isSection())
		    {
			IniSection&s = inifile.getSection (iname);
			int wb = s.getRewriteBy (); // bug #19066 
			string filename = getFileName (iname, wb);

			deleted_sections.erase (filename);
			if (!s.isDirty ()) {
			    y2debug ("Skipping file %s that was not changed.", filename.c_str());
			    continue;
			}
			s.initReadBy ();
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
    IniFileIndex&index = ini.getIndex();
    IniFileIndex::iterator i = index.begin();
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
    if (readby>=0 && readby<section_len)
	{
	    snprintf (out_buffer, 2048, sections[readby].begin_out, ini.getName());
	    of << indent << out_buffer << "\n";
	}
    
    for (;i!=index.end();i++)
	{
	    if ((*i).isSection())
		{
		    IniSection&s = ini.getSection ((*i).name.c_str());
		    write_helper (s, of, depth + 1);
		    s.clean();
		}
	    else
		{
		    IniEntry&e = ini.getEntry ((*i).name.c_str());
		    if (e.getComment ()[0])
			of << indent2 << e.getComment();
		    if (e.getReadBy()>=0 && e.getReadBy()<param_len)
			snprintf (out_buffer, 2048, params[e.getReadBy ()].out, (*i).name.c_str(), e.getValue());
		    of << indent2 << out_buffer << "\n";
		    e.clean();
		}
	}

    if (ini.getEndComment ()[0])
        of << indent << ini.getEndComment();
    if (readby>=0 && readby<section_len && sections[readby].end_valid)
	{
	    snprintf (out_buffer, 2048, sections[readby].end_out, ini.getName());
	    of << indent << out_buffer << "\n";
	}
    ini.clean();
    return 0;
}
string IniParser::getFileName (const string&sec, int rb)
{
    string file = sec;
    if (-1 != rb && rewrite_len > rb)
    {
	int max = strlen(rewrites[rb].out) + sec.length () + 1;
	char*buf = new char[max + 1];
	snprintf (buf, max, rewrites[rb].out, sec.c_str());
	y2debug ("Rewriting %s to %s", sec.c_str(), buf);
	file = buf;
	delete [] buf;
    }
    return file;
}
