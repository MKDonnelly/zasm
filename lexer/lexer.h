#pragma once

//enum { TYPE_MNEMONIC, TYPE_REGISTER, TYPE_MEMORY, TYPE_IMM, TYPE_COMMA,
//       TYPE_PRAGMA, TYPE_SYNOP, TYPE_VAR, TYPE_NL, TYPE_STR, TYPE_LABEL };

enum { TYPE_MNEMONIC, TYPE_REGISTER, TYPE_NUMBER, TYPE_STRING, 
       TYPE_IDENTIFIER, }

typedef struct{
   int type;
   char *value;
}token_t;

//When commented, these make the program segfault.
//Why?
token_t *lexer(char *input, int *total);
