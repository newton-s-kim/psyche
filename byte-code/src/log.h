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
#define LAX_LOG_ARRAY(a)                                                                                               \
    {                                                                                                                  \
        printf("[");                                                                                                   \
        for (int i = 0; i < (a).count; i++) {                                                                          \
            if (i)                                                                                                     \
                printf(", ");                                                                                          \
            printValue((a).values[i]);                                                                                 \
        }                                                                                                              \
        printf("]\n");                                                                                                 \
    }
#else // QP_DEBUG
#define LAX_LOG(...)
#define LAX_LOG_ARRAY(a)
#endif // QP_DEBUG
