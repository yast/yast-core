/*
 * SystemAgent.cc
 *
 * An agent for handling commands on the system
 *
 * Authors: Klaus Kaempf <kkaempf@suse.de>
 *          Michal Svec <msvec@suse.cz>
 *          Petr Blahos <pblahos@suse.cz>
 *
 * $Id$
 */

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <resolv.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <linux/lp.h>
#include <string>

#include <config.h>
#include <YCP.h>
#include <y2/pathsearch.h>
#include <ycp/YCPParser.h>
#include <ycp/y2log.h>

#include "SystemAgent.h"
#include "ShellCommand.h"


/**
 * Filter . and ..
 * @return 0 if . or .. are file names
 */
static int return_one (const struct dirent *entry)
{
    if ('.' == entry->d_name[0])
    {
	if ('\0' == entry->d_name[1])
	    return 0;
	if ('.' == entry->d_name[1] && '\0' == entry->d_name[2])
	    return 0;
    }
    return 1;
}


/**
 * remove directory and all its subfiles and subdirectories.
 * Depth is maximal depth it goes to.
 * @param path path to remove
 * @param depth max. depth
 */
static const void
remove_directory (const string& path, int depth)
{
    struct stat buf;
    if (!depth)     // check for too many nested directories
	return;
    if (lstat (path.c_str(), &buf)) // we do not want to follow symlinks
    {
	y2error ("Can't stat %s: %s", path.c_str(), strerror(errno));
	return;
    }
    if (S_ISDIR (buf.st_mode))
    { // directory, process all files
	string filename;
	struct dirent **eps;
	int len;

	len = scandir (path.c_str(), &eps, return_one, alphasort);
	if (len >= 0)
	{
	    int i;
	    for (i = 0; i < len; i++)
	    {
		filename = path + '/' + eps[i]->d_name;
		remove_directory (filename, depth-1);
	    }
	}
	else
	{
	    y2error ("Can't scandir %s: %s", path.c_str(), strerror(errno));
	}
    }
    // remove file
    y2debug ("Removing temporary file %s", path.c_str());
    remove (path.c_str());
}


//========================================================================


/**
 * Constructor
 */
SystemAgent::SystemAgent ()
{
    char tmp1[25];
    snprintf (tmp1, 25, "/tmp/YaST2-%05d-XXXXXX", getpid ());

    const char* tmp2 = mkdtemp (tmp1);
    if (!tmp2)
    {
	y2error ("Can't create tmp directory: %m");
	exit (EXIT_FAILURE);
    }

    tempdir = tmp2;
    y2debug ("tmp directory is %s", tempdir.c_str ());
}


/**
 * Destructor
 */
SystemAgent::~SystemAgent ()
{
    // remove temp directory and all its subdirectories.
    remove_directory (tempdir.c_str(), 20);
}


/**
 * Read file to string
 */
static int read_file_to_string (const char* filename, string& output)
{
    int fd = open (filename, O_RDONLY);
    output = "";
    if (fd < 0)
    {
	return errno;
    }

    while (true)
    {
	char buffer[4096];
	ssize_t bytes_read = read (fd, buffer, 4095);
	if (bytes_read <= 0)
	    break;
	buffer[bytes_read] = 0; // 0 terminate string
	output += buffer;
    }
    close(fd);
    return 0;
}


/**
 *  Fills a ycp map with informations of a stat structure.
 */
static YCPMap
stat2map (const struct stat& sb)
{
    YCPMap result;

    result->add (YCPString ("inode"), YCPInteger (sb.st_ino));

    result->add (YCPString ("isreg"), YCPBoolean (S_ISREG (sb.st_mode)));
    result->add (YCPString ("isdir"), YCPBoolean (S_ISDIR (sb.st_mode)));
    result->add (YCPString ("ischr"), YCPBoolean (S_ISCHR (sb.st_mode)));
    result->add (YCPString ("isblock"), YCPBoolean (S_ISBLK (sb.st_mode)));
    result->add (YCPString ("isfifo"), YCPBoolean (S_ISFIFO (sb.st_mode)));
    result->add (YCPString ("islink"), YCPBoolean (S_ISLNK (sb.st_mode)));
    result->add (YCPString ("issock"), YCPBoolean (S_ISSOCK (sb.st_mode)));

    result->add (YCPString ("nlink"), YCPInteger (sb.st_nlink));

    result->add (YCPString ("uid"), YCPInteger (sb.st_uid));
    result->add (YCPString ("gid"), YCPInteger (sb.st_gid));

    result->add (YCPString ("size"), YCPInteger (sb.st_size));

    result->add (YCPString ("atime"), YCPInteger (sb.st_atime));
    result->add (YCPString ("mtime"), YCPInteger (sb.st_mtime));
    result->add (YCPString ("ctime"), YCPInteger (sb.st_ctime));

    return result;
}


