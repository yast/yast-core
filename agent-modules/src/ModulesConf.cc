/* ModulesConf.cc
 *
 * Classes for reading the modules.conf configuration file.
 *
 * Author: Michal Svec <msvec@suse.cz>
 *         Daniel Vesely <dan@suse.cz>
 *
 * $Id$
 */

#include "config.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

#include <iostream>
#include <fstream>

using std::ifstream;
using std::ofstream;
using std::endl;

#include <Y2.h>

#include "ModulesConf.h"
#include "Y2Logger.h"

#define BACKUP_EXTENSION ".YaST2save"
#define FINAL_COMMENT "YaST2_final_modules_conf_comment"

/**
 * Destrucutor
 */
ModuleEntry::~ModuleEntry () {
}

/**
 * Manage the dirty_flag
 */
bool ModuleEntry::Set(Mode m) {
    switch(m) {
    case INIT:
        dirtyflag=false;
        return true;
    case SET:
        dirtyflag=true;
        return true;
    case REINIT:
        return !dirtyflag;
    }
    return false;
}

bool ModuleEntry::setArgument(const string arg, Mode m) {
    y2debug("MAGIC_arg: %s", arg.c_str());

    if (!Set (m)) return false;
    argument[MAGIC_ENTRY] = arg;
    return true;
}

bool ModuleEntry::setOption(const string option, const string value, Mode m) {
    y2debug("OPTION_arg: %s %s", option.c_str(), value.c_str());

    if (!Set (m)) return false;
    argument.erase(MAGIC_ENTRY);
    argument[option]=value;
    return true;
}

bool ModuleEntry::setOptions(const EntryArg &arg, Mode m) {
    y2debug("OPTIONS_arg");

    if(!Set (m)) return false;
    if(arg.empty())
        Y2_RETURN_FALSE("setOptions:Empty map");
    argument = arg;
    return true;
}

ModuleEntry::EntryCom ModuleEntry::getComment() const {

    return (comment.length () ?
            (comment.find_last_of ("\n") + 1 == comment.length () ? comment : comment + "\n") :
            comment);
}

/**
 * Constructor
 */
ModulesConf::ModulesConf(const string &fname)
    : file_name (fname), modules_conf_map (), modules_conf_index (), modified (false) {
    y2debug("ModulesConf()");
    if(!parseFile(fname, ModuleEntry::INIT, true))
        y2warning("ModulesConf: parseFile failed");
}

/**
 * Destructor
 */
ModulesConf::~ModulesConf() {
    if (!writeFile())
        y2error("Can't write configuration file in destructor.");
}

/*
 * is* functions
 */

bool ModulesConf::isDirective(const string directive) const {
    return modules_conf_map.find(directive) != modules_conf_map.end();
}

bool ModulesConf::isModule(const string directive, const string module) {
    return isDirective (directive) &&
	modules_conf_map[directive].find(module) != modules_conf_map[directive].end();
}

bool ModulesConf::isOption(const string module, const string option) {
    return isModule ("options", module) &&
	modules_conf_map["options"][module].getArgument().find(option) !=  modules_conf_map["options"][module].getArgument().end();
}

/*
 * get* functions
 */


ModulesConf::ModulesConfMap ModulesConf::getDirectives() {
    updateIfModified ();
    return modules_conf_map;
}

ModulesConf::ModuleEntryMap ModulesConf::getModules(const string directive) {
    updateIfModified ();
    if(!isDirective(directive)) {
	y2error("Bad directive: %s", directive.c_str());
        return ModulesConf::ModuleEntryMap();
    }
    return modules_conf_map[directive];
}

string ModulesConf::getArgument(const string directive, const string module) {
    updateIfModified ();
    if(!isModule(directive,module))
	Y2_RETURN_STR("Bad directive or module: %s, %s", directive.c_str(), module.c_str());

    ModuleEntry::EntryArg ea = modules_conf_map[directive][module].getArgument();
    y2debug("getArgument(%s,%s) %p",directive.c_str(),module.c_str(), this);
    if(ea.find(MAGIC_ENTRY)==ea.end()) {
        y2warning("Bad request for string while there are options (%s, %s).", directive.c_str(), module.c_str());
        return getOptionsAsString(module);
    }
    y2debug("MAGIC: %s", ea[MAGIC_ENTRY].c_str());
    return ea[MAGIC_ENTRY];
}

