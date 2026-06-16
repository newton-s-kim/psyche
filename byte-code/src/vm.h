//> A Virtual Machine vm-h
#ifndef clox_vm_h
#define clox_vm_h

/* A Virtual Machine vm-h < Calls and Functions vm-include-object
#include "chunk.h"
*/
//> Calls and Functions vm-include-object
#include "object.h"
//< Calls and Functions vm-include-object
//> Hash Tables vm-include-table
#include "table.h"
//< Hash Tables vm-include-table
//> vm-include-value
#include "value.h"
//< vm-include-value
//> stack-max

//< stack-max
#include "dl.h"

typedef struct {
    /* A Virtual Machine vm-h < Calls and Functions frame-array
      Chunk* chunk;
    */
    /* A Virtual Machine ip < Calls and Functions frame-array
      uint8_t* ip;
    */
    ObjThread* thread;
    //> Global Variables vm-globals
    Table symtabGlobals;
    ValueArray globals;
    //< Global Variables vm-globals
    //> Hash Tables vm-strings
    Table strings;
    //< Hash Tables vm-strings
    //> Methods and Initializers vm-init-string
    ObjString* initString;
    //< Methods and Initializers vm-init-string
    //> Closures open-upvalues-field
    ObjUpvalue* openUpvalues;
    //< Closures open-upvalues-field
    //> Garbage Collection vm-fields

    size_t bytesAllocated;
    size_t nextGC;
    //< Garbage Collection vm-fields
    //> Strings objects-root
    Obj* objects;
    //< Strings objects-root
    //> Garbage Collection vm-gray-stack
    int grayCount;
    int grayCapacity;
    Obj** grayStack;
    //< Garbage Collection vm-gray-stack
    DL** dls;
    size_t dlCount;
    size_t dlCapacity;
    ObjClass* numClass;
    ObjClass* listClass;
    ObjClass* mapClass;
} VM;

//> interpret-result
typedef enum { INTERPRET_OK, INTERPRET_COMPILE_ERROR, INTERPRET_RUNTIME_ERROR } InterpretResult;

//< interpret-result
//> Strings extern-vm
extern VM vm;

void defineNative(const char* name, NativeFn function);
//< Strings extern-vm
void initVM();
void freeVM();
/* A Virtual Machine interpret-h < Scanning on Demand vm-interpret-h
InterpretResult interpret(Chunk* chunk);
*/
//> Scanning on Demand vm-interpret-h
InterpretResult interpret(const char* source);
//< Scanning on Demand vm-interpret-h
//> push-pop
void push(Value value);
Value pop();
//< push-pop
bool loadLibrary(Path* path, String* dl_name);
void runtimeError(const char* format, ...);
bool runThread(Value f, int argc, Value* argv);
uint16_t getGlobalAddress(ObjString* name);
const char* undefinedSymbol(uint16_t addr);

#endif