/**
 * Run shell command and returns its output.
 */
static YCPMap
shellcommand_output (const string& script, const string& tempdir)
{
    int ret = shellcommand (script, tempdir);

    string output_stdout;
    int ret1 = read_file_to_string (string (tempdir + "/stdout").c_str (), output_stdout);
    if (ret1 != 0)
    {
	y2error ("open for %s failed: %s/stdout", tempdir.c_str (), strerror (ret1));
    }

    string output_stderr;
    int ret2 = read_file_to_string (string (tempdir + "/stderr").c_str (), output_stderr);
    if (ret2 != 0)
    {
	y2error ("open for %s failed: %s/stderr", tempdir.c_str (), strerror (ret2));
    }

    YCPMap result;
    result->add (YCPString ("exit"), YCPInteger (ret));
    result->add (YCPString ("stdout"), YCPString (output_stdout));
    result->add (YCPString ("stderr"), YCPString (output_stderr));

    return result;
}


/**
 * indent output by level
 */
string
indent_output (int level)
{
    string ret = "";
    while (level-- > 0)
	ret += "  ";
    return ret;
}

/**
 * recursively dump value to file
 */
string
dump_value (int level, const YCPValue& value)
{
    string ret = "";

    if (value.isNull())
	return "";

    y2debug ( "%s\n", value->toString ().c_str ());

    switch (value->valuetype()) {
	case YT_LIST:
	    {
	    YCPList list = value->asList ();
	    ret += "[";
	    for (int i = 0; i < list->size (); i++) {
		ret += "\n";
		ret += indent_output (level+1);
		ret += dump_value (level+1, list->value (i));
		if ( i != list->size()-1)
		    ret += ",";
	    }
	    ret += "\n";
	    ret += indent_output (level);
	    ret += "]";
	    }
	    break;
	case YT_MAP:
	    {
	    YCPMap map = value->asMap ();
	    ret += "$[";
	    for (YCPMapIterator i = map->begin (); i != map->end (); i++) {
		if ( i != map->begin () )
		    ret += ",";
		ret += "\n";
		ret += indent_output (level+1);
		ret += dump_value (level+1, i.key ());
		ret += " : ";
		ret += dump_value (level+1, i.value ());
	    }
	    ret += "\n";
	    ret += indent_output (level);
	    ret += "]";
	    }
	    break;
	default:
	    ret = value->toString ().c_str ();
	    break;
    }

    return ret;
}

/**
 * Read function
 */
