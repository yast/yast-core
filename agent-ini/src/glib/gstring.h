/* GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GLib Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GLib Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GLib at ftp://ftp.gtk.org/pub/gtk/.
 */

#ifndef __G_STRING_H__
#define __G_STRING_H__

#include "gtypes.h"

G_BEGIN_DECLS

typedef struct _GString         GString;

struct _GString
{
  gchar  *str;
  gsize len;
  gsize allocated_len;
};

GString*     g_string_new               (const gchar     *init);

gchar*       g_string_free              (GString         *string,
                                         gboolean         free_segment);
GString*     g_string_append            (GString         *string,
                                         const gchar     *val);
GString*     g_string_append_len        (GString         *string,
                                         const gchar     *val,
                                         gssize           len);
GString*     g_string_append_c          (GString         *string,
                                         gchar            c);

GString*     g_string_insert_c          (GString         *string,
                                         gssize           pos,
                                         gchar            c);

G_END_DECLS

#endif /* __G_STRING_H__ */
