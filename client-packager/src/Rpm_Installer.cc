
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
#include <fstream>

using std::ofstream;
using std::ios;
using std::endl;

#include "Rpm_Installer.h"
#include <y2/ExternalDataSource.h>
#include <ycp/y2log.h>


Rpm_Installer::Rpm_Installer(string name_of_root,
			     string name_of_dbpath,
			     string package_name)
    : rootfs(name_of_root),
      rpmDBPath(name_of_dbpath),
      package(package_name),
      globaLogShortRPM ( false ),
      process(0),
      exit_code(-1)
{
}


Rpm_Installer::~Rpm_Installer()
{
    delete process;
}


// Create all parent directories of @param name, as necessary

static void
create_directories(string name)
{
    size_t pos = 0;

    while (pos = name.find('/', pos + 1), pos != string::npos)
	mkdir (name.substr(0, pos).c_str(), 0777);
}


// prepend rootfs if needed
// check if rootfs ends in "/" and if path begins with "/" (rooted_path)
//

static const string prepend_rootfs (const string& rootfs, const string& path)
{
    bool rooted_path = (path[0] == '/');

    // remove trailing '/'

    if (rootfs == "/")
	return (rooted_path ? path : (string ("/")+path));

    if (rootfs[rootfs.size()-1] == '/') {
	if (!rooted_path) {
	    string norootpath (path, 1);
	    return rootfs + norootpath;
	}
	return rootfs + path;
    }
    return (rooted_path ? (rootfs + path) : (rootfs + "/" + path));
}


// Splitting a string into columns

void Rpm_Installer::splitt( string source, string *column, int max_column,
			    string seperator )
{
    string::size_type begin, end;
    int counter;

    for ( counter = 0; counter<max_column ; counter++ )
    {
	column[counter] = "";
    }

    // evaluate all columns
    begin = source.find_first_not_of ( seperator );
    counter = 0;
    while ( begin != string::npos && counter < max_column )
    {
	end = source.find_first_of ( seperator, begin );

	// line-end ?
	if ( end == string::npos )
	{
	    end= source.length();
	}
	if ( counter < max_column-1 )
	{
	    column[counter].assign ( source, begin, end-begin );
	}
	else
	{
	    // Rest of string into the last value
	    column[counter].assign (source, begin, string::npos);
	}

	begin = source.find_first_not_of ( seperator, end );
	counter++;
    }

    for ( counter = 0; counter<max_column ; counter++ )
    {
	// extract \n
	if (column[counter][column[counter].length()-1] == '\n' )
	{
	    column[counter].assign ( column[counter],
				     0,
				     column[counter].length()-1 );
	}
    }
}


// Logging rpm-command

