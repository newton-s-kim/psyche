//> Chunks of Bytecode value-c
#include <stdio.h>
//> Strings value-include-string
#include <string.h>
//< Strings value-include-string

//> Strings value-include-object
#include "object.h"
//< Strings value-include-object
#include "common.h"
#include "log.h"
#include "memory.h"
#include "value.h"

DEFINE_ARRAY(Value, Value);

//> print-value
void printValue(Value value)
{
//> Optimization print-value
#ifdef NAN_BOXING
    if (IS_BOOL(value)) {
        printf(AS_BOOL(value) ? "true" : "false");
    }
    else if (IS_NIL(value)) {
        printf("nil");
    }
    else if (IS_UNDEF(value)) {
        printf("undef");
    }
    else if (IS_NUMBER(value)) {
        printf("%.14g", AS_NUMBER(value));
    }
    else if (IS_OBJ(value)) {
        printObject(value);
    }
#else
    //< Optimization print-value
    /* Chunks of Bytecode print-value < Types of Values print-number-value
      printf("%g", value);
    */
    /* Types of Values print-number-value < Types of Values print-value
     printf("%g", AS_NUMBER(value));
     */
    //> Types of Values print-value
    switch (value.type) {
    case VAL_BOOL:
        printf(AS_BOOL(value) ? "true" : "false");
        break;
    case VAL_NIL:
        printf("nil");
        break;
    case VAL_NUMBER:
        printf("%g", AS_NUMBER(value));
        break;
        //> Strings call-print-object
    case VAL_OBJ:
        printObject(value);
        break;
        //< Strings call-print-object
    }
//< Types of Values print-value
//> Optimization end-print-value
#endif
    //< Optimization end-print-value
}
//< print-value
//> Types of Values values-equal
bool valuesEqual(Value a, Value b)
{
//> Optimization values-equal
#ifdef NAN_BOXING
    //> nan-equality
    if (IS_NUMBER(a) && IS_NUMBER(b)) {
        return AS_NUMBER(a) == AS_NUMBER(b);
    }
    else if (a != b) {
        if (IS_STRING(a) && IS_STRING(b)) {
            return objectsEqual(AS_OBJ(a), AS_OBJ(b));
        }
        else if (IS_VECTOR(a) && IS_VECTOR(b)) {
            return objectsEqual(AS_OBJ(a), AS_OBJ(b));
        }
        else if (IS_MATRIX(a) && IS_MATRIX(b)) {
            return objectsEqual(AS_OBJ(a), AS_OBJ(b));
        }
        else {
            return false;
        }
    }
    //< nan-equality
    return true;
#else
    //< Optimization values-equal
    if (a.type != b.type)
        return false;
    switch (a.type) {
    case VAL_BOOL:
        return AS_BOOL(a) == AS_BOOL(b);
    case VAL_NIL:
        return true;
    case VAL_NUMBER:
        return AS_NUMBER(a) == AS_NUMBER(b);
        /* Strings strings-equal < Hash Tables equal
            case VAL_OBJ: {
              ObjString* aString = AS_STRING(a);
              ObjString* bString = AS_STRING(b);
              return aString->length == bString->length &&
                  memcmp(aString->chars, bString->chars,
                         aString->length) == 0;
            }
         */
        //> Hash Tables equal
    case VAL_OBJ:
        return objectsEqual(AS_OBJ(a), AS_OBJ(b));
        //< Hash Tables equal
    default:
        return false; // Unreachable.
    }
//> Optimization end-values-equal
#endif
    //< Optimization end-values-equal
}
//< Types of Values values-equal