YCPValue
SystemAgent::Read (const YCPPath& path, const YCPValue& arg)
{
    y2debug ("Read (%s)", path->toString().c_str());

    if (path->isRoot())
    {
	return YCPError ("Read () called without sub-path");
    }

    const string cmd = path->component_str (0); // just a shortcut

    /**
     * @builtin intro
     * Paths described here have to be prefixed by .target
     * (e.g. <tt>SCR::Read (.target.tmpdir)</tt>). Only as an
     * exception during the installation they can be prefixed by
     * .local (e.g. <tt>WFM::Read (.local.tmpdir)</tt>).
     */

    if (cmd == "tmpdir")
    {
	/**
	 * @builtin Read (.target.tmpdir) -> string
	 * Returns the (instance specific) directory for storing temporary
	 * files. The directory (and its contents) will be removed by the
	 * SystemAgent destructor (usually when yast2 exits)
	 *
	 * @example Read (.target.tmpdir) -> "/some/temp/dir"
	 */

	return YCPString (tempdir);
    }

    if (arg.isNull())
    {
	return YCPError ("Filename arg for Read is nil");
    }

    YCPValue default_value = YCPNull();

    string filename;

    if (arg->isString())
    {
	filename = arg->asString()->value();
    }
    else if (arg->isList()
	     && (arg->asList()->size() == 2)
	     && (arg->asList()->value(0)->isString()))
    {
	default_value = arg->asList()->value(1);
	filename = arg->asList()->value(0)->asString()->value();
    }
    else
    {
	y2error ("Read (%s, %s) failed !", cmd.c_str(), arg->toString().c_str());
	return YCPError ("Bad filename arg for Read (...)");
    }

    if (cmd == "string")
    {
	/**
	 * @builtin Read (.target.string, string filename) -> string
	 * Opens an Ascii file and reads the contents to a single
	 * string. Newlines are preserved.
	 *
	 * @example Read (.target.string, "/some/file") -> "a contents"
	 */

	string output;
	int ret = read_file_to_string (filename.c_str (), output);
	if (ret == 0)
	{
	    return YCPString (output);
	}
	else if (!default_value.isNull())
	{
	    return default_value;
	}
	else
	{
	    return YCPError (string ("Read (.string, \"")
			     + filename
			     + "\") failed: "
			     + strerror (ret));
	}
    }

    else if (cmd == "byte")
    {
	/**
	 * @builtin Read (.target.byte, string filename) -> byteblock
	 * Opens a binary file and reads its contents into a single byteblock.
	 */

	int fd = open (filename.c_str (), O_RDONLY);
	if (fd == -1)
	{
	    if (!default_value.isNull())
	    {
		return default_value;
	    }
	    else
	    {
		return YCPError (string ("Read (.byte, \"") + filename +
			     "\") failed: " + strerror (errno));
	    }
	}

	// determine filesize
	struct stat sb;
	fstat (fd, &sb);
	size_t filesize = sb.st_size;

	// don't try to allocate this on the stack
	unsigned char *buffer = (unsigned char *) malloc (filesize);

	if (!buffer)
	{
	    close (fd);
	    return YCPError (string ("Read (.byte, \"") + filename +
			     "\") failed: " + strerror (errno));
	}

	size_t read_bytes = read (fd, buffer, filesize);
	if (read_bytes != filesize)
	{
	    free (buffer);
	    close (fd);
	    return YCPError (string ("Read (.byte, \"") +
			     filename + "\") failed: " +
			     strerror (errno));
	}

	YCPByteblock ret = YCPByteblock (buffer, filesize);
	free (buffer);
	close (fd);
	return ret;
    }

    else if (cmd == "ycp" || cmd == "yast2")
    {
	/**
	 * @builtin Read (.target.ycp, string filename) -> any
	 * @builtin Read (.target.ycp, string filename, [any default = nil]) -> any
	 * Opens a file that must be in YCP syntax and contain exactly
	 * one value, parses that file and returns the parsed value.
	 * Returns 'default', if the file didn't exist, was not readable or
	 * didn't not contain a valid YCP value.
	 * A warning in the log is omitted if a default value is given.
	 */

	/**
	 * @builtin Read (.target.yast2, string filename) -> any
	 * @builtin Read (.target.yast2, string filename, [any default = nil]) -> any
	 * Opens a file that must be in YCP syntax and contain exactly
	 * one value, parses that file and returns the parsed value.
	 * Returns default, if the file didn't exist, was not readable or
	 * did not contain a valid YCP value.
	 *
	 * The purpose of this function is to load data located in
	 * ydatadir. The data may also be located in any of the paths in
	 * source/core/liby2/src/pathsearch.cc appended by "data/".
	 * The filename must be relative to one of those paths.
	 *
	 * A warning in the log is omitted if a default value is given.
	 */

	if (cmd == "yast2")
	{
	    string tmp = Y2PathSearch::findy2 ("data/" + filename); // FIXME use ydatadir
	    if (!tmp.empty ())
	    {
		filename = tmp;
	    }
	    else
	    {
		if (!default_value.isNull ())
		{
		    return default_value;
		}
		else
		{
		    return YCPError ("can't find '" + filename + "'");
		}
	    }
	}

	int fd = open (filename.c_str (), O_RDONLY);
	if (fd < 0)
	{
	    if (!default_value.isNull())
	    {
		return default_value;
	    }
	    else
	    {
		return YCPError ("Open file '" + filename + "' failed: " + strerror (errno));
	    }
	}
	YCPParser parser (fd, filename.c_str ());
	parser.setBuffered(); // Read from file. Buffering is always possible here
	YCPValue contents = parser.parse();
	close(fd);
	return !contents.isNull() ? contents : YCPVoid();
    }

    else if (cmd == "dir")
    {
	/**
	 * @builtin Read (.target.dir, string path) -> list
	 * @builtin Read (.target.dir, [string path, list default]) -> list
	 * Reads a directory. Returns a list of strings, one string for
	 * each file contained in the directory path is pointing to.
	 * The entries '.' and '..' are NOT returned. Returns nil and
	 * doesn't log an error, if path does not point to a readable
	 * directory.
	 * If a default value is given, this is returned if path isn't
	 * accessible.
	 *
	 * @example Read (.target.dir, "/proc/self") -> [ "cmdline", "cwd", "environ", ... ]
	 */

	DIR *dir = opendir (filename.c_str());
	if (!dir)
	{
	    if (!default_value.isNull())
	    {
		return default_value;
	    }
	    return YCPError ("Can't access directory '" + filename + "': " + strerror (errno));
	}

	YCPList dirlist;
	struct dirent *entry;
	while ((entry = readdir (dir)))
	{
	    if (!strcmp(entry->d_name, ".") || !(strcmp(entry->d_name, "..")))
		continue;
	    dirlist->add (YCPString (entry->d_name));
	}
	closedir (dir);
	return dirlist;
    }

    else if (cmd == "size")
    {
	/**
	 * @builtin Read (.target.size, string filename) -> integer
	 * return current size of file
	 * returns -1 if the file does not exist
	 */

	struct stat sb;
	long long retval = stat (filename.c_str (), &sb);
	if (retval == 0)
	{
	    retval = sb.st_size;
	}
	return YCPInteger (retval);
    }

    else if (cmd == "stat")
    {
	/**
	 * @builtin Read (.target.stat, string filename) -> map
	 * Return a map with file information (see stat(2)). If
	 * the file does not exist return an empty map.
	 */

	YCPMap result;

	struct stat sb;
	if (stat (filename.c_str (), &sb) == 0)
	{
	    result = stat2map (sb);
	}

	return result;
    }

    else if (cmd == "lstat")
    {
	/**
	 * @builtin Read (.target.lstat, string filename) -> map
	 * Return a map with file information (see stat(2)). If
	 * the file does not exist return an empty map.
	 */

	YCPMap result;

	struct stat sb;
	if (lstat (filename.c_str (), &sb) == 0)
	{
	    result = stat2map (sb);
	}

	return result;
    }

    else if (cmd == "symlink")
    {
	/**
	 * @builtin Read (.target.symlink, string filename) -> string
	 *
	 * Returns the content of the symbolic link filename. If the filename
	 * does not exist or is no symbolic link, nil is returned and an error
	 * logged.
	 *
	 * @example Read (.target.symlink, "/var/X11R6/bin/X")
	 */

	const int size = 1024;
	char buffer[size];
	int ret = readlink (filename.c_str (), buffer, size - 1);

	if (ret == -1)
	{
	    return YCPError (string ("failed to read symlink ") + filename +
			     ": " + strerror (errno));
	}

	buffer[ret] = '\0';
	return YCPString (buffer);
    }

    return YCPError (string("Undefined subpath for Read (") + path->toString() + ")");
}


