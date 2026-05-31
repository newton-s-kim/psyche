#pragma once

#include "object.h"
#include "path.h"
#include "str.h"

typedef void (*libsym)(const char***, Value**);

typedef struct {
    void* m_handle;
    libsym m_sym;
    String* m_sym_name;
    String* m_name;
} DL;
DL* dlNew(Path* path, String* name);
void dlFree(DL* dl);
bool dlSymbols(DL* dl, const char*** names, Value** values);
