/* glibconfig.h
 *
 * This is a generated file.  Please modify 'configure.ac'
 */

#ifndef __GLIBCONFIG_H__
#define __GLIBCONFIG_H__

#include "gmacros.h"

G_BEGIN_DECLS

typedef signed long gssize;
typedef unsigned long gsize;

#define G_MAXSIZE   G_MAXULONG

#define g_memmove(dest,src,len) G_STMT_START { memmove ((dest), (src), (len)); } G_STMT_END

G_END_DECLS

#endif /* __GLIBCONFIG_H__ */