/**
 * Write function
 */
YCPValue
SystemAgent::Write (const YCPPath& path, const YCPValue& value,
		    const YCPValue& arg)
{
    y2debug ("Write (%s)", path->toString().c_str());

    if (path->isRoot())
    {
	return YCPError ("Write () called without sub-path", YCPBoolean (false));
    }

    const string cmd = path->component_str (0); // just a shortcut

    if (cmd == "passwd")
    {
	/**
	 * @builtin Write (.target.passwd.&lt;name&gt;, string cryptval) -> bool
	 * .passwd can be used to set or modify the encrypted password
	 * of an already existing user in /etc/passwd and /etc/shadow.
	 *
	 * This call returns true on success and false, if it fails.
	 *
	 * @example Write (.target.passwd.root, crypt (a_passwd))
	 */

	if (path->length() != 2)
	{
	    return YCPError ("Bad path argument in call to Write (.passwd.name)",
			     YCPBoolean (false));
	}

	if (value.isNull() || !value->isString())
	{
	    return YCPError ("Bad password argument in call to Write (.passwd)",
			     YCPBoolean (false));
	}

	string passwd = value->asString()->value();

	string bashcommand =
	    string ("/bin/echo '") +
	    path->component_str (1).c_str () + ":" + passwd +
	    "' |/usr/sbin/chpasswd -e >/dev/null 2>&1";

	// Don't write the password into the log - even though it's crypted
	// y2debug("Executing: '%s'", bashcommand.c_str());

	int exitcode = system(bashcommand.c_str());

	return YCPBoolean (WIFEXITED (exitcode) && WEXITSTATUS (exitcode) == 0);
    }

    else if (cmd == "string")
    {
	/**
	 * @builtin Write (.target.string, string filename, string value) -> boolean
	 * Writes the string <tt>value</tt> into a file. If the file already
	 * exists, the existing file is overwritten. The return value is
	 * true, if the file has been written successfully.
	 */

	if (value.isNull() || !value->isString())
	{
	    return YCPError ("Bad filename arg for Write (.string ...)", YCPBoolean (false));
	}

	if (arg.isNull() || !arg->isString())
	{
	    return YCPError ("Bad string value for Write (.string ...)", YCPBoolean (false));
	}

	string filename = value->asString()->value();
	int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd >= 0)
	{
	    string cont = arg->asString()->value();
	    const char *buffer = cont.c_str();
	    size_t length = cont.length();
	    size_t written = write(fd, buffer, length);
	    close(fd);
	    return YCPBoolean (written == length);
	}
	return YCPError (string ("Write (.string, \"")
			 + filename
			 + "\") failed: "
			 + strerror (errno), YCPBoolean(false));
    }

    else if (cmd == "byte")
    {
	/**
	 * @builtin Write (.target.byte, string filename, byteblock) -> boolean
	 * Write a byteblock into a file.
	 */

	if (value.isNull () || !value->isString ())
	{
	    return YCPError ("Bad filename arg for Write (.byte, ...)",
			     YCPBoolean (false));
	}

	if (arg.isNull () || !arg->isByteblock ())
	{
	    return YCPError ("Bad value for Write (.byte, filename, byteblock)",
			     YCPBoolean (false));
	}

	string filename = value->asString ()->value ();
	YCPByteblock byteblock = arg->asByteblock ();

	int fd = open (filename.c_str (), O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd >= 0)
	{
	    size_t size = byteblock->size ();
	    size_t write_size = write (fd, byteblock->value (), size);
	    close (fd);
	    return YCPBoolean (write_size == size);
	}

	return YCPError (string ("Write (.byte, \"") + filename + "\") failed: "
			 + strerror (errno), YCPBoolean (false));
    }

    else if (cmd == "ycp" || cmd == "yast2")
    {
	/**
	 * @builtin Write (.target.ycp, string filename, any value) -> boolean
	 * Opens a file for writing and prints the value <tt>value</tt> in
	 * YCP syntax to that file. Returns true, if the file has
	 * been written, false otherwise. The newly created file gets
	 * the mode 0644 minus umask. Furthermore any missing directory in the
	 * pathname <tt>filename</tt> is created automatically.
	 */

	/**
	 * @builtin Write (.target.ycp, [ string filename, integer mode], any value) -> boolean
	 * Opens a file for writing and prints the value <tt>value</tt> in
	 * YCP syntax to that file. Returns true, if the file has
	 * been written, false otherwise. The newly created file gets
	 * the mode mode minus umask. Furthermore any missing directory in the
	 * pathname <tt>filename</tt> is created automatically.
	 */

	// either string or list

	if (value.isNull() || !(value->isString() || value->isList()))
	{
	    return YCPError ("Bad arguments to Write (" + cmd + ", string filename ...)",
			     YCPBoolean (false));
	}

	string filename;
	mode_t filemode = 0644;

	if (value->isString())
	{
	    filename = value->asString()->value();
	}
	else
	{			// value is list
	    YCPList flist = value->asList();
	    if ((flist->size() != 2)
		|| (!flist->value(0)->isString())
		|| (!flist->value(1)->isInteger()))
	    {
		return YCPError ("Bad [filename, mode] list in call to Write (" +
				 cmd + ", [ string filename, integer mode ], ...)",
				 YCPBoolean (false));
	    }
	    filename = flist->value(0)->asString()->value();
	    filemode = (int)(flist->value(1)->asInteger()->value());
	}

	if (filename.length() == 0)
	{
	    return YCPError ("Invalid empty filename in Write (" + cmd + ", ...)",
			     YCPBoolean (false));
	}

	// Create directory, if missing
	size_t pos = 0;
	while (pos = filename.find('/', pos + 1), pos != string::npos)
	    mkdir (filename.substr(0, pos).c_str(), 0775);

	// Remove file, if existing
	remove (filename.c_str());

	int fd = open (filename.c_str(), O_WRONLY | O_CREAT |  O_TRUNC , filemode);
	bool success = false;
	if (fd < 0)
	{
	    return YCPError ("Error opening '" + filename + "': " + strerror (errno),
			     YCPBoolean (false));
	}

	// string contents = (arg.isNull() ? "" : arg->toString());
	string contents = (arg.isNull() ? "" : dump_value(0, arg));
	ssize_t size = contents.length();
	if (size == write(fd, contents.c_str(), size)
	    && write(fd, "\n", 1) == 1)
	    success = true;
	close(fd);

	return YCPBoolean(success);
    }

    return YCPError (string("Undefined subpath for Write (") + path->toString() + ")",
		     YCPBoolean (false));
}


