/*
 * Y2PackagerComponent.cc
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/statvfs.h>

#include "Y2PackagerComponent.h"
#include "Rpm_Installer.h"
#include <ycp/y2log.h>

#define RETURN_OK      YCPSymbol("ok",     true)     // `ok
#define RETURN_ERROR   YCPSymbol("error",  true)     // `error
#define RETURN_CANCEL  YCPSymbol("cancel", true)     // `cancel


static void
get_disk_stats(const char *fs, long long *used, long long *size)
{
  struct statvfs sb;
  if (statvfs(fs, &sb) < 0)
    {
      *used = *size = -1;
      return;
    }
  long long blocksize = sb.f_frsize ? : sb.f_bsize;
  *size = sb.f_blocks * blocksize;
  *used = (sb.f_blocks - sb.f_bfree) * blocksize;
}

// PackagerComponent

YCPValue
Y2PackagerComponent::doActualWork(const YCPList& options, Y2Component *displayserver)
{
  report_macro = (options->value(0)->isVoid() ? ""
		  : options->value(0)->asString()->value());

  YCPValue op = options->value(1);
  string op_name =
    (op->isTerm() ? op->asTerm()->symbol() : op->asSymbol())->symbol();
  if (op_name == "init")
    operation = Init;
  else if (op_name == "add" || op_name == "remove")
    {
      operation = op_name == "add" ? Add : Remove;
      package_prefix = op->asTerm()->value(0)->asString()->value();
      if (package_prefix[package_prefix.size()-1] != '/')
	package_prefix = package_prefix + "/";
      packages = op->asTerm()->value(1)->asList();
    }
  else
    y2error ("Invalid operation: %s", op_name.c_str());

  // Parse options
  int argc = options->size();
  for (int i = 2; i < argc; i++)
    {
      YCPValue v = options->value(i);
      YCPTerm t = v->isTerm() ? v->asTerm() : YCPNull();
      string option = v->isTerm() ? t->symbol()->symbol()
	: v->asSymbol()->symbol();
      if (option == "root")
	// Name of root filesystem
	rootfs = t->value(0)->asString()->value();
      else if (option == "number")
	start_number = t->value(0)->asInteger()->value();
      else if (option == "rpmpath")
	rpmDBPath = t->value(0)->asString()->value();
      else if (option == "manual_scripts")
	manual_scripts = true;
    }

  y2debug ("rootfs = %s", rootfs.c_str());
  y2debug ("rpmDBPath = %s", rpmDBPath.c_str());
  y2debug ("operation = %s", operation == Init ? "init" : operation == Add ? "add" : "remove");
  if (package_prefix[0])
    y2debug ("Package prefix = %s", package_prefix.c_str());
  if (manual_scripts)
    y2debug ("manual_scripts");

  switch (operation)
    {
    case Init:
      {
	Rpm_Installer rpm(rootfs, rpmDBPath);
	rpm.init_database();
	if (rpm.status() == 0) return RETURN_OK;
	else	               return RETURN_ERROR;
      }

    case Add:
      {
	int n_files = packages->size();

	y2debug ("Installing: root = %s%s", rootfs.c_str(), manual_scripts ? ", manual_scripts" : "");

	YCPSymbol returncode = RETURN_OK;
	for (int current_number = 0; current_number < n_files;
	     current_number++)
	  {
	    YCPValue package =
	      packages->value(current_number)->asList()->value(0);
	    string fullname = package_prefix + package->asString()->value();
	    Rpm_Installer rpm(rootfs, rpmDBPath, fullname);

	    // Figure out the package name
	    string current_package = rpm.query_package_name();

	    if (current_package == "")
	      {
		// Skip missing packages
		y2warning ("Package %s not found", fullname.c_str());

		rpm.logRPM ( "Package " + fullname + " not found", false );
		// set RETURN_ERROR and report_progress with error=true
		returncode = RETURN_ERROR;
		report_progress(displayserver,
				fullname, current_number, 0, true);
		continue;
	      }

	    y2milestone ("Installing %s", current_package.c_str());

	    YCPValue val =
	      report_progress(displayserver,
			      current_package, current_number, 0, false);
	    if (!val->isVoid())
	      {
		rpm.kill();
		return val;
	      }

	    bool is_installed = rpm.is_installed();
	    
	    if (manual_scripts)
	      // Execute preinstall script
	      execute_script(rpm.query_preinstall_script(),
			     is_installed);

	    // Start installation
	    rpm.install_package(manual_scripts);

	    string line;
	    double old_percent = 0.0;
	    while (rpm.read_line(line))
	      {
		if (line[0] == '%' && line[1] == '%')
		  {
		    double percent;
		    sscanf (line.c_str () + 2, "%lg", &percent);
		    if (percent >= old_percent + 5.0)
		      {
			old_percent = int (percent / 5) * 5;
			YCPValue val =
			  report_progress (displayserver, current_package,
					   current_number, percent, false);
			if (!val->isVoid())
			  {
			    rpm.kill();
			    return val;
			  }
		      }
		  }
		else
		  y2warning ("rpm: %s", line.c_str());
	      }
	    int rpm_status = rpm.status();
	    
	    if (rpm_status != 0)
	      {
		y2error ("rpm returned %d", rpm_status);
		returncode = RETURN_ERROR;
		report_progress (displayserver, current_package,
				 current_number, 100, true);
	      }

	    if (manual_scripts)
	      // Execute postinstall script
	      execute_script(rpm.query_postinstall_script(),
			     is_installed);
	  }
	
	y2debug("RETURN from packager: %s", returncode->toString().c_str());

	return returncode;
	break;
      }

    case Remove:
      {
	int n_files = packages->size();

	y2debug ("Removing: root = %s%s", rootfs.c_str(), manual_scripts ? ", manual_scripts" : "");

	YCPSymbol returncode = RETURN_OK;
	for (int current_number = 0; current_number < n_files;
	     current_number++)
	{
	    YCPValue package =
	      packages->value(current_number)->asList()->value(0);
	    string package_name = package->asString()->value();

	    Rpm_Installer rpm(rootfs, rpmDBPath, package_name);

	    // Figure out the package name
	    package =
	      packages->value(current_number)->asList()->value(1);
	    string current_package =  package->asString()->value();

	    y2milestone ("Deleting %s", current_package.c_str());

	    YCPValue val =
	      report_progress(displayserver,
			      package_name, current_number, 0, false);
	    if (!val->isVoid())
	    {
		rpm.kill();
		return val;
	    }

	    // Start remove
	    rpm.remove_package();

	    string line;
	    double old_percent = 0.0;
	    while (rpm.read_line(line))
	    {
	       if (line[0] == '%' && line[1] == '%')
	       {
		    double percent;
		    sscanf (line.c_str () + 2, "%lg", &percent);
		    if (percent >= old_percent + 5.0)
		    {
			old_percent = int (percent / 5) * 5;
			YCPValue val =
			  report_progress (displayserver, package_name,
					   current_number, percent, false);
			if (!val->isVoid())
			{
			    rpm.kill();
			    return val;
			}
		    }
	       }
	       else
		  y2warning ("rpm: %s", line.c_str());
	    }
	    int rpm_status = rpm.status();
	    if (rpm_status != 0)
	    {
		y2warning ("rpm returned %d", rpm_status);
		returncode = RETURN_ERROR;
		report_progress (displayserver, package_name,
				 current_number, 100, true);
	    }
	  }
	return returncode;
	break;
      }
    default:
      return YCPVoid();
    }
}


/*
  report progress to UI

  send <report_macro> ( <name>, <label>, <percent>, <number>, <disk_used>, <disk_size>)

  to UI

  Unchanged values (i.e. <name> and <label> are nil (YCPVoid())

*/


