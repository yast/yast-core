/**
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

#include <ycp/y2log.h>
#include <stdio.h>
#include <ctype.h>
#include <set>
#include <cassert>

#include "IniFile.h"
#include "IniParser.h"

/**
 * Converts ycp value to string.
 * Returns the string representation if the ycp value isn't a YCPString.
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
 * Return the YCPList or YCPNull if it is not one. Log an error.
 */
static
YCPList as_list (const YCPValue& v, const char * context)
{
    if (v->isList ())
	return v->asList ();
    ycp2error ("Expected a list for %s, got %s %s",
	       context, v->valuetype_str(), v->toString().c_str());
    return YCPNull ();
}

/**
 * Return the YCPString or YCPNull if it is not one. Log an error.
 */
static
YCPString as_string (const YCPValue& v, const char * context)
{
    if (v->isString ())
	return v->asString ();
    ycp2error ("Expected a string for %s, got %s %s",
	       context, v->valuetype_str(), v->toString().c_str());
    return YCPNull ();
}

/**
 * Return the YCPInteger or YCPNull if it is not one. Log an error.
 */
static
YCPInteger as_integer (const YCPValue& v, const char * context)
{
    if (v->isInteger ())
	return v->asInteger ();
    ycp2error ("Expected an integer for %s, got %s %s",
	       context, v->valuetype_str(), v->toString().c_str());
    return YCPNull ();
}

/**
 * Return the YCPBoolean or YCPNull if it is not one. Log an error.
 */
static
YCPBoolean as_boolean (const YCPValue& v, const char * context)
{
    if (v->isBoolean ())
	return v->asBoolean ();
    ycp2error ("Expected a boolean for %s, got %s %s",
	       context, v->valuetype_str(), v->toString().c_str());
    return YCPNull ();
}

void IniSection::initValue (const string&key,const string&val,const string&comment,int rb)
{
    string k = ip->changeCase (key);
    IniEntry e;
    IniEntryIdxIterator exi;
    if (!ip->repeatNames () && (exi = ivalues.find (k)) != ivalues.end ())
	{
	    IniIterator ei = exi->second;
	    // update existing value
	    // copy the old value
	    e = ei->e ();
	    // remove and unindex the old value
	    // This means that container needs to be a list, not vector,
	    // so that iterators kept in ivalues are still valid
	    container.erase (ei);
	    ivalues.erase (exi);
	}
    else
	{
	    // nothing
	}
    // create new value
    e.init (k, comment, rb, val);
    // insert it
    IniContainerElement ce (e);
    container.push_back (ce);
    // index it
    ivalues.insert (IniEntryIndex::value_type (k, --container.end ()));

}
IniSection& IniSection::initSection (const string&name,const string&comment,int rb, int wb)
{
    string k = ip->changeCase (name);
    
    IniSection s (ip);
    IniSectionIdxIterator sxi;
    if (!ip->repeatNames () && (sxi = isections.find (k)) != isections.end ())
	{
	    IniIterator si = sxi->second;
	    s = si->s ();
	    if (!s.dirty)
		{
		    s.comment = comment;
		    s.read_by = rb;
		    if (wb != -2) s.rewrite_by = wb;
		    s.name = k;
		}

	    // remove and unindex the old section
	    container.erase (si);
	    isections.erase (sxi);
	}
    else
	{			// new section
	    s.dirty = false;
	    s.comment = comment;
	    s.read_by = rb;
	    if (wb != -2) s.rewrite_by = wb;
	    s.name = k;
	    s.ip = ip;
	}
    // insert it
    IniContainerElement ce (s);
    container.push_back (ce);
    // index it
    isections.insert (IniSectionIndex::value_type (k, --container.end ()));
    // return reference to the new copy(!)
    return container.back().s();
}

IniSection& IniSection::findSection(const vector<string>&path, int from)
{
    string k = ip->changeCase (path[from]);
    IniSectionIdxIterator v = isections.find(k);
    if (v == isections.end ())
	{
	    y2error ("Internal error. Section %s not found. This can't happen.\n", k.c_str());
	    abort();
	}
    IniSection &s = v->second->s ();
    return from+1 >= (int)path.size() ? s : s.findSection (path, from+1);
}

