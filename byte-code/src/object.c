//> Strings object-c
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
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
#include "psmalloc.h"

#include <cblas.h>

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
    initValueArray(&klass->methods);
    initValueArray(&klass->staticMethods);
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
    initValueArray(&instance->fields);
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
static int hexStrToInt(const char* chars, int length)
{
    int val = 0;
    int digit = 0;
    const char* e = chars + length;
    for (const char* p = chars; p < e; p++) {
        if ('0' <= *p && '9' >= *p)
            digit = *p - '0';
        else if ('a' <= *p && 'f' >= *p)
            digit = *p - 'a' + 10;
        else if ('A' <= *p && 'F' >= *p)
            digit = *p - 'A' + 10;
        else
            return -1;
        val <<= 4;
        val |= 0xf & digit;
    }
    LAX_LOG("->0x%x", val);
    return val;
}
static int intToUtf8(char* chars, int v)
{
    int adv = 0;
    if (0x7f >= v) {
        *chars = v & 0x7f;
        adv = 1;
    }
    else if (0x7ff >= v) {
        // 11 bits art spreaded over 2 bytes: 110xxxxx 10xxxxxx
        *chars++ = 0xc0 | ((v & 0x7c0) >> 6);
        *chars = 0x80 | (v & 0x3f);
        adv = 2;
    }
    else if (0xffff >= v) {
        // 16 bits are spreaded over 3 bytes: 1110xxxx 10xxxxxx 10xxxxxx
        *chars++ = 0xe0 | ((v & 0xf000) >> 12);
        *chars++ = 0x80 | ((v & 0xfc0) >> 6);
        *chars = 0x80 | ((v & 0x3f));
        adv = 3;
    }
    else if (0x10ffff >= v) {
        // 21 bits are spreaded over 4 bytes: 1110xxxx 10xxxxxx 10xxxxxx
        *chars++ = 0xf0 | ((v & 0x1c00000) >> 18);
        *chars++ = 0x80 | ((v & 0x3f000) >> 12);
        *chars++ = 0x80 | ((v & 0xfc0) >> 6);
        *chars = 0x80 | ((v & 0x3f));
        adv = 4;
    }
    return adv;
}
ObjString* copyString(const char* chars, int length)
{
    // TODO:unify hash
    //> Hash Tables copy-string-hash
    uint32_t hash = hashString(chars, length);
    //> copy-string-intern
    ObjString* interned = tableFindString(&vm.strings, chars, length, hash);
    if (interned != NULL)
        return interned;

    //< copy-string-intern
    //< Hash Tables copy-string-hash
    char* heapChars = ALLOCATE(char, length + 1);
    // memcpy(heapChars, chars, length);
    const char* e = chars + length;
    int index = 0;
    char esc = 0;
    for (const char* p = chars; p < e; p++) {
        if ('\\' == *p) {
            p++;
            switch (*p) {
            case '"':
                esc = '"';
                break;
            case '\\':
                esc = '\\';
                break;
            case '%':
                esc = '\%';
                break;
            case '0':
                esc = '\0';
                break;
            case 'a':
                esc = '\a';
                break;
            case 'e':
                esc = '\0';
                break;
            case 'f':
                esc = '\f';
                break;
            case 'n':
                esc = '\n';
                break;
            case 'r':
                esc = '\r';
                break;
            case 't':
                esc = '\t';
                break;
            case 'x':
                p++;
                esc = (char)hexStrToInt(p, 2);
                p += 1;
                break;
            case 'u': {
                p++;
                int l = hexStrToInt(p, 4);
                p += 3;
                index += intToUtf8(heapChars + index, l);
                continue;
                break;
            }
            case 'U': {
                p++;
                int l = hexStrToInt(p, 8);
                p += 7;
                index += intToUtf8(heapChars + index, l);
                continue;
                break;
            }
            }
            heapChars[index++] = esc;
        }
        else {
            heapChars[index++] = *p;
        }
    }
    heapChars[index] = '\0';
    /* Strings object-c < Hash Tables copy-string-allocate
      return allocateString(heapChars, length);
    */
    //> Hash Tables copy-string-allocate
    return allocateString(heapChars, index, hash);
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
ObjThread* newThread(ThreadType type)
{
    ObjThread* thread = ALLOCATE_OBJ(ObjThread, OBJ_THREAD);
    thread->type = type;
    thread->frameCount = 0;
    thread->stackTop = thread->stack;
    thread->caller = NULL;
    return thread;
}
Value list_each(Value receiver, int argc, Value* argv)
{
    int index = 0;
    if (1 != argc) {
        runtimeError("Expects 1 argument.");
        return NIL_VAL;
    }
    if (!IS_CLOSURE(argv[0])) {
        runtimeError("Expects closure.");
        return NIL_VAL;
    }
    ObjList* list = AS_LIST(receiver);
    Value args[2];
    for (index = 0; index < list->array.count; index++) {
        args[0] = NUMBER_VAL(index);
        args[1] = list->array.values[index];
        if (!runThread(argv[0], 2, args))
            return FALSE_VAL;
    }
    return TRUE_VAL;
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
static Value list_new(int argc, Value* argv)
{
    LAX_LOG("list_new(%d)", argc);
    ObjList* list = ALLOCATE_OBJ(ObjList, OBJ_LIST);
    initValueArray(&list->array);
    if (0 < argc)
        for (int index = 0; index < argc; index++)
            writeValueArray(&list->array, argv[index]);
    LAX_LOG("list type: %d", list->obj.type);
    return OBJ_VAL(list);
}
static Value list_filled(int argc, Value* argv)
{
    LAX_LOG("list_filled(%d)", argc);
    ObjList* list = ALLOCATE_OBJ(ObjList, OBJ_LIST);
    initValueArray(&list->array);
    if (2 != argc) {
        runtimeError("Invalid number of arguments.");
        return NIL_VAL;
    }
    if (!IS_NUMBER(argv[0])) {
        runtimeError("Size must be a number.");
        return NIL_VAL;
    }
    double num = AS_NUMBER(argv[0]);
    if (0 > num) {
        runtimeError("Size cannot be negative.");
        return NIL_VAL;
    }
    if (num != (int)num) {
        runtimeError("Size must be an integer.");
        return NIL_VAL;
    }
    for (int index = 0; index < num; index++)
        writeValueArray(&list->array, argv[1]);
    return OBJ_VAL(list);
}
ObjClass* newListClass()
{
    ObjString* name = copyString("List", 4);
    ObjClass* klass = newClass(name);
    int method = getMethodAddress(copyString("add", 3));
    setAtValueArray(&klass->methods, method, OBJ_VAL(newNativeBoundMethod(list_add)));
    method = getMethodAddress(copyString("size", 4));
    setAtValueArray(&klass->methods, method, OBJ_VAL(newNativeBoundMethod(list_size)));
    method = getMethodAddress(copyString("insert", 6));
    setAtValueArray(&klass->methods, method, OBJ_VAL(newNativeBoundMethod(list_insert)));
    method = getMethodAddress(copyString("contains", 8));
    setAtValueArray(&klass->methods, method, OBJ_VAL(newNativeBoundMethod(list_contains)));
    method = getMethodAddress(copyString("clear", 5));
    setAtValueArray(&klass->methods, method, OBJ_VAL(newNativeBoundMethod(list_clear)));
    method = getMethodAddress(copyString("indexOf", 7));
    setAtValueArray(&klass->methods, method, OBJ_VAL(newNativeBoundMethod(list_indexof)));
    method = getMethodAddress(copyString("each", 4));
    setAtValueArray(&klass->methods, method, OBJ_VAL(newNativeBoundMethod(list_each)));
    method = getMethodAddress(copyString("filled", 6));
    setAtValueArray(&klass->staticMethods, method, OBJ_VAL(newNative(list_filled)));
    klass->call = newNative(list_new);
    return klass;
}

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
static Value map_clear(Value receiver, int argc, Value* argv)
{
    (void)argc;
    (void)argv;
    ObjMap* map = AS_MAP(receiver);
    freeTable(&map->map);
    initTable(&map->map);
    return NIL_VAL;
}
static Value map_each(Value receiver, int argc, Value* argv)
{
    int index = 0;
    if (1 != argc) {
        runtimeError("Expects 1 argument.");
        return NIL_VAL;
    }
    if (!IS_CLOSURE(argv[0])) {
        runtimeError("Expects closure.");
        return NIL_VAL;
    }
    ObjMap* map = AS_MAP(receiver);
    Value args[2];
    for (index = 0; index < map->map.capacity; index++) {
        if (NULL == map->map.entries[index].key)
            continue;
        args[0] = OBJ_VAL(map->map.entries[index].key);
        args[1] = map->map.entries[index].value;
        if (!runThread(argv[0], 2, args))
            return FALSE_VAL;
    }
    return TRUE_VAL;
}
static Value map_new(int argc, Value* argv)
{
    (void)argc;
    (void)argv;
    LAX_LOG("map_new(%d)", argc);
    ObjMap* map = ALLOCATE_OBJ(ObjMap, OBJ_MAP);
    initTable(&map->map);
    if (0 < argc)
        for (int index = 0; index < argc; index += 2)
            tableSet(&map->map, AS_STRING(argv[index]), argv[index + 1]);
    return OBJ_VAL(map);
}
ObjClass* newMapClass()
{
    ObjString* name = copyString("Map", 3);
    ObjClass* klass = newClass(name);
    int method = getMethodAddress(copyString("remove", 6));
    setAtValueArray(&klass->methods, method, OBJ_VAL(newNativeBoundMethod(map_remove)));
    method = getMethodAddress(copyString("size", 4));
    setAtValueArray(&klass->methods, method, OBJ_VAL(newNativeBoundMethod(map_size)));
    method = getMethodAddress(copyString("clear", 5));
    setAtValueArray(&klass->methods, method, OBJ_VAL(newNativeBoundMethod(map_clear)));
    method = getMethodAddress(copyString("each", 4));
    setAtValueArray(&klass->methods, method, OBJ_VAL(newNativeBoundMethod(map_each)));
    klass->call = newNative(map_new);
    return klass;
}
ObjVector* duplicateVector(ObjVector* vector)
{
    ObjVector* result = ALLOCATE_OBJ(ObjVector, OBJ_VECTOR);
    result->isRow = vector->isRow;
    result->size = vector->size;
    result->values = PMALLOC(result->size * sizeof(double));
    memcpy(result->values, vector->values, result->size * sizeof(double));
    return result;
}
ObjVector* newVector(unsigned int size, double v)
{
    ObjVector* vector = ALLOCATE_OBJ(ObjVector, OBJ_VECTOR);
    vector->isRow = false;
    vector->size = size;
    vector->values = PMALLOC(vector->size * sizeof(double));
    if (0 == v) {
        memset(vector->values, 0, vector->size * sizeof(double));
    }
    else {
        for (int i = 0; i < vector->size; i++)
            vector->values[i] = v;
    }
    return vector;
}
static Value vector_transpose(Value receiver, int argc, Value* argv)
{
    (void)argc;
    (void)argv;
    ObjVector* vector = AS_VECTOR(receiver);
    ObjVector* result = duplicateVector(vector);
    result->isRow = (vector->isRow) ? false : true;
    return OBJ_VAL(result);
}
static Value vector_abs(Value receiver, int argc, Value* argv)
{
    (void)argc;
    (void)argv;
    ObjVector* vector = AS_VECTOR(receiver);
    double norm = cblas_dnrm2(vector->size, vector->values, 1);
    return NUMBER_VAL(norm);
}
static Value vector_dot(int argc, Value* argv)
{
    if (argc != 2) {
        runtimeError("Requires 2 arguments.");
        return NIL_VAL;
    }
    if (!IS_VECTOR(argv[0]) || !IS_VECTOR(argv[1])) {
        runtimeError("Vectors are expected.");
        return NIL_VAL;
    }
    ObjVector* x = AS_VECTOR(argv[0]);
    ObjVector* y = AS_VECTOR(argv[1]);
    if (x->size != y->size) {
        runtimeError("Vectors are not identical.");
        return NIL_VAL;
    }
    double result = cblas_ddot(x->size, x->values, 1, y->values, 1);
    return NUMBER_VAL(result);
}
static Value vector_cross(int argc, Value* argv)
{
    if (argc != 2) {
        runtimeError("Requires 2 arguments.");
        return NIL_VAL;
    }
    if (!IS_VECTOR(argv[0]) || !IS_VECTOR(argv[1])) {
        runtimeError("Vectors are expected.");
        return NIL_VAL;
    }
    ObjVector* x = AS_VECTOR(argv[0]);
    ObjVector* y = AS_VECTOR(argv[1]);
    if (x->size != y->size) {
        runtimeError("Vectors are not identical.");
        return NIL_VAL;
    }
    if (x->size != 3) {
        runtimeError("Vectors should be 3 dimensional");
        return NIL_VAL;
    }
    // Construct the 3x3 skew-symmetric matrix from vector 'a' in row-major order
    // clang-format off
    double A_skew[9] = {
         0.0,  -x->values[2],   x->values[1],
         x->values[2],   0.0,  -x->values[0],
        -x->values[1],   x->values[0],   0.0
    };
    // clang-format on

    ObjVector* result = ALLOCATE_OBJ(ObjVector, OBJ_VECTOR);
    result->isRow = false;
    result->size = 3;
    result->values = PMALLOC(result->size * sizeof(double));

    // Matrix-Vector Multiplication: result = 1.0 * A_skew * b + 0.0 * result
    cblas_dgemv(CblasRowMajor,    // Storage layout of matrix A
                CblasNoTrans,     // Do not transpose A_skew
                3, 3,             // Dimensions (Rows, Columns)
                1.0,              // Alpha multiplier
                A_skew, 3,        // Matrix A and its leading dimension (stride)
                y->values, 1,     // Vector b and its stride
                0.0,              // Beta multiplier
                result->values, 1 // Output vector and its stride
    );
    return OBJ_VAL(result);
}
static Value vector_new(int argc, Value* argv)
{
    LAX_LOG("vector_new(%d)", argc);
    ObjVector* vector = ALLOCATE_OBJ(ObjVector, OBJ_VECTOR);
    vector->isRow = false;
    if (0 < argc) {
        vector->size = argc;
        vector->values = PMALLOC(vector->size * sizeof(double));
        for (int index = 0; index < argc; index++)
            vector->values[index] = AS_NUMBER(argv[index]);
    }
    else {
        vector->size = 0;
        vector->values = NULL;
    }
    LAX_LOG("vector type: %d", vector->obj.type);
    return OBJ_VAL(vector);
}
ObjClass* newVectorClass()
{
    ObjString* name = copyString("Vector", 6);
    ObjClass* klass = newClass(name);
    int method = getMethodAddress(copyString("transpose", 9));
    setAtValueArray(&klass->methods, method, OBJ_VAL(newNativeBoundMethod(vector_transpose)));
    method = getMethodAddress(copyString("abs", 3));
    setAtValueArray(&klass->methods, method, OBJ_VAL(newNativeBoundMethod(vector_abs)));
    method = getMethodAddress(copyString("dot", 3));
    setAtValueArray(&klass->staticMethods, method, OBJ_VAL(newNative(vector_dot)));
    method = getMethodAddress(copyString("cross", 5));
    setAtValueArray(&klass->staticMethods, method, OBJ_VAL(newNative(vector_cross)));
    klass->call = newNative(vector_new);
    return klass;
}
ObjMatrix* newMatrix(unsigned int rows, unsigned int columns, double v)
{
    ObjMatrix* matrix = ALLOCATE_OBJ(ObjMatrix, OBJ_MATRIX);
    matrix->rows = rows;
    matrix->columns = columns;
    matrix->values = PMALLOC(matrix->rows * matrix->columns * sizeof(double));
    if (0 == v) {
        memset(matrix->values, 0, matrix->rows * matrix->columns * sizeof(double));
    }
    else {
        int size = matrix->rows * matrix->columns;
        for (int i = 0; i < size; i++)
            matrix->values[i] = v;
    }
    return matrix;
}
ObjMatrix* duplicateMatrix(ObjMatrix* matrix)
{
    ObjMatrix* result = ALLOCATE_OBJ(ObjMatrix, OBJ_MATRIX);
    result->rows = matrix->rows;
    result->columns = matrix->columns;
    result->values = PMALLOC(result->rows * result->columns * sizeof(double));
    memcpy(result->values, matrix->values, result->rows * result->columns * sizeof(double));
    return result;
}
static Value matrix_transpose(Value receiver, int argc, Value* argv)
{
    (void)argc;
    (void)argv;
    ObjMatrix* matrix = AS_MATRIX(receiver);
    ObjMatrix* result = ALLOCATE_OBJ(ObjMatrix, OBJ_MATRIX);
    result->rows = matrix->columns;
    result->columns = matrix->rows;
    result->values = PMALLOC(result->rows * result->columns * sizeof(double));
    cblas_domatcopy(CblasRowMajor, CblasTrans,
                    matrix->rows,    // Number of rows in source matrix A
                    matrix->columns, // Number of columns in source matrix A
                    1.0,             // Scaling factor (use 1.0 for simple transpose)
                    matrix->values,  // Pointer to source matrix A
                    result->rows,    // Leading dimension of A
                    result->values,  // Pointer to destination matrix B
                    result->columns  // Leading dimension of B
    );
    return OBJ_VAL(result);
}
static Value matrix_new(int argc, Value* argv)
{
    LAX_LOG("matrix_new(%d)", argc);
    ObjMatrix* matrix = ALLOCATE_OBJ(ObjMatrix, OBJ_MATRIX);
    if (0 < argc) {
        matrix->values = PMALLOC(argc * sizeof(double));
        int row = 1, col = 0;
        for (int index = 0; index < argc; index++) {
            if (IS_NIL(argv[index])) {
                row++;
                col = 0;
            }
            else {
                col++;
            }
        }
        matrix->rows = row;
        matrix->columns = col;
        row = 0;
        col = 0;
        for (int index = 0; index < argc; index++) {
            if (IS_NIL(argv[index])) {
                row++;
            }
            else {
                matrix->values[col + row * matrix->columns] = AS_NUMBER(argv[index]);
            }
        }
    }
    else {
        matrix->rows = 0;
        matrix->columns = 0;
        matrix->values = NULL;
    }
    LAX_LOG("vector type: %d", matrix->obj.type);
    return OBJ_VAL(matrix);
}
ObjClass* newMatrixClass()
{
    ObjString* name = copyString("Matrix", 6);
    ObjClass* klass = newClass(name);
    int method = getMethodAddress(copyString("transpose", 9));
    setAtValueArray(&klass->methods, method, OBJ_VAL(newNativeBoundMethod(matrix_transpose)));
    klass->call = newNative(matrix_new);
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
    int method = getMethodAddress(copyString("abs", 3));
    setAtValueArray(&klass->methods, method, OBJ_VAL(newNativeBoundMethod(num_abs)));
    method = getMethodAddress(copyString("sign", 4));
    setAtValueArray(&klass->methods, method, OBJ_VAL(newNativeBoundMethod(num_sign)));
    method = getMethodAddress(copyString("ceil", 4));
    setAtValueArray(&klass->methods, method, OBJ_VAL(newNativeBoundMethod(num_ceil)));
    method = getMethodAddress(copyString("floor", 5));
    setAtValueArray(&klass->methods, method, OBJ_VAL(newNativeBoundMethod(num_floor)));
    method = getMethodAddress(copyString("round", 5));
    setAtValueArray(&klass->methods, method, OBJ_VAL(newNativeBoundMethod(num_round)));
    method = getMethodAddress(copyString("truncate", 8));
    setAtValueArray(&klass->methods, method, OBJ_VAL(newNativeBoundMethod(num_truncate)));
    method = getMethodAddress(copyString("fraction", 8));
    setAtValueArray(&klass->methods, method, OBJ_VAL(newNativeBoundMethod(num_fraction)));
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
            if (IS_STRING(list->array.values[i]))
                printf("\"");
            printValue(list->array.values[i]);
            if (IS_STRING(list->array.values[i]))
                printf("\"");
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
    case OBJ_VECTOR: {
        ObjVector* vector = AS_VECTOR(value);
        if (vector->isRow) {
            for (int i = 0; i < vector->size; i++) {
                if (i)
                    printf(" ");
                printf("%f", vector->values[i]);
            }
        }
        else {
            for (int i = 0; i < vector->size; i++) {
                if (i)
                    printf("\n");
                printf("%f", vector->values[i]);
            }
        }
        break;
    }
    case OBJ_MATRIX: {
        ObjMatrix* matrix = AS_MATRIX(value);
        for (int row = 0; row < matrix->rows; row++) {
            for (int column = 0; column < matrix->columns; column++) {
                if (column)
                    printf(" ");
                printf("%f", matrix->values[column + matrix->columns * row]);
            }
            printf("\n");
        }
        break;
    }
    case OBJ_THREAD:
        printf("<thread>");
        break;
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
        LAX_LOG("%s(%x) vs %s(%x)", stra->chars, stra->hash, strb->chars, strb->hash);
        if (stra->hash == strb->hash && !strcmp(stra->chars, strb->chars))
            retVal = true;
        break;
    }
    case OBJ_LIST: {
        retVal = true;
        ObjList* lsta = (ObjList*)a;
        ObjList* lstb = (ObjList*)b;
        if (lsta->array.count != lstb->array.count) {
            retVal = false;
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
    case OBJ_VECTOR: {
        ObjVector* vcta = (ObjVector*)a;
        ObjVector* vctb = (ObjVector*)b;
        if (vcta->size != vctb->size) {
            retVal = false;
        }
        else {
            ObjVector* diff = duplicateVector(vcta);
            cblas_daxpy(vcta->size, -1.0, vctb->values, 1, diff->values, 1);
            int max_idx = cblas_idamax(diff->size, diff->values, 1);
            retVal = (diff->values[max_idx]) ? false : true;
        }
        break;
    }
    case OBJ_MATRIX: {
        ObjMatrix* mtxa = (ObjMatrix*)a;
        ObjMatrix* mtxb = (ObjMatrix*)b;
        if (mtxa->rows != mtxb->rows || mtxa->columns != mtxb->columns) {
            retVal = false;
        }
        else {
            ObjMatrix* diff = duplicateMatrix(mtxa);
            cblas_daxpy(mtxa->rows * mtxa->columns, -1.0, mtxb->values, 1, diff->values, 1);
            int max_idx = cblas_idamax(diff->rows * diff->columns, diff->values, 1);
            retVal = (diff->values[max_idx]) ? false : true;
        }
        break;
    }
    default:
        retVal = true;
    }
    return retVal;
}
//< print-object
