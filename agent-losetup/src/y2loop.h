#include <linux/posix_types.h>
#undef dev_t
#define dev_t __kernel_old_dev_t
#include "loop.h"
#undef dev_t
