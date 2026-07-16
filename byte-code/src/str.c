#include "str.h"
#include "common.h"
#include "log.h"

#include "psmalloc.h"

#include <stdlib.h>
#include <string.h>

String* strNewWithLength(const char* str, const size_t length)
{
    LAX_LOG("strNewWithLength(%s,%lu)", str, length);
    if (NULL == str)
        return NULL;
    String* ret = PMALLOC(sizeof(String));
    ret->size = length;
    ret->c_str = PCALLOC(ret->size + 1, sizeof(char));
    strncpy(ret->c_str, str, length);
    return ret;
}

String* strNew(const char* str)
{
    return strNewWithLength(str, strlen(str));
}

void strFree(String* str)
{
    if (!str)
        return;
    if (str->c_str)
        PFREE(str->c_str);
    PFREE(str);
}

bool strAppend(String* str, const char* cstr)
{
    if (!str)
        return false;
    if (!str->c_str)
        return false;
    if (!cstr)
        return false;
    str->size += strlen(cstr);
    str->c_str = PREALLOC(str->c_str, str->size + 1);
    strcat(str->c_str, cstr);
    str->c_str[str->size] = 0;
    return true;
}
