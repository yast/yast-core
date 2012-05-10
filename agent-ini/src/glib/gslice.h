/* GLIB sliced memory - fast threaded memory chunk allocator
 * Copyright (C) 2005 Tim Janik
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

#ifndef __G_SLICE_H__
#define __G_SLICE_H__

#include "gtypes.h"

G_BEGIN_DECLS

/* slices - fast allocation/release of small memory blocks
 */
gpointer g_slice_alloc          	(gsize	       block_size) G_GNUC_MALLOC G_GNUC_ALLOC_SIZE(1);
void     g_slice_free1          	(gsize         block_size,
					 gpointer      mem_block);
#define  g_slice_new(type)      ((type*) g_slice_alloc (sizeof (type)))
#define g_slice_free(type, mem)				do {	\
  if (1) g_slice_free1 (sizeof (type), (mem));			\
  else   (void) ((type*) 0 == (mem)); 				\
} while (0)

G_END_DECLS

#endif /* __G_SLICE_H__ */
