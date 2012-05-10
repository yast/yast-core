/* gshell.c - Shell-related utilities
 *
 *  Copyright 2000 Red Hat, Inc.
 *  g_execvpe implementation based on GNU libc execvp:
 *   Copyright 1991, 92, 95, 96, 97, 98, 99 Free Software Foundation, Inc.
 *
 * GLib is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * GLib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GLib; see the file COPYING.LIB.  If not, write
 * to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <string.h>

#include "gshell.h"

#include "gstrfuncs.h"
#include "gstring.h"
#include "gmem.h"

static gboolean 
unquote_string_inplace (gchar* str, gchar** end)
{
  gchar* dest;
  gchar* s;
  gchar quote_char;
  
  if( ( end == NULL) || ( str == NULL))
    return FALSE;

  dest = s = str;

  quote_char = *s;
  
  if (!(*s == '"' || *s == '\''))
    {
      *end = str;
      return FALSE;
    }

  /* Skip the initial quote mark */
  ++s;

  if (quote_char == '"')
    {
      while (*s)
        {
          switch (*s)
            {
            case '"':
              /* End of the string, return now */
              *dest = '\0';
              ++s;
              *end = s;
              return TRUE;
              break;

            case '\\':
              /* Possible escaped quote or \ */
              ++s;
              switch (*s)
                {
                case '"':
                case '\\':
                case '`':
                case '$':
                case '\n':
                  *dest = *s;
                  ++s;
                  ++dest;
                  break;

                default:
                  /* not an escaped char */
                  *dest = '\\';
                  ++dest;
                  /* ++s already done. */
                  break;
                }
              break;

            default:
              *dest = *s;
              ++dest;
              ++s;
              break;
            }
        }
    }
  else
    {
      int escape = 0;

      while (*s)
        {
          if ( (*s == '\'') && !escape)
            {
              /* End of the string, return now */
              *dest = '\0';
              ++s;
              *end = s;
              return TRUE;
            }
          else
            {
              escape = *s == '\\' ? 1 : 0;
              *dest = *s;
              ++dest;
              ++s;
            }
        }
    }
  
  /* If we reach here this means the close quote was never encountered */

  *dest = '\0';
  *end = s;

  return FALSE;
}

/**
 * g_shell_quote:
 * @unquoted_string: a literal string
 * 
 * Quotes a string so that the shell (/bin/sh) will interpret the
 * quoted string to mean @unquoted_string. If you pass a filename to
 * the shell, for example, you should first quote it with this
 * function.  The return value must be freed with g_free(). The
 * quoting style used is undefined (single or double quotes may be
 * used).
 * 
 * Return value: quoted string
 **/
gchar*
g_shell_quote (const gchar *unquoted_string)
{
  /* We always use single quotes, because the algorithm is cheesier.
   * We could use double if we felt like it, that might be more
   * human-readable.
   */

  const gchar *p;
  GString *dest;

  if( unquoted_string == NULL)
    return NULL;
  
  dest = g_string_new ("'");

  p = unquoted_string;

  /* could speed this up a lot by appending chunks of text at a
   * time.
   */
  while (*p)
    {
      /* Replace literal ' with a close ', a \', and a open ' */
      if (*p == '\'')
        g_string_append (dest, "'\\''");
      else
        g_string_append_c (dest, *p);

      ++p;
    }

  /* close the quote */
  g_string_append_c (dest, '\'');
  
  return g_string_free (dest, FALSE);
}

/**
 * g_shell_unquote:
 * @quoted_string: shell-quoted string
 * @error: error return location or NULL
 * 
 * Unquotes a string as the shell (/bin/sh) would. Only handles
 * quotes; if a string contains file globs, arithmetic operators,
 * variables, backticks, redirections, or other special-to-the-shell
 * features, the result will be different from the result a real shell
 * would produce (the variables, backticks, etc. will be passed
 * through literally instead of being expanded). This function is
 * guaranteed to succeed if applied to the result of
 * g_shell_quote(). If it fails, it returns %NULL and sets the
 * error. The @quoted_string need not actually contain quoted or
 * escaped text; g_shell_unquote() simply goes through the string and
 * unquotes/unescapes anything that the shell would. Both single and
 * double quotes are handled, as are escapes including escaped
 * newlines. The return value must be freed with g_free(). Possible
 * errors are in the #G_SHELL_ERROR domain.
 * 
 * Shell quoting rules are a bit strange. Single quotes preserve the
 * literal string exactly. escape sequences are not allowed; not even
 * \' - if you want a ' in the quoted text, you have to do something
 * like 'foo'\''bar'.  Double quotes allow $, `, ", \, and newline to
 * be escaped with backslash. Otherwise double quotes preserve things
 * literally.
 *
 * Return value: an unquoted string
 **/
gchar*
g_shell_unquote (const gchar *quoted_string)
{
  gchar *unquoted;
  gchar *end;
  gchar *start;
  GString *retval;
  
  if( quoted_string == NULL)
    return NULL;
  
  unquoted = g_strdup (quoted_string);

  start = unquoted;
  end = unquoted;
  retval = g_string_new (NULL);

  /* The loop allows cases such as
   * "foo"blah blah'bar'woo foo"baz"la la la\'\''foo'
   */
  while (*start)
    {
      /* Append all non-quoted chars, honoring backslash escape
       */
      
      while (*start && !(*start == '"' || *start == '\''))
        {
          if (*start == '\\')
            {
              /* all characters can get escaped by backslash,
               * except newline, which is removed if it follows
               * a backslash outside of quotes
               */
              
              ++start;
              if (*start)
                {
                  if (*start != '\n')
                    g_string_append_c (retval, *start);
                  ++start;
                }
            }
          else
            {
              g_string_append_c (retval, *start);
              ++start;
            }
        }

      if (*start)
        {
          if (!unquote_string_inplace (start, &end))
            {
              goto error;
            }
          else
            {
              g_string_append (retval, start);
              start = end;
            }
        }
    }

  g_free (unquoted);
  return g_string_free (retval, FALSE);
  
 error:
  g_free (unquoted);
  g_string_free (retval, TRUE);

  return NULL;
}
