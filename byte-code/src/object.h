//> Strings object-h
#ifndef clox_object_h
#define clox_object_h

#include "common.h"
//> Calls and Functions object-include-chunk
#include "chunk.h"
//< Calls and Functions object-include-chunk
//> Classes and Instances object-include-table
#include "table.h"
//< Classes and Instances object-include-table
#include "value.h"
//> obj-type-macro

#define OBJ_TYPE(value) (AS_OBJ(value)->type)
//< obj-type-macro
//> is-string

//> Methods and Initializers is-bound-method
#define IS_BOUND_METHOD(value) isObjType(value, OBJ_BOUND_METHOD)
//< Methods and Initializers is-bound-method
//> Classes and Instances is-class
#define IS_CLASS(value) isObjType(value, OBJ_CLASS)
//< Classes and Instances is-class
//> Closures is-closure
#define IS_CLOSURE(value) isObjType(value, OBJ_CLOSURE)
//< Closures is-closure
//> Calls and Functions is-function
#define IS_FUNCTION(value) isObjType(value, OBJ_FUNCTION)
//< Calls and Functions is-function
//> Classes and Instances is-instance
#define IS_INSTANCE(value) isObjType(value, OBJ_INSTANCE)
//< Classes and Instances is-instance
//> Calls and Functions is-native
#define IS_NATIVE(value) isObjType(value, OBJ_NATIVE)
#define IS_NATIVE_BOUND_METHOD(value) isObjType(value, OBJ_NATIVE_BOUND_METHOD)
//< Calls and Functions is-native
#define IS_STRING(value) isObjType(value, OBJ_STRING)
//< is-string
#define IS_COMPLEX(value) isObjType(value, OBJ_COMPLEX)
//> as-string
#define IS_LIST(value) isObjType(value, OBJ_LIST)
#define IS_MAP(value) isObjType(value, OBJ_MAP)
#define IS_VECTOR(value) isObjType(value, OBJ_VECTOR)
#define IS_MATRIX(value) isObjType(value, OBJ_MATRIX)
#define IS_THREAD(value) isObjType(value, OBJ_THREAD)

//> Methods and Initializers as-bound-method
#define AS_BOUND_METHOD(value) ((ObjBoundMethod*)AS_OBJ(value))
//< Methods and Initializers as-bound-method
//> Classes and Instances as-class
#define AS_CLASS(value) ((ObjClass*)AS_OBJ(value))
//< Classes and Instances as-class
//> Closures as-closure
#define AS_CLOSURE(value) ((ObjClosure*)AS_OBJ(value))
//< Closures as-closure
//> Calls and Functions as-function
#define AS_FUNCTION(value) ((ObjFunction*)AS_OBJ(value))
//< Calls and Functions as-function
//> Classes and Instances as-instance
#define AS_INSTANCE(value) ((ObjInstance*)AS_OBJ(value))
//< Classes and Instances as-instance
//> Calls and Functions as-native
#define AS_NATIVE(value) (((ObjNative*)AS_OBJ(value))->function)
#define AS_NATIVE_BOUND_METHOD(value) (((ObjNativeBoundMethod*)AS_OBJ(value))->method)
//< Calls and Functions as-native
#define AS_STRING(value) ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)
//< as-string
#define AS_COMPLEX(value) ((ObjComplex*)AS_OBJ(value))
//> obj-type
#define AS_LIST(value) ((ObjList*)AS_OBJ(value))
#define AS_MAP(value) ((ObjMap*)AS_OBJ(value))
#define AS_VECTOR(value) ((ObjVector*)AS_OBJ(value))
#define AS_MATRIX(value) ((ObjMatrix*)AS_OBJ(value))
#define AS_THREAD(value) ((ObjThread*)AS_OBJ(value))

typedef enum {
    //> Methods and Initializers obj-type-bound-method
    OBJ_BOUND_METHOD,
    //< Methods and Initializers obj-type-bound-method
    //> Classes and Instances obj-type-class
    OBJ_CLASS,
    //< Classes and Instances obj-type-class
    //> Closures obj-type-closure
    OBJ_CLOSURE,
    //< Closures obj-type-closure
    //> Calls and Functions obj-type-function
    OBJ_FUNCTION,
    //< Calls and Functions obj-type-function
    //> Classes and Instances obj-type-instance
    OBJ_INSTANCE,
    //< Classes and Instances obj-type-instance
    //> Calls and Functions obj-type-native
    OBJ_NATIVE,
    OBJ_NATIVE_BOUND_METHOD,
    //< Calls and Functions obj-type-native
    OBJ_STRING,
    //> Closures obj-type-upvalue
    OBJ_UPVALUE,
    //< Closures obj-type-upvalue
    OBJ_COMPLEX,
    OBJ_LIST,
    OBJ_MAP,
    OBJ_VECTOR,
    OBJ_MATRIX,
    OBJ_THREAD
} ObjType;
//< obj-type

struct Obj {
    ObjType type;
    //> Garbage Collection is-marked-field
    bool isMarked;
    //< Garbage Collection is-marked-field
    //> next-field
    struct Obj* next;
    //< next-field
};
//> Calls and Functions obj-function

typedef struct {
    Obj obj;
    int arity;
    //> Closures upvalue-count
    int upvalueCount;
    //< Closures upvalue-count
    Chunk chunk;
    ObjString* name;
} ObjFunction;
//< Calls and Functions obj-function
//> Calls and Functions obj-native

