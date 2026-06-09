//> Strings object-c
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
//> Hash Tables object-include-table
#include "table.h"
//< Hash Tables object-include-table
#include "value.h"
#include "vm.h"
//> allocate-obj
#include "common.h"
#include "log.h"

#define ALLOCATE_OBJ(type, objectType) (type*)allocateObject(sizeof(type), objectType)
//< allocate-obj
//> allocate-object

static Obj* allocateObject(size_t size, ObjType type)
{
    Obj* object = (Obj*)reallocate(NULL, 0, size);
    object->type = type;
    //> Garbage Collection init-is-marked
    object->isMarked = false;
    //< Garbage Collection init-is-marked
    //> add-to-list

    object->next = vm.objects;
    vm.objects = object;
    //< add-to-list
    //> Garbage Collection debug-log-allocate

#ifdef DEBUG_LOG_GC
    printf("%p allocate %zu for %d\n", (void*)object, size, type);
#endif

    //< Garbage Collection debug-log-allocate
    return object;
}
//< allocate-object
//> Methods and Initializers new-bound-method
ObjBoundMethod* newBoundMethod(Value receiver, ObjClosure* method)
{
    ObjBoundMethod* bound = ALLOCATE_OBJ(ObjBoundMethod, OBJ_BOUND_METHOD);
    bound->receiver = receiver;
    bound->method = method;
    return bound;
}
//< Methods and Initializers new-bound-method
//> Classes and Instances new-class
ObjClass* newClass(ObjString* name)
{
    ObjClass* klass = ALLOCATE_OBJ(ObjClass, OBJ_CLASS);
    klass->name = name; // [klass]
                        //> Methods and Initializers init-methods
    initTable(&klass->methods);
    //< Methods and Initializers init-methods
    klass->call = NULL;
    return klass;
}
//< Classes and Instances new-class
//> Closures new-closure
ObjClosure* newClosure(ObjFunction* function)
{
    //> allocate-upvalue-array
    ObjUpvalue** upvalues = ALLOCATE(ObjUpvalue*, function->upvalueCount);
    for (int i = 0; i < function->upvalueCount; i++) {
        upvalues[i] = NULL;
    }

    //< allocate-upvalue-array
    ObjClosure* closure = ALLOCATE_OBJ(ObjClosure, OBJ_CLOSURE);
    closure->function = function;
    //> init-upvalue-fields
    closure->upvalues = upvalues;
    closure->upvalueCount = function->upvalueCount;
    //< init-upvalue-fields
    return closure;
}
//< Closures new-closure
//> Calls and Functions new-function
ObjFunction* newFunction()
{
    ObjFunction* function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
    function->arity = 0;
    //> Closures init-upvalue-count
    function->upvalueCount = 0;
    //< Closures init-upvalue-count
    function->name = NULL;
    initChunk(&function->chunk);
    return function;
}
//< Calls and Functions new-function
//> Classes and Instances new-instance
ObjInstance* newInstance(ObjClass* klass)
{
    ObjInstance* instance = ALLOCATE_OBJ(ObjInstance, OBJ_INSTANCE);
    instance->klass = klass;
    initTable(&instance->fields);
    return instance;
}
//< Classes and Instances new-instance
//> Calls and Functions new-native
ObjNative* newNative(NativeFn function)
{
    ObjNative* native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
    native->function = function;
    return native;
}
//< Calls and Functions new-native
ObjNativeBoundMethod* newNativeBoundMethod(NativeBoundMethod function)
{
    ObjNativeBoundMethod* native = ALLOCATE_OBJ(ObjNativeBoundMethod, OBJ_NATIVE_BOUND_METHOD);
    native->method = function;
    return native;
}

