/* ModulesConf.h -*- c++ -*-
 *
 * Classes for reading the modules.conf configuration file.
 *
 * Author: Michal Svec <msvec@suse.cz>
 *         Daniel Vesely <dan@suse.cz>
 *
 * $Id$
 */

#ifndef ModulesConf_h
#define ModulesConf_h

#include <string>
#include <list>
#include <map>

using std::string;
using std::list;
using std::map;

#define MAX_LINE_LENGTH	256
#define WHITESPACE	" \t\n"

#define MAGIC_ENTRY	"Ctrl and Alt keys stuck -- press Del to continue."

/**
 * This class contains one entry from the modules.conf file.
 *
 * @short One entry in the modules.conf file.
 * @author Michal Svec <msvec@suse.cz>
 * @author Dan Vesely <dan@suse.cz>
 * @version $Id$
 * @see ModulesConf
 *
 */
class ModuleEntry {

public:

    enum Mode { INIT, SET, REINIT };
    typedef map <const string, string> EntryArg;
    typedef string EntryCom;

	/**
	 * Default constructor.
	 */
    ModuleEntry() : comment(), argument(), dirtyflag(false) {}

	/**
	 * Destructor.
	 */
    ~ModuleEntry();

	/**
	 * Return an entry comment.
	 * @return an entry comment
	 */
    EntryCom getComment() const;
	/**
	 * Return an entry argument.
	 * @return an entry argument
	 */
    EntryArg getArgument() const { return argument; }
	/**
	 * Set an entry comment.
	 * @param com The entry comment
	 * @param m Indicates if the option is set during (re)initialization or by an agent.
	 * @return if the operation was successful
	 */
    bool setComment(const EntryCom &com, Mode m) { comment = com; return true; }
	/**
	 * Set an entry argument.
	 * @param arg The entry argument
	 * @param m Indicates if the option is set during (re)initialization or by an agent.
	 * @return if the operation was successful
	 */
    bool setArgument(const string arg, Mode m);
	/**
	 * Set an entry option.
	 * @param option The entry option name.
	 * @param value The entry option value.
	 * @param m Indicates if the option is set during (re)initialization or by an agent.
	 * @return if the operation was successful
	 */
    bool setOption(const string option, const string value, Mode m);
	/**
	 * Set entry options.
	 * @param arg The entry options.
	 * @param m Indicates if the option is set during (re)initialization or by an agent.
	 * @return if the operation was successful
	 */
    bool setOptions(const EntryArg &arg, Mode m);

	/**
	 * Sets dirty_flag. This flag stands for indicating if the entry
	 * should be reinitialized if it was modified
	 * externally in the file.
	 * @param m Indicates if the operation is provided during
	 * (re)initialization or is done by the agent.
	 * @return true if the entry can be changed.
	 */
    bool Set(Mode m);

private:
    EntryCom comment;
    EntryArg argument;
    bool dirtyflag;

};

/**
 * This class contains the modules.conf file.
 *
 * @short The modules.conf file
 * @author Michal Svec <msvec@suse.cz>
 * @author Dan Vesely <dan@suse.cz>
 * @version $Id$
 * @see ModuleEntry
 *
 */
class ModulesConf {

public:
    typedef list<string> ModulesConfIndex;
    typedef map<const string, ModuleEntry> ModuleEntryMap;
    typedef map<const string, ModuleEntryMap> ModulesConfMap;

	/**
	 * Default constructor.
	 * @param fname A path to the modules.conf file (usually /etc/modules.conf)
	 */
    ModulesConf(const string &fname);
	/**
	 * Destructor
	 */
    ~ModulesConf();

	/**
	 * Returns map of all directives present in the current modules.conf.
	 * @return a map of all directives
	 */
    ModulesConfMap getDirectives();
	/**
	 * For the given directive returns all modules in a map.
	 * @param directive A directive for which the modules are returned.
	 * @return a map of all modules for the given directive
	 */
    ModuleEntryMap getModules(const string directive);
	/**
	 * For the given module returns all options in a map.
	 * @param module A module for which the options are returned.
	 * @return a map of all options for the given module
	 */
    ModuleEntry::EntryArg getOptions(const string module);
	/**
	 * For the given module returns all options as one string.
	 * @param module A module for which the options are returned.
	 * @return all options for the given module in one string
	 */
    string getOptionsAsString (const string module);
	/**
	 * For the given module and option returns the option parameter.
	 * @param module A module for which the parameter is returned.
	 * @param option An option for which the parameter is returned.
	 * @return an option parameter
	 */
    string getOption(const string module, const string option);
	/**
	 * For the given directive and module returns the entry argument.
	 * @param directive A directive for which the argument is returned.
	 * @param module A module for which the argument is returned.
	 * @return an entry argument
	 */
    string getArgument(const string directive, const string module);
	/**
	 * For the given directive and module returns the entry comment.
	 * @param directive A directive for which the comment is returned.
	 * @param module A module for which the comment is returned.
	 * @return an entry comment
	 */
    string getComment(const string directive, const string module);

