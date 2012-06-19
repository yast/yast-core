/**
 * YaST2: Core system
 *
 * Description:
 *   YaST2 SCR: Functions for shell like (un)quoting
 *
 * Authors:
 *   Michal Filka <mfilka@suse.cz>
 *
 * $Id$
 */

#ifndef __QUOTES_H__
#define __QUOTES_H__

#include <string>

std::string quote( const std::string & quoted_string);
std::string unquote( const std::string & unquoted_string);

#endif
