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
#include <stdio.h>
#include <ctype.h>
#include <set>

#include "IniFile.h"

/**
 * Converts ycp value to string. Returns empty string if ycp value isn't
 * YCPString.
 * @param v YCPValue to convert
 * @return converted string
 */
string to_string (const YCPValue&v)
{
    string s;
    if (v->isString ())
	s = v->asString ()->value ();
    else
	s = v->toString ();
    return s;
}

/**
 * change case of string
 * @param str string to change
 * @param do_it 
 * @param style
 * @return changed string
 */
string change_case (const string&str, bool do_it, int style)
{
    string tmp = str;
    if (!do_it)
      return tmp;
    switch (style)
    {
	case 1:
	    for (string::iterator it = tmp.begin(); it != tmp.end(); ++it)
	      *it = toupper (*it);
	    break;
	default:
	    for (string::iterator it = tmp.begin(); it != tmp.end(); ++it)
	      *it = tolower (*it);
	    break;
	 ;
    }
    if (2 == style)
    {
	string::iterator it = tmp.begin ();
	if (it != tmp.end ())
	    *it = toupper (*it);
    }
    return tmp;
}

void IniSection::initValue (const string&key,const string&val,const string&comment,int rb)
{
    string k = change_case (key, ignore_case, ignore_style);
    IniEntryMapIterator v = values.find(k);
    if (v != values.end())
	{
	    // update value
	    v->second.init(comment, val, rb);
	    index.remove(IniName (k, IniName::VALUE));
	}
    else
	{
	    // create new value
	    IniEntry e;
	    e.init (comment, val, rb);
	    values[k] = e;
	}
    index.push_back(IniName (k,IniName::VALUE));
}
void IniSection::initSection (const string&name,const string&comment,int rb, int wb)
{
    string k = change_case (name, ignore_case, ignore_style);
    map<const string,IniSection>::iterator v = sections.find(k);
    if (v != sections.end())
	{
	    if (!v->second.dirty)
		{
		    v->second.comment = comment;
		    v->second.read_by = rb;
		    if (wb != -2) v->second.rewrite_by = wb;
		    v->second.name = k;
		}
	    index.remove(IniName (k,IniName::SECTION));
	}
    else
	{
	    IniSection s;
	    s.dirty = false;
	    s.comment = comment;
	    s.read_by = rb;
	    if (wb != -2) s.rewrite_by = wb;
	    s.name = k;
	    s.setIgnoreCase (ignore_case, ignore_style);
	    s.allow_sections = s.allow_subsub = allow_subsub;
	    sections[k] = s;
	}
    index.push_back(IniName (k,IniName::SECTION));
}
IniSectionMapIterator IniSection::findSection(const vector<string>&path, int from)
{
    string k = change_case (path[from], ignore_case, ignore_style);
    IniSectionMapIterator v = sections.find(k);
    if (v == sections.end ())
	{
	    y2error ("Internal error. Section %s not found. This can't happen.\n", k.c_str());
	    abort();
	}
    return from+1>=(int)path.size() ? v : (*v).second.findSection(path, from+1);
}
int IniSection::findEndFromUp (const vector<string>&path, int wanted, int found, int from)
{
    if (read_by == wanted)
	found = from;
    if (from < (int)path.size())
    {
	string k = change_case (path[from], ignore_case, ignore_style);
	IniSectionMapIterator v = sections.find(k);
	if (sections.end () == v)
	{
	    y2error ("Internal error. Value %s not found. This can't happen.\n", path[from].c_str());
	    abort ();
	}
	found = (*v).second.findEndFromUp (path, wanted, found, from + 1);
    }
    return found;
}
void IniSection::Dump ()
{
    IniFileIndex::iterator i = index.begin();

    printf("%s<%s>\n", comment.c_str(), name.c_str());
    for (;i!=index.end();i++)
	{
	    if (IniName::VALUE==(*i).type)
		{
		    IniEntryMapIterator it = values.find((*i).name);
		    if (it == values.end())
			{
			    printf ("Internal error. Value %s not found. This can't happen.\n", (*i).name.c_str());
			    abort ();
			}
		    printf ("%s%s = %s\n",(*it).second.getComment(), (*i).name.c_str(), (*it).second.getValue());
		}
	    else
		{
		    IniSectionMapIterator it = sections.find((*i).name);
		    if (it == sections.end())
			{
			    printf ("Internal error. Section %s not found. This can't happen.\n", (*i).name.c_str());
			    abort ();
			}
		    (*it).second.Dump();
		}
	}
    printf("</%s>\n", name.c_str());
}

