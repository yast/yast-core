// -*- c++ -*-

#ifndef Y2PackagerComponent_h
#define Y2PackagerComponent_h

#include <Y2.h>

/**
 * @short Install and uninstall packages
 */
class Y2PackagerComponent : public Y2Component
{
private:
  // Name of macro to call for progress bars, or "" if quiet
  string report_macro;

  // What to do
  enum { Init, Add, Remove } operation;

  // List of package names
  YCPList packages;

  // Prefix for package names
  string package_prefix;

  // Name of the root filesystem, defaults to "/"
  string rootfs;

  // path of the rpm-DB
  string rpmDBPath;

  // Ordinal number of the first package
  int start_number;

  // Whether to execute install scripts manually
  bool manual_scripts;

public:
  /**
   * Create a new packager component
   */
  Y2PackagerComponent::Y2PackagerComponent()
    : report_macro(""), packages(YCPNull()),
      rootfs("/"), rpmDBPath( "/var/lib/rpm/" ), manual_scripts(false)
  {}

  /**
   * What I'm called: "packager"
   */
  static string component_name() { return "packager"; }
  string name() const { return component_name(); }

  /**
   * Do the actual work of installing and uninstalling
   */
  YCPValue doActualWork(const YCPList& options, Y2Component *displayserver);

  /**
   * Set the client arguments:
   *
   * packager(macro, operation, options...)
   *
   * macro is either nil or a symbol that is called during installation as
   *   macro(packname, packdescr, percent, number, disk_used, disk_size)
   *     packname	name of current package
   *     packdesc	package summary line
   *     percent	how much of the current package is installed
   *	 number		the ordinal number of the current package
   *			beginning with start (default 1)
   *     disk_used	used bytes on filesystem that contains the root
   *	 disk_size	total space in bytes on that filesystem
   *
   * Possible operations are:
   *   add(prefix, [package...])	install the packages
   *   remove(prefix, [package...])	uninstall the packages
   *     each package is a list [name, summary_line].
   *   init				set up the root filesystem
   *
   * Possible options are:
   *   root(name)			the root filesystem to use (defaut "/")
   *   number(start)                    number of the first package to install
   *   manual_scripts			run install scripts manually
   */
private:
  YCPValue report_progress(Y2Component *displayserver,
			   string name, int number, double percent,
			   bool error );

  void execute_script(string script, bool is_installed );
};

#endif // Y2PackagerComponent_h