int IniSection::findEndFromUp (const vector<string>&path, int wanted, int found, int from)
{
/*
    y2internal ("read_by (mine: %d, wanted %d) found: %d from: %d "
		"path[from]: %s iam: %s",
		read_by, wanted, found, from,
		(from < (int)path.size()) ? path[from].c_str() : "*NONE*", name.c_str());
*/

    if (read_by == wanted)
    {
	found = from;
//	y2internal ("%s", " match");
    }

    if (from < (int)path.size())
    {
	string k = ip->changeCase (path[from]);
	// find the _last_ section with key k
	pair <IniSectionIdxIterator, IniSectionIdxIterator> r =
	    isections.equal_range (k);
	if (r.first == r.second) // empty range = not found
	{
	    y2error ("Internal error. Value %s not found. This can't happen.\n", path[from].c_str());
	    abort ();
	}
	IniSection&  s = (--r.second)->second->s ();
	found = s.findEndFromUp (path, wanted, found, from + 1);
    }
//    y2internal ("return %d", found);
    return found;
}

void IniSection::Dump ()
{
    printf("%s<%s>\n", comment.c_str(), name.c_str());

    printf ("{Natural order}\n");
    IniIterator
	ci = getContainerBegin (),
	ce = getContainerEnd ();

    for (;ci != ce; ++ci)
    {
	printf ("{@%p}\n", &*ci);
	IniType t = ci->t ();
	if (t == VALUE)
	{
	    IniEntry &v = ci->e ();
	    printf ("%s%s = %s\n", v.getComment (), v.getName (), v.getValue ());
	}
	else if (t == SECTION)
	{
	    ci->s ().Dump ();
	}
	else
	{
	    printf ("{Unknown type %u}\n", t);
	}
    }

    printf ("{Sections}\n");
    IniSectionIdxIterator
	sxi = isections.begin (),
	sxe = isections.end ();

    for (; sxi != sxe; ++sxi)
    {
	printf ("{%s @%p}\n", sxi->first.c_str (), &*sxi->second);	
    }

    printf ("{Values}\n");
    IniEntryIdxIterator
	exi = ivalues.begin (),
	exe = ivalues.end ();

    for (; exi != exe; ++exi)
    {
	printf ("{%s @%p}\n", exi->first.c_str (), &*exi->second);	
    }

    printf("</%s>\n", name.c_str());
}

void IniSection::reindex ()
{
    IniIterator
	ci = getContainerBegin (),
	ce = getContainerEnd ();

    ivalues.clear ();
    isections.clear ();

    for (;ci != ce; ++ci)
    {

	if (ci->t () == VALUE)
	{
	    string k = ip->changeCase (ci->e ().getName ());
	    ivalues.insert (IniEntryIndex::value_type (k, ci));
	}
	else
	{
	    string k = ip->changeCase (ci->s ().getName ());
	    isections.insert (IniSectionIndex::value_type (k, ci));
	}
    }    
}

int IniSection::getMyValue (const YCPPath &p, YCPValue &out, int what, int depth)
{
    string k = ip->changeCase (p->component_str (depth));
    // Find all values and return them according to repeat_names
    YCPList results;
    bool found = false;
    pair <IniEntryIdxIterator, IniEntryIdxIterator> r =
	ivalues.equal_range (k);
    IniEntryIdxIterator xi = r.first, xe = r.second;
    for (; xi != xe; ++xi)
    {
	found = true;
	IniEntry& e = xi->second-> e ();
	switch (what)
	{
	    case 0:  out = YCPString (e.getValue ());  break;
	    case 1:  out = YCPString (e.getComment()); break;
	    default: out = YCPInteger(e.getReadBy ()); break;
	}
	results->add (out);
    }

    if (ip->repeatNames ())
    {
	out = results;
	return 0;
    }
    else if (found)
    {
	// nonempty range, the cycle ran once, out has the right value
	return 0;
    }
    // empty range, no such key

    y2debug ("Read: Invalid path %s [%d]", p->toString ().c_str(), depth);
    return -1;
}