/* Strings allocate-string < Hash Tables allocate-string
static ObjString* allocateString(char* chars, int length) {
*/
//> allocate-string
//> Hash Tables allocate-string
static ObjString* allocateString(char* chars, int length, uint32_t hash)
{
    //< Hash Tables allocate-string
    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->length = length;
    string->chars = chars;
    //> Hash Tables allocate-store-hash
    string->hash = hash;
    //< Hash Tables allocate-store-hash
    //> Hash Tables allocate-store-string
    //> Garbage Collection push-string

    push(OBJ_VAL(string));
    //< Garbage Collection push-string
    tableSet(&vm.strings, string, NIL_VAL);
    //> Garbage Collection pop-string
    pop();

    //< Garbage Collection pop-string
    //< Hash Tables allocate-store-string
    return string;
}
//< allocate-string
//> Hash Tables hash-string
static uint32_t hashString(const char* key, int length)
{
    uint32_t hash = 2166136261u;
    for (int i = 0; i < length; i++) {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }
    return hash;
}
//< Hash Tables hash-string
//> take-string
ObjString* takeString(char* chars, int length)
{
    /* Strings take-string < Hash Tables take-string-hash
      return allocateString(chars, length);
    */
    //> Hash Tables take-string-hash
    uint32_t hash = hashString(chars, length);
    //> take-string-intern
    ObjString* interned = tableFindString(&vm.strings, chars, length, hash);
    if (interned != NULL) {
        FREE_ARRAY(char, chars, length + 1);
        return interned;
    }

    //< take-string-intern
    return allocateString(chars, length, hash);
    //< Hash Tables take-string-hash
}
//< take-string
ObjString* copyString(const char* chars, int length)
{
    //> Hash Tables copy-string-hash
    uint32_t hash = hashString(chars, length);
    //> copy-string-intern
    ObjString* interned = tableFindString(&vm.strings, chars, length, hash);
    if (interned != NULL)
        return interned;

    //< copy-string-intern
    //< Hash Tables copy-string-hash
    char* heapChars = ALLOCATE(char, length + 1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';
    /* Strings object-c < Hash Tables copy-string-allocate
      return allocateString(heapChars, length);
    */
    //> Hash Tables copy-string-allocate
    return allocateString(heapChars, length, hash);
    //< Hash Tables copy-string-allocate
}
//> Closures new-upvalue
ObjUpvalue* newUpvalue(Value* slot)
{
    ObjUpvalue* upvalue = ALLOCATE_OBJ(ObjUpvalue, OBJ_UPVALUE);
    //> init-closed
    upvalue->closed = NIL_VAL;
    //< init-closed
    upvalue->location = slot;
    //> init-next
    upvalue->next = NULL;
    //< init-next
    return upvalue;
}
//< Closures new-upvalue
ObjComplex* newComplex(double real, double imag)
{
    ObjComplex* cmplx = ALLOCATE_OBJ(ObjComplex, OBJ_COMPLEX);
    cmplx->real = real;
    cmplx->imag = imag;
    return cmplx;
}
Value list_indexof(Value receiver, int argc, Value* argv)
{
    int index = 0;
    if (1 != argc) {
        runtimeError("Expects 2 arguments");
        return NIL_VAL;
    }
    ObjList* list = AS_LIST(receiver);
    for (index = 0; index < list->array.count; index++) {
        if (valuesEqual(argv[0], list->array.values[index]))
            break;
    }
    if (index == list->array.count)
        index = -1;
    return NUMBER_VAL(index);
}
Value list_clear(Value receiver, int argc, Value* argv)
{
    (void)argc;
    (void)argv;
    ObjList* list = AS_LIST(receiver);
    freeValueArray(&list->array);
    initValueArray(&list->array);
    return NIL_VAL;
}
Value list_size(Value receiver, int argc, Value* argv)
{
    (void)argc;
    (void)argv;
    ObjList* list = AS_LIST(receiver);
    return NUMBER_VAL(list->array.count);
}
Value list_contains(Value receiver, int argc, Value* argv)
{
    bool tf = false;
    if (1 != argc) {
        runtimeError("Expects 2 arguments");
        return NIL_VAL;
    }
    ObjList* list = AS_LIST(receiver);
    for (int index = 0; index < list->array.count; index++) {
        if (valuesEqual(list->array.values[index], argv[0])) {
            tf = true;
            break;
        }
    }
    return BOOL_VAL(tf);
}
Value list_insert(Value receiver, int argc, Value* argv)
{
    LAX_LOG("list_insert(%d)", argc);
    if (2 != argc) {
        runtimeError("Expects 2 arguments");
        return NIL_VAL;
    }
    if (!IS_NUMBER(argv[0])) {
        runtimeError("Index must be a number.");
        return NIL_VAL;
    }
    double index = AS_NUMBER(argv[0]);
    ObjList* list = AS_LIST(receiver);
    if (0 > index)
        index += list->array.count + 1;
    if (index < 0 || list->array.count < index) {
        runtimeError("Index is out of bounds.");
        return NIL_VAL;
    }
    if (index != (int)index) {
        runtimeError("Index must be an integer.");
        return NIL_VAL;
    }
    insertValueArray(&list->array, index, argv[1]);
    LAX_LOG("list size: %d", list->array.count);
    return argv[1];
}
Value list_add(Value receiver, int argc, Value* argv)
{
    LAX_LOG("list_add(%d)", argc);
    if (0 == argc)
        return NIL_VAL;
    ObjList* list = AS_LIST(receiver);
    for (int index = 0; index < argc; index++)
        writeValueArray(&list->array, argv[index]);
    LAX_LOG("list size: %d", list->array.count);
    return argv[0];
}
static Value list_new(Value receiver, int argc, Value* argv)
{
    LAX_LOG("list_new(%d)", argc);
    ObjList* list = ALLOCATE_OBJ(ObjList, OBJ_LIST);
    list->klass = AS_CLASS(receiver);
    initValueArray(&list->array);
    if (0 < argc)
        for (int index = 0; index < argc; index++)
            writeValueArray(&list->array, argv[index]);
    LAX_LOG("list type: %d", list->obj.type);
    return OBJ_VAL(list);
}
ObjClass* newListClass()
{
    ObjString* name = copyString("List", 4);
    ObjClass* klass = newClass(name);
    ObjString* method = copyString("add", 3);
    tableSet(&klass->methods, method, OBJ_VAL(newNativeBoundMethod(list_add)));
    method = copyString("size", 4);
    tableSet(&klass->methods, method, OBJ_VAL(newNativeBoundMethod(list_size)));
    method = copyString("insert", 6);
    tableSet(&klass->methods, method, OBJ_VAL(newNativeBoundMethod(list_insert)));
    method = copyString("contains", 8);
    tableSet(&klass->methods, method, OBJ_VAL(newNativeBoundMethod(list_contains)));
    method = copyString("clear", 5);
    tableSet(&klass->methods, method, OBJ_VAL(newNativeBoundMethod(list_clear)));
    method = copyString("indexOf", 7);
    tableSet(&klass->methods, method, OBJ_VAL(newNativeBoundMethod(list_indexof)));
    klass->call = newNativeBoundMethod(list_new);
    return klass;
}
Value map_remove(Value receiver, int argc, Value* argv)
{
    LAX_LOG("map_remove(%d)", argc);
    if (0 == argc)
        return NIL_VAL;
    ObjMap* map = AS_MAP(receiver);
    for (int index = 0; index < argc; index++) {
        if (!IS_STRING(argv[index])) {
            runtimeError("Exepects a string");
            return NIL_VAL;
        }
        tableDelete(&map->map, AS_STRING(argv[index]));
    }
    return NIL_VAL;
}
static Value map_size(Value receiver, int argc, Value* argv)
{
    (void)receiver;
    (void)argc;
    (void)argv;
    ObjMap* map = AS_MAP(receiver);
    int size = 0;
    for (int i = 0; i < map->map.capacity; i++) {
        if (map->map.entries[i].key)
            size++;
    }
    return NUMBER_VAL(size);
}
static Value map_new(Value receiver, int argc, Value* argv)
{
    (void)argc;
    (void)argv;
    LAX_LOG("map_new(%d)", argc);
    ObjMap* map = ALLOCATE_OBJ(ObjMap, OBJ_MAP);
    map->klass = AS_CLASS(receiver);
    initTable(&map->map);
    return OBJ_VAL(map);
}
ObjClass* newMapClass()
{
    ObjString* name = copyString("Map", 3);
    ObjClass* klass = newClass(name);
    ObjString* method = copyString("remove", 6);
    tableSet(&klass->methods, method, OBJ_VAL(newNativeBoundMethod(map_remove)));
    method = copyString("size", 4);
    tableSet(&klass->methods, method, OBJ_VAL(newNativeBoundMethod(map_size)));
    klass->call = newNativeBoundMethod(map_new);
    return klass;
}
static Value num_fraction(Value receiver, int argc, Value* argv)
{
    (void)argc;
    (void)argv;
    double v = AS_NUMBER(receiver);
    return NUMBER_VAL(v - (int)v);
}
static Value num_truncate(Value receiver, int argc, Value* argv)
{
    (void)argc;
    (void)argv;
    return NUMBER_VAL(trunc(AS_NUMBER(receiver)));
}
static Value num_round(Value receiver, int argc, Value* argv)
{
    (void)argc;
    (void)argv;
    return NUMBER_VAL(round(AS_NUMBER(receiver)));
}
static Value num_floor(Value receiver, int argc, Value* argv)
{
    (void)argc;
    (void)argv;
    return NUMBER_VAL(floor(AS_NUMBER(receiver)));
}
static Value num_ceil(Value receiver, int argc, Value* argv)
{
    (void)argc;
    (void)argv;
    return NUMBER_VAL(ceil(AS_NUMBER(receiver)));
}
static Value num_abs(Value receiver, int argc, Value* argv)
{
    (void)argc;
    (void)argv;
    return NUMBER_VAL(fabs(AS_NUMBER(receiver)));
}
static Value num_sign(Value receiver, int argc, Value* argv)
{
    (void)argc;
    (void)argv;
    int d = 0;
    double v = AS_NUMBER(receiver);
    if (0 > v)
        d = -1;
    if (0 < v)
        d = 1;
    return NUMBER_VAL(d);
}
ObjClass* newNumClass()
{
    ObjString* name = copyString("Number", 6);
    ObjClass* klass = newClass(name);
    ObjString* method = copyString("abs", 3);
    tableSet(&klass->methods, method, OBJ_VAL(newNativeBoundMethod(num_abs)));
    method = copyString("sign", 4);
    tableSet(&klass->methods, method, OBJ_VAL(newNativeBoundMethod(num_sign)));
    method = copyString("ceil", 4);
    tableSet(&klass->methods, method, OBJ_VAL(newNativeBoundMethod(num_ceil)));
    method = copyString("floor", 5);
    tableSet(&klass->methods, method, OBJ_VAL(newNativeBoundMethod(num_floor)));
    method = copyString("round", 5);
    tableSet(&klass->methods, method, OBJ_VAL(newNativeBoundMethod(num_round)));
    method = copyString("truncate", 8);
    tableSet(&klass->methods, method, OBJ_VAL(newNativeBoundMethod(num_truncate)));
    method = copyString("fraction", 8);
    tableSet(&klass->methods, method, OBJ_VAL(newNativeBoundMethod(num_fraction)));
    return klass;
}
//> Calls and Functions print-function-helper
static void printFunction(ObjFunction* function)
{
    //> print-script
    if (function->name == NULL) {
        printf("<script>");
        return;
    }
    //< print-script
    printf("<fn %s>", function->name->chars);
}
//< Calls and Functions print-function-helper
//> print-object
void printObject(Value value)
{
    switch (OBJ_TYPE(value)) {
        //> Methods and Initializers print-bound-method
    case OBJ_BOUND_METHOD:
        printFunction(AS_BOUND_METHOD(value)->method->function);
        break;
        //< Methods and Initializers print-bound-method
        //> Classes and Instances print-class
    case OBJ_CLASS:
        printf("%s", AS_CLASS(value)->name->chars);
        break;
        //< Classes and Instances print-class
        //> Closures print-closure
    case OBJ_CLOSURE:
        printFunction(AS_CLOSURE(value)->function);
        break;
        //< Closures print-closure
        //> Calls and Functions print-function
    case OBJ_FUNCTION:
        printFunction(AS_FUNCTION(value));
        break;
        //< Calls and Functions print-function
        //> Classes and Instances print-instance
    case OBJ_INSTANCE:
        printf("%s instance", AS_INSTANCE(value)->klass->name->chars);
        break;
        //< Classes and Instances print-instance
        //> Calls and Functions print-native
    case OBJ_NATIVE:
        printf("<native fn>");
        break;
        //< Calls and Functions print-native
    case OBJ_NATIVE_BOUND_METHOD:
        printf("<native method>");
        break;
    case OBJ_STRING:
        printf("%s", AS_CSTRING(value));
        break;
        //> Closures print-upvalue
    case OBJ_UPVALUE:
        printf("<upvalue>");
        break;
        //< Closures print-upvalue
    case OBJ_COMPLEX: {
        ObjComplex* c = AS_COMPLEX(value);
        if (c->real) {
            printf("%.14g", c->real);
        }
        if (0 < c->imag) {
            if (c->real)
                printf("+");
        }
        else {
            printf("-");
        }
        printf("%.14gj", c->imag);
        break;
    }
    case OBJ_LIST: {
        ObjList* list = AS_LIST(value);
        printf("[");
        for (int i = 0; i < list->array.count; i++) {
            if (0 < i)
                printf(",");
            printValue(list->array.values[i]);
        }
        printf("]");
        break;
    }
    case OBJ_MAP: {
        ObjMap* map = AS_MAP(value);
        printf("{");
        bool found = false;
        for (int i = 0; i < map->map.capacity; i++) {
            if (map->map.entries[i].key) {
                if (found)
                    printf(",");
                printf("\"%s\":", map->map.entries[i].key->chars);
                Value v = map->map.entries[i].value;
                if (IS_STRING(v))
                    printf("\"");
                printValue(v);
                if (IS_STRING(v))
                    printf("\"");
                found = true;
            }
        }
        printf("}");
        break;
    }
    }
}

bool objectsEqual(Obj* a, Obj* b)
{
    bool retVal = false;
    if (a->type != b->type)
        return retVal;
    switch (a->type) {
    case OBJ_STRING: {
        ObjString* stra = (ObjString*)a;
        ObjString* strb = (ObjString*)b;
        if (stra->hash == strb->hash && !strcmp(stra->chars, strb->chars))
            retVal = false;
        break;
    }
    case OBJ_LIST: {
        retVal = true;
        ObjList* lsta = (ObjList*)a;
        ObjList* lstb = (ObjList*)b;
        if (lsta->array.count != lstb->array.count) {
            retVal = false;
            break;
        }
        else {
            for (int index = 0; index < lsta->array.count; index++) {
                if (!valuesEqual(lsta->array.values[index], lstb->array.values[index])) {
                    retVal = false;
                    break;
                }
            }
        }
        break;
    }
    default:
        retVal = true;
    }
    return retVal;
}
//< print-object