int IniSection::getValue (const YCPPath&p, YCPValue&out,int what, int depth)
{
    string k = change_case (p->component_str (depth), ignore_case, ignore_style);
    if (depth+1>=p->length())
	{   //We are in THE section. Find value here
	    IniEntryMapIterator it = values.find (k);
	    if (it == values.end())
		{
		    y2debug ("Read: Invalid path %s [%d]", p->toString().c_str(), depth);
		    return -1;
		}
	    switch (what)
	    {
	    case 0:	out = YCPString ((*it).second.getValue ());   break;
	    case 1:	out = YCPString ((*it).second.getComment());  break;
	    default:	out = YCPInteger((*it).second.getReadBy ());  break;
	    }
	    return 0;
	}
    // Otherwise it must be section
    IniSectionMapIterator it = sections.find (k);
    if (it == sections.end())
	{
	    y2debug ("Read: Invalid path %s [%d]", p->toString().c_str(), depth);
	    return -1;
	}
    return (*it).second.getValue (p, out, what, depth+1);
}
int IniSection::getSectionProp (const YCPPath&p, YCPValue&out, int what, int depth)
{
    if (depth>=p->length())
	{   //We are in THE section. Find the comment here 
	    if (what == 0)
		out = YCPString (comment);
	    else if (what == 1)
		out = YCPInteger (rewrite_by);
	    else
		out = YCPInteger (read_by);
	    return 0;
	}
    // Otherwise it must be section
    string k = change_case (p->component_str (depth), ignore_case, ignore_style);
    IniSectionMapIterator it = sections.find (k);
    if (it == sections.end())
	{
	    y2debug ("Read: Invalid path %s [%d]", p->toString().c_str(), depth);
	    return -1;
	}
    return (*it).second.getSectionProp (p, out, what, depth+1);
}
int IniSection::Delete (const YCPPath&p)
{
    if (flat)
	return delValueFlat (p);
    if (p->length() < 2)
    {
	y2error ("I do not know what to delete at %s.", p->toString().c_str());
	return -1;
    }
    string s = p->component_str (0);
    if (s == "v" || s == "value")
	return delValue (p, 1);
    if (s == "s" || s == "section")
      return delSection (p, 1);
    return -1;
}
int IniSection::Write (const YCPPath&p, const YCPValue&v, bool rewrite)
{
    if (flat)
	return setValueFlat (p, v);
    if (p->length() < 2)
    {
	y2error ("I do not know what to write to %s.", p->toString().c_str());
	return -1;
    }
    string s = p->component_str (0);
    if (s == "v" || s == "value")
	return setValue (p, v, 0, 1);
    if ((s == "vc" || s == "value_comment" || s == "valuecomment") && v->isString ())
      return setValue (p, v, 1, 1);
    if ((s == "vt" || s == "value_type" || s == "valuetype") && v->isInteger ())
      return setValue (p, v, 2, 1);
    if ((s == "s" || s == "section" || s == "sc" || s == "section_comment" || s == "sectioncomment") && v->isString ())
      return setSectionProp (p, v, 0, 1);
    if ((s == "st" || s == "section_type" || s == "sectiontype") && v->isInteger ())
      return setSectionProp (p, v, rewrite? 1:2, 1);
    return -1;
}
int IniSection::setSectionProp (const YCPPath&p,const YCPValue&in, int what, int depth)
{
    string k = change_case (p->component_str (depth), ignore_case, ignore_style);
    if (depth+1>=p->length())
	{   //We are in THE section. Find value here
	    IniSectionMapIterator it = sections.find (k);
	    if (it == sections.end())
		{
		    y2debug ("Adding section %s", p->toString().c_str());
		    IniSection s;	
		    s.comment = "";
		    s.read_by = 0;
		    s.rewrite_by = 0;
		    if (what == 0)
		    {
			s.comment = in->asString()->value();
		    }
		    else if (what == 1)
		    {
			s.rewrite_by = in->asInteger()->value();
		    }
		    else
		    {
			s.read_by = in->asInteger()->value();
		    }
		    s.name = k;
		    s.setIgnoreCase (ignore_case, ignore_style);
		    s.allow_sections = s.allow_subsub = allow_subsub;
		    s.dirty = true;
		    sections[k] = s;
		    index.push_back (IniName (k, IniName::SECTION));
		    dirty = true;
		}
	    else
		if (what == 0)
		    (*it).second.setComment (in->asString()->value_cstr());
		else if (what == 1)
		    (*it).second.setRewriteBy (in->asInteger()->value());
		else
		    (*it).second.setReadBy (in->asInteger()->value());
	    return 0;
	}
    // Otherwise it must be section
    IniSectionMapIterator it = sections.find (k);
    if (it == sections.end())
	{
	    // section not found, add it. (Recursive section addition)
	    IniSection s (ignore_case, ignore_style, allow_subsub, k);
	    sections[k] = s;
	    index.push_back (IniName (k, IniName::SECTION));
	    y2debug ("Write: adding recursively %s to %s", k.c_str (), p->toString().c_str());
	    it = sections.find (k);
	}
    return (*it).second.setSectionProp (p, in, what, depth+1);
}
int IniSection::delSection(const YCPPath&p, int depth)
{
    string k = change_case (p->component_str (depth), ignore_case, ignore_style);
    if (depth+1>=p->length())
    {   //We are in THE section. Find section to delete here
	if (sections.end () != sections.find (k))
	{
	    sections.erase (k);
	    dirty = true;
	    index.remove (IniName (k,IniName::SECTION));
	}
	else
	    y2debug ("Can not delete %s. Key does not exist.", p->toString().c_str());
	return 0;
    }
    // Otherwise it must be section
    IniSectionMapIterator it = sections.find (k);
    if (it == sections.end())
    {
	y2error ("Delete: Invalid path %s [%d]", p->toString().c_str(), depth);
	return -1;
    }
    return (*it).second.delSection (p, depth+1);
}

