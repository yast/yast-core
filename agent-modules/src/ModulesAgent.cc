/* ModulesAgent.cc
 *
 * An agent for reading the modules.conf configuration file.
 *
 * Author: Michal Svec <msvec@suse.cz>
 *         Daniel Vesely <dan@suse.cz>
 *
 * $Id$
 *
 */


#include "Y2Logger.h"
#include "ModulesAgent.h"
#include "ModulesConf.h"

#define MAGIC_DIRECTIVE "extra"

#define PC(n)       (path->component_str(n))
#define VAL2STR(v)  ((v)->asString()->value())
#define VAL2CSTR(v) ((v)->asString()->value_cstr())

/*
  PATH:
        MAP[STRING].MAP[STRING].MAP[STRING].(MAP[STRING].STRING|STRING)

  READ:
        .modules.options.<module>.<option>
                           "options".STRING.MAP
        .modules.options.<module>.<option>.<parameters>
                           "options".STRING.STRING.STRING
        .modules.<directive>.<module>.<argument>
                           STRING.STRING.STRING
        .modules.<directive>.<module>.comment.<comment>
                           STRING.STRING."comment".STRING
        .modules.<directive>.<module>
                           STRING.STRING.LIST
        .modules.<directive>
                           STRING.LIST
        .modules
                           LIST
*/

/**
 * Constructor
 */
ModulesAgent::ModulesAgent() : SCRAgent(), modules_conf(NULL) {
}

/**
 * Destructor
 */
ModulesAgent::~ModulesAgent() {
    if (modules_conf != NULL)
	delete modules_conf;
}

/**
 * Simple template function for converting c++ map into YCP list
 */
template <class T> YCPList map2list(const T &m) {
    YCPList list;
    typename T::const_iterator it = m.begin ();
	
    for (; it != m.end (); ++it)
	/* Preserve listing of the final comment */
	if(it->first != "YaST2_final_modules_conf_comment")
	    list->add(YCPString (it->first));
	
    return list;
}

/**
 * Simple template function for converting c++ map into YCP map
 */
template <class T> YCPMap map2ycpmap(const T &m) {
    YCPMap ret_map;
    typename T::const_iterator it = m.begin ();
	
    for (; it != m.end(); ++it)
	ret_map->add (YCPString (it->first), YCPString (it->second));
	
    return ret_map;
}

/**
 * Simple function for converting YCP map into c++ map
 */
ModuleEntry::EntryArg ycpmap2map (const YCPMap &m) {
    ModuleEntry::EntryArg ret_map;
    YCPMapIterator it = m->begin ();

    for (; it != m->end(); ++it)
	if (it.key()->isString () && it.value ()->isString ())
	    ret_map[VAL2STR(it.key ())] = VAL2STR(it.value ());
	else {
	    y2error("Map element must be string!");
	    return ModuleEntry::EntryArg();
	}
    
    return ret_map;
}


/**
 * Dir
 */
YCPValue ModulesAgent::Dir(const YCPPath& path) {
    YCPList list;
    string elem;

    if (modules_conf == NULL)
	Y2_RETURN_VOID("Can't execute Dir before being mounted.");

    switch (path->length ()) {

	case 0:
	    /* .modules -> ["alias","options"] */
	    return YCPList (map2list (modules_conf->getDirectives ()));

	case 1:
	    if(PC(0) == MAGIC_DIRECTIVE)
		Y2_RETURN_VOID("Dir() doesn't support the .%s directive", MAGIC_DIRECTIVE);

	    /* .modules.options -> ["eth0","eth1"] */
	    return YCPList (map2list (modules_conf->getModules(PC(0))));

	case 2:
	    /* .modules.options.eth0 -> ["irq","io"] */
	    if(PC(0) == "options")
		return YCPList (map2list (modules_conf->getOptions(PC(1))));

    }

    Y2_RETURN_VOID("Wrong path '%s' in Dir().", path->toString().c_str());
}


/**
 * Read
 */
