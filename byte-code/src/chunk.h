//> Chunks of Bytecode chunk-h
#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
//> chunk-h-include-value
#include "value.h"
//< chunk-h-include-value
//> op-enum

typedef enum {
#define OPCODE(name) name,
#include "opcode.h"
#undef OPCODE
} OpCode;
//< op-enum
//> chunk-struct

typedef struct {
    //> count-and-capacity
    int count;
    int capacity;
    //< count-and-capacity
    uint8_t* code;
    //> chunk-lines
    int* lines;
    //< chunk-lines
    //> chunk-constants
    ValueArray constants;
    //< chunk-constants
} Chunk;
//< chunk-struct
//> init-chunk-h

void initChunk(Chunk* chunk);
//< init-chunk-h
//> free-chunk-h
void freeChunk(Chunk* chunk);
//< free-chunk-h
/* Chunks of Bytecode write-chunk-h < Chunks of Bytecode write-chunk-with-line-h
void writeChunk(Chunk* chunk, uint8_t byte);
*/
//> write-chunk-with-line-h
void writeChunk(Chunk* chunk, uint8_t byte, int line);
//< write-chunk-with-line-h
//> add-constant-h
int addConstant(Chunk* chunk, Value value);
//< add-constant-h

#endif
