//> A Virtual Machine vm-c
//> Types of Values include-stdarg
#include <stdarg.h>
//< Types of Values include-stdarg
//> vm-include-stdio
#include <stdio.h>
//> Strings vm-include-string
#include <string.h>
//< Strings vm-include-string
//> Calls and Functions vm-include-time
#include <time.h>
//< Calls and Functions vm-include-time
#include <math.h>

//< vm-include-stdio
#include "common.h"
//> Scanning on Demand vm-include-compiler
#include "compiler.h"
//< Scanning on Demand vm-include-compiler
//> vm-include-debug
#include "debug.h"
//< vm-include-debug
//> Strings vm-include-object-memory
#include "memory.h"
#include "object.h"
//< Strings vm-include-object-memory
#include "vm.h"

#include "log.h"

#include <cblas.h>

#define PUSH(value) (*vm.thread->stackTop++ = (value))
#define POP() (*(--vm.thread->stackTop))
#define DROP() (--vm.thread->stackTop)
#define PEEK() (*(vm.thread->stackTop - 1))
#define NPEEK(n) (*(vm.thread->stackTop - 1 - n))
#define IS_FALSEY(value) (IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value)))

VM vm; // [one]
//> Calls and Functions clock-native
static Value rangeNative(int argCount, Value* args)
{
    ObjList* list = AS_LIST(vm.listClass->call->function(0, NULL));
    int start = 0, end = 0, increment = 1;
    switch (argCount) {
    case 3:
        start = AS_NUMBER(args[0]);
        end = AS_NUMBER(args[1]);
        increment = AS_NUMBER(args[2]);
        break;
    case 2:
        start = AS_NUMBER(args[0]);
        end = AS_NUMBER(args[1]);
        break;
    case 1:
        end = AS_NUMBER(args[0]);
        break;
    default:
        runtimeError("Invalid number of arguments.");
        return NIL_VAL;
        break;
    }
    for (int i = start; i <= end; i += increment) {
        writeValueArray(&list->array, NUMBER_VAL(i));
    }
    return OBJ_VAL(list);
}
static Value clockNative(int argCount, Value* args)
{
    (void)argCount;
    (void)args;
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}
//< Calls and Functions clock-native
//> reset-stack
static void resetStack()
{
    vm.thread->stackTop = vm.thread->stack;
    //> Calls and Functions reset-frame-count
    vm.thread->frameCount = 0;
    //< Calls and Functions reset-frame-count
    //> Closures init-open-upvalues
    vm.openUpvalues = NULL;
    //< Closures init-open-upvalues
}
//< reset-stack
//> Types of Values runtime-error
void runtimeError(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    /* Types of Values runtime-error < Calls and Functions runtime-error-temp
      size_t instruction = vm.ip - vm.chunk->code - 1;
      int line = vm.chunk->lines[instruction];
    */
    /* Calls and Functions runtime-error-temp < Calls and Functions runtime-error-stack
      CallFrame* frame = &vm.frames[vm.frameCount - 1];
      size_t instruction = frame->ip - frame->function->chunk.code - 1;
      int line = frame->function->chunk.lines[instruction];
    */
    /* Types of Values runtime-error < Calls and Functions runtime-error-stack
      fprintf(stderr, "[line %d] in script\n", line);
    */
    //> Calls and Functions runtime-error-stack
    for (int i = vm.thread->frameCount - 1; i >= 0; i--) {
        CallFrame* frame = &vm.thread->frames[i];
        /* Calls and Functions runtime-error-stack < Closures runtime-error-function
            ObjFunction* function = frame->function;
        */
        //> Closures runtime-error-function
        ObjFunction* function = frame->closure->function;
        //< Closures runtime-error-function
        size_t instruction = frame->ip - function->chunk.code - 1;
        fprintf(stderr, "[line %d] in ", // [minus]
                function->chunk.lines[instruction]);
        if (function->name == NULL) {
            fprintf(stderr, "script\n");
        }
        else {
            fprintf(stderr, "%s()\n", function->name->chars);
        }
    }

    //< Calls and Functions runtime-error-stack
    resetStack();
}
//< Types of Values runtime-error

static void defineValue(const char* name, Value value)
{
    ObjString* on = copyString(name, (int)strlen(name));
    PUSH(OBJ_VAL(on));
    PUSH(value);
    uint16_t offset = getGlobalAddress(on, NIL_VAL);
    vm.globals.values[offset] = vm.thread->stack[1];
    DROP();
    DROP();
}

//> Calls and Functions define-native
void defineNative(const char* name, NativeFn function)
{
    defineValue(name, OBJ_VAL(newNative(function)));
}

//< Calls and Functions define-native

void initVM()
{
    //> Strings init-objects-root
    vm.objects = NULL;
    //< Strings init-objects-root
    vm.thread = newThread(THREAD_TYPE_PROCESS);
    //> call-reset-stack
    resetStack();
    //< call-reset-stack
    //> Garbage Collection init-gc-fields
    vm.bytesAllocated = 0;
    vm.nextGC = 1024 * 1024;
    //< Garbage Collection init-gc-fields
    //> Garbage Collection init-gray-stack

    vm.grayCount = 0;
    vm.grayCapacity = 0;
    vm.grayStack = NULL;
    //< Garbage Collection init-gray-stack
    //> Global Variables init-globals

    initTable(&vm.symtabGlobals);
    initTable(&vm.symtabMethods);
    initValueArray(&vm.globals);
    //< Global Variables init-globals
    //> Hash Tables init-strings
    initTable(&vm.strings);
    //< Hash Tables init-strings
    //> Methods and Initializers init-init-string

    //> null-init-string
    vm.initString = NULL;
    //< null-init-string
    vm.initString = copyString("init", 4);
    vm.initAddress = getMethodAddress(vm.initString);
    LAX_LOG("initAddress = %d", vm.initAddress);
    //< Methods and Initializers init-init-string
    //> Calls and Functions define-native-clock

    // native interfaces
    vm.systemClass = newSystemClass();

    vm.numClass = newNumClass();
    vm.stringClass = newStringClass();
    vm.listClass = newListClass();
    vm.mapClass = newMapClass();
    vm.vectorClass = newVectorClass();
    vm.matrixClass = newMatrixClass();
    defineValue("List", OBJ_VAL(newListClass()));
    defineValue("Map", OBJ_VAL(newMapClass()));
    defineValue("Vector", OBJ_VAL(newVectorClass()));
    defineValue("Matrix", OBJ_VAL(newMatrixClass()));
    defineValue("System", OBJ_VAL(newSystemClass()));
    defineNative("range", rangeNative);
    defineNative("clock", clockNative);
    //< Calls and Functions define-native-clock
    vm.dls = NULL;
    vm.dlCount = 0;
    vm.dlCapacity = 0;
}

