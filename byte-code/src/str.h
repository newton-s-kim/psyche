#ifndef clox_str_h
#define clox_str_h

#include <stddef.h>

typedef struct {
    char* c_str;
    int size;
} String;

String* strNew(const char* str);
String* strNewWithLength(const char* str, const size_t length);
void strFree(String* str);
bool strAppend(String* str, const char* cstr);
#endif