void Rpm_Installer::logRPM ( string line, bool onlyLongFormat,
			     bool withoutCR )
{
    if ( line[0] != '%' && line[1] != '%')
    {
	// logging no percent display
	string filename = prepend_rootfs (rootfs, "/var/log/y2logRPM");
	ofstream fpRpmLog ( filename.c_str(), ios::app | ios::ate );
	if ( fpRpmLog )
	{
	    fpRpmLog << line << endl;
	}

	if ( !onlyLongFormat )
	{
	    // short form of logging
	    filename = prepend_rootfs (rootfs, "/var/log/y2logRPMShort");
	    ofstream fpRpmLogShort ( filename.c_str() , ios::app | ios::ate );
	    if ( fpRpmLogShort )
	    {

		//
		// filter out message if backup does not differ from config file
		//
		// /etc/crontab created as /etc/crontab.rpmnew
		if ( line.find( "saved as" ) != string::npos ||
		     line.find( "created as" ) != string::npos )
		{
		    string column[5];
		    string command;

		    fpRpmLogShort << line << endl;

		    splitt( line, column, 5, " " );
		    command =  "diff -q ";
		    command += column[1] + " ";
		    command += column[4];
		    if ( fpRpmLog )
		    {
			fpRpmLog << command << endl;
		    }

		    command += " >/dev/null 2>&1";

		    if ( system ( command.c_str() ) == 0 )
		    {
			unlink( column[4].c_str() );
			if ( fpRpmLog )
			{
			    fpRpmLog << "removing: " << column[4] << endl ;
			}
		    }
		    else
		    {
			if ( fpRpmLog )
			{
			    fpRpmLog << "Files " << column[1] << " and "
				     << column[4] << " differ" << endl;
			}
			string::size_type pos = column[1].find_last_of ( "^*/" );
			string logfile;

			if ( pos != string::npos )
			{
			    string file;
			    file.assign (column[1], pos + 1, string::npos);
			    logfile =  prepend_rootfs (rootfs,
						       "/var/adm/notify/warnings/Configfile_"
						       + file) ;
			}
			else
			{
			    logfile = prepend_rootfs (rootfs,
						      "/var/adm/notify/warnings/Configfile_"
						      + column[1]) ;
			}
			ofstream log( logfile.c_str(), ios::out | ios::app );
			log << line << endl;
			log.close();
		    }
		}

		//
		// filter out errors for nonempty and missing dirs upon delete
		//
		else if ( line.find ( "package " ) != string::npos &&
			  line.find ( " not " ) != string::npos &&
			  ( line.find ( "listed" ) != string::npos ||
			    line.find ( "found" ) != string::npos ) &&
			  line.find ( "in " ) != string::npos  &&
			  line.find ( " index" ) )
		{
		    // nothing !!!
		}
		else if ( line.find( "removal of" ) != string::npos ||
			  line.find( "rmdir of" ) != string::npos ||
			  line.find( "cannot remove" ) != string::npos )
		{

		    if ( line.find( "directory not empty" ) != string::npos ||
			 line.find( "operation not permitted" ) != string::npos ||
			 line.find( "no such file or directory" ) != string::npos )
		    {
			// nothing !!!
		    }
		}
		else
		{
		    // logging anyway
		    if ( !withoutCR )
		    {
			fpRpmLogShort << line << endl;
		    }
		    else
		    {
			fpRpmLogShort << line << "  ";
		    }
		}
	    }
	}
    }
}


// Initialize the rpm database

void
Rpm_Installer::init_database()
{
    y2debug ("init_database");
    create_directories (prepend_rootfs (rootfs, rpmDBPath));
    const char *const opts[] = { "--initdb" };
    globaLogShortRPM = false;
    run_rpm(sizeof(opts) / sizeof(*opts), opts);
}


// Query the name of the current package

string
Rpm_Installer::query_package_name()
{
    return query_package("%{NAME}");
}


// Return the contents of the preinstall script for the current package

string
Rpm_Installer::query_preinstall_script()
{
    string script = query_package("%{PREIN}");
    if (script == "(none)")
	script = "";
    return script;
}


// Return the contents of the postinstall script for the current package

string
Rpm_Installer::query_postinstall_script()
{
    string script = query_package("%{POSTIN}");
    if (script == "(none)")
	script = "";
    return script;
}


bool
Rpm_Installer::is_installed()
{
    string packageName = query_package_name();
    const char *const opts[] = {
	"-q", "--qf", "%{NAME}",  packageName.c_str()
    };
    globaLogShortRPM = false;
    run_rpm(sizeof(opts) / sizeof(*opts), opts, ExternalProgram::Discard_Stderr);
    string value ="";
    string output = process->receiveLine();

    while ( output.length() > 0 )
    {
	value += output;
	output = process->receiveLine();
    }

    logRPM ( value, true );

    status();
    if ( value.length() > 0 )
    {
	return true;
    }
    else
    {
	return false;
    }
}


// Query the current package using the specified query format

string
Rpm_Installer::query_package(const char *format)
{
//  y2debug ("query_package %s", format);
    const char *const opts[] = {
	"-q", "-p", "--qf", format, package.c_str()
    };
    globaLogShortRPM = false;
    run_rpm(sizeof(opts) / sizeof(*opts), opts, ExternalProgram::Discard_Stderr);
    string value ="";
    string output = process->receiveLine();

    while ( output.length() > 0 )
    {
	value += output;
	output = process->receiveLine();
    }

    logRPM ( value, true );

    status();
    return value;
}


// Install the current package

