#pragma once
#include <stdlib.h>

//This holds the parsed input line.
//For example, "movrrq rax, rbx" turns into
// .mnemonic = "movrrq",
// .total_operands = 2;
// .operands[0] = "rax"
// .operands[1] = "rbx"
#define MNEMONIC_MAX_LEN        20
#define MAX_OPERANDS            2
#define OPERANDS_MAX_LEN        20
#define TYPE_DATA	1
#define TYPE_CODE	2
typedef struct{
   int type;
   char mnemonic[MNEMONIC_MAX_LEN];
   int total_operands;
   char operands[MAX_OPERANDS][OPERANDS_MAX_LEN];
}parsed_asm_t;

typedef struct{
   char *buffer;
   int len;
}buffer_t;

static inline buffer_t *create_buffer(int start_len){
   buffer_t *new_buffer = malloc(sizeof(buffer_t));
   new_buffer->buffer = malloc(start_len);
   new_buffer->len = 0;
   return new_buffer;
}

static inline void buffer_append(buffer_t *buf, char val){
   buf->buffer[buf->len] = val;
   buf->len++;
}

char *strip_chars(char *input, char *remove);
void substr_replace(char *line, char *search_for, char *replace_with);