int IniSection::setValue (const YCPPath&p,const YCPValue&in,int what, int depth)
{
    string k = change_case (p->component_str (depth), ignore_case, ignore_style);
    if (depth+1>=p->length())
	{   //We are in THE section. Find value here
	    IniEntryMapIterator it = values.find (k);
	    if (it == values.end())
		{
		    y2debug ("Adding value %s = %s", p->toString().c_str(), in->toString().c_str ());
		    if (what)
			{
			    y2error ("You must add value before changing comment/type. %s", p->toString().c_str());
			    return -1;
			}
		    IniEntry e;
		    e.setValue (to_string (in));
		    values[k] = e;
		    index.push_back (IniName (k, IniName::VALUE));
		}
	    else
		{
		    switch (what)
		    {
		    case 0:	it->second.setValue   (to_string (in));		 break;
		    case 1:	it->second.setComment (in->asString()->value()); break;
		    default:	it->second.setReadBy  (in->asInteger()->value());break;
		    }
		}
	    dirty = true;
	    return 0;
	}
    // Otherwise it must be section
    IniSectionMapIterator it = sections.find (k);
    if (it == sections.end())
	{
	    // section not found, add it. (Recursive section addition)
	    IniSection s (ignore_case, ignore_style, allow_subsub, k);
	    sections[k] = s;
	    index.push_back (IniName (k, IniName::SECTION));
	    y2debug ("Write: adding recursively %s to %s", k.c_str (), p->toString().c_str());
	    it = sections.find (k);
	}
    return (*it).second.setValue (p, in, what, depth+1);
}
int IniSection::delValue (const YCPPath&p, int depth)
{
    string k = change_case (p->component_str (depth), ignore_case, ignore_style);
    if (depth+1>=p->length())
	{   //We are in THE section. Find value here
	    if (values.end () != values.find (k))
		{
		    values.erase (k);
		    dirty = true;
		    index.remove (IniName (k,IniName::VALUE));
		}
	    else
		y2debug ("Can not delete %s. Key does not exist.", p->toString().c_str());
	    return 0;
	}
    // Otherwise it must be section
    IniSectionMapIterator it = sections.find (k);
    if (it == sections.end())
	{
	    y2error ("Delete: Invalid path %s [%d]", p->toString().c_str(), depth);
	    return -1;
	}
    return (*it).second.delValue (p, depth+1);
}

int IniSection::dirValueFlat (const YCPPath&p, YCPList&l)
{
    // This function used to discard p and always return Dir (.)
    // #21574
    if (p->length () != 0)
    {
	// Leave l empty.
	// Maybe we should differentiate between Dir (.existing_value)
	// and Dir (.bogus) ?
	return 0;
    }

    IniFileIndex::iterator i = index.begin ();
    for (;i!=index.end();i++)
	if (!(*i).isSection ())
	    l->add(YCPString((*i).name));
    return 0;
}

int IniSection::getValueFlat (const YCPPath&p, YCPValue&out)
{
    if (!p->length ())
	return -1;
    string k = change_case (p->component_str (0), ignore_case, ignore_style);
    IniEntryMapIterator i = values.find(k);
    if (values.end() != i)
    {
	out = (p->length()>1 && p->component_str(1)=="comment")
	    ? YCPString ((*i).second.getComment ()) :
	      YCPString ((*i).second.getValue ());
	return 0;
    }
    return -1;
}

int IniSection::delValueFlat (const YCPPath&p)
{
    if (!p->length ())
	return -1;
    string k = change_case (p->component_str (0), ignore_case, ignore_style);
    IniEntryMapIterator i = values.find(k);
    if (values.end() != i)
    {
	values.erase (k);
	dirty = true;
	index.remove (IniName (k,IniName::VALUE));
    }
    return 0;
}