static string last_name;       // leave here due to compiler bug

YCPValue
Y2PackagerComponent::report_progress(Y2Component *displayserver,
				     string name, int number, double percent,
				     bool error)
{
  if (report_macro == "")
    return YCPVoid();

  YCPTerm t(report_macro, false);
  if (name != last_name ||
      error )
  {
    t->add(YCPString(name));
    YCPValue this_package = packages->value(number);
    t->add(this_package->asList()->value(1));
    last_name = name;
  }
  else {
    t->add(YCPVoid());
    t->add(YCPVoid());
  }

  t->add(YCPInteger((long long)percent));
  t->add(YCPInteger(number + start_number));

  long long disk_used, disk_size;

  get_disk_stats(rootfs.c_str(), &disk_used, &disk_size);

  t->add(YCPInteger(disk_used));
  t->add(YCPInteger(disk_size));
  t->add(YCPBoolean(error));

  YCPValue val = displayserver->evaluate(t);
  if (!val->isVoid())
  {
      if (val->isSymbol() && val->asSymbol()->symbol() == "cancel") return val;
      else
      {
	  y2error ("displayserver returned %s", val->toString().c_str());
	  return RETURN_ERROR;
      }
    }
  return val;
}

void
Y2PackagerComponent::execute_script(string script, bool is_installed )
{
  if (script == "")
    return;

  string script_option = "1"; //new install
  if ( is_installed )
  {
     script_option = "2"; // update
  }

  string full_script =
      string("cd ") + rootfs + " || exit 1; " + script;

  FILE* file = fopen( "/tmp/execute_script", "w" );

  if ( ! file )
  {
      y2error("Can't open /tmp/execute_script for writing");
      return;
  }

  fprintf( file,
	   "%s",
	   full_script.c_str() );
  
  if ( file ) fclose( file );  

  const char *argv[] = {
    "/bin/sh",
    "/tmp/execute_script",
    script_option.c_str(),
    (char *) 0
  };

  ExternalProgram shell(argv, ExternalProgram::Stderr_To_Stdout);

  string line;
  while (1)
    {
      line = shell.receiveLine();
      if (line.length() == 0)
	break;
      if (line[line.length() - 1] == '\n')
	line.erase(line.length() - 1);
      y2warning ("%s", line.c_str());
    }
  int status = shell.close();
  if (status != 0)
    y2warning ("shell returned %d", status);

  remove ( "/tmp/execute_script" );
}