int IniSection::getValue (const YCPPath&p, YCPValue&out,int what, int depth)
{
    string k = ip->changeCase (p->component_str (depth));
    if (depth + 1 < p->length())
    {
	// it must be a section
	// Get any section of that name
	IniSectionIdxIterator sxi = isections.find (k);
	if (sxi != isections.end())
	{
	    return sxi->second->s ().getValue (p, out, what, depth+1);
	}
	// else error
    }
    else
	{   //We are in THE section. Find value here
	    return getMyValue (p, out, what, depth);
	}

    y2debug ("Read: Invalid path %s [%d]", p->toString().c_str(), depth);
    return -1;
}

// Read calls us with the path length >= 2
int IniSection::getSectionProp (const YCPPath&p, YCPValue&out, int what, int depth)
{
    string k = ip->changeCase (p->component_str (depth));
    // Find the matching sections.
    // If we need to recurse, choose one
    // Otherwise gather properties of all of the leaf sections

    pair <IniSectionIdxIterator, IniSectionIdxIterator> r =
	isections.equal_range (k);
    IniSectionIdxIterator xi = r.first, xe = r.second;

    if (depth + 1 < p->length())
    {
	// recurse
	if (xi != xe)
	{
	    // there's something
	    IniSection& s = (--xe)->second->s ();
	    return s.getSectionProp (p, out, what, depth+1);
	}
	//error
    }
    else
    {
	// bottom level, take all
	YCPList results;
	bool found = false;
	for (; xi != xe; ++xi)
	{
	    found = true;
	    IniSection& s = xi->second->s ();
	    if (what == 0)
		out = YCPString (s.comment);
	    else if (what == 1)
		out = YCPInteger (s.rewrite_by);
	    else
		out = YCPInteger (s.read_by);
	    results->add (out);
	}

	if (ip->repeatNames ())
	{
	    out = results;
	    return 0;
	}
	else if (found)
	{
	    // nonempty range, the cycle ran once, out has the right value
	    return 0;
	}
	// empty range, no such key
    }

    y2debug ("Read: Invalid path %s [%d]", p->toString().c_str(), depth);
    return -1;
}

int IniSection::getAll (const YCPPath&p, YCPValue&out, int depth)
{
    if (depth < p->length ())
    {
	// recurse to find the starting section
	// Get any section of that name
	string k = ip->changeCase (p->component_str (depth));
	IniSectionIdxIterator sxi = isections.find (k);
	if (sxi != isections.end())
	{
	    return sxi->second->s ().getAll (p, out, depth+1);
	}
	// else error
    }
    else
    {
	out = getAllDoIt ();
	return 0;
    }

    y2error ("Read: Invalid path %s [%d]", p->toString().c_str(), depth);
    return -1;
}

YCPMap IniSection::getAllDoIt ()
{
    YCPMap m = IniBase::getAllDoIt ();

    m->add (YCPString ("kind"), YCPString ("section"));
    m->add (YCPString ("file"), YCPInteger (rewrite_by));

    YCPList v;
    IniIterator
	ci = getContainerBegin (),
	ce = getContainerEnd ();

    for (;ci != ce; ++ci)
    {
	// the method is virtual,
	// but the container does not exploit the polymorphism
	YCPMap vm;
	IniType t = ci->t ();
	if (t == VALUE)
	{
	    vm = ci->e ().getAllDoIt ();
	}
	else //if (t == SECTION)
	{
	    vm = ci->s ().getAllDoIt ();
	}
	v->add (vm);
    }

    m->add (YCPString ("value"), v);
    return m;
}

int IniSection::Delete (const YCPPath&p)
{
    if (ip->isFlat ())
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
    if (ip->isFlat ())
	return setValueFlat (p, v);

    if (p->length() >= 1 && p->component_str (0) == "all")
    {
	return setAll (p, v, 1);
    }

    if (p->length() < 2)
    {
	y2error ("I do not know what to write to %s.", p->toString().c_str());
	return -1;
    }
    string s = p->component_str (0);
    if (s == "v" || s == "value")
	return setValue (p, v, 0, 1);
    if (s == "vc" || s == "value_comment" || s == "valuecomment")
      return setValue (p, v, 1, 1);
    if (s == "vt" || s == "value_type" || s == "valuetype")
      return setValue (p, v, 2, 1);
    if (s == "s" || s == "section" || s == "sc" || s == "section_comment" || s == "sectioncomment")
      return setSectionProp (p, v, 0, 1);
    if (s == "st" || s == "section_type" || s == "sectiontype")
      return setSectionProp (p, v, rewrite? 1:2, 1);
    if (s == "section_private")
      return setSectionProp (p, v, 3, 1);

    return -1;
}