	/**
	 * For the given module and options set the option parameter.
	 * @param module A module for which the parameter is set.
	 * @param option An option for which the parameter is set.
	 * @param value The option value.
	 * @return if the operation was successful
	 */
    bool setOption(const string module, const string option, const string value, ModuleEntry::Mode m);
	/**
	 * For the given module set all options as a map.
	 * @param module A module for which the options are set.
	 * @param value The options as a map.
	 * @param m Indicates if the option is set during (re)initialization or by an agent.
	 * @return if the operation was successful
	 */
    bool setOptions(const string module, const ModuleEntry::EntryArg arg, ModuleEntry::Mode m);
	/**
	 * For the given directive and module set the entry argument.
	 * @param directive A directive for which the argument is set.
	 * @param module A module for which the argument is set.
	 * @param m Indicates if the option is set during (re)initialization or by an agent.
         * @param arg An entry argument.
	 * @return if the operation was successful
	 */
    bool setArgument(const string directive, const string module, const string arg, ModuleEntry::Mode m);
	/**
	 * For the given directive and module set the entry comment.
	 * @param directive A directive for which the comment is set.
	 * @param module A module for which the comment is set.
	 * @param m Indicates if the option is set during (re)initialization or by an agent.
         * @param arg An entry comment.
	 * @return if the operation was successful
	 */
    bool setComment(const string directive, const string module, const string arg, ModuleEntry::Mode m);

	/**
	 * Remove one entry from the modules.conf.
	 * @param directive A removed entry directive.
	 * @param module A removed entry module.
	 * @param m Indicates if the option is set during (re)initialization or by an agent.
	 * @return if the operation was successful
	 */
    bool removeEntry(const string directive, const string module);

	/**
	 * Write the modules.conf file to the disk.
	 * @param fname A file name of the written file.
	 * @return if the operation was successful
	 */
    bool writeFile(const string fname = "");

private:
    string file_name;
    ModulesConfMap modules_conf_map;
    ModulesConfIndex modules_conf_index;

    bool modified;		// was the stucture modified from ycp

    struct ModuleLine {
        string directive;
        string module;
        string argument;
        ModuleEntry::EntryArg options;
        string comment;
    };

	/**
	 * The return type of getTimeStamp()
	 */
    typedef time_t TimeStamp;
	/**
	 * Return a time stamp of the file.
	 * @param fname A file name of the file to be stamped.
	 * @return the file time stamp
	 */
    TimeStamp getTimeStamp(const string &fname);

    TimeStamp time_stamp;

	/**
	 * Test the existence of directive.
	 * @param directive The tested directive.
	 * @return if the directive exists
	 */
    bool isDirective(const string directive) const;
	/**
	 * Test the existence of module.
	 * @param directive The directive of the tested module.
	 * @param module The tested module.
	 * @return if the module exists
	 */
    bool isModule(const string directive, const string module);
	/**
	 * Test the existence of option.
	 * @param directive The directive of the tested option.
	 * @param module The module of the tested option.
	 * @return if the option exists
	 */
    bool isOption(const string module, const string option);

	/**
	 * Update internal structures if the file has been modified.
	 * @return if file has been modified
	 */
    bool updateIfModified();
	/**
	 * Update the file time stamp.
	 * @return if the operation was successful
	 */
    bool updateTimeStamp();
	/**
	 * Update the file index.
	 * @param directive A directive for which the index is updated.
	 * @param module A module for which the index is updated.
	 * @return if the operation was successful
	 */
    bool updateIndex(const string directive, const string module);
	/**
	 * Parse one line of modules.conf.
	 * @param line The line to be parsed.
	 * @param l The parsed line.
	 * @return if the operation was successful
	 */
    bool parseLine(const string &line, ModuleLine &l) const;
	/**
	 * Parse the modules.conf.
	 * @param file_name The name of the parsed file.
	 * @param m Indicates if the option is set during (re)initialization or by an agent.
	 * @param with_comment If the comment should be parsed.
	 * @return if the operation was successful
	 */
    bool parseFile(const string &file_name, ModuleEntry::Mode m, const bool with_comment = true);

};


#endif /* ModulesConf_h */
