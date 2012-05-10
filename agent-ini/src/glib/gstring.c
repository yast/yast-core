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

/*
 * MT safe
 */

#include <stdlib.h>
#include <string.h>

#include "gstring.h"
#include "gmem.h"
#include "gslice.h"

/**
 * SECTION:strings
 * @title: Strings
 * @short_description: text buffers which grow automatically
 *     as text is added
 *
 * A #GString is an object that handles the memory management
 * of a C string for you. You can think of it as similar to a
 * Java StringBuffer. In addition to the string itself, GString
 * stores the length of the string, so can be used for binary
 * data with embedded nul bytes. To access the C string managed
 * by the GString @string, simply use @string->str.
 */

/**
 * GString:
 * @str: points to the character data. It may move as text is added.
 *   The @str field is null-terminated and so
 *   can be used as an ordinary C string.
 * @len: contains the length of the string, not including the
 *   terminating nul byte.
 * @allocated_len: the number of bytes that can be stored in the
 *   string before it needs to be reallocated. May be larger than @len.
 *
 * The GString struct contains the public fields of a GString.
 */


#define MY_MAXSIZE ((gsize)-1)

static inline gsize
nearest_power (gsize base, gsize num)
{
  if (num > MY_MAXSIZE / 2)
    {
      return MY_MAXSIZE;
    }
  else
    {
      gsize n = base;

      while (n < num)
        n <<= 1;

      return n;
    }
}

static void
g_string_maybe_expand (GString *string,
                       gsize    len)
{
  if (string->len + len >= string->allocated_len)
    {
      string->allocated_len = nearest_power (1, string->len + len + 1);
      string->str = g_realloc (string->str, string->allocated_len);
    }
}

/**
 * g_string_sized_new:
 * @dfl_size: the default size of the space allocated to
 *     hold the string
 *
 * Creates a new #GString, with enough space for @dfl_size
 * bytes. This is useful if you are going to add a lot of
 * text to the string and don't want it to be reallocated
 * too often.
 *
 * Returns: the new #GString
 */
static GString *
g_string_sized_new (gsize dfl_size)
{
  GString *string = g_slice_new (GString);

  string->allocated_len = 0;
  string->len   = 0;
  string->str   = NULL;

  g_string_maybe_expand (string, MAX (dfl_size, 2));
  string->str[0] = 0;

  return string;
}

/**
 * g_string_new:
 * @init: the initial text to copy into the string
 *
 * Creates a new #GString, initialized with the given string.
 *
 * Returns: the new #GString
 */
GString *
g_string_new (const gchar *init)
{
  GString *string;

  if (init == NULL || *init == '\0')
    string = g_string_sized_new (2);
  else
    {
      gint len;

      len = strlen (init);
      string = g_string_sized_new (len + 2);

      g_string_append_len (string, init, len);
    }

  return string;
}

/**
 * g_string_free:
 * @string: a #GString
 * @free_segment: if %TRUE, the actual character data is freed as well
 *
 * Frees the memory allocated for the #GString.
 * If @free_segment is %TRUE it also frees the character data.  If
 * it's %FALSE, the caller gains ownership of the buffer and must
 * free it after use with g_free().
 *
 * Returns: the character data of @string
 *          (i.e. %NULL if @free_segment is %TRUE)
 */
gchar *
g_string_free (GString  *string,
               gboolean  free_segment)
{
  gchar *segment;

  if( string == NULL)
    return NULL;

  if (free_segment)
    {
      g_free (string->str);
      segment = NULL;
    }
  else
    segment = string->str;

  g_slice_free (GString, string);

  return segment;
}

/**
 * g_string_insert_len:
 * @string: a #GString
 * @pos: position in @string where insertion should
 *       happen, or -1 for at the end
 * @val: bytes to insert
 * @len: number of bytes of @val to insert
 *
 * Inserts @len bytes of @val into @string at @pos.
 * Because @len is provided, @val may contain embedded
 * nuls and need not be nul-terminated. If @pos is -1,
 * bytes are inserted at the end of the string.
 *
 * Since this function does not stop at nul bytes, it is
 * the caller's responsibility to ensure that @val has at
 * least @len addressable bytes.
 *
 * Returns: @string
 */