int IniSection::setSectionProp (const YCPPath&p,const YCPValue&in, int what, int depth)
{
    string k = ip->changeCase (p->component_str (depth));
    // Find the matching sections.
    // If we need to recurse, choose one, creating if necessary
    // Otherwise set properties of all of the leaf sections,
    //  creating and deleting if the number of them does not match

    pair <IniSectionIdxIterator, IniSectionIdxIterator> r =
	isections.equal_range (k);
    IniSectionIdxIterator xi = r.first, xe = r.second;

    if (depth + 1 < p->length())
    {
	// recurse
	IniIterator si;
	if (xi == xe)
	{
	    // not found, need to add it;
	    y2debug ("Write: adding recursively %s to %s", k.c_str (), p->toString().c_str());

	    IniSection s (ip, k);
	    container.push_back (IniContainerElement (s));
	    isections.insert (IniSectionIndex::value_type (k, --container.end ()));

	    si = --container.end ();
	}
	else
	{
	    // there's something, choose last
	    si = (--xe)->second;
	}
	return si->s ().setSectionProp (p, in, what, depth+1);
    }
    else
    {
	// bottom level

	// make sure we have a list of values
	YCPList props;
	if (ip->repeatNames ())
	{
	    props = as_list (in, "property of section with repeat_names");
	    if (props.isNull())
		return -1;
	}
	else
	{
	    props->add (in);
	}
	int pi = 0, pe = props->size ();

	// Go simultaneously through the found sections
	// and the list of parameters, while _either_ lasts
	// Fewer sections-> add them, more sections-> delete them

	while (pi != pe || xi != xe)
	{
	    // watch out for validity of iterators!

	    if (pi == pe)
	    {
		// deleting a section
		delSection1 (xi++);
		// no ++pi
	    }
	    else
	    {
		YCPValue prop = props->value (pi);
		IniIterator si;
		if (xi == xe)
		{
		    ///need to add a section ...
		    y2debug ("Adding section %s", p->toString().c_str());
		    // prepare it to have its property set
		    // create it
		    IniSection s (ip, k);
		    s.dirty = true;
		    // insert and index
		    container.push_back (IniContainerElement (s));
		    isections.insert (IniSectionIndex::value_type (k, --container.end ()));
		    si = --container.end ();
		}
		else
		{
		    si = xi->second;
		}

		// set a section's property
		IniSection & s = si->s ();
		if (what == 0) {
		    YCPString str = as_string (prop, "section_comment");
		    if (str.isNull())
			return -1;
		    s.setComment (str->value_cstr());
		}
		else if (what == 1) {
		    YCPInteger i = as_integer (prop, "section_rewrite");
		    if (i.isNull())
			return -1;
		    s.setRewriteBy (i->value());
		}
		else if (what == 2) {
		    YCPInteger i = as_integer (prop, "section_type");
		    if (i.isNull())
			return -1;
		    s.setReadBy (i->value());
		}
		else if (what == 3) {
		    YCPBoolean b = as_boolean (prop, "section_private");
		    if (b.isNull())
			return -1;
		    s.setPrivate (b->value());
		}

		if (xi != xe)
		{
		    ++xi;
		}
		++pi;
	    }
	    // iterators have been advanced already
	}
	return 0;
    }
}

void IniSection::delSection1 (IniSectionIdxIterator sxi)
{
    dirty = true;
    IniIterator si = sxi->second;
    container.erase (si);
    isections.erase (sxi);
}

