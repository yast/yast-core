// -*- C++ -*-

#ifndef Rpm_Installer_h
#define Rpm_Installer_h

#include <YCP.h>
#include <y2/ExternalProgram.h>
#include <fstream>
#include <iostream>


/**
 * @short Interface to the rpm program
 */

class Rpm_Installer
{
public:
  /**
   * Create an new instance.
   * @param name_of_root The name of the install root
   * @param name_of_dbpath The path of the rpm-db
   * @param package_name The full name of the package file if any
   */
  Rpm_Installer(string name_of_root,
		string name_of_dbpath,
		string package_name = "");

  /**
   * Clean up.
   */
  ~Rpm_Installer();

  /**
   * Initialize the rpm database
   */
  void init_database();

  /**
   * Install the current package.
   * @param noscript If true don't execute install scripts
   */
  void install_package(bool noscripts = false);

  /**
   * Uninstall the current package.
   */
  void remove_package();

  /**
   * Query the name of the current package.
   */
  string query_package_name();

  /**
   * Return the contents of the preinstall script for the current package.
   */
  string query_preinstall_script();

  /**
   * Return the contents of the postinstall script for the current package.
   */
  string query_postinstall_script();

  /**
   * Query the current package.
   * @param format The query format to use.
   */
  string query_package( const char *format );

   /**
    * Check, if the package is installed
    */
   bool is_installed();
   
  /**
   * Read a line from the rpm process.
   */
  bool read_line(string &line);

  /**
   * Return the exit status of the rpm process, closing the connection if
   * not already done.
   */
  int status();

  /**
   * Forcably kill the rpm process
   */
  void kill();

   /**
    * Logging rpm-command and results /var/log/y2logRPM and
    * /var/log/y2logRPMShort
    */
  void logRPM ( string line, bool onlyLongFormat, bool withoutCR = false );   

private:

  /**
   * The name of the install root.
   */
  string rootfs;

  /**
   * Path of the rpm-db
   */
   string rpmDBPath;
   
  /**
   * The name of the current package.
   */
  string package;


   /**
    * Flag, if the output and the return-value
    * have to be logged in the short form
    **/
   bool globaLogShortRPM;
   
  /**
   * The connection to the rpm process.
   */
  ExternalProgram *process;

   /**
    * Splitting strings
    **/
   void splitt( string source, string *column, int max_column,
		string seperator );
   
  /**
   * Run rpm with the specified arguments and handle stderr.
   * @param n_opts The number of arguments
   * @param options Array of the arguments, @ref n_opts elements
   * @param stderr_disp How to handle stderr, merged with stdout by default
   */
  void run_rpm(int n_opts, const char *const *options,
	       ExternalProgram::Stderr_Disposition stderr_disp =
	       ExternalProgram::Stderr_To_Stdout);

  /**
   * The exit code of the rpm process, or -1 if not yet known.
   */
  int exit_code;
};

#endif
