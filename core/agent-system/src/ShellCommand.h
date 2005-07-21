/*
 * ShellCommand.h
 *
 * Functions for running shell commands with output attached to y2log
 *
 * Author: Michal Svec <msvec@suse.cz>
 *
 * $Id$
 */

#ifndef ShellCommand_h
#define ShellCommand_h

#include <string>

/**
 * Execute shell command and feed its output to y2log
 */
int shellcommand (const string &command, const string &tempdir = "");

/**
 * Execute shell command on background
 */
int shellcommand_background (const string &command);

#endif /* ShellCommand_h */