static GString *
g_string_insert_len (GString     *string,
                     gssize       pos,
                     const gchar *val,
                     gssize       len)
{
  if( string == NULL)
    return NULL;
  if( ( len != 0) && ( val == NULL))
    return string;

  if (len == 0)
    return string;

  if (len < 0)
    len = strlen (val);

  if (pos < 0)
    pos = string->len;
  else
  {
    if( pos > string->len)
      return string;
  }

  /* Check whether val represents a substring of string.
   * This test probably violates chapter and verse of the C standards,
   * since ">=" and "<=" are only valid when val really is a substring.
   * In practice, it will work on modern archs.
   */
  if (val >= string->str && val <= string->str + string->len)
    {
      gsize offset = val - string->str;
      gsize precount = 0;

      g_string_maybe_expand (string, len);
      val = string->str + offset;
      /* At this point, val is valid again.  */

      /* Open up space where we are going to insert.  */
      if (pos < string->len)
        g_memmove (string->str + pos + len, string->str + pos, string->len - pos);

      /* Move the source part before the gap, if any.  */
      if (offset < pos)
        {
          precount = MIN (len, pos - offset);
          memcpy (string->str + pos, val, precount);
        }

      /* Move the source part after the gap, if any.  */
      if (len > precount)
        memcpy (string->str + pos + precount,
                val + /* Already moved: */ precount + /* Space opened up: */ len,
                len - precount);
    }
  else
    {
      g_string_maybe_expand (string, len);

      /* If we aren't appending at the end, move a hunk
       * of the old string to the end, opening up space
       */
      if (pos < string->len)
        g_memmove (string->str + pos + len, string->str + pos, string->len - pos);

      /* insert the new string */
      if (len == 1)
        string->str[pos] = *val;
      else
        memcpy (string->str + pos, val, len);
    }

  string->len += len;

  string->str[string->len] = 0;

  return string;
}

/**
 * g_string_append:
 * @string: a #GString
 * @val: the string to append onto the end of @string
 *
 * Adds a string onto the end of a #GString, expanding
 * it if necessary.
 *
 * Returns: @string
 */
GString *
g_string_append (GString     *string,
                 const gchar *val)
{
  if( string == NULL)
    return NULL;
  if( val == NULL)
    return string;

  return g_string_insert_len (string, -1, val, -1);
}

/**
 * g_string_append_len:
 * @string: a #GString
 * @val: bytes to append
 * @len: number of bytes of @val to use
 *
 * Appends @len bytes of @val to @string. Because @len is
 * provided, @val may contain embedded nuls and need not
 * be nul-terminated.
 *
 * Since this function does not stop at nul bytes, it is
 * the caller's responsibility to ensure that @val has at
 * least @len addressable bytes.
 *
 * Returns: @string
 */
GString *
g_string_append_len (GString     *string,
                     const gchar *val,
                     gssize       len)
{
  if( string == NULL)
    return NULL;
  if( ( len != 0) && (val == NULL) )
    return string;

  return g_string_insert_len (string, -1, val, len);
}

/**
 * g_string_append_c:
 * @string: a #GString
 * @c: the byte to append onto the end of @string
 *
 * Adds a byte onto the end of a #GString, expanding
 * it if necessary.
 *
 * Returns: @string
 */
#undef g_string_append_c
GString *
g_string_append_c (GString *string,
                   gchar    c)
{
  if( string == NULL)
    return NULL;

  return g_string_insert_c (string, -1, c);
}

/**
 * g_string_insert_c:
 * @string: a #GString
 * @pos: the position to insert the byte
 * @c: the byte to insert
 *
 * Inserts a byte into a #GString, expanding it if necessary.
 *
 * Returns: @string
 */
GString *
g_string_insert_c (GString *string,
                   gssize   pos,
                   gchar    c)
{
  if( string == NULL)
    return NULL;

  g_string_maybe_expand (string, 1);

  if (pos < 0)
    pos = string->len;
  else
  {
    if( pos > string->len)
      return string;
  }

  /* If not just an append, move the old stuff */
  if (pos < string->len)
    g_memmove (string->str + pos + 1, string->str + pos, string->len - pos);

  string->str[pos] = c;

  string->len += 1;

  string->str[string->len] = 0;

  return string;
}
