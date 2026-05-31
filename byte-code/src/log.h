#pragma once

#include <stdio.h>

#define RETURN_NULL return NULL;

#ifdef LAX_DEBUG
#define LAX_LOG(...)                                                                                                   \
    {                                                                                                                  \
        fprintf(stderr, "%s(%d):%s  ", __FILE__, __LINE__, __FUNCTION__);                                              \
        fprintf(stderr, ##__VA_ARGS__);                                                                                \
        fprintf(stderr, "\n");                                                                                         \
    }
#else // QP_DEBUG
#define LAX_LOG(...)
#endif // QP_DEBUG
