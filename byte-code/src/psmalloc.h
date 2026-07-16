#ifndef __PS_MALLOC_H__
#define __PS_MALLOC_H__

#include "common.h"

#ifdef USE_DLMALLOC
#include "dlmalloc/malloc.h"
#define PMALLOC dlmalloc
#define PCALLOC dlcalloc
#define PREALLOC dlrealloc
#define PFREE dlfree
#else // USE_DLMALLOC
#include <stdlib.h>
#define PMALLOC malloc
#define PCALLOC calloc
#define PREALLOC realloc
#define PFREE free
#endif // USE_DLMALLOC

#endif //__PS_MALLOC_H__