string ModulesConf::getComment(const string directive, const string module) {
    updateIfModified ();
    if(!isModule(directive,module))
	Y2_RETURN_STR("Bad directive or module: %s, %s", directive.c_str(), module.c_str());

    return modules_conf_map[directive][module].getComment();
}

ModuleEntry::EntryArg ModulesConf::getOptions(const string module) {
    updateIfModified ();
    if(!isModule("options",module)) {
	y2error("Bad options for module: %s", module.c_str());
        return ModuleEntry::EntryArg();
    }
    ModuleEntry::EntryArg ea = modules_conf_map["options"][module].getArgument();
    if(ea.find(MAGIC_ENTRY)!=ea.end()) {
	y2error("Bad request for options while there is only a string (%s).", module.c_str());
	return ModuleEntry::EntryArg();
    }
    return ea;
}

/*
 * Move test for MAGIC_ENTRY to ModuleEntry _above_ AND
 * return modules_conf_map[][].getOption(option); _below
 * has been planned, but probably remains here, because of
 * better possibility of error reporting.
 */

string ModulesConf::getOption(const string module, const string option) {
    updateIfModified ();
    if(!isOption(module,option))
	Y2_RETURN_STR("Bad module or option: %s, %s", module.c_str(), option.c_str());
    ModuleEntry::EntryArg ea = modules_conf_map["options"][module].getArgument();
    if(ea.find(MAGIC_ENTRY)!=ea.end())
	Y2_RETURN_STR("Bad request for option while there is a string (%s, %s).", module.c_str(), option.c_str());
    if(ea.find(option)==ea.end())
	Y2_RETURN_STR("Bad request for option %s (%s).", option.c_str(), module.c_str());
    y2debug("OPTION: %s", ea[option].c_str());
    return ea[option];
}

string ModulesConf::getOptionsAsString (const string module) {
    ModuleEntry::EntryArg::const_iterator it;
    ModuleEntry::EntryArg entry;
    string ret;

    entry = getOptions(module);
    it = entry.begin();
    for (; it != entry.end(); ++it)
	ret += " " + it->first + (it->second != "" ? (it->first == "-o" ? " " : "=") + it->second : "");

    return ret;
}

/*
 * set* functions
 */


bool ModulesConf::setOption(const string module, const string option, const string value, ModuleEntry::Mode m) {
    if(option == "" || module == "" || value == "") {
	y2error("empty argument: %s, %s, %s", module.c_str(), option.c_str(), value.c_str());
	return false;
    }
    modified |= (m == ModuleEntry::SET);
    updateIndex("options",module);
    modules_conf_map["options"][module].setOption(option, value, m);
    return true;
}

bool ModulesConf::setOptions(const string module, const ModuleEntry::EntryArg arg, ModuleEntry::Mode m) {
    modified |= (m == ModuleEntry::SET);
    if(arg.empty())
        Y2_RETURN_FALSE("setOptions:Empty map");
    updateIndex("options",module);
    modules_conf_map["options"][module].setOptions(arg, m);
    return true;
}

bool ModulesConf::setArgument(const string directive, const string module, const string arg, ModuleEntry::Mode m) {
    if((directive == "alias" || directive == "options" || directive == "pre-install" || directive == "post-install" ) && (directive == "" || module == "" || arg == "")) {
	y2error("empty argument: %s, %s, %s", directive.c_str(), module.c_str(), arg.c_str());
	return false;
    }
    modified |= (m == ModuleEntry::SET);
    updateIndex(directive,module);
    modules_conf_map[directive][module].setArgument(arg, m);
    return true;
}

bool ModulesConf::setComment(const string directive, const string module, const string arg, ModuleEntry::Mode m) {
    modified |= (m == ModuleEntry::SET);
    modules_conf_map[directive][module].setComment(arg, m);
    return true;
}

/*
 *
 */

bool ModulesConf::updateIndex(const string directive, const string module) {
    if (!isModule (directive, module))
	modules_conf_index.push_back("."+directive+"."+module);
    return true;
}

bool ModulesConf::updateTimeStamp() {
    time_stamp = getTimeStamp(file_name);
    return (time_stamp != 0);
}

bool ModulesConf::updateIfModified() {
    if (time_stamp != getTimeStamp(file_name)) {
	y2warning("Config file has been changed by an external program.");
	if(!parseFile(file_name, ModuleEntry::REINIT))
	    Y2_RETURN_FALSE("updateIfModified: parseFile failed");
    }
    return true;
}

