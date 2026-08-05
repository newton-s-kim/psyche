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

void initValueArray(ValueArray* array)
{
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
    array->base = -1;
}
void insertValueArray(ValueArray* array, int index, Value value)
{
    if (array->capacity < array->count + 1) {
        int oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(oldCapacity);
        array->values = GROW_ARRAY(Value, array->values, oldCapacity, array->capacity);
    }
    if (index == array->count) {
        array->values[index] = value;
        array->count++;
    }
    /*
    else if (index == -1) {
        //memmove(array->values, array->values + 1, sizeof(Value) * array->count);
        for(int i = array->count - 1; i >= 0; i--) array->values[i + 1] = array->values[i];
        array->values[0] = value;
        array->count++;
    }
    */
    else if (0 <= index && index < array->count) {
        // memmove(array->values + index, array->values + index + 1, sizeof(Value) * array->count - index);
        for (int i = array->count - 1; i >= index; i--)
            array->values[i + 1] = array->values[i];
        array->values[index] = value;
        array->count++;
    }
    if (-1 == array->base)
        array->base = 0;
}
/*
static inline int power2Ceil(int n)
{
        n--;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        n++;
        return n;
}
*/
void setAtValueArray(ValueArray* array, int index, Value value)
{
    LAX_LOG("enter(0x%p, %d, 0x%lx)", array, index, value);
    LAX_LOG_ARRAY(*array);
    if (array->capacity < index + 1) {
        int oldCapacity = array->capacity;
        while (array->capacity < index + 1) {
            array->capacity = GROW_CAPACITY(array->capacity);
        }
        // array->capacity = power2Ceil(index + 1);
        array->values = GROW_ARRAY(Value, array->values, oldCapacity, array->capacity);
        Value* end = array->values + array->capacity;
        for (Value* i = array->values + oldCapacity; i < end; i++)
            *i = UNDEF_VAL;
    }
    array->values[index] = value;
    if (array->count <= index)
        array->count = index + 1;
    LAX_LOG_ARRAY(*array);
    if (-1 == array->base || array->base > index)
        array->base = index;
}
//> write-value-array
void writeValueArray(ValueArray* array, Value value)
{
    if (array->capacity < array->count + 1) {
        int oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(oldCapacity);
        array->values = GROW_ARRAY(Value, array->values, oldCapacity, array->capacity);
    }

    array->values[array->count] = value;
    array->count++;
    if (-1 == array->base)
        array->base = 0;
}
//< write-value-array
//> free-value-array
void freeValueArray(ValueArray* array)
{
    LAX_LOG("freeValueArray:0x%p", array->values);
    FREE_ARRAY(Value, array->values, array->capacity);
    initValueArray(array);
}
//< free-value-array
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