void freeVM()
{
    //> Global Variables free-globals
    freeTable(&vm.symtabGlobals);
    freeTable(&vm.symtabMethods);
    freeValueArray(&vm.globals);
    //< Global Variables free-globals
    //> Hash Tables free-strings
    freeTable(&vm.strings);
    //< Hash Tables free-strings
    //> Methods and Initializers clear-init-string
    vm.initString = NULL;
    //< Methods and Initializers clear-init-string
    //> Strings call-free-objects
    freeObjects();
    //< Strings call-free-objects
    if (vm.dls) {
        for (size_t i = 0; i < vm.dlCount; i++) {
            dlFree(vm.dls[i]);
        }
        FREE_ARRAY(DL, vm.dls, vm.dlCapacity);
    }
}
//> push
void push(Value value)
{
    *vm.thread->stackTop = value;
    vm.thread->stackTop++;
}
//< push
//> pop
Value pop()
{
    vm.thread->stackTop--;
    return *vm.thread->stackTop;
}
//< pop
//> Types of Values peek
/*
static Value peek(int distance)
{
    return vm.stackTop[-1 - distance];
}
*/
//< Types of Values peek
/* Calls and Functions call < Closures call-signature
static bool call(ObjFunction* function, int argCount) {
*/
//> Calls and Functions call
//> Closures call-signature
static bool call(ObjClosure* closure, int argCount)
{
    //< Closures call-signature
    /* Calls and Functions check-arity < Closures check-arity
      if (argCount != function->arity) {
        runtimeError("Expected %d arguments but got %d.",
            function->arity, argCount);
    */
    //> Closures check-arity
    if (argCount != closure->function->arity) {
        LAX_LOG("Expected %d arguments but got %d.", closure->function->arity, argCount);
        runtimeError("Expected %d arguments but got %d.", closure->function->arity, argCount);
        //< Closures check-arity
        //> check-arity
        return false;
    }

    //< check-arity
    //> check-overflow
    if (vm.thread->frameCount == FRAMES_MAX) {
        runtimeError("Stack overflow.");
        return false;
    }

    //< check-overflow
    CallFrame* frame = &vm.thread->frames[vm.thread->frameCount++];
    /* Calls and Functions call < Closures call-init-closure
      frame->function = function;
      frame->ip = function->chunk.code;
    */
    //> Closures call-init-closure
    frame->closure = closure;
    frame->ip = closure->function->chunk.code;
    //< Closures call-init-closure
    frame->slots = vm.thread->stackTop - argCount - 1;
    return true;
}
//< Calls and Functions call
//> Calls and Functions call-value
static bool callValue(Value callee, int argCount)
{
    if (IS_OBJ(callee)) {
        LAX_LOG("obj_type(callee): %d", OBJ_TYPE(callee));
        switch (OBJ_TYPE(callee)) {
            //> Methods and Initializers call-bound-method
        case OBJ_BOUND_METHOD: {
            ObjBoundMethod* bound = AS_BOUND_METHOD(callee);
            //> store-receiver
            vm.thread->stackTop[-argCount - 1] = bound->receiver;
            //< store-receiver
            return call(bound->method, argCount);
        }
            //< Methods and Initializers call-bound-method
            //> Classes and Instances call-class
        case OBJ_CLASS: {
            ObjClass* klass = AS_CLASS(callee);
            Value instance = NIL_VAL;
            if (klass->call) {
                instance = klass->call->function(argCount, vm.thread->stackTop - argCount);
                vm.thread->stackTop -= argCount;
                argCount = 0;
            }
            else {
                instance = OBJ_VAL(newInstance(klass));
            }
            vm.thread->stackTop[-argCount - 1] = instance;
            //> Methods and Initializers call-init
            if (klass->methods.count > vm.initAddress) {
                ClassMember initializer = klass->methods.values[vm.initAddress];
                LAX_LOG("initialiser=0x%lx", initializer.as.value);
                // if (tableGet(&klass->methods, vm.initString, &initializer)) {
                if (MEMBER_VALUE == initializer.type) {
                    return call(AS_CLOSURE(initializer.as.value), argCount);
                    //> no-init-arity-error
                }
            }
            else if (argCount != 0) {
                LAX_LOG("Expected 0 arguments but got %d.", argCount);
                runtimeError("Expected 0 arguments but got %d.", argCount);
                return false;
                //< no-init-arity-error
            }
            //< Methods and Initializers call-init
            return true;
        }
            //< Classes and Instances call-class
            //> Closures call-value-closure
        case OBJ_CLOSURE:
            return call(AS_CLOSURE(callee), argCount);
            //< Closures call-value-closure
            /* Calls and Functions call-value < Closures call-value-closure
                  case OBJ_FUNCTION: // [switch]
                    return call(AS_FUNCTION(callee), argCount);
            */
            //> call-native
        case OBJ_NATIVE: {
            NativeFn native = AS_NATIVE(callee);
            Value result = native(argCount, vm.thread->stackTop - argCount);
            vm.thread->stackTop -= argCount + 1;
            PUSH(result);
            return true;
        }
            //< call-native
        default:
            break; // Non-callable object type.
        }
    }
    runtimeError("Can only call functions and classes.");
    return false;
}
//< Calls and Functions call-value
//> Methods and Initializers invoke-from-class
static bool invokeFromClass(ObjClass* klass, int name, int argCount, bool isStatic)
{
    ClassMember method;
    if (isStatic) {
        LAX_LOG_ARRAY(klass->staticMethods);
        if (klass->staticMethods.count > name) {
            runtimeError("Undefined property '%s'.", undefinedMethod(name));
            return false;
        }
        method = klass->staticMethods.values[name];
    }
    else {
        LAX_LOG_ARRAY(klass->methods);
        if (klass->methods.count > name) {
            runtimeError("Undefined property '%s'.", undefinedMethod(name));
            return false;
        }
        method = klass->methods.values[name];
    }
    bool retVal = false;
    switch (method.type) {
    case MEMBER_UNDEFINED: {
        runtimeError("Undefined property '%s'.", undefinedMethod(name));
        return false;
        break;
    }
    case MEMBER_VALUE: {
        retVal = callValue(method.as.value, argCount);
        break;
    }
    case MEMBER_NATIVE_FN: {
        Value result = method.as.nativeFn(argCount, vm.thread->stackTop - argCount);
        vm.thread->stackTop -= argCount + 1;
        PUSH(result);
        retVal = true;
        break;
    }
    case MEMBER_NATIVE_BOUND_METHOD: {
        break;
    }
    }
    return retVal;
}
static bool invokeFromNative(Value receiver, ObjClass* klass, int name, int argCount)
{
    if (klass->methods.count > name) {
        runtimeError("Undefined property '%s'.", undefinedMethod(name));
        return false;
    }
    ClassMember method = klass->methods.values[name];
    bool retVal = false;
    switch (method.type) {
    case MEMBER_UNDEFINED: {
        runtimeError("Undefined property '%s'.", undefinedMethod(name));
        return false;
        break;
    }
    case MEMBER_VALUE: {
        retVal = callValue(method.as.value, argCount);
        break;
    }
    case MEMBER_NATIVE_BOUND_METHOD: {
        Value result = method.as.nativeBoundMethod(receiver, argCount, vm.thread->stackTop - argCount);
        vm.thread->stackTop -= argCount + 1;
        PUSH(result);
        retVal = true;
        break;
    }
    case MEMBER_NATIVE_FN: {
        break;
    }
    }
    return retVal;
}
//< Methods and Initializers invoke-from-class
//> Methods and Initializers invoke
static bool invoke(int name, int argCount)
{
    Value receiver = NPEEK(argCount);
    //> invoke-check-type

    LAX_LOG("receiver type: %d", AS_OBJ(receiver)->type);
    LAX_LOG("method ID: %d", name);
    if (IS_INSTANCE(receiver)) {
        //< invoke-check-type
        ObjInstance* instance = AS_INSTANCE(receiver);
        //> invoke-field

        LAX_LOG_ARRAY(instance->fields);
        if (instance->fields.count > name) {
            ClassMember method = instance->fields.values[name];
            // if (tableGet(&instance->fields, name, &value)) {
            if (MEMBER_VALUE == method.type) {
                LAX_LOG("method %d is found", name);
                vm.thread->stackTop[-argCount - 1] = method.as.value;
                return callValue(method.as.value, argCount);
            }
            // TODO: handling native fn and native bound method
        }

        //< invoke-field
        return invokeFromClass(instance->klass, name, argCount, false);
    }
    else if (IS_LIST(receiver)) {
        return invokeFromNative(receiver, vm.listClass, name, argCount);
    }
    else if (IS_MAP(receiver)) {
        return invokeFromNative(receiver, vm.mapClass, name, argCount);
    }
    else if (IS_VECTOR(receiver)) {
        return invokeFromNative(receiver, vm.vectorClass, name, argCount);
    }
    else if (IS_MATRIX(receiver)) {
        return invokeFromNative(receiver, vm.matrixClass, name, argCount);
    }
    else if (IS_NUMBER(receiver)) {
        return invokeFromNative(receiver, vm.numClass, name, argCount);
    }
    else if (IS_STRING(receiver)) {
        return invokeFromNative(receiver, vm.stringClass, name, argCount);
    }
    else if (IS_CLASS(receiver)) {
        return invokeFromClass(AS_CLASS(receiver), name, argCount, true);
    }
    else {
        runtimeError("Only instances have methods.");
        return false;
    }
}
//< Methods and Initializers invoke
//> Methods and Initializers bind-method
static bool bindMethod(ObjClass* klass, int name)
{
    if (klass->methods.count <= name) {
        runtimeError("Undefined property '%s'.", undefinedMethod(name));
        return false;
    }
    ClassMember method = klass->methods.values[name];

    switch (method.type) {
    case MEMBER_UNDEFINED: {
        runtimeError("Undefined property '%s'.", undefinedMethod(name));
        return false;
        break;
    }
    case MEMBER_NATIVE_FN: {
        ObjNative* native = newNative(method.as.nativeFn);
        DROP();
        PUSH(OBJ_VAL(native));
        break;
    }
    case MEMBER_NATIVE_BOUND_METHOD: {
        ObjNativeBoundMethod* native = newNativeBoundMethod(method.as.nativeBoundMethod);
        DROP();
        PUSH(OBJ_VAL(native));
        break;
    }
    case MEMBER_VALUE: {
        ObjBoundMethod* bound = newBoundMethod(PEEK(), AS_CLOSURE(method.as.value));
        DROP();
        PUSH(OBJ_VAL(bound));
        break;
    }
    }
    return true;
}
//< Methods and Initializers bind-method
//> Closures capture-upvalue
static ObjUpvalue* captureUpvalue(Value* local)
{
    //> look-for-existing-upvalue
    ObjUpvalue* prevUpvalue = NULL;
    ObjUpvalue* upvalue = vm.openUpvalues;
    while (upvalue != NULL && upvalue->location > local) {
        prevUpvalue = upvalue;
        upvalue = upvalue->next;
    }

    if (upvalue != NULL && upvalue->location == local) {
        return upvalue;
    }

    //< look-for-existing-upvalue
    ObjUpvalue* createdUpvalue = newUpvalue(local);
    //> insert-upvalue-in-list
    createdUpvalue->next = upvalue;

    if (prevUpvalue == NULL) {
        vm.openUpvalues = createdUpvalue;
    }
    else {
        prevUpvalue->next = createdUpvalue;
    }

    //< insert-upvalue-in-list
    return createdUpvalue;
}
//< Closures capture-upvalue
//> Closures close-upvalues
static void closeUpvalues(Value* last)
{
    while (vm.openUpvalues != NULL && vm.openUpvalues->location >= last) {
        ObjUpvalue* upvalue = vm.openUpvalues;
        upvalue->closed = *upvalue->location;
        upvalue->location = &upvalue->closed;
        vm.openUpvalues = upvalue->next;
    }
}
//< Closures close-upvalues
//> Methods and Initializers define-method
static void defineMethod(int name)
{
    LAX_LOG("defineMethod(%d)", name);
    Value method = PEEK();
    ObjClass* klass = AS_CLASS(NPEEK(1));
    ClassMember defmbr;
    defmbr.type = MEMBER_UNDEFINED;
    ClassMember mbr;
    mbr.type = MEMBER_VALUE;
    mbr.as.value = method;
    setAtClassMemberArray(&klass->methods, name, mbr, defmbr);
    LAX_LOG_ARRAY(klass->methods);
    DROP();
}
//< Methods and Initializers define-method
//> Types of Values is-falsey
/*
static bool isFalsey(Value value)
{
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}
*/
//< Types of Values is-falsey
//> Strings concatenate
static void concatenate()
{
    /* Strings concatenate < Garbage Collection concatenate-peek
      ObjString* b = AS_STRING(POP());
      ObjString* a = AS_STRING(POP());
    */
    //> Garbage Collection concatenate-peek
    ObjString* b = AS_STRING(PEEK());
    ObjString* a = AS_STRING(NPEEK(1));
    //< Garbage Collection concatenate-peek

    int length = a->length + b->length;
    char* chars = ALLOCATE(char, length + 1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    ObjString* result = takeString(chars, length);
    //> Garbage Collection concatenate-pop
    DROP();
    DROP();
    //< Garbage Collection concatenate-pop
    PUSH(OBJ_VAL(result));
}
//< Strings concatenate
//> run
static InterpretResult run()
{
    //> Calls and Functions run
    CallFrame* frame = &vm.thread->frames[vm.thread->frameCount - 1];

/* A Virtual Machine run < Calls and Functions run
#define READ_BYTE() (*vm.ip++)
*/
#define READ_BYTE() (*frame->ip++)
/* A Virtual Machine read-constant < Calls and Functions run
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
*/

/* Jumping Back and Forth read-short < Calls and Functions run
#define READ_SHORT() \
    (vm.ip += 2, (uint16_t)((vm.ip[-2] << 8) | vm.ip[-1]))
*/
#define READ_SHORT() (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))

/* Calls and Functions run < Closures read-constant
#define READ_CONSTANT() \
    (frame->function->chunk.constants.values[READ_BYTE()])
*/
//> Closures read-constant
#define READ_CONSTANT() (frame->closure->function->chunk.constants.values[READ_SHORT()])
//< Closures read-constant

//< Calls and Functions run
//> Global Variables read-string
#define READ_STRING() AS_STRING(READ_CONSTANT())
//< Global Variables read-string
/* A Virtual Machine binary-op < Types of Values binary-op
#define BINARY_OP(op) \
    do { \
      double b = POP(); \
      double a = POP(); \
      PUSH(a op b); \
    } while (false)
*/
//> Types of Values binary-op
#define BINARY_CMP(valueType, op)                                                                                      \
    do {                                                                                                               \
        if (!IS_NUMBER(PEEK()) || !IS_NUMBER(NPEEK(1))) {                                                              \
            runtimeError("Operands must be numbers.");                                                                 \
            return INTERPRET_RUNTIME_ERROR;                                                                            \
        }                                                                                                              \
        double b = AS_NUMBER(POP());                                                                                   \
        double a = AS_NUMBER(POP());                                                                                   \
        PUSH(valueType(a op b));                                                                                       \
    } while (false)
    //< Types of Values binary-op

#define BINARY_OP(valueType, op)                                                                                       \
    do {                                                                                                               \
        if (IS_NUMBER(PEEK())) {                                                                                       \
            double b = AS_NUMBER(POP());                                                                               \
            if (IS_NUMBER(PEEK())) {                                                                                   \
                double a = AS_NUMBER(POP());                                                                           \
                PUSH(NUMBER_VAL(a op b));                                                                              \
            }                                                                                                          \
            else if (IS_COMPLEX(PEEK())) {                                                                             \
                ObjComplex* a = AS_COMPLEX(POP());                                                                     \
                PUSH(OBJ_VAL(newComplex(a->real op b, a->imag)));                                                      \
            }                                                                                                          \
            else {                                                                                                     \
                runtimeError("Operands must be numbers.");                                                             \
                return INTERPRET_RUNTIME_ERROR;                                                                        \
            }                                                                                                          \
        }                                                                                                              \
        else if (IS_COMPLEX(PEEK())) {                                                                                 \
            ObjComplex* b = AS_COMPLEX(POP());                                                                         \
            if (IS_NUMBER(PEEK())) {                                                                                   \
                double a = AS_NUMBER(POP());                                                                           \
                PUSH(OBJ_VAL(newComplex(a op b->value)));                                                              \
            }                                                                                                          \
            else if (IS_COMPLEX(PEEK())) {                                                                             \
                ObjComplex* a = AS_COMPLEX(POP());                                                                     \
                PUSH(OBJ_VAL(newComplex(a->value op b->value)));                                                       \
            }                                                                                                          \
            else {                                                                                                     \
                runtimeError("Operands must be numbers.");                                                             \
                return INTERPRET_RUNTIME_ERROR;                                                                        \
            }                                                                                                          \
        }                                                                                                              \
        else {                                                                                                         \
            runtimeError("Operands must be numbers.");                                                                 \
            return INTERPRET_RUNTIME_ERROR;                                                                            \
        }                                                                                                              \
    } while (false)
    //< Types of Values binary-op

#define POWER_OP(valueType, fn)                                                                                        \
    do {                                                                                                               \
        if (!IS_NUMBER(PEEK()) || !IS_NUMBER(NPEEK(1))) {                                                              \
            runtimeError("Operands must be numbers.");                                                                 \
            return INTERPRET_RUNTIME_ERROR;                                                                            \
        }                                                                                                              \
        double b = AS_NUMBER(POP());                                                                                   \
        double a = AS_NUMBER(POP());                                                                                   \
        PUSH(valueType(fn(a, b)));                                                                                     \
    } while (false)
    //< Types of Values binary-op

    for (;;) {
//> trace-execution
#ifdef DEBUG_TRACE_EXECUTION
        //> trace-stack
        printf("          ");
        for (Value* slot = vm.thread->stack; slot < vm.thread->stackTop; slot++) {
            printf("[ ");
            printValue(*slot);
            printf(" ]");
        }
        printf("\n");
        //< trace-stack
        /* A Virtual Machine trace-execution < Calls and Functions trace-execution
            disassembleInstruction(vm.chunk,
                                   (int)(vm.ip - vm.chunk->code));
        */
        /* Calls and Functions trace-execution < Closures disassemble-instruction
            disassembleInstruction(&frame->function->chunk,
                (int)(frame->ip - frame->function->chunk.code));
        */
        //> Closures disassemble-instruction
        disassembleInstruction(&frame->closure->function->chunk,
                               (int)(frame->ip - frame->closure->function->chunk.code));
//< Closures disassemble-instruction
#endif

        //< trace-execution
        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            //> op-constant
        case OP_CONSTANT: {
            Value constant = READ_CONSTANT();
            /* A Virtual Machine op-constant < A Virtual Machine push-constant
                    printValue(constant);
                    printf("\n");
            */
            //> push-constant
            PUSH(constant);
            //< push-constant
            break;
        }
            //< op-constant
            //> Types of Values interpret-literals
        case OP_NIL:
            PUSH(NIL_VAL);
            break;
        case OP_TRUE:
            PUSH(BOOL_VAL(true));
            break;
        case OP_FALSE:
            PUSH(BOOL_VAL(false));
            break;
            //< Types of Values interpret-literals
            //> Global Variables interpret-pop
        case OP_POP:
            DROP();
            break;
            //< Global Variables interpret-pop
            //> Local Variables interpret-get-local
        case OP_GET_LOCAL: {
            uint16_t slot = READ_SHORT();
            /* Local Variables interpret-get-local < Calls and Functions push-local
                    PUSH(vm.stack[slot]); // [slot]
            */
            //> Calls and Functions push-local
            PUSH(frame->slots[slot]);
            //< Calls and Functions push-local
            break;
        }
            //< Local Variables interpret-get-local
            //> Local Variables interpret-set-local
        case OP_SET_LOCAL: {
            uint16_t slot = READ_SHORT();
            /* Local Variables interpret-set-local < Calls and Functions set-local
                    vm.stack[slot] = PEEK();
            */
            //> Calls and Functions set-local
            frame->slots[slot] = PEEK();
            //< Calls and Functions set-local
            break;
        }
            //< Local Variables interpret-set-local
            //> Global Variables interpret-get-global
        case OP_GET_GLOBAL: {
            uint16_t name = READ_SHORT();
            Value value;
            LAX_LOG("global.count=%d", vm.globals.count);
            if (vm.globals.count <= name || IS_UNDEF(vm.globals.values[name])) {
                runtimeError("Undefined variable '%s'.", undefinedSymbol(name));
                return INTERPRET_RUNTIME_ERROR;
            }
            else {
                value = vm.globals.values[name];
            }
            PUSH(value);
            break;
        }
            //< Global Variables interpret-get-global
            //> Global Variables interpret-define-global
        case OP_DEFINE_GLOBAL: {
            uint16_t name = READ_SHORT();
            vm.globals.values[name] = PEEK();
            DROP();
            break;
        }
            //< Global Variables interpret-define-global
            //> Global Variables interpret-set-global
        case OP_SET_GLOBAL: {
            uint16_t name = READ_SHORT();
            if (vm.globals.count <= name || IS_UNDEF(vm.globals.values[name])) {
                // tableDelete(&vm.globals, name); // [delete]
                runtimeError("Undefined variable '%s'.", undefinedSymbol(name));
                return INTERPRET_RUNTIME_ERROR;
            }
            else {
                vm.globals.values[name] = PEEK();
            }
            break;
        }
            //< Global Variables interpret-set-global
            //> Closures interpret-get-upvalue
        case OP_GET_UPVALUE: {
            uint16_t slot = READ_SHORT();
            PUSH(*frame->closure->upvalues[slot]->location);
            break;
        }
            //< Closures interpret-get-upvalue
            //> Closures interpret-set-upvalue
        case OP_SET_UPVALUE: {
            uint16_t slot = READ_SHORT();
            *frame->closure->upvalues[slot]->location = PEEK();
            break;
        }
            //< Closures interpret-set-upvalue
            //> Classes and Instances interpret-get-property
        case OP_GET_PROPERTY: {
            //> get-not-instance
            if (IS_INSTANCE(PEEK())) {

                //< get-not-instance
                ObjInstance* instance = AS_INSTANCE(PEEK());
                int name = READ_SHORT();

                if (instance->fields.count > name) {
                    ClassMember value = instance->fields.values[name];
                    DROP(); // Instance.
                    switch (value.type) {
                    case MEMBER_VALUE: {
                        PUSH(value.as.value);
                        break;
                    }
                    case MEMBER_NATIVE_FN: {
                        ObjNative* native = newNative(value.as.nativeFn);
                        PUSH(OBJ_VAL(native));
                        break;
                    }
                    case MEMBER_NATIVE_BOUND_METHOD: {
                        ObjNativeBoundMethod* native = newNativeBoundMethod(value.as.nativeBoundMethod);
                        PUSH(OBJ_VAL(native));
                        break;
                    }
                    case MEMBER_UNDEFINED:
                        break;
                    }
                }
                //> get-undefined

                //< get-undefined
                /* Classes and Instances get-undefined < Methods and Initializers get-method
                        runtimeError("Undefined property '%s'.", name->chars);
                        return INTERPRET_RUNTIME_ERROR;
                */
                //> Methods and Initializers get-method
                if (!bindMethod(instance->klass, name)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
            }
            else {
                runtimeError("Only instances have properties.");
                return INTERPRET_RUNTIME_ERROR;
            }
            break;
            //< Methods and Initializers get-method
        }
            //< Classes and Instances interpret-get-property
            //> Classes and Instances interpret-set-property
        case OP_SET_PROPERTY: {
            //> set-not-instance
            if (!IS_INSTANCE(NPEEK(1))) {
                runtimeError("Only instances have fields.");
                return INTERPRET_RUNTIME_ERROR;
            }

            //< set-not-instance
            ObjInstance* instance = AS_INSTANCE(NPEEK(1));
            ClassMember defmbr;
            defmbr.type = MEMBER_UNDEFINED;
            ClassMember mbr;
            Value v = PEEK();
            if (IS_NATIVE(v)) {
                mbr.type = MEMBER_NATIVE_FN;
                mbr.as.nativeFn = AS_NATIVE(v);
            }
            else if (IS_NATIVE_BOUND_METHOD(v)) {
                mbr.type = MEMBER_NATIVE_BOUND_METHOD;
                mbr.as.nativeBoundMethod = AS_NATIVE_BOUND_METHOD(v);
            }
            else {
                mbr.type = MEMBER_VALUE;
                mbr.as.value = v;
            }
            setAtClassMemberArray(&instance->fields, READ_SHORT(), mbr, defmbr);
            Value value = POP();
            DROP();
            PUSH(value);
            break;
        }
            //< Classes and Instances interpret-set-property
        case OP_GET_ELEMENT: {
            size_t argCount = READ_BYTE();
            Value value = NIL_VAL;
            if (1 == argCount) {
                if (IS_NUMBER(PEEK())) {
                    double index = AS_NUMBER(PEEK());
                    if (IS_LIST(NPEEK(1))) {
                        ObjList* list = AS_LIST(NPEEK(1));
                        if (0 > index)
                            index += list->array.count;
                        if (0 <= index && index < list->array.count) {
                            value = list->array.values[(int)index];
                        }
                        else {
                            runtimeError("Out of bound");
                            return INTERPRET_RUNTIME_ERROR;
                        }
                    }
                    else if (IS_STRING(NPEEK(1))) {
                        ObjString* str = AS_STRING(NPEEK(1));
                        int idx = (int)index;
                        if (0 > idx)
                            idx += str->length;
                        if (0 <= idx && idx < str->length) {
                            ObjString* r = copyString(str->chars + idx, 1);
                            value = OBJ_VAL(r);
                        }
                        else {
                            runtimeError("Out of bound");
                            return INTERPRET_RUNTIME_ERROR;
                        }
                    }
                    else {
                        runtimeError("Expects List");
                        return INTERPRET_RUNTIME_ERROR;
                    }
                }
                else if (IS_STRING(PEEK())) {
                    ObjString* key = AS_STRING(PEEK());
                    if (IS_MAP(NPEEK(1))) {
                        ObjMap* map = AS_MAP(NPEEK(1));
                        if (!tableGet(&map->map, key, &value)) {
                            runtimeError("Invalid key");
                            return INTERPRET_RUNTIME_ERROR;
                        }
                    }
                    else {
                        runtimeError("Expects Map");
                        return INTERPRET_RUNTIME_ERROR;
                    }
                }
                else {
                    runtimeError("Invalid index");
                    return INTERPRET_RUNTIME_ERROR;
                }
                DROP();
                DROP();
            }
            else {
                runtimeError("Invalid index dimension");
                return INTERPRET_RUNTIME_ERROR;
            }
            PUSH(value);
            break;
        }
        case OP_SET_ELEMENT: {
            size_t argCount = READ_BYTE();
            Value value = PEEK();
            if (1 == argCount) {
                if (IS_NUMBER(NPEEK(1))) {
                    double index = AS_NUMBER(NPEEK(1));
                    if (IS_LIST(NPEEK(2))) {
                        ObjList* list = AS_LIST(NPEEK(2));
                        if (0 > index)
                            index += list->array.count;
                        if (0 <= index && index < list->array.count) {
                            list->array.values[(int)index] = value;
                        }
                        else {
                            runtimeError("Out of bound");
                            return INTERPRET_RUNTIME_ERROR;
                        }
                    }
                    else {
                        runtimeError("Expects List");
                        return INTERPRET_RUNTIME_ERROR;
                    }
                }
                else if (IS_STRING(NPEEK(1))) {
                    ObjString* key = AS_STRING(NPEEK(1));
                    if (IS_MAP(NPEEK(2))) {
                        ObjMap* map = AS_MAP(NPEEK(2));
                        tableSet(&map->map, key, value);
                    }
                    else {
                        runtimeError("Expects Map");
                        return INTERPRET_RUNTIME_ERROR;
                    }
                }
                else {
                    runtimeError("Invalid index dimension");
                    return INTERPRET_RUNTIME_ERROR;
                }
                DROP();
                DROP();
                DROP();
                PUSH(value);
            }
            break;
        }
            //> Superclasses interpret-get-super
        case OP_GET_SUPER: {
            int name = READ_SHORT();
            ObjClass* superclass = AS_CLASS(POP());

            if (!bindMethod(superclass, name)) {
                return INTERPRET_RUNTIME_ERROR;
            }
            break;
        }
            //< Superclasses interpret-get-super
            //> Types of Values interpret-equal
        case OP_EQUAL: {
            Value b = POP();
            Value a = POP();
            PUSH(BOOL_VAL(valuesEqual(a, b)));
            break;
        }
            //< Types of Values interpret-equal
            //> Types of Values interpret-comparison
        case OP_GREATER:
            BINARY_CMP(BOOL_VAL, >);
            break;
        case OP_LESS:
            BINARY_CMP(BOOL_VAL, <);
            break;
            //< Types of Values interpret-comparison
            /* A Virtual Machine op-binary < Types of Values op-arithmetic
                  case OP_ADD:      BINARY_OP(+); break;
                  case OP_SUBTRACT: BINARY_OP(-); break;
                  case OP_MULTIPLY: BINARY_OP(*); break;
                  case OP_DIVIDE:   BINARY_OP(/); break;
            */
            /* A Virtual Machine op-negate < Types of Values op-negate
                  case OP_NEGATE:   PUSH(-POP()); break;
            */
            /* Types of Values op-arithmetic < Strings add-strings
                  case OP_ADD:      BINARY_OP(NUMBER_VAL, +); break;
            */
            //> Strings add-strings
        case OP_ADD: {
            if (IS_NUMBER(PEEK())) {
                double b = AS_NUMBER(POP());
                if (IS_NUMBER(PEEK())) {
                    double a = AS_NUMBER(POP());
                    PUSH(NUMBER_VAL(a + b));
                }
                else if (IS_COMPLEX(PEEK())) {
                    ObjComplex* a = AS_COMPLEX(POP());
                    PUSH(OBJ_VAL(newComplex(a->real + b, a->imag)));
                }
                else {
                    runtimeError("Operands must be two numbers or two strings.");
                    return INTERPRET_RUNTIME_ERROR;
                }
            }
            else if (IS_COMPLEX(PEEK())) {
                ObjComplex* b = AS_COMPLEX(POP());
                if (IS_NUMBER(PEEK())) {
                    double a = AS_NUMBER(POP());
                    PUSH(OBJ_VAL(newComplex(a + b->real, b->imag)));
                }
                else if (IS_COMPLEX(PEEK())) {
                    ObjComplex* a = AS_COMPLEX(POP());
                    PUSH(OBJ_VAL(newComplex(a->real + b->real, a->imag + b->imag)));
                }
                else {
                    runtimeError("Operands must be two numbers or two strings.");
                    return INTERPRET_RUNTIME_ERROR;
                }
            }
            else if (IS_STRING(PEEK()) && IS_STRING(NPEEK(1))) {
                concatenate();
            }
            else if (IS_VECTOR(PEEK()) && IS_VECTOR(NPEEK(1))) {
                ObjVector* b = AS_VECTOR(POP());
                ObjVector* a = AS_VECTOR(POP());
                if (a->isRow != b->isRow || a->size != b->size) {
                    runtimeError("Vector dimension mismatch.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                ObjVector* r = duplicateVector(b);
                // Performs: y = alpha*x + y
                cblas_daxpy(r->size, 1.0, a->values, 1, r->values, 1);
                PUSH(OBJ_VAL(r));
            }
            else if (IS_MATRIX(PEEK()) && IS_MATRIX(NPEEK(1))) {
                ObjMatrix* b = AS_MATRIX(POP());
                ObjMatrix* a = AS_MATRIX(POP());
                if (b->rows != a->rows || b->columns != a->columns) {
                    runtimeError("Matrix dimension mismatch.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                ObjMatrix* r = duplicateMatrix(b);
                // Performs: y = alpha*x + y
                cblas_daxpy(r->rows * r->columns, 1.0, a->values, 1, r->values, 1);
                PUSH(OBJ_VAL(r));
            }
            else {
                runtimeError("Operands must be two numbers or two strings.");
                return INTERPRET_RUNTIME_ERROR;
            }
            break;
        }
            //< Strings add-strings
            //> Types of Values op-arithmetic
        case OP_SUBTRACT: {
            if (IS_NUMBER(PEEK())) {
                double b = AS_NUMBER(POP());
                if (IS_NUMBER(PEEK())) {
                    double a = AS_NUMBER(POP());
                    PUSH(NUMBER_VAL(a - b));
                }
                else if (IS_COMPLEX(PEEK())) {
                    ObjComplex* a = AS_COMPLEX(POP());
                    PUSH(OBJ_VAL(newComplex(a->real - b, a->imag)));
                }
                else {
                    runtimeError("Operands must be numbers.");
                    return INTERPRET_RUNTIME_ERROR;
                }
            }
            else if (IS_COMPLEX(PEEK())) {
                ObjComplex* b = AS_COMPLEX(POP());
                if (IS_NUMBER(PEEK())) {
                    double a = AS_NUMBER(POP());
                    PUSH(OBJ_VAL(newComplex(a - b->real, b->imag)));
                }
                else if (IS_COMPLEX(PEEK())) {
                    ObjComplex* a = AS_COMPLEX(POP());
                    PUSH(OBJ_VAL(newComplex(a->real - b->real, a->imag - b->imag)));
                }
                else {
                    runtimeError("Operands must be numbers.");
                    return INTERPRET_RUNTIME_ERROR;
                }
            }
            else if (IS_VECTOR(PEEK()) && IS_VECTOR(NPEEK(1))) {
                ObjVector* b = AS_VECTOR(POP());
                ObjVector* a = AS_VECTOR(POP());
                if (a->isRow != b->isRow || a->size != b->size) {
                    runtimeError("Vector dimension mismatch.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                ObjVector* r = duplicateVector(a);
                // Performs: y = alpha*x + y
                cblas_daxpy(r->size, -1.0, b->values, 1, r->values, 1);
                PUSH(OBJ_VAL(r));
            }
            else if (IS_MATRIX(PEEK()) && IS_MATRIX(NPEEK(1))) {
                ObjMatrix* b = AS_MATRIX(POP());
                ObjMatrix* a = AS_MATRIX(POP());
                if (b->rows != a->rows || b->columns != a->columns) {
                    runtimeError("Matrix dimension mismatch.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                ObjMatrix* r = duplicateMatrix(a);
                // Performs: y = alpha*x + y
                cblas_daxpy(r->rows * r->columns, -1.0, b->values, 1, r->values, 1);
                PUSH(OBJ_VAL(r));
            }
            else {
                runtimeError("Operands must be numbers.");
                return INTERPRET_RUNTIME_ERROR;
            }
            break;
        }
        case OP_MULTIPLY: {
            if (IS_NUMBER(PEEK())) {
                double b = AS_NUMBER(POP());
                if (IS_NUMBER(PEEK())) {
                    double a = AS_NUMBER(POP());
                    PUSH(NUMBER_VAL(a * b));
                }
                else if (IS_COMPLEX(PEEK())) {
                    ObjComplex* a = AS_COMPLEX(POP());
                    PUSH(OBJ_VAL(newComplex(a->real * b, a->imag * b)));
                }
                else {
                    runtimeError("Operands must be numbers.");
                    return INTERPRET_RUNTIME_ERROR;
                }
            }
            else if (IS_COMPLEX(PEEK())) {
                ObjComplex* b = AS_COMPLEX(POP());
                if (IS_NUMBER(PEEK())) {
                    double a = AS_NUMBER(POP());
                    PUSH(OBJ_VAL(newComplex(a * b->real, a * b->imag)));
                }
                else if (IS_COMPLEX(PEEK())) {
                    ObjComplex* a = AS_COMPLEX(POP());
                    PUSH(OBJ_VAL(
                        newComplex(a->real * b->real - a->imag * b->imag, a->real * b->imag + a->imag * b->real)));
                }
                else {
                    runtimeError("Operands must be numbers.");
                    return INTERPRET_RUNTIME_ERROR;
                }
            }
            else if (IS_VECTOR(PEEK())) {
                ObjVector* b = AS_VECTOR(POP());
                if (IS_NUMBER(PEEK())) {
                    double a = AS_NUMBER(POP());
                    ObjVector* r = duplicateVector(b);
                    cblas_dscal(r->size, a, r->values, 1);
                    PUSH(OBJ_VAL(r));
                }
                else if (IS_VECTOR(PEEK())) {
                    ObjVector* a = AS_VECTOR(POP());
                    if (a->size != b->size || a->isRow == b->isRow) {
                        runtimeError("Vector dimensions mismatch.");
                        return INTERPRET_RUNTIME_ERROR;
                    }
                    int M = (a->isRow) ? 1 : a->size; // rows of A
                    int N = (b->isRow) ? b->size : 1; // columns of B
                    int K = (a->isRow) ? a->size : 1; // rows of B == columns of A
                    double alpha = 1.0;
                    double beta = 0.0;
                    int lda = K;
                    int ldb = N;
                    int ldc = N;
                    ObjMatrix* r = newMatrix(M, N, 0);
                    // Perform matrix multiplication: C = alpha * A * B * beta * C
                    cblas_dgemm(CblasRowMajor,  // Layout: row-by-row storage
                                CblasNoTrans,   // TransA: Do not transpose matrix A
                                CblasNoTrans,   // TransB: Do not transpose matrix B
                                M, N, K,        // Dimensions: rows of A, cols of B, cols of A
                                alpha,          // Scalar scaling product of A and B
                                a->values, lda, // Matrix A pointer and its leading dimension
                                b->values, ldb, // Matrix B pointer and its leading dimension
                                beta,           // Scalar scaling matrix C
                                r->values, ldc  // Matrix C pointer and its leading dimension
                    );
                    PUSH(OBJ_VAL(r));
                }
                else if (IS_MATRIX(PEEK())) {
                    ObjMatrix* a = AS_MATRIX(POP());
                    if (b->isRow) {
                        if (a->columns != 1) {
                            runtimeError("Matirx vs Vector dimension mismatches.");
                            return INTERPRET_RUNTIME_ERROR;
                        }
                    }
                    else {
                        if (a->columns != b->size) {
                            runtimeError("Matirx vs Vector dimension mismatches.");
                            return INTERPRET_RUNTIME_ERROR;
                        }
                    }
                    int M = a->rows;                  // rows of A
                    int N = (b->isRow) ? b->size : 1; // columns of B
                    int K = a->columns;               // rows of B == columns of A
                    double alpha = 1.0;
                    double beta = 0.0;
                    int lda = K;
                    int ldb = N;
                    int ldc = N;
                    ObjMatrix* r = newMatrix(M, N, 0);
                    // Perform matrix multiplication: C = alpha * A * B * beta * C
                    cblas_dgemm(CblasRowMajor,  // Layout: row-by-row storage
                                CblasNoTrans,   // TransA: Do not transpose matrix A
                                CblasNoTrans,   // TransB: Do not transpose matrix B
                                M, N, K,        // Dimensions: rows of A, cols of B, cols of A
                                alpha,          // Scalar scaling product of A and B
                                a->values, lda, // Matrix A pointer and its leading dimension
                                b->values, ldb, // Matrix B pointer and its leading dimension
                                beta,           // Scalar scaling matrix C
                                r->values, ldc  // Matrix C pointer and its leading dimension
                    );
                    PUSH(OBJ_VAL(r));
                }
                else {
                    runtimeError("Operands must be numbers.");
                    return INTERPRET_RUNTIME_ERROR;
                }
            }
            else if (IS_MATRIX(PEEK())) {
                ObjMatrix* b = AS_MATRIX(POP());
                if (IS_NUMBER(PEEK())) {
                    double a = AS_NUMBER(POP());
                    ObjMatrix* r = duplicateMatrix(b);
                    cblas_dscal(r->rows * r->columns, a, r->values, 1);
                    PUSH(OBJ_VAL(r));
                }
                else if (IS_VECTOR(PEEK())) {
                    ObjVector* a = AS_VECTOR(POP());
                    if (a->isRow) {
                        if (a->size != b->columns) {
                            runtimeError("Matirx vs Vector dimension mismatches.");
                            return INTERPRET_RUNTIME_ERROR;
                        }
                    }
                    else {
                        if (1 != b->rows) {
                            runtimeError("Matirx vs Vector dimension mismatches.");
                            return INTERPRET_RUNTIME_ERROR;
                        }
                    }
                    int M = (a->isRow) ? 1 : a->size; // rows of A
                    int N = b->columns;               // columns of B
                    int K = (a->isRow) ? a->size : 1; // rows of B == columns of A
                    double alpha = 1.0;
                    double beta = 0.0;
                    int lda = K;
                    int ldb = N;
                    int ldc = N;
                    ObjMatrix* r = newMatrix(M, N, 0);
                    // Perform matrix multiplication: C = alpha * A * B * beta * C
                    cblas_dgemm(CblasRowMajor,  // Layout: row-by-row storage
                                CblasNoTrans,   // TransA: Do not transpose matrix A
                                CblasNoTrans,   // TransB: Do not transpose matrix B
                                M, N, K,        // Dimensions: rows of A, cols of B, cols of A
                                alpha,          // Scalar scaling product of A and B
                                a->values, lda, // Matrix A pointer and its leading dimension
                                b->values, ldb, // Matrix B pointer and its leading dimension
                                beta,           // Scalar scaling matrix C
                                r->values, ldc  // Matrix C pointer and its leading dimension
                    );
                    PUSH(OBJ_VAL(r));
                }
                else if (IS_MATRIX(PEEK())) {
                    ObjMatrix* a = AS_MATRIX(POP());
                    if (a->columns != b->rows) {
                        runtimeError("Matirx dimension mismatches.");
                        return INTERPRET_RUNTIME_ERROR;
                    }
                    int M = a->rows;    // rows of A
                    int N = b->columns; // columns of B
                    int K = a->columns; // rows of B == columns of A
                    double alpha = 1.0;
                    double beta = 0.0;
                    int lda = K;
                    int ldb = N;
                    int ldc = N;
                    ObjMatrix* r = newMatrix(M, N, 0);
                    // Perform matrix multiplication: C = alpha * A * B * beta * C
                    cblas_dgemm(CblasRowMajor,  // Layout: row-by-row storage
                                CblasNoTrans,   // TransA: Do not transpose matrix A
                                CblasNoTrans,   // TransB: Do not transpose matrix B
                                M, N, K,        // Dimensions: rows of A, cols of B, cols of A
                                alpha,          // Scalar scaling product of A and B
                                a->values, lda, // Matrix A pointer and its leading dimension
                                b->values, ldb, // Matrix B pointer and its leading dimension
                                beta,           // Scalar scaling matrix C
                                r->values, ldc  // Matrix C pointer and its leading dimension
                    );
                    PUSH(OBJ_VAL(r));
                }
                else {
                    runtimeError("Operands must be numbers.");
                    return INTERPRET_RUNTIME_ERROR;
                }
            }
            else {
                runtimeError("Operands must be numbers.");
                return INTERPRET_RUNTIME_ERROR;
            }
            break;
        }
        case OP_DIVIDE: {
            if (IS_NUMBER(PEEK())) {
                double b = AS_NUMBER(POP());
                if (IS_NUMBER(PEEK())) {
                    double a = AS_NUMBER(POP());
                    PUSH(NUMBER_VAL(a / b));
                }
                else if (IS_COMPLEX(PEEK())) {
                    ObjComplex* a = AS_COMPLEX(POP());
                    PUSH(OBJ_VAL(newComplex(a->real / b, a->imag / b)));
                }
                else if (IS_VECTOR(PEEK())) {
                    ObjVector* a = AS_VECTOR(POP());
                    ObjVector* r = duplicateVector(a);
                    cblas_dscal(r->size, 1 / b, r->values, 1);
                    PUSH(OBJ_VAL(r));
                }
                else if (IS_MATRIX(PEEK())) {
                    ObjMatrix* a = AS_MATRIX(POP());
                    ObjMatrix* r = duplicateMatrix(a);
                    cblas_dscal(r->rows * r->columns, 1 / b, r->values, 1);
                    PUSH(OBJ_VAL(r));
                }
                else {
                    runtimeError("Operands must be numbers.");
                    return INTERPRET_RUNTIME_ERROR;
                }
            }
            else if (IS_COMPLEX(PEEK())) {
                ObjComplex* b = AS_COMPLEX(POP());
                if (IS_NUMBER(PEEK())) {
                    double a = AS_NUMBER(POP());
                    double denom = b->real * b->real - b->imag * b->imag;
                    PUSH(OBJ_VAL(newComplex(a * b->real / denom, -a * b->imag / denom)));
                }
                else if (IS_COMPLEX(PEEK())) {
                    ObjComplex* a = AS_COMPLEX(POP());
                    double denom = b->real * b->real - b->imag * b->imag;
                    PUSH(OBJ_VAL(newComplex((a->real * b->real + a->imag * b->imag) / denom,
                                            (a->real * b->imag - a->imag * b->real) / denom)));
                }
                else {
                    runtimeError("Operands must be numbers.");
                    return INTERPRET_RUNTIME_ERROR;
                }
            }
            else {
                runtimeError("Operands must be numbers.");
                return INTERPRET_RUNTIME_ERROR;
            }

            break;
        }
        case OP_MODULO:
            POWER_OP(NUMBER_VAL, fmod);
            break;
        case OP_EXPONENT:
            POWER_OP(NUMBER_VAL, pow);
            break;
            //< Types of Values op-arithmetic
            //> Types of Values op-not
        case OP_NOT: {
            Value b = POP();
            bool v = IS_FALSEY(b);
            PUSH(BOOL_VAL(v));
            break;
        }
            //< Types of Values op-not
            //> Types of Values op-negate
        case OP_NEGATE:
            if (IS_NUMBER(PEEK())) {
                double v = AS_NUMBER(POP());
                PUSH(NUMBER_VAL(-v));
            }
            else if (IS_VECTOR(PEEK())) {
                ObjVector* a = AS_VECTOR(POP());
                ObjVector* r = duplicateVector(a);
                cblas_dscal(r->size, -1, r->values, 1);
                PUSH(OBJ_VAL(r));
            }
            else if (IS_MATRIX(PEEK())) {
                ObjMatrix* a = AS_MATRIX(POP());
                ObjMatrix* r = duplicateMatrix(a);
                cblas_dscal(r->rows * r->columns, -1, r->values, 1);
                PUSH(OBJ_VAL(r));
            }
            else {
                runtimeError("Operand must be a number.");
                return INTERPRET_RUNTIME_ERROR;
            }
            break;
            //< Types of Values op-negate
            //> Global Variables interpret-print
        case OP_PRINT: {
            printValue(POP());
            printf("\n");
            break;
        }
            //< Global Variables interpret-print
            //> Jumping Back and Forth op-jump
        case OP_JUMP: {
            uint16_t offset = READ_SHORT();
            /* Jumping Back and Forth op-jump < Calls and Functions jump
                    vm.ip += offset;
            */
            //> Calls and Functions jump
            frame->ip += offset;
            //< Calls and Functions jump
            break;
        }
            //< Jumping Back and Forth op-jump
            //> Jumping Back and Forth op-jump-if-false
        case OP_JUMP_IF_FALSE: {
            uint16_t offset = READ_SHORT();
            /* Jumping Back and Forth op-jump-if-false < Calls and Functions jump-if-false
                    if (IS_FALSEY(PEEK())) vm.ip += offset;
            */
            //> Calls and Functions jump-if-false
            if (IS_FALSEY(PEEK()))
                frame->ip += offset;
            //< Calls and Functions jump-if-false
            break;
        }
            //< Jumping Back and Forth op-jump-if-false
            //> Jumping Back and Forth op-loop
        case OP_LOOP: {
            uint16_t offset = READ_SHORT();
            /* Jumping Back and Forth op-loop < Calls and Functions loop
                    vm.ip -= offset;
            */
            //> Calls and Functions loop
            frame->ip -= offset;
            //< Calls and Functions loop
            break;
        }
            //< Jumping Back and Forth op-loop
            //> Calls and Functions interpret-call
        case OP_CALL: {
            int argCount = READ_BYTE();
            if (!callValue(NPEEK(argCount), argCount)) {
                return INTERPRET_RUNTIME_ERROR;
            }
            //> update-frame-after-call
            frame = &vm.thread->frames[vm.thread->frameCount - 1];
            //< update-frame-after-call
            break;
        }
            //< Calls and Functions interpret-call
            //> Methods and Initializers interpret-invoke
        case OP_INVOKE: {
            int method = READ_SHORT();
            int argCount = READ_BYTE();
            if (!invoke(method, argCount)) {
                return INTERPRET_RUNTIME_ERROR;
            }
            frame = &vm.thread->frames[vm.thread->frameCount - 1];
            break;
        }
            //< Methods and Initializers interpret-invoke
            //> Superclasses interpret-super-invoke
        case OP_SUPER_INVOKE: {
            int method = READ_SHORT();
            int argCount = READ_BYTE();
            ObjClass* superclass = AS_CLASS(POP());
            if (!invokeFromClass(superclass, method, argCount, false)) {
                return INTERPRET_RUNTIME_ERROR;
            }
            frame = &vm.thread->frames[vm.thread->frameCount - 1];
            break;
        }
            //< Superclasses interpret-super-invoke
            //> Closures interpret-closure
        case OP_CLOSURE: {
            ObjFunction* function = AS_FUNCTION(READ_CONSTANT());
            ObjClosure* closure = newClosure(function);
            PUSH(OBJ_VAL(closure));
            //> interpret-capture-upvalues
            for (int i = 0; i < closure->upvalueCount; i++) {
                uint8_t isLocal = READ_BYTE();
                uint8_t index = READ_BYTE();
                if (isLocal) {
                    closure->upvalues[i] = captureUpvalue(frame->slots + index);
                }
                else {
                    closure->upvalues[i] = frame->closure->upvalues[index];
                }
            }
            //< interpret-capture-upvalues
            break;
        }
            //< Closures interpret-closure
            //> Closures interpret-close-upvalue
        case OP_CLOSE_UPVALUE:
            closeUpvalues(vm.thread->stackTop - 1);
            DROP();
            break;
            //< Closures interpret-close-upvalue
        case OP_RETURN: {
            /* A Virtual Machine print-return < Global Variables op-return
                    printValue(POP());
                    printf("\n");
            */
            /* Global Variables op-return < Calls and Functions interpret-return
                    // Exit interpreter.
            */
            /* A Virtual Machine run < Calls and Functions interpret-return
                    return INTERPRET_OK;
            */
            //> Calls and Functions interpret-return
            Value result = POP();
            //> Closures return-close-upvalues
            closeUpvalues(frame->slots);
            //< Closures return-close-upvalues
            vm.thread->frameCount--;
            if (vm.thread->frameCount == 0) {
                if (NULL == vm.thread->caller) {
                    DROP();
                    return INTERPRET_OK;
                }
                else {
                    ThreadType t = vm.thread->type;
                    vm.thread = vm.thread->caller;
                    frame = &vm.thread->frames[vm.thread->frameCount - 1];
                    if (THREAD_TYPE_EPHIMERAL == t) {
                        return INTERPRET_OK;
                    }
                    else {
                        continue;
                    }
                }
            }

            vm.thread->stackTop = frame->slots;
            PUSH(result);
            frame = &vm.thread->frames[vm.thread->frameCount - 1];
            break;
            //< Calls and Functions interpret-return
        }
            //> Classes and Instances interpret-class
        case OP_CLASS:
            PUSH(OBJ_VAL(newClass(READ_STRING())));
            break;
            //< Classes and Instances interpret-class
            //> Superclasses interpret-inherit
        case OP_LIST: {
            size_t argCount = READ_BYTE();
            Value lst = vm.listClass->call->function(argCount, vm.thread->stackTop - argCount);
            vm.thread->stackTop -= argCount;
            PUSH(lst);
            break;
        }
        case OP_MAP: {
            size_t argCount = READ_BYTE();
            Value lst = vm.mapClass->call->function(argCount, vm.thread->stackTop - argCount);
            vm.thread->stackTop -= argCount;
            PUSH(lst);
            break;
        }
        case OP_INHERIT: {
            Value superclass = NPEEK(1);
            //> inherit-non-class
            if (!IS_CLASS(superclass)) {
                runtimeError("Superclass must be a class.");
                return INTERPRET_RUNTIME_ERROR;
            }

            //< inherit-non-class
            ObjClass* pSuperClass = AS_CLASS(superclass);
            ObjClass* subclass = AS_CLASS(PEEK());
            // tableAddAll(&AS_CLASS(superclass)->methods, &subclass->methods);
            for (int i = 0; i < pSuperClass->methods.count; i++) {
                writeClassMemberArray(&subclass->methods, pSuperClass->methods.values[i]);
                /*
            if (IS_UNDEF(pSuperClass->methods.values[i]))
                continue;
            setAtValueArray(&subclass->methods, i, pSuperClass->methods.values[i]);
            */
            }
            LAX_LOG_ARRAY(subclass->methods);
            DROP(); // Subclass.
            break;
        }
            //< Superclasses interpret-inherit
            //> Methods and Initializers interpret-method
        case OP_METHOD:
            defineMethod(READ_SHORT());
            break;
            //< Methods and Initializers interpret-method
        }
    }

#undef READ_BYTE
//> Jumping Back and Forth undef-read-short
#undef READ_SHORT
//< Jumping Back and Forth undef-read-short
//> undef-read-constant
#undef READ_CONSTANT
//< undef-read-constant
//> Global Variables undef-read-string
#undef READ_STRING
//< Global Variables undef-read-string
//> undef-binary-op
#undef BINARY_OP
    //< undef-binary-op
}
//< run
//> omit
void hack(bool b)
{
    // Hack to avoid unused function error. run() is not used in the
    // scanning chapter.
    run();
    if (b)
        hack(false);
}
//< omit
//> interpret
/* A Virtual Machine interpret < Scanning on Demand vm-interpret-c
InterpretResult interpret(Chunk* chunk) {
  vm.chunk = chunk;
  vm.ip = vm.chunk->code;
  return run();
*/
//> Scanning on Demand vm-interpret-c
InterpretResult interpret(const char* source)
{
    /* Scanning on Demand vm-interpret-c < Compiling Expressions interpret-chunk
      compile(source);
      return INTERPRET_OK;
    */
    /* Compiling Expressions interpret-chunk < Calls and Functions interpret-stub
      Chunk chunk;
      initChunk(&chunk);

      if (!compile(source, &chunk)) {
        freeChunk(&chunk);
        return INTERPRET_COMPILE_ERROR;
      }

      vm.chunk = &chunk;
      vm.ip = vm.chunk->code;
    */
    //> Calls and Functions interpret-stub
    ObjFunction* function = compile(source);
    if (function == NULL)
        return INTERPRET_COMPILE_ERROR;

    PUSH(OBJ_VAL(function));
    //< Calls and Functions interpret-stub
    /* Calls and Functions interpret-stub < Calls and Functions interpret
      CallFrame* frame = &vm.frames[vm.frameCount++];
      frame->function = function;
      frame->ip = function->chunk.code;
      frame->slots = vm.stack;
    */
    /* Calls and Functions interpret < Closures interpret
      call(function, 0);
    */
    //> Closures interpret
    ObjClosure* closure = newClosure(function);
    DROP();
    PUSH(OBJ_VAL(closure));
    call(closure, 0);
    //< Closures interpret
    //< Scanning on Demand vm-interpret-c
    //> Compiling Expressions interpret-chunk

    /* Compiling Expressions interpret-chunk < Calls and Functions end-interpret
      InterpretResult result = run();

      freeChunk(&chunk);
      return result;
    */
    //> Calls and Functions end-interpret
    return run();
    //< Calls and Functions end-interpret
    //< Compiling Expressions interpret-chunk
}

bool loadLibrary(Path* path, String* dl_name)
{
    const char** names;
    Value* values = NULL;

    if (NULL == vm.dls) {
        vm.dlCapacity = GROW_CAPACITY(vm.dlCapacity);
        vm.dls = ALLOCATE(DL*, vm.dlCapacity);
    }
    else if (vm.dlCount == vm.dlCapacity) {
        vm.dlCapacity++;
        vm.dlCapacity = GROW_CAPACITY(vm.dlCapacity);
        vm.dls = GROW_ARRAY(DL*, vm.dls, vm.dlCount, vm.dlCapacity);
    }
    DL* dl = dlNew(path, dl_name);
    vm.dls[vm.dlCount++] = dl;
    dlSymbols(dl, &names, &values);
    for (size_t i = 0; NULL != names[i]; i++)
        defineValue(names[i], values[i]);
    return true;
}

bool runThread(Value f, int argc, Value* argv)
{
    ObjThread* thread = newThread(THREAD_TYPE_EPHIMERAL);
    thread->caller = vm.thread;
    vm.thread = thread;
    for (int i = 0; i < argc; i++)
        PUSH(argv[i]);
    bool ret = callValue(f, argc);
    if (ret)
        run();
    return ret;
}

uint16_t getGlobalAddress(ObjString* name, Value defval)
{
    Value v;
    uint16_t offset = 0;
    if (!tableGet(&vm.symtabGlobals, name, &v)) {
        offset = vm.globals.count;
        tableSet(&vm.symtabGlobals, name, NUMBER_VAL(vm.globals.count));
        writeValueArray(&vm.globals, defval);
    }
    else {
        offset = AS_NUMBER(v);
    }
    LAX_LOG("%s->%d", name->chars, offset);
    return offset;
}

uint16_t getMethodAddress(ObjString* name)
{
    static unsigned int methodsCount = 0;
    Value v;
    uint16_t offset = 0;
    if (!tableGet(&vm.symtabMethods, name, &v)) {
        offset = methodsCount++;
        tableSet(&vm.symtabMethods, name, NUMBER_VAL(offset));
    }
    else {
        offset = AS_NUMBER(v);
    }
    LAX_LOG("%s->%d", name->chars, offset);
    return offset;
}

const char* undefinedSymbol(uint16_t addr)
{
    Value v = NUMBER_VAL(addr);
    for (int i = 0; i < vm.symtabGlobals.capacity; i++) {
        if (valuesEqual(vm.symtabGlobals.entries[i].value, v)) {
            return vm.symtabGlobals.entries[i].key->chars;
        }
    }
    return "";
}

const char* undefinedMethod(uint16_t addr)
{
    Value v = NUMBER_VAL(addr);
    for (int i = 0; i < vm.symtabMethods.capacity; i++) {
        if (valuesEqual(vm.symtabMethods.entries[i].value, v)) {
            return vm.symtabMethods.entries[i].key->chars;
        }
    }
    return "";
}
//< interpret
