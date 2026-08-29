#include "memory.h"
#include "object.h"
#include "vm.h"

Value sys_gc(int argc, Value* argv)
{
    (void)argc;
    (void)argv;
    collectGarbage();
    return NIL_VAL;
}
ObjClass* newSystemClass()
{
    ObjString* name = copyString("System", 6);
    ObjClass* klass = newClass(name);
    int method = getMethodAddress(copyString("gc", 2));
    setAtValueArray(&klass->staticMethods, method, OBJ_VAL(newNative(sys_gc)));
    return klass;
}

void system_symbols(const char*** names, Value** values)
{
    static const char* system_names[] = {"System", NULL};
    static Value system_syms[1];
    int cnt = 0;

    system_syms[cnt++] = OBJ_VAL(newSystemClass());

    *names = system_names;
    *values = system_syms;
}
