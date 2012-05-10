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

#include "gmem.h"

#include <stdlib.h>

#define standard_malloc	malloc
#define standard_realloc	realloc
#define standard_free		free

/* --- variables --- */
static GMemVTable glib_mem_vtable = {
  standard_malloc,
  standard_realloc,
  standard_free,
  NULL,
  NULL,
  NULL
};

/**
 * SECTION:memory
 * @Short_Description: general memory-handling
 * @Title: Memory Allocation
 * 
 * These functions provide support for allocating and freeing memory.
 * 
 * <note>
 * If any call to allocate memory fails, the application is terminated.
 * This also means that there is no need to check if the call succeeded.
 * </note>
 * 
 * <note>
 * It's important to match g_malloc() with g_free(), plain malloc() with free(),
 * and (if you're using C++) new with delete and new[] with delete[]. Otherwise
 * bad things can happen, since these allocators may use different memory
 * pools (and new/delete call constructors and destructors). See also
 * g_mem_set_vtable().
 * </note>
 */

/* --- functions --- */
/**
 * g_malloc:
 * @n_bytes: the number of bytes to allocate
 * 
 * Allocates @n_bytes bytes of memory.
 * If @n_bytes is 0 it returns %NULL.
 * 
 * Returns: a pointer to the allocated memory
 */
gpointer
g_malloc (gsize n_bytes)
{
  if (G_LIKELY (n_bytes))
    {
      gpointer mem;

      mem = glib_mem_vtable.malloc (n_bytes);
      if (mem)
	return mem;
    }

  return NULL;
}

/**
 * g_realloc:
 * @mem: the memory to reallocate
 * @n_bytes: new size of the memory in bytes
 * 
 * Reallocates the memory pointed to by @mem, so that it now has space for
 * @n_bytes bytes of memory. It returns the new address of the memory, which may
 * have been moved. @mem may be %NULL, in which case it's considered to
 * have zero-length. @n_bytes may be 0, in which case %NULL will be returned
 * and @mem will be freed unless it is %NULL.
 * 
 * Returns: the new address of the allocated memory
 */
gpointer
g_realloc (gpointer mem,
	   gsize    n_bytes)
{
  gpointer newmem;

  if (G_LIKELY (n_bytes))
    {
      newmem = glib_mem_vtable.realloc (mem, n_bytes);
      if (newmem)
	return newmem;
    }

  if (mem)
    glib_mem_vtable.free (mem);

  return NULL;
}

/**
 * g_free:
 * @mem: the memory to free
 * 
 * Frees the memory pointed to by @mem.
 * If @mem is %NULL it simply returns.
 */
void
g_free (gpointer mem)
{
  if (G_LIKELY (mem))
    glib_mem_vtable.free (mem);
}

/**
 * g_malloc_n:
 * @n_blocks: the number of blocks to allocate
 * @n_block_bytes: the size of each block in bytes
 * 
 * This function is similar to g_malloc(), allocating (@n_blocks * @n_block_bytes) bytes,
 * but care is taken to detect possible overflow during multiplication.
 * 
 * Since: 2.24
 * Returns: a pointer to the allocated memory
 */
gpointer
g_malloc_n (gsize n_blocks,
	    gsize n_block_bytes)
{
  return g_malloc (n_blocks * n_block_bytes);
}
