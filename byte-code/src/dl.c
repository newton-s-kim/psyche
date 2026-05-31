#include "dl.h"

#include <dlfcn.h>
#include <stdlib.h>

DL* dlNew(Path* path, String* name)
{
    DL* dl = malloc(sizeof(DL));
    memset(dl, 0, sizeof(DL));
    dl->m_handle = dlopen(pathToString(path), RTLD_LAZY);
    if (!dl->m_handle) {
        free(dl);
        return NULL;
    }

    dl->m_name = name;
    dl->m_sym_name = strNew(name->c_str);
    strAppend(dl->m_sym_name, "_symbols");
    return dl;
}

void dlFree(DL* dl)
{
    if (!dl)
        return;
    if (dl->m_handle)
        dlclose(dl->m_handle);
    if (dl->m_name)
        strFree(dl->m_name);
    if (dl->m_sym_name)
        strFree(dl->m_sym_name);
    free(dl);
}

bool dlSymbols(DL* dl, const char*** names, Value** values)
{
    if (NULL == dl->m_sym) {
        dl->m_sym = (libsym)dlsym(dl->m_handle, dl->m_sym_name->c_str);
        if (!dl->m_sym)
            return false;
    }
    dl->m_sym(names, values);
    return true;
}