YCPValue ModulesAgent::Read(const YCPPath &path, const YCPValue& arg) {
	    
    if (modules_conf == NULL)
	Y2_RETURN_VOID("Can't execute Read before being mounted.");
	    
    y2debug("Read(%s)", path->toString().c_str());

    switch (path->length ()) {

    case 0:
	/* .modules -> ["alias","options"] */
	return YCPList (map2list (modules_conf->getDirectives()));

    case 1:
	/* FIXME: remove */
	if (!arg.isNull() && arg->isString ()) {
	    y2error("Obsolete interface, don't use any more!");
	    if (PC(0) == "options")
		return YCPMap (map2ycpmap (modules_conf->getOptions(VAL2STR(arg))));
	    else
		return YCPString (modules_conf->getArgument(PC(0), VAL2STR(arg)));
	}

	/* .modules.options -> ["eth0","eth1"] */
	return YCPList (map2list (modules_conf->getModules(PC(0))));

#if 0 /* some attempts to support other directives */

	/* .modules.keep -> "" */
	/* .modules.prune -> "filename" */
	if (PC(0) == "keep" || PC(0) == "prune") {
	    YCPPath newpath;
	    newpath->append(PC(0));
	    newpath->append(string(""));
	    //newpath->append(string("\"\""));
	    return Read(newpath, arg);
	}

	/* .modules.keep -> "" */
	/* .modules.prune -> "filename" */
	if (PC(0) != "keep" && PC(0) != "prune")
	    Y2_RETURN_VOID("Unsupported simple directive: %s,"
		    "please inform maintainer", PC(0).c_str());

	/* .modules.keep -> "" [zero args directives -> ignore 2nd arg] */
	if(PC(0) == "keep")
	    return YCPString ("");

	/* .modules.prune, "filename" */
	return YCPString (modules_conf->getArgument(PC(0), ""));
#endif

	Y2_RETURN_VOID("Wrong path '%s' in Read().", path->toString().c_str());

    case 2:
	/* FIXME: remove */
	if (!arg.isNull () && arg->isString ()) {
	    y2error("Obsolete interface, don't use any more!");
	    if (PC(1) == "comment")
		return YCPString (modules_conf->getComment(PC(0), VAL2STR(arg)));
	    if (PC(0) == "options")
		return YCPString (modules_conf->getOption(VAL2STR(arg), PC(1)));
	}

	/* .modules.options.eth0 -> $["irq":"7"] */
	if(PC(0) == "options")
	    return YCPMap (map2ycpmap (modules_conf->getOptions(PC(1))));
	/* .modules.alias.eth0 -> "off" */
	else
	    return YCPString (modules_conf->getArgument(PC(0), PC(1)));

    case 3:
	/* .modules.alias.eth0.comment -> "# comment\n" */
	if (PC(2) == "comment")
	    return YCPString (modules_conf->getComment(PC(0), PC(1)));
	/* .modules.options.eth0.irq -> "7" */
	if (PC(0) == "options")
	    return YCPString (modules_conf->getOption(PC(1), PC(2)));

    }
    
    Y2_RETURN_VOID("Wrong path '%s' in Read().", path->toString().c_str());
}

/**
 * Write
 */