ModulesConf::TimeStamp ModulesConf::getTimeStamp(const string &fname) {
    struct stat st;
    if (stat(fname.c_str(), &st)) {
	y2error("Failed to stat %s: %s", fname.c_str(), strerror(errno));
	return 0;
    }
    return st.st_mtime;
}

/**
 * Parser
 */

/* Temporary hack, the whole parser should be reworked */

//const char *killspaces(const string s) {
string killspaces(const string s) {
  string tmp = s;
  signed ind = (signed) tmp.find_first_not_of(" ");
  if((signed)ind!=-1)
    tmp = tmp.substr(ind);
  ind = (signed) tmp.find_last_not_of(" ");
  if(ind < (signed)tmp.size()-1 && tmp.size())
    tmp = tmp.substr(0,ind+1);
  return tmp; //tmp.c_str();
}


/**
 * Parse one line
 */

bool ModulesConf::parseLine(const string &line, ModuleLine &l) const {
    string::size_type length, hash;
    string buf;
    length = line.length();
    hash = line.find_first_of("#");

    if(hash<length)
	l.comment = line.substr(hash,length) + "\n";

#define Y2_STRING(dest,source,ret) { \
  char *ss; \
  do { \
    ss = strsep(&source,WHITESPACE); \
    if(ss==NULL) { \
      if(!ret && line!="keep") \
        y2error("Parse error: %s (%s)",dest.c_str(),line.c_str()); \
      free (line_str); \
      return ret; \
    } \
  } while((dest=ss)==""); \
}

    char *line_str = strdup(line.substr(0,hash).c_str());
    char *tmp = line_str;

    Y2_STRING(l.directive,tmp,false)
    Y2_STRING(l.module,tmp,false)

    if(l.directive == "options")
        for(;;) {
            Y2_STRING(buf,tmp,true)
            if(buf=="-o") {
                Y2_STRING(buf,tmp,false)
                l.options["-o"] = buf;
            }
            else {
                string::size_type equal = buf.find_first_of("=");
                string opt = buf.substr(0,equal);
                if((signed)equal != -1) {
                    string par = buf.substr(equal+1,buf.length());
                    l.options[opt] = par;
                }
                else
                    l.options[opt] = "";
            }
        }
    else
        l.argument = tmp?tmp:"";

    l.argument = killspaces(l.argument);

#undef Y2_STRING

    free (line_str);
    return true;
}

/**
 * Parse the file ...
 */
bool ModulesConf::parseFile(const string &fname, ModuleEntry::Mode m, const bool with_comment) {
    ifstream is(fname.c_str());
    string line, temp_line;
    string comment;
    bool backslash;
    bool condition;

    while (is) {
	backslash = false;
	line = "";
	do {
	    temp_line = "";
	    getline(is,temp_line);
	    temp_line.erase(temp_line.find_last_not_of (WHITESPACE) + 1, temp_line.length ());

	    if (!temp_line.empty ())
		if (backslash = (temp_line.substr (temp_line.length () - 1) == "\\"))
		    line += temp_line.substr (0, temp_line.length () - 1);
	} while (backslash && is);

	line += temp_line;

	if(line == "") {
	    comment += "\n";
	    continue;
	}

	line.erase(0, line.find_first_not_of(WHITESPACE));
	string::size_type hash =  line.find_first_of("#");

        condition = ((line.length () > 1) ? (line.substr (0, 2) == "if") : false);

        /* process "if-else" directives as comment */
        if (condition)
        {
            comment += line + "\n";
            while (condition && is) {
                temp_line = "";
                getline(is, temp_line);
                temp_line.erase(temp_line.find_last_not_of (WHITESPACE) + 1, temp_line.length ());

                if (!temp_line.empty ())
                {
                    comment += temp_line + "\n";
                    condition = (temp_line.find ("endif"));
                }

            }
            continue;
        }

	y2debug("parseFile(): %s", line.c_str());
        if(hash!=0) {
            ModuleLine l;
            parseLine(line,l);
            y2debug("parseFile1: %s", line.c_str());
            y2debug("parseFile2: %s", l.directive.c_str());
            y2debug("parseFile3: %s", l.module.c_str());
            y2debug("parseFile4: %s", l.argument.c_str());
            comment += l.comment;
            if(l.directive == "options")
                setOptions(l.module, l.options, m);
            else
                setArgument(l.directive, l.module,l.argument, m);
            if(with_comment)
                setComment(l.directive, l.module,comment, m);
            comment = "";
        }
        else
            comment += line + "\n";
    }

                                // keep the final comment
    if (comment.length () && with_comment)
    {
        setArgument(FINAL_COMMENT, "", "", m);
        setComment(FINAL_COMMENT, "", comment, m);
    }

    return updateTimeStamp();
}