int IniSection::setValueFlat (const YCPPath&p, const YCPValue&out)
{
    if (!p->length ())
	return -1;
    string k = change_case (p->component_str (0), ignore_case, ignore_style);
    IniEntryMapIterator i = values.find(k);
    int type = 0;
    if (p->length()>1 && p->component_str(1)=="comment")
	type = 1;
    if (values.end() != i)
    {
	if (type)
	   (*i).second.setComment (to_string (out));
	else
	   (*i).second.setValue (to_string (out));
    }
    else
    {
	if (type)
	    return -1;
	IniEntry e;
	e.setValue (to_string (out));
	values[k] = e;
	index.push_back (IniName (k, IniName::VALUE));
    }
    dirty = true;
    return 0;
}

int IniSection::Read (const YCPPath&p, YCPValue&out, bool rewrite)
{
    if (flat)
	return getValueFlat (p, out);
    if (p->length()<2)
	{
	    y2error ("I do not know what to read from %s.", p->toString().c_str());
	    return -1;
	}
    string s = p->component_str(0);
    if (s == "v" || s == "value")
	return getValue (p, out, 0, 1);
    else if (s == "vc" || s == "value_comment" || s == "valuecomment")
	return getValue (p, out, 1, 1);
    else if (s == "vt" || s == "value_type" || s == "valuetype")
	return getValue (p, out, 2, 1);
    else if (s == "sc" || s == "section_comment" || s == "sectioncomment")
	return getSectionProp (p, out, 0, 1);
    else if (s == "st" || s == "section_type" || s == "sectiontype")
	return getSectionProp (p, out, rewrite? 1:2, 1);

    y2error ("I do not know what to read from %s.", p->toString().c_str());
    return -1;
}
int IniSection::Dir (const YCPPath&p, YCPList&l)
{
    if (flat)
	return dirValueFlat (p, l);
    if (p->length()<1)
	{
	    y2error ("I do not know what to dir from %s.", p->toString().c_str());
	    return -1;
	}

    string s = p->component_str(0);
    if (s == "v" || s == "value")
	return dirHelper (p, l, 0, 1);
    else if (s == "s" || s == "section")
	return dirHelper (p, l, 1, 1);

    y2error ("I do not know what to dir from %s.", p->toString().c_str());
    return -1;
}
int IniSection::dirHelper (const YCPPath&p, YCPList&out,int get_sect,int depth)
{
    if (depth>=p->length())
	{   //We are in THE section. Find the comment here
	    IniFileIndex::iterator i = index.begin();
	    for (;i!=index.end();i++)
		if (get_sect && (*i).isSection())
		    out->add(YCPString ((*i).name));
		else if (!get_sect && !(*i).isSection())
		    out->add(YCPString ((*i).name));
	    return 0;
	}
    // Otherwise it must be section
    string k = change_case (p->component_str (depth), ignore_case, ignore_style);
    IniSectionMapIterator it = sections.find (k);
    if (it == sections.end())
	{
	    y2debug ("Dir: Invalid path %s [%d]", p->toString().c_str(), depth);
	    return -1;
	}
    return (*it).second.dirHelper (p, out, get_sect, depth+1);
}
IniEntry&IniSection::getEntry (const char*n)
{
    IniEntryMapIterator i = values.find(n);
    if (i == values.end())
	{
	    y2error ("Internal error. Value %s not found in section %s", n, name.c_str());
	    abort();
	}
    return (*i).second;
}

int IniSection::getSubSectionRewriteBy (const char*name)
{
    IniSectionMapIterator i = sections.find(name);
    if (i == sections.end())
	return -1;
    return (*i).second.getRewriteBy ();
}
IniSection&IniSection::getSection (const char*n)
{
    IniSectionMapIterator i = sections.find(n);
    if (i == sections.end())
	{
	    y2error ("Internal error. Section %s not found in section %s", n, name.c_str());
	    abort();
	}
    return (*i).second;
}
void IniSection::setIgnoreCase (bool ic, int style)
{
    ignore_case = ic;
    ignore_style = style;
}

void IniSection::setNesting (bool no_sub_sec, bool global_val)
{
    allow_subsub = !no_sub_sec;
    allow_sections = true;
    allow_values = global_val;
}
void IniSection::setEndComment (const char*c)   
{ 
    if (comment.empty () && !values.size () && !sections.size ())
	comment = c;
    else
	end_comment = c;
}
bool IniSection::isDirty ()  
{
    if (dirty)
        return true;
    // every write dirtyfies not only value but section too
    // so it is enough for us to find the first dirty section
    IniSectionMapIterator i = sections.begin ();
    for (;i != sections.end ();i++)
	if ((*i).second.isDirty ())
	  return true;
    return false;
}
void IniSection::clean() 
{
    dirty = false; 
    IniSectionMapIterator i = sections.begin ();
    for (;i != sections.end ();i++)
	(*i).second.clean ();
    IniEntryMapIterator e = values.begin ();
    for (;e != values.end ();e++)
	(*e).second.clean ();
}