/**
 * Execute functions
 */
YCPValue
SystemAgent::Execute (const YCPPath& path, const YCPValue& value,
		      const YCPValue& arg)
{
    y2debug ("Execute (%s)", path->toString().c_str());

    if (path->isRoot ())
    {
	return YCPError ("Execute () called without sub-path");
    }

    if (value.isNull ())
    {
	return YCPError (string("Execute (")+path->toString()+") without argument.");
    }

    const string cmd = path->component_str (0); // just a shortcut

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // .bash*

    if (cmd == "bash"
	|| cmd == "bash_output"
	|| cmd == "bash_background")
    {

	YCPValue environment = YCPVoid();

	if (!arg.isNull())
	    environment = arg;

	/**
	 * @builtin Execute (.target.bash, string command, map environment) -> integer
	 * @builtin Execute (.target.bash_background, string command, map environment) -> integer
	 * @builtin Execute (.target.bash_output, string command, map environment) -> map
	 *
	 * Runs a bash command. The command is stated as string.
	 * The map variables can be used to give initial environment
	 * definitions to the target. The keys have to be strings,
	 * the values can be of any type. If you use string values,
	 * the strings may _not_ contain single quotes. Escape them
	 * with double backqoute, if you need them. This is subject
	 * to change.
	 *
	 * The return value will be either an integer with the exitcode of
	 * the shell script or a map:<pre>
	 * $[
	 * <dd> "exit" : &lt;integer&gt;,  //exitcode from shell script
	 * <dd> "stdout" : &lt;string&gt;, //stdout of the command
	 * <dd> "stderr" : &lt;string&gt;  //stderr of the command
	 * ]</pre>
	 *
	 * @example Execute (.target.bash, "/bin/touch $FILE ; exit 5", $["FILE":"/somedir/somefile"]) -> 5
	 * @example Execute (.target.bash_output, "/bin/touch $FILE ; exit 5", $["FILE":"/somedir/somefile"]) -> $[ "exit" : 5, "stdout" : "", "stderr" : ""]
	 *
	 */

	if (value.isNull() || !value->isString())
	{
	    return YCPError ("Bad command argument to Execute (.bash, string command [, map env])");
	}

	/* shell command must have rooted path */
	string bashcommand = value->asString()->value();
#if 0
	if (bashcommand[0] != '/')
	{
	    ycp2warning ("", 0, "Execute (.bash, ...) without full path !");
	}
#endif

	/* check for and construct shell enviroment */
	YCPMap variables;
	if (environment->isMap())
	{
	    variables = environment->asMap();
	}

	string exports = "";
	for (YCPMapIterator pos = variables->begin(); pos != variables->end(); ++pos)
	{
	    YCPValue key   = pos.key();
	    YCPValue value = pos.value();
	    if (!key->isString())
	    {
		return YCPError (string("Invalid value '")
				 + key->toString()
				 + "' for target variable name, which must be a string");

	    }
	    exports += "export " + key->asString()->value() + "='";
	    string valstr;
	    if (value->isString())
	    {
		valstr = value->asString()->value();
	    }
	    else
	    {
		valstr = value->toString();
	    }
	    exports += valstr + "'\n";
	}

	/* execute script and return YCP{Integer|Map} */
	if (cmd == "bash")
	{
	    return YCPInteger (shellcommand (exports + bashcommand));
	}
	else if (cmd == "bash_output")
	{
	    return shellcommand_output (exports + bashcommand, tempdir);
	}
	else if (cmd == "bash_background")
	{
	    return YCPInteger (shellcommand_background (exports + bashcommand));
	}
    } // .bash*

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    else if (cmd == "bash_input")
    {
	/**
	 * @builtin Execute (.target.bash_input, string command, string input) -> integer
	 *
	 * Note: Function has only one used within YaST2 and is subject to
	 * sudden change or removal.
	 */

	if (value.isNull() || !value->isString())
	{
	    return YCPError ("Bad command argument to Execute (.bash_input, string "
			     "command, string stdin");
	}

	if (arg.isNull() || !arg->isString())
	{
	    return YCPError ("Bad command argument to Execute (.bash_input, string "
			     "command, string stdin");
	}

	string command = value->asString ()->value ();
	command += ">/dev/null 2>&1";

	string input = arg->asString ()->value ();
	input += "\n";

	FILE* p = popen (command.c_str (), "w");
	if (!p)
	{
	    return YCPError ("popen failed");
	}

	fwrite (input.c_str (), input.length (), 1, p);
	int ret = pclose (p);

	if (WIFEXITED (ret))
	    return YCPInteger (WEXITSTATUS (ret));
	return YCPInteger (WTERMSIG (ret) + 128);
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    else if (cmd == "symlink")
    {
	/**
	 * @builtin Execute (.target.symlink, string oldpath, string newpath) -> boolean
	 *
	 * Creates a symbolic link named newpath which contains the
	 * string oldpath.
	 *
	 * Symbolic links are interpreted at run-time as if the  con­
	 * tents of the link had been substituted into the path being
	 * followed to find a file or directory.
	 *
	 * The return value is true or false, depending of the success.
	 *
	 * @example Execute (.target.symlink, "/lib/YaST2", "Y2")
	 */

	if (value.isNull () || arg.isNull () || !value->isString () ||
	    !arg->isString ())
	{
	    return YCPError ("Bad arguments to Execute (.symlink, string old, string new)");
	}
	const char *oldpath = value->asString()->value_cstr();
	const string newpath = arg->asString()->value();

	y2milestone ("symlink %s -> %s", oldpath, newpath.c_str());

	remove (newpath.c_str());
	return YCPBoolean (symlink (oldpath, newpath.c_str()) == 0);

    } // .symlink

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    else if (cmd == "mkdir")
    {
	/**
	 * @builtin Execute (.target.mkdir, string path <, integer mode>)) -> boolean
	 *
	 * Creates a directory and all its parents, if necessary.
	 * All created elements will have mode 0755 if mode is omitted.
	 *
	 * The return value is true or false, depending of the success, ie if
	 * the directory exists afterwards.
	 *
	 * @example Execute (.target.mkdir, "/var/adm/mount")
	 */

	if (value.isNull() || !value->isString())
	{
	    return YCPError ("Bad path argument to Execute (.mkdir, string path)");
	}

	string path = value->asString()->value();

	int mode = 0755;

	if (!arg.isNull())
	{
	    if (arg->isInteger())
	    {
		mode = arg->asInteger()->value();
	    }
	    else
	    {
		return YCPError ("Bad mode argument to Execute (.mkdir, string path, integer mode)");
	    }
	}

	size_t pos = 0;

	y2milestone ("mkdir %s", path.c_str());

	// Create leading components
	while (pos = path.find('/', pos + 1), pos != string::npos)
	{
	    mkdir (path.substr(0, pos).c_str(), mode);
	}
	// Create the last part
	mkdir (path.substr(0, pos).c_str(), mode);

	struct stat sb;
	return YCPBoolean(stat(path.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode));
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    else if (cmd == "mount")
    {
	/**
	 * @builtin Execute (.target.mount, [ string device, string mountpoint <, string logfile>], [, string options])) -> boolean
	 *
	 * Mounts a (block) device at a mountpoint.
	 * If logfile is given, the stderr of the mount cmd will be appended to
	 * this file
	 *
	 * The return value is true or false, depending of the success
	 *
	 * @example Execute (.target.mount, ["/dev/fd0", "/floppy"], "-t msdos")
	 * @example Execute (.target.mount, ["/dev/fd0", "/floppy", "/var/log/y2mountlog"], "-t msdos")
	 */

	if (value.isNull() || !value->isList())
	{
	    return YCPError ("Bad path argument to Execute (.mount, [ string device, string mountpoint <, string y2mountlog> ])");
	}

	YCPList mountlist = value->asList();
	if (mountlist->size() < 2
	    || !mountlist->value(0)->isString()
	    || !mountlist->value(1)->isString())
	{
	    return YCPError ("Bad list values in argument to Execute (.mount, [ string device, string mountpoint ])");
	}

	string mountcmd = "/bin/mount ";

	// arg is mount options, this is optional

	if (!arg.isNull())
	{
	    if (arg->isString())
	    {
		mountcmd += arg->asString()->value() + " ";
	    }
	    else
	    {
		return YCPError ("Bad type of mode argument to Execute (.mount, string|list path, string mode)", YCPBoolean (false));
	    }
	}

	// device
	mountcmd += mountlist->value(0)->asString()->value() + " ";
	// mountpoint
	mountcmd += mountlist->value(1)->asString()->value();

	if (mountlist->size() == 3)
	{
	    if (mountlist->value(2)->isString())
	    {
		mountcmd += " 2> " + mountlist->value(2)->asString()->value();
	    }
	    else
	    {
		return YCPError ("Bad logfile argument to Execute (.mount, [ string device, string mountpoint, string logfile ])", YCPBoolean (false));
	    }
	}

	return YCPBoolean (shellcommand (mountcmd) == 0);
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    else if (cmd == "smbmount")
    {
	/**
	 * @builtin Execute (.target.smbmount, [ string server_and_dir, string mountpoint <, string logfile>], [, string options])) -> boolean
	 *
	 * Mounts a SMB share at a mountpoint.
	 * if logfile is given, the stderr of the mount cmd will be appended to
	 * this file
	 *
	 * The return value is true or false, depending of the success
	 *
	 * @example Execute (.target.smbmount, ["//windows/crap", "/crap"], "-o guest")
	 * @example Execute (.target.smbmount, ["//smb/share", "/smbshare", "/var/log/y2mountlog"])
	 */

	if (value.isNull() || !value->isList())
	{
	    return YCPError ("Bad share argument to Execute (.smbmount, [ string share, string mountpoint <, string y2mountlog> ])");
	}

	YCPList mountlist = value->asList();
	if (mountlist->size() < 2
	    || !mountlist->value(0)->isString()
	    || !mountlist->value(1)->isString())
	{
	    return YCPError ("Bad list values in argument to Execute (.smbmount, [ string share, string mountpoint ])");
	}

	string mountcmd = "/usr/bin/smbmount ";

	if (!arg.isNull() && arg->isString())
	{
	    mountcmd += arg->asString()->value() + " ";
	}
	else
	{
	    return YCPError ("Bad mode argument to Execute (.smbmount, string path, integer mode)");
	}

	// share
	mountcmd += mountlist->value(0)->asString()->value() + " ";

	// mountpoint
	mountcmd += mountlist->value(1)->asString()->value();

	// logfile
	if (mountlist->size() == 3)
	{
	    if (mountlist->value(2)->isString())
	    {
		mountcmd += " 2> " + mountlist->value(2)->asString()->value();
	    }
	    else
	    {
		return YCPError ("Bad logfile argument to Execute (.smbmount, [ string device, string mountpoint, string logfile ])");
	    }
	}

	return YCPBoolean (shellcommand (mountcmd) == 0);
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    else if (cmd == "umount")
    {
	/**
	 * @builtin Execute (.target.umount, string mountpoint) -> boolean
	 * Unmounts a (block) device at a mountpoint.
	 *
	 * The return value is true or false, depending of the success.
	 *
	 * @example Execute (.target.umount, "/floppy")
	 */
	if (value.isNull() || !value->isString())
	{
	    return YCPError ("Bad mountpoint in Execute (.umount, string mountpoint)");
	}

	string umountcmd = "/bin/umount " + value->asString()->value();

	return YCPBoolean (shellcommand (umountcmd) == 0);
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    else if (cmd == "remove")
    {
	/**
	 * @builtin Execute (.target.remove, string file) -> boolean
	 * Remove a file.
	 *
	 * The return value is true or false depending on the success.
	 *
	 * @example Execute (.target.remove, "/tmp/xyz")
	 */
	if (value.isNull() || !value->isString())
	{
	    return YCPError ("Bad file in Execute (.remove, string file)");
	}

	int ret = unlink (value->asString ()->value_cstr ());

	return YCPBoolean (ret == 0);
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    else if (cmd == "insmod")
    {
	/**
	 * @builtin Execute (.target.insmod, string module, string options) -> boolean
	 * Load module in target system.
	 *
	 * The return value is true or false, depending of the success.
	 *
	 * @example Execute (.target.insmod, "a_module", "an option")
	 */

	if (value.isNull() || !value->isString())
	{
	    return YCPError ("Bad source in Execute (.insmod, string module, string options)");
	}

	string insmodcmd = "/sbin/insmod " + value->asString()->value();

	if (!arg.isNull() && arg->isString())
	{
	    insmodcmd += string (" ") + arg->asString()->value();
	}

	return YCPBoolean (shellcommand (insmodcmd) == 0);
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    else if (cmd == "modprobe")
    {
	/**
	 * @builtin Execute (.target.modprobe, string module, string options) -> boolean
	 * Load module in target system.
	 *
	 * The return value is true or false, depending of the success.
	 *
	 * @example Execute (.target.modprobe, "a_module", "an option")
	 */

	if (value.isNull() || !value->isString())
	{
	    return YCPError ("Bad source in Execute (.modprobe, string module, string options)", YCPBoolean (false));
	}

	string modprobecmd = "/sbin/modprobe " + value->asString()->value();

	if (!arg.isNull() && arg->isString())
	{
	    modprobecmd += string (" ") + arg->asString()->value();
	}

	return YCPBoolean (shellcommand (modprobecmd) == 0);
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    else if (cmd == "kill") {

	/**
	 * @builtin Execute(.target.kill, integer pid [, integer signal]) -> boolean
	 * Kill process with signal (SIGTERM if not specified).
	 *
	 * The return value is true or false, depending of the success.
	 *
	 * @example Execute (.target.kill, 1, 9)
	 */

	if (value.isNull() || !value->isInteger())
	    return YCPError("Bad PID in Execute (.kill, integer pid, integer signal)", YCPBoolean(false));

	int signal = 15;
	int pid = value->asInteger()->value();

	if (!arg.isNull() && arg->isInteger())
	    signal = arg->asInteger()->value();

	return YCPBoolean (kill(pid,signal) != -1);
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    else if (cmd == "control")
    {

	if (path->length()<2)
	    return YCPError(string("Undefined subpath for Execute (.control."));

	if (path->component_str(1) == "printer_reset")
	{

	    /**
	     * @builtin Execute (.target.control.printer_reset, string device) -> boolean
	     * Reset the given printer (trigger ioctl)
	     *
	     * The return value is true or false, depending of the success
	     *
	     */

	    if (value.isNull() || !value->isString())
	    {
		return YCPError ("Bad filename in Execute (.control.printer_reset, string device");
	    }

	    string device = value->asString()->value();

	    int fd = open(device.c_str(), O_RDWR);
	    if (fd < 0)
	    {
		return YCPError (string("Open failed: \"" + device + "\" " + string(strerror(errno)?:"")));
	    }

	    int ret = ioctl(fd, LPRESET, NULL);
	    close(fd);

	    return YCPBoolean (ret == 0);
	}

    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    return YCPError (string("Undefined subpath for Execute (") + path->toString() + ")");
}

/* EOF */