int IniSection::delSection(const YCPPath&p, int depth)
{
    string k = ip->changeCase (p->component_str (depth));

    // Find the matching sections.
    // If we need to recurse, choose one
    // Otherwise kill them all

    pair <IniSectionIdxIterator, IniSectionIdxIterator> r =
	isections.equal_range (k);
    IniSectionIdxIterator xi = r.first, xe = r.second;

    if (depth + 1 < p->length())
    {
	// recurse
	if (xi != xe)
	{
	    // there's something
	    IniSection& s = (--xe)->second->s ();
	    return s.delSection (p, depth+1);
	}
	//error
	y2error ("Delete: Invalid path %s [%d]", p->toString().c_str(), depth);
	return -1;
    }
    else
    {
	// bottom level, massacre begins
	if (xi == xe)
	{
	    y2debug ("Can not delete %s. Key does not exist.", p->toString().c_str());
	}
	while (xi != xe)
	{
	    delSection1 (xi++);
	}
    }
    return 0;
}

int IniSection::setAll (const YCPPath&p, const YCPValue& in, int depth)
{
    if (depth < p->length ())
    {
	// recurse to find the starting section
	// Get any section of that name
	string k = ip->changeCase (p->component_str (depth));
	IniSectionIdxIterator sxi = isections.find (k);
	if (sxi != isections.end())
	{
	    return sxi->second->s ().setAll (p, in, depth+1);
	}
	// else error
    }
    else
    {
	return setAllDoIt (in->asMap ());
    }

    y2debug ("Write: Invalid path %s [%d]", p->toString().c_str(), depth);
    return -1;
}

int IniSection::setAllDoIt (const YCPMap &in)
{
    int ret = IniBase::setAllDoIt (in);
    if (ret != 0)
    {
	return ret;
    }

    string kind;
    if (!getMapString (in, "kind", kind) || kind != "section")
    {
	y2error ("Kind should be 'section'");
	return -1;
    }

    if (!getMapInteger (in, "file", rewrite_by))
    {
	return -1;
    }

    YCPValue v = in->value (YCPString ("value"));
    if (v.isNull () || !v->isList ())
    {
	y2error ("Missing in Write (.all): %s", "value");
	return -1;
    }
    YCPList l = v->asList ();

    container.clear ();		// bye, old data
    int i, len = l->size ();
    for (i = 0; i < len; ++i)
    {
	YCPValue item = l->value (i);
	if (!item->isMap ())
	{
	    y2error ("Item in Write (.all) not a map");
	    ret = -1;
	    break;
	}
	YCPMap mitem = item->asMap ();

	if (!getMapString (mitem, "kind", kind))
	{
	    y2error ("Item in Write (.all) of unspecified kind");
	    ret = -1;
	    break;
	}

	if (kind == "section")
	{
	    // check whether we are deleting a file-section
	    YCPValue mv = mitem->value (YCPString ("value"));
	    if (mv->isVoid ())
	    {
		string del_name;
		int del_rb;		
		if (!getMapString (mitem, "name", del_name) ||
		    !getMapInteger (mitem, "file", del_rb))
		{
		    ret = -1;
		    break;
		}
		const_cast<IniParser *>(ip)->deleted_sections.insert (
		    ip->getFileName (del_name, del_rb)
		    );
	    }
	    else
	    {
		IniSection s (ip);
		ret = s.setAllDoIt (mitem);
		if (ret != 0)
		{
		    break;
		}
		container.push_back (IniContainerElement (s));
	    }
	}
	else if (kind == "value")
	{
	    IniEntry e;
// FIXME ret =
	    e.setAllDoIt (mitem);
	    if (ret != 0)
	    {
		break;
	    }
	    container.push_back (IniContainerElement (e));
	}
	else
	{
	    y2error ("Item in Write (.all) of unrecognized kind %s", kind.c_str ());
	    ret = -1;
	    break;
	}
    }

    reindex ();
    return ret;
}

