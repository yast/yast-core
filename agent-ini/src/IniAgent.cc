/*
 * YaST2: Core system
 *
 * Description:
 *   YaST2 SCR: Ini agent implementation
 *
 * Authors:
 *   Petr Blahos <pblahos@suse.cz>
 *
 * $Id$
 */

#include "IniAgent.h"
#include "IniParser.h"
#include "IniFile.h"

/**
 * Constructor
 */
IniAgent::IniAgent() : SCRAgent()
{
}

/**
 * Destructor
 */
IniAgent::~IniAgent()
{
    if (parser.isStarted())
	parser.write();
}

/**
 * Dir
 */
YCPValue IniAgent::Dir(const YCPPath& path)
{
    if (!parser.isStarted())
	{
	    y2warning("Can't execute Dir before being mounted.");
	    return YCPVoid();
	}
    parser.UpdateIfModif ();

    YCPList l;
    if (!parser.inifile.Dir (path, l))
	return l;

    return YCPVoid();
}

/**
 * Read
 */
YCPValue IniAgent::Read(const YCPPath &path, const YCPValue& arg)
{
    if (!parser.isStarted())
	{
	    y2warning("Can't execute Read before being mounted.");
	    return YCPVoid();
	}
    parser.UpdateIfModif ();

    YCPValue out = YCPVoid ();
    if (!parser.inifile.Read (path, out, parser.HaveRewrites ()))
	return out;

    return YCPVoid();
}

/**
 * Write
 */
YCPValue IniAgent::Write(const YCPPath &path, const YCPValue& value, const YCPValue& arg)
{
    if (!parser.isStarted())
    {
	y2warning("Can't execute Write before being mounted.");
	return YCPVoid();
    }
    // no need to update if modified, we are changing value
    bool ok = false;
    YCPBoolean b (true);

    if (0 == path->length ())
    {
	if (value->isString() && value->asString()->value() == "force")
	    parser.inifile.setDirty();
	else if (value->isString () && value->asString()->value() == "clean")
	    parser.inifile.clean ();
	if (0 != parser.write ())
	    b = false;
	ok = true;
    }
    else
    {
	if (value->isString () || value->isInteger())
	    {
		ok = true;
		if (parser.inifile.Write (path, value, parser.HaveRewrites ()))
		    b = false;
	    }
        else if (value->isVoid ())
	    {
		int wb  = -1;
		string del_sec = "";
		ok = true;
		if (2 == path->length ())
		{
		    string pc = path->component_str(0);
		    if ("s" == pc || "section" == pc)
		    {	// request to delete section. Find the file name
			del_sec = path->component_str (1);
			wb = parser.inifile.getSubSectionRewriteBy (del_sec.c_str());
		    }
		}
		if (parser.inifile.Delete (path))
		    b = false;
		else if (del_sec != "")
		{
		    parser.deleted_sections.insert (parser.getFileName (del_sec, wb));
		}
	    }
	else
	{
	    return YCPError (string ("Wrong value for path ") +  path->toString () + string (": ") + value->toString ());
	}
    }
    if (!ok)
    	return YCPError (
		string ("Wrong path '")
		+ path->toString()
		+ string ("' in Write()."));

    return b;
}

/**
 * otherCommand
 */
YCPValue IniAgent::otherCommand(const YCPTerm& term)
{
    string sym = term->symbol()->symbol();
    if (sym == "SysConfigFile") 
    {
	if (term->size () != 1 || !term->value (0)->isString ())
	{
	    return YCPError ("Bad number of arguments. Expecting SysConfigFile (\"filename\")");
	}
	string file_name;
	file_name = term->value(0)->asString()->value();
	YCPTerm tt = generateSysConfigTemplate (file_name);
	parser.inifile.setFlat ();
	return otherCommand (tt);
    }
    if (sym == "IniAgent") 
    {
	if (2 == term->size()) // fixme: we will provide some default actions if 2nd arg is missing in future
	{
	    if (!term->value(0)->isString () && !term->value(0)->isList ())
	        return YCPError ("Bad initialization of IniFile(): first argument must be string or list of strings.");
	    if (!term->value(1)->isMap())
		return YCPError ("Bad initialization of IniFile(): second argument is not map.");
	    if (term->value (0)->isString ())
		parser.initFiles(term->value(0)->asString()->value().c_str());
	    else
		parser.initFiles(term->value(0)->asList());
	    parser.initMachine (term->value(1)->asMap());
	    parser.parse ();
	    return YCPVoid ();
	}
	return YCPError ("Bad initialization of IniFile(): 2 arguments expected.");
    }

    return YCPNull ();
}

YCPTerm IniAgent::generateSysConfigTemplate (string fn)
{
    YCPTerm t (string ("IniAgent"), false);
    YCPMap m;
    YCPList l;

    // file name
    t->add (YCPString (fn));

    l->add (YCPString ("line_can_continue"));
    l->add (YCPString ("global_values"));
    l->add (YCPString ("join_multiline"));
    l->add (YCPString ("comments_last"));
    m->add (YCPString ("options"), l);

    // comments
    l = YCPList ();
    l->add (YCPString ("^[ \t]*#.*$"));
    l->add (YCPString ("#.*"));
    l->add (YCPString ("^[ \t]*$"));
    m->add (YCPString ("comments"), l);

    // sections are empty so we will omit them

    // params
    YCPMap param;
    YCPList lp;
    l = YCPList ();
    lp->add (YCPString ("([a-zA-Z0-9_]+)[ \t]*=[ \t]*\"([^\"]*)\""));
    lp->add (YCPString ("%s=\"%s\""));
    param->add (YCPString ("match"), lp);
    lp = YCPList ();
    lp->add (YCPString ("([a-zA-Z0-9_]+)[ \t]*=[ \t]*\"([^\"]*)"));
    lp->add (YCPString ("([^\"]*)\""));
    param->add (YCPString ("multiline"), lp);
    l->add (param);

    lp = YCPList ();
    param = YCPMap ();
    lp->add (YCPString ("^[ \t]*([a-zA-Z_][a-zA-Z0-9_]*)[ \t]*=[ \t]*'([^']*)'"));
    lp->add (YCPString ("%s='%s'"));
    param->add (YCPString ("match"), lp);
    lp = YCPList ();
    lp->add (YCPString ("([a-zA-Z_][a-zA-Z0-9_]*)[ \t]*=[ \t]*'([^']*)"));
    lp->add (YCPString ("([^']*)'"));
    param->add (YCPString ("multiline"), lp);
    l->add (param);

    lp = YCPList ();
    param = YCPMap ();
    lp->add (YCPString ("([a-zA-Z0-9_]+)[ \t]*=[ \t]*([^\"]*[^ \t\"]|)[ \t]*$"));
    lp->add (YCPString ("%s=\"%s\""));
    param->add (YCPString ("match"), lp);
    l->add (param);

    m->add (YCPString ("params"), l);
    t->add (m);
    return t;
}
