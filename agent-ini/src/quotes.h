/*
 * Copyright (c) 2012 Novell, Inc.
 *
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, contact Novell, Inc.
 *
 * To contact Novell about this file by physical or electronic mail, you may
 * find current contact information at www.novell.com.
 */

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

namespace YaST
{

    /*
     * Enclose given string into quotes to get bash ready string.
     */
    std::string shell_quote( const std::string & quoted_string);

    /*
     * Removes quotes and escape sequences according bash rules.
     */
    std::string shell_unquote( const std::string & unquoted_string);

}

#endif