int IniSection::setMyValue (const YCPPath &p, const YCPValue&in, int what, int depth)
{
    // assert (depth == p->length ()); //not, it can have a .comment suffix
    string k = ip->changeCase (p->component_str (depth));
    pair <IniEntryIdxIterator, IniEntryIdxIterator> r =
	ivalues.equal_range (k);
    IniEntryIdxIterator xi = r.first, xe = r.second;

    // make sure we have a list of values
    YCPList props;
    if (ip->repeatNames ())
    {
	props = as_list (in, "section with repeat_names");
	if (props.isNull())
	    return -1;
    }
    else
    {
	props->add (in);
    }
    int pi = 0, pe = props->size ();

    // Go simultaneously through the found values
    // and the list of parameters, while _either_ lasts
    // Fewer values-> add them, more values-> delete them
    while (pi != pe || xi != xe)
    {
	// watch out for validity of iterators!

	if (pi == pe)
	{
	    // deleting a value
	    delValue1 (xi++);
	    // no ++pi
	}
	else
	{
	    YCPValue prop = props->value (pi);
	    IniIterator ei;
	    if (xi == xe)
	    {
		///need to add a value ...
		y2debug ("Adding value %s = %s",
			 p->toString ().c_str (), prop->toString().c_str ());
		if (what)
		{
		    y2error ("You must add value before changing comment/type. %s",
			     p->toString ().c_str ());
		    return -1;
		}
		// prepare it to have its property set
		// create it
		IniEntry e;
		// need to set its name
		e.setName (k);

		// insert and index
		container.push_back (IniContainerElement (e));
		ivalues.insert (IniEntryIndex::value_type (k, --container.end ()));
		ei = --container.end ();
	    }
	    else
	    {
		ei = xi->second;
	    }

	    // set a value's property
	    IniEntry & e = ei->e ();
	    switch (what)
	    {
		case 0:
		    e.setValue (to_string (prop));
		    break;
		case 1: {
		    YCPString s = as_string (prop, "comment");
		    if (s.isNull())
			return -1;
		    e.setComment (s->value());
		    break;
		}
		default: {
		    YCPInteger i = as_integer (prop, "value_type");
		    if (i.isNull())
			return -1;
		    e.setReadBy (i->value());
		    break;
		}
	    }

	    if (xi != xe)
	    {
		++xi;
	    }
	    ++pi;
	}
	// iterators have been advanced already
    }
    dirty = true;
    return 0;
}

int IniSection::setValue (const YCPPath&p,const YCPValue&in,int what, int depth)
{
    string k = ip->changeCase (p->component_str (depth));
    // Find the matching sections.
    // If we need to recurse, choose one, creating if necessary
    // Otherwise set all the matching values
    //  creating and deleting if the number of them does not match

    if (depth + 1 < p->length())
    {
	// recurse
	pair <IniSectionIdxIterator, IniSectionIdxIterator> r =
	    isections.equal_range (k);
	IniSectionIdxIterator xi = r.first, xe = r.second;

	IniIterator si;
	if (xi == xe)
	{
	    // not found, need to add it;
	    y2debug ("Write: adding recursively %s to %s", k.c_str (), p->toString().c_str());

	    IniSection s (ip, k);
	    container.push_back (IniContainerElement (s));
	    isections.insert (IniSectionIndex::value_type (k, --container.end ()));

	    si = --container.end ();
	}
	else
	{
	    // there's something, choose last
	    si = (--xe)->second;
	}
	return si->s ().setValue (p, in, what, depth+1);
    }
    else
    {
	// bottom level
	return setMyValue (p, in, what, depth);
    }
}

void IniSection::delValue1 (IniEntryIdxIterator exi)
{
    dirty = true;
    IniIterator ei = exi->second;
    container.erase (ei);
    ivalues.erase (exi);
}

void IniSection::delMyValue (const string &k)
{
    pair <IniEntryIdxIterator, IniEntryIdxIterator> r =
	ivalues.equal_range (k);
    IniEntryIdxIterator xi = r.first, xe = r.second;

    if (xi == xe)
    {
	y2debug ("Can not delete %s. Key does not exist.", k.c_str());
    }
    while (xi != xe)
    {
	delValue1 (xi++);
    }
}

int IniSection::delValue (const YCPPath&p, int depth)
{
    string k = ip->changeCase (p->component_str (depth));
    // Find the matching sections.
    // If we need to recurse, choose one
    // Otherwise kill all values of the name

    if (depth + 1 < p->length())
    {
	// recurse
	pair <IniSectionIdxIterator, IniSectionIdxIterator> r =
	    isections.equal_range (k);
	IniSectionIdxIterator xi = r.first, xe = r.second;

	if (xi != xe)
	{
	    // there's something
	    IniSection& s = (--xe)->second->s ();
	    return s.delValue (p, depth+1);
	}
	//error
	y2error ("Delete: Invalid path %s [%d]", p->toString().c_str(), depth);
	return -1;
    }
    else
    {
	// bottom level, massacre begins
	delMyValue (k);
    }
    return 0;
}

