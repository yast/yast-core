/*
 * FindYpserv.cc
 *
 * Does a RPC broadcast to find NIS servers for a given domain
 *
 * Authors:
 *   Martin Vidner <mvidner@suse.cz> (adaptation to yast)
 *   Thorsten Kukuk <kukuk@suse.de>  (rpc code)
 *
 * $Id$
 */

#include <set>
#include <string>

std::set<std::string>
findYpservers (const std::string &domain);