void
Rpm_Installer::install_package(bool noscripts)
{
    const char *opts[] = {
	"-U", "--replacepkgs", "--oldpackage", "--replacefiles",
	"--nodeps", "--ignoresize", "--percent",
	0, 0
    };
    int n_opts = sizeof(opts) / sizeof(*opts) - 2;
    struct stat  dummyStat;

    if (noscripts)
	opts[n_opts++] = "--noscripts";
    opts[n_opts++] = package.c_str();

    globaLogShortRPM = true;

    run_rpm(n_opts, opts,
	    noscripts ? ExternalProgram::Discard_Stderr
	    : ExternalProgram::Stderr_To_Stdout);

    string::size_type pos = package.find_last_of ( "/" );
    string packageName;

    if ( pos != string::npos )
    {
	packageName.assign (package, pos + 1, string::npos);
    }
    else
    {
	packageName = package;
    }

    pos = packageName.find ( ".rpm" );
    if ( pos != string::npos )
    {
	packageName.assign ( packageName, 0, pos );
    }

    string output = "Installing ";
    output += packageName.c_str();
    logRPM ( output,false, true );

    // Checking, if rpm-file exists.
    if (  stat( package.c_str(), &dummyStat ) == -1 )
    {
	output = "ERRROR: file ";
	output += package + " does not exists.";
	logRPM ( output,false );
    }
}


// Uninstall the current package

void
Rpm_Installer::remove_package()
{
    const char *const opts[] = {
	"-e", "--nodeps", package.c_str()
    };

    globaLogShortRPM = true;

    run_rpm(sizeof(opts) / sizeof(*opts), opts);

    string::size_type pos = package.find_last_of ( "/" );
    string packageName;

    if ( pos != string::npos )
    {
	packageName.assign (package, pos + 1, string::npos);
    }
    else
    {
	packageName = package;
    }

    pos = packageName.find ( ".rpm" );
    if ( pos != string::npos )
    {
	packageName.assign ( packageName, 0, pos );
    }

    string output = "Removing ";
    output += packageName.c_str();
    logRPM ( output,false, true );
}


// Run rpm with the specified arguments, handling stderr as specified
// by disp

void
Rpm_Installer::run_rpm(int n_opts, const char *const *options,
		       ExternalProgram::Stderr_Disposition disp)
{
    exit_code = -1;
    int argc = n_opts + 1 + 5 /* rpm --root <root> --dbpath <dbpath>*/
	+ 1 /* NULL */;

    // Create the argument array
    const char *argv[argc];
    int i = 0;
    argv[i++] = "rpm";
    argv[i++] = "--root";
    argv[i++] = rootfs.c_str();
    argv[i++] = "--dbpath";
    argv[i++] = rpmDBPath.c_str();
    for (int j = 0; j < n_opts; j++)
	argv[i++] = options[j];
    argv[i] = 0;


    string output = "";
    int k;

    for ( k = 0; k < argc-2; k++ )
    {
	output = output + " " + argv[k];
    }

    y2debug ("rpm command: %s", output.c_str() );
    logRPM ( output, true );

    // Launch the program
    process = new ExternalProgram(argv, disp);
}


// Read a line from the rpm process

bool
Rpm_Installer::read_line(string &line)
{
    line = process->receiveLine();
    if (line.length() == 0)
	return false;
    if (line[line.length() - 1] == '\n')
	line.erase(line.length() - 1);

    if ( globaLogShortRPM )
    {
	logRPM ( line, false );
    }
    else
    {
	logRPM ( line, true ); // only long-form of logging
    }

    return true;
}


// Return the exit status of the rpm process, closing the connection if
// not already done

int
Rpm_Installer::status()
{
    if (exit_code == -1)
	exit_code = process->close();

    char number[20];
    sprintf ( number ,"%d", exit_code );
    string output = "Return :";
    output += number;
    logRPM ( output,true );

    if ( globaLogShortRPM )
    {
	if ( exit_code == 0 )
	{
	    logRPM ( "... OK",false );
	}
	else
	{
	    logRPM ( "... ERROR",false );
	}
    }
    return exit_code;
}


// Forcably kill the rpm process

void
Rpm_Installer::kill()
{
    if (process) process->kill();
}