int IniSection::myDir (YCPList& l, IniType what)
{
    IniIterator i = container.begin (), e = container.end ();
    for (; i != e; ++i)
    {
	if (i->t () == what)
	{
	    string n = (what == SECTION) ?
		i->s ().getName () :
		i->e ().getName ();
	    l->add (YCPString (n));
	}
    }
    return 0;
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

    return myDir (l, VALUE);
}

int IniSection::getValueFlat (const YCPPath&p, YCPValue&out)
{
    if (!p->length ())
	return -1;
    string k = ip->changeCase (p->component_str (0));
    bool want_comment = p->length()>1 && p->component_str(1)=="comment";

    return getMyValue (p, out, want_comment, 0);
}

int IniSection::delValueFlat (const YCPPath&p)
{
    if (!p->length ())
	return -1;
    string k = ip->changeCase (p->component_str (0));

    delMyValue (k);
    return 0;
}

int IniSection::setValueFlat (const YCPPath&p, const YCPValue &in)
{
    if (!p->length ())
	return -1;
    string k = ip->changeCase (p->component_str (0));
    bool want_comment = p->length()>1 && p->component_str(1)=="comment";

    return setMyValue (p, in, want_comment, 0);
}

int IniSection::Read (const YCPPath&p, YCPValue&out, bool rewrite)
{
    if (ip->isFlat ())
	return getValueFlat (p, out);

    if (p->length() >= 1 && p->component_str (0) == "all")
    {
	return getAll (p, out, 1);
    }

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
    if (ip->isFlat ())
	return dirValueFlat (p, l);
    if (p->length()<1)
	{
	    l.add (YCPString ("section"));
	    l.add (YCPString ("value"));
	    return 0;
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
    if (depth >= p->length())
    {
	return myDir (out, get_sect? SECTION: VALUE);
    }

    // recurse
    string k = ip->changeCase (p->component_str (depth));

    pair <IniSectionIdxIterator, IniSectionIdxIterator> r =
	isections.equal_range (k);
    IniSectionIdxIterator xi = r.first, xe = r.second;

    if (xi != xe)
    {
	// there's something
	IniSection& s = (--xe)->second->s ();
	return s.dirHelper (p, out, get_sect, depth+1);
    }
    //error
    y2debug ("Dir: Invalid path %s [%d]", p->toString().c_str(), depth);
    return -1;
}

/*
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
*/

int IniSection::getSubSectionRewriteBy (const char*name)
{
    pair <IniSectionIdxIterator, IniSectionIdxIterator> r =
	isections.equal_range (name);
    IniSectionIdxIterator xi = r.first, xe = r.second;

    if (xi == xe)
    {
	return -1;
    }
    return (--xe)->second->s ().getRewriteBy ();
}

IniSection&IniSection::getSection (const char*n)
{
    pair <IniSectionIdxIterator, IniSectionIdxIterator> r =
	isections.equal_range (n);
    IniSectionIdxIterator xi = r.first, xe = r.second;

    if (xi == xe)
    {
	y2error ("Internal error. Section %s not found in section %s", n, name.c_str());
	abort();
    }
    return (--xe)->second->s ();
}

void IniSection::setEndComment (const char*c)
{
    if (comment.empty () && container.empty ())
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
    IniSectionIdxIterator xi = isections.begin (), xe = isections. end ();
    for (; xi != xe; ++xi)
    {
	if (xi->second->s ().isDirty ())
	  return true;
    }
    return false;
}
void IniSection::clean()
{
    dirty = false;
    IniIterator i = container.begin (), e = container.end ();
    for (; i != e; ++i)
    {
	if (i->t () == SECTION)
	{
	    i->s ().clean ();
	}
	else
	{
	    i->e ().clean ();
	}
    }
}

IniIterator IniSection::getContainerBegin ()
{
    return container.begin ();
}

IniIterator IniSection::getContainerEnd ()
{
    return container.end ();
}
