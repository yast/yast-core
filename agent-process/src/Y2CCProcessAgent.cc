/* Y2CCProcessAgent.cc
 *
 * ------------------------------------------------------------------------------
 * Copyright (c) 2008 Novell, Inc. All Rights Reserved.
 *
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, contact Novell, Inc.
 *
 * To contact Novell about this file by physical or electronic mail, you may find
 * current contact information at www.novell.com.
 * ------------------------------------------------------------------------------
 *
 * Process agent implementation
 *
 * Authors: Ladislav Slezák <lslezak@novell.com>
 *
 * $Id: Y2CCProcessAgent.cc 27914 2006-02-13 14:32:08Z locilka $
 */

#include <scr/Y2AgentComponent.h>
#include <scr/Y2CCAgentComponent.h>

#include "ProcessAgent.h"

typedef Y2AgentComp <ProcessAgent> Y2ProcessAgentComp;

Y2CCAgentComp <Y2ProcessAgentComp> g_y2ccag_process ("ag_process");
