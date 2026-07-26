#pragma once

#define DECLARE_ARRAY(name, type)                                                                                      \
    typedef struct {                                                                                                   \
        int capacity;                                                                                                  \
        int count;                                                                                                     \
        type* values;                                                                                                  \
    } name##Array;                                                                                                     \
    void init##name##Array(name##Array* array);                                                                        \
    void write##name##Array(name##Array* array, type value);                                                           \
    void insert##name##Array(name##Array* array, int index, type value);                                               \
    void setAt##name##Array(name##Array* array, int index, type value, type defval);                                   \
    void free##name##Array(name##Array* array);

#define DEFINE_ARRAY(name, type)                                                                                       \
    void init##name##Array(name##Array* array)                                                                         \
    {                                                                                                                  \
        array->values = NULL;                                                                                          \
        array->capacity = 0;                                                                                           \
        array->count = 0;                                                                                              \
    }                                                                                                                  \
    void insert##name##Array(name##Array* array, int index, type value)                                                \
    {                                                                                                                  \
        if (array->capacity < array->count + 1) {                                                                      \
            int oldCapacity = array->capacity;                                                                         \
            array->capacity = GROW_CAPACITY(oldCapacity);                                                              \
            array->values = GROW_ARRAY(type, array->values, oldCapacity, array->capacity);                             \
        }                                                                                                              \
        if (index == array->count) {                                                                                   \
            array->values[index] = value;                                                                              \
            array->count++;                                                                                            \
        }                                                                                                              \
        else if (0 <= index && index < array->count) {                                                                 \
            for (int i = array->count - 1; i >= index; i--)                                                            \
                array->values[i + 1] = array->values[i];                                                               \
            array->values[index] = value;                                                                              \
            array->count++;                                                                                            \
        }                                                                                                              \
    }                                                                                                                  \
    void setAt##name##Array(name##Array* array, int index, type value, type defVal)                                    \
    {                                                                                                                  \
        LAX_LOG("enter(0x%p, %d, 0x%lx)", array, index, value);                                                        \
        LAX_LOG_ARRAY(*array);                                                                                         \
        if (array->capacity < index + 1) {                                                                             \
            int oldCapacity = array->capacity;                                                                         \
            while (array->capacity < index + 1) {                                                                      \
                array->capacity = GROW_CAPACITY(array->capacity);                                                      \
            }                                                                                                          \
            array->values = GROW_ARRAY(type, array->values, oldCapacity, array->capacity);                             \
            type* end = array->values + array->capacity;                                                               \
            for (type* i = array->values + oldCapacity; i < end; i++)                                                  \
                *i = defVal;                                                                                           \
        }                                                                                                              \
        array->values[index] = value;                                                                                  \
        if (array->count <= index)                                                                                     \
            array->count = index + 1;                                                                                  \
        LAX_LOG_ARRAY(*array);                                                                                         \
    }                                                                                                                  \
    void write##name##Array(name##Array* array, type value)                                                            \
    {                                                                                                                  \
        if (array->capacity < array->count + 1) {                                                                      \
            int oldCapacity = array->capacity;                                                                         \
            array->capacity = GROW_CAPACITY(oldCapacity);                                                              \
            array->values = GROW_ARRAY(type, array->values, oldCapacity, array->capacity);                             \
        }                                                                                                              \
        array->values[array->count] = value;                                                                           \
        array->count++;                                                                                                \
    }                                                                                                                  \
    void free##name##Array(name##Array* array)                                                                         \
    {                                                                                                                  \
        FREE_ARRAY(type, array->values, array->capacity);                                                              \
        init##name##Array(array);                                                                                      \
    }