/**
 * remove one entry ...
 */
bool ModulesConf::removeEntry(const string directive, const string module) {
    modified = true;
    if(!isModule(directive,module)) {
        y2warning("removeEntry: no such directive or module (%s,%s)",directive.c_str(),module.c_str());
	return false;
    }
    if(modules_conf_map[directive].erase(module) < 1)
        Y2_RETURN_FALSE("removeEntry: erase failed (%s,%s)",directive.c_str(),module.c_str());
    if(modules_conf_map[directive].empty())
        if(modules_conf_map.erase(directive) < 1)
            Y2_RETURN_FALSE("removeEntry: erase failed (%s)",directive.c_str());
    modules_conf_index.remove("."+directive+"."+module);
    return true;
}

/**
 * write the file ...
 */
bool ModulesConf::writeFile(const string fname) {
    updateIfModified();

    if (modified) {
	string dest_name;
	int dr;

        string temp_name = ((fname != "") ? fname : file_name) + ".YaST2.tmp";

	ofstream of(temp_name.c_str ());

	if (!of.good())
	    Y2_RETURN_FALSE("Unable to write '%s'.", temp_name.c_str ());

	ModulesConfIndex::iterator it = modules_conf_index.begin ();

	string localinclude = "";
	string dir, mod, arg, com;
	for (; it != modules_conf_index.end (); ++it)
        {

	    dir = it->substr (1, it->find_first_of (".", 1) - 1);
	    mod = it->substr (it->find_first_of (".", 1) + 1, it->length ());

	    if (isModule (dir, mod))
            {
		/* options directiove */
		if (dir == "options")
		    of << getComment(dir,mod) << dir + " " + mod + getOptionsAsString(mod) << endl;
		/* no arguments */
		else if(dir == "keep")
		    of << getComment(dir,mod) << dir << endl;
		/* comment at EOF */
		else if(dir == FINAL_COMMENT)
		    of << getComment(dir,mod); // << endl;
		/* postpone directive */
		else if(dir == "include" && (mod == "/etc/modules.conf.local" || mod == "/etc/modprobe.conf.local"))
		    localinclude = getComment(dir,mod) + dir + " " + mod + " " + getArgument(dir,mod) + "\n";
		/* normal directive */
		else
		    of << getComment(dir,mod) << dir + " " + mod + " " + getArgument(dir,mod) << endl;

                if (of.fail ())
		{
		    of.close ();
		    Y2_RETURN_FALSE ("Unable to write '%s'.", temp_name.c_str ());
                }

		modules_conf_map[dir][mod].Set(ModuleEntry::INIT);

            }
	    else
            {
		y2error("Wrong path in index '%s'.", it->c_str ());
            }
        }

	if(localinclude != "") {
	    y2debug("Include last: %s", localinclude.c_str());
	    of << localinclude;
	}

	if (fname == "")
	    dest_name = file_name;
	else
	    dest_name = fname;

	string backup_name = dest_name + BACKUP_EXTENSION;

	if (rename (dest_name.c_str (), backup_name.c_str ()) < 0)
	    y2warning ("Error while creating backup file in writeFile (): %s", strerror(errno));

	if (rename (temp_name.c_str (), dest_name.c_str()) < 0)
        {
            rename (backup_name.c_str (), dest_name.c_str ());
	    Y2_RETURN_FALSE ("Error while moving file in writeFile (): %s", strerror(errno));
        }

	dr = system ("/sbin/depmod -a -F /boot/System.map-`uname -r` `uname -r` 2> /dev/null");
	if (dr < 0 || dr == 127)
	    Y2_RETURN_FALSE ("Error while calling depmod in writeFile ()");

	modified = false;
    }
    else
	y2milestone("Modules not modified, not writing");

    return updateTimeStamp ();
}

/* EOF */