YCPValue ModulesAgent::Write(const YCPPath &path, const YCPValue& value, const YCPValue& arg) {

    if (modules_conf == NULL)
	Y2_RETURN_VOID("Can't execute Write before being mounted.");

    if (path->isRoot() && value->isVoid ())
	return YCPBoolean(modules_conf->writeFile());

    switch (path->length ()) {

    case 1:
	/* FIXME: remove */
	if (!arg.isNull () && arg->isString ()) {
	    y2error("Obsolete interface, don't use any more!");
	    if (value->isVoid ())
		return YCPBoolean (modules_conf->removeEntry (PC(0), VAL2STR(arg)));
	    if (PC(0) == "options") {
		if (value->isMap ())
		    return YCPBoolean (modules_conf->setOptions(VAL2STR(arg),
								ycpmap2map (value->asMap ()),
								ModuleEntry::SET));
		else 
		    Y2_RETURN_YCP_FALSE("Argument for Write () not map.");
	    }
	    return YCPBoolean (modules_conf->setArgument (PC(0), VAL2STR(arg),
							  VAL2STR(value),
							  ModuleEntry::SET));
	}

#if 0 /* some attempts to support other directives */

	/* .modules.keep, nil */
	/* .modules.prune, nil */
	if (PC(0) != "keep" && PC(0) != "prune")
	    Y2_RETURN_YCP_FALSE("Unsupported simple directive: %s,"
		    "please inform maintainer", PC(0).c_str());

	if (value->isVoid ())
	    return YCPBoolean (modules_conf->removeEntry (PC(0), ""));

	/* .modules.keep, "" [zero args directives -> ignore 2nd arg] */
	if (PC(0) == "keep")
	    return YCPBoolean (modules_conf->setArgument (PC(0), "",
			"", ModuleEntry::SET));

	if (!value->isString())
	    Y2_RETURN_YCP_FALSE("Argument for Write() is not string: %s.",
		    value->toString().c_str());

	/* .modules.prune, "filename" */
	return YCPBoolean (modules_conf->setArgument (PC(0), "",
		    VAL2STR(value), ModuleEntry::SET));
#endif

	Y2_RETURN_YCP_FALSE("Argument (2nd) for Write() is not string.");

    case 2:
	/* FIXME: remove */
	if (value->isString () && !arg.isNull () && arg->isString ()) {
	    y2error("Obsolete interface, don't use any more!");
	    if (PC(0) == "options")
		return YCPBoolean (modules_conf->setOption (VAL2STR(arg), PC(1),
							    VAL2STR(value),
							    ModuleEntry::SET));
	    if (PC(1) == "comment")
		return YCPBoolean (modules_conf->setComment (PC(0), VAL2STR(arg),
							     VAL2STR(value),
							     ModuleEntry::SET));
	}

	/* .modules.alias.eth0, nil */
	if (value->isVoid ())
	    return YCPBoolean (modules_conf->removeEntry (PC(0), PC(1)));

	/* .modules.options.eth0, $["irq":"7"] */
	if (PC(0) == "options") {
	    if (value->isMap ())
		return YCPBoolean (modules_conf->setOptions(PC(1),
			    ycpmap2map (value->asMap ()), ModuleEntry::SET));
	    else 
		Y2_RETURN_YCP_FALSE("Argument for Write(.options) not map: %s.",
			value->toString().c_str());
	}

	if (!value->isString())
	    Y2_RETURN_YCP_FALSE("Argument for Write() is not string: %s.",
		    value->toString().c_str());

	/* .modules.alias.eth0, "off" */
	return YCPBoolean (modules_conf->setArgument (PC(0), PC(1),
		    VAL2STR(value), ModuleEntry::SET));

    case 3:
	if (!value->isString())
	    Y2_RETURN_YCP_FALSE("Argument for Write() is not string: %s.",
		    value->toString().c_str());

	/* .modules.alias.eth0.comment, "# First tulip\n" */
	if (PC(2) == "comment") {
	    return YCPBoolean (modules_conf->setComment (PC(0), PC(1),
			VAL2STR(value), ModuleEntry::SET));
	}
	/* .modules.options.eth0.irq, "7" */
	if (PC(0) == "options")
	    return YCPBoolean (modules_conf->setOption (PC(1), PC(2),
			VAL2STR(value), ModuleEntry::SET));
    }

    Y2_RETURN_VOID("Wrong path '%s' in Write().", path->toString().c_str());
}


/**
 * otherCommand
 */
YCPValue ModulesAgent::otherCommand(const YCPTerm& term) {
    string sym = term->symbol()->symbol();

    if (sym == "ModulesConf" && term->size() == 1) {
	if (term->value(0)->isString()) {
	    YCPString s = term->value(0)->asString();
	    if (modules_conf != NULL)
		delete modules_conf;
	    modules_conf = new ModulesConf(s->value());
	    return YCPVoid();
	} else 
	    Y2_RETURN_VOID("Bad first arg of ModulesConf(): is not a string.");
    }

    return YCPNull();
}