typedef Value (*NativeFn)(int argCount, Value* args);

typedef struct {
    Obj obj;
    NativeFn function;
} ObjNative;
//< Calls and Functions obj-native
//> obj-string

struct ObjString {
    Obj obj;
    int length;
    char* chars;
    //> Hash Tables obj-string-hash
    uint32_t hash;
    //< Hash Tables obj-string-hash
};
//< obj-string
//> Closures obj-upvalue
typedef struct ObjUpvalue {
    Obj obj;
    Value* location;
    //> closed-field
    Value closed;
    //< closed-field
    //> next-field
    struct ObjUpvalue* next;
    //< next-field
} ObjUpvalue;
//< Closures obj-upvalue
//> Closures obj-closure
typedef struct {
    Obj obj;
    ObjFunction* function;
    //> upvalue-fields
    ObjUpvalue** upvalues;
    int upvalueCount;
    //< upvalue-fields
} ObjClosure;
//< Closures obj-closure
//> Classes and Instances obj-class

typedef Value (*NativeBoundMethod)(Value receiver, int argCount, Value* args);

typedef struct {
    Obj obj;
    NativeBoundMethod method;
} ObjNativeBoundMethod;

typedef struct {
    Obj obj;
    ObjString* name;
    //> Methods and Initializers class-methods
    ValueArray methods;
    //< Methods and Initializers class-methods
    ValueArray staticMethods;
    ObjNative* call;
} ObjClass;
//< Classes and Instances obj-class
//> Classes and Instances obj-instance

typedef struct {
    Obj obj;
    ObjClass* klass;
    ValueArray fields; // [fields]
} ObjInstance;
//< Classes and Instances obj-instance

//> Methods and Initializers obj-bound-method
typedef struct {
    Obj obj;
    Value receiver;
    ObjClosure* method;
} ObjBoundMethod;

typedef struct {
    Obj obj;
    double real;
    double imag;
} ObjComplex;

typedef struct {
    Obj obj;
    ValueArray array;
} ObjList;

typedef struct {
    Obj obj;
    Table map;
} ObjMap;

typedef struct {
    Obj obj;
    bool isRow;
    int size;
    double* values;
} ObjVector;

typedef struct {
    Obj obj;
    int rows;
    int columns;
    double* values;
} ObjMatrix;

/* A Virtual Machine stack-max < Calls and Functions frame-max
#define STACK_MAX 256
*/
//> Calls and Functions frame-max
#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

//> Calls and Functions call-frame

typedef struct {
    /* Calls and Functions call-frame < Closures call-frame-closure
      ObjFunction* function;
    */
    //> Closures call-frame-closure
    ObjClosure* closure;
    //< Closures call-frame-closure
    uint8_t* ip;
    Value* slots;
} CallFrame;
//< Calls and Functions call-frame
typedef enum {
    // called by another thread and hands over the control to the caller
    THREAD_TYPE_PROCESS,
    // called by VM directly,  simply stops running
    THREAD_TYPE_EPHIMERAL
} ThreadType;
typedef struct sObjThread {
    Obj obj;
    ThreadType type;
    //> Calls and Functions frame-array
    CallFrame frames[FRAMES_MAX];
    int frameCount;

    //< Calls and Functions frame-array
    //> vm-stack
    Value stack[STACK_MAX];
    Value* stackTop;
    //< vm-stack
    struct sObjThread* caller;
} ObjThread;

//< Methods and Initializers obj-bound-method
//> Methods and Initializers new-bound-method-h
ObjBoundMethod* newBoundMethod(Value receiver, ObjClosure* method);
//< Methods and Initializers new-bound-method-h
//> Classes and Instances new-class-h
ObjClass* newClass(ObjString* name);
//< Classes and Instances new-class-h
//> Closures new-closure-h
ObjClosure* newClosure(ObjFunction* function);
//< Closures new-closure-h
//> Calls and Functions new-function-h
ObjFunction* newFunction();
//< Calls and Functions new-function-h
//> Classes and Instances new-instance-h
ObjInstance* newInstance(ObjClass* klass);
//< Classes and Instances new-instance-h
//> Calls and Functions new-native-h
ObjNative* newNative(NativeFn function);
ObjNativeBoundMethod* newNativeBoundMethod(NativeBoundMethod function);
//< Calls and Functions new-native-h
//> take-string-h
ObjString* takeString(char* chars, int length);
//< take-string-h
//> copy-string-h
ObjString* copyString(const char* chars, int length);
//> Closures new-upvalue-h
ObjUpvalue* newUpvalue(Value* slot);
//< Closures new-upvalue-h
ObjComplex* newComplex(double real, double imag);
ObjThread* newThread(ThreadType type);
ObjClass* newNumClass();
ObjClass* newListClass();
ObjClass* newMapClass();
ObjClass* newVectorClass();
ObjClass* newMatrixClass();
ObjClass* newSystemClass();
//> print-object-h
ObjVector* newVector(unsigned int size, double v);
ObjVector* duplicateVector(ObjVector* vector);
ObjMatrix* newMatrix(unsigned int row, unsigned int column, double v);
ObjMatrix* duplicateMatrix(ObjMatrix* matrix);
void printObject(Value value);
//< print-object-h

//< copy-string-h
//> is-obj-type
static inline bool isObjType(Value value, ObjType type)
{
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}
bool objectsEqual(Obj* a, Obj* b);

//< is-obj-type
#endif
