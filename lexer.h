#pragma once

enum { TYPE_MNEMONIC, TYPE_REGISTER, TYPE_MEMORY, TYPE_IMM, TYPE_COMMA,
       TYPE_PRAGMA, TYPE_SYNOP, TYPE_VAR, TYPE_NL };

#define is_operand( token ) \
     ( (token).type == TYPE_REGISTER ||  \
       (token).type == TYPE_MEMORY   ||  \
       (token).type == TYPE_IMM      ||  \
       (token).type == TYPE_VAR )

typedef struct{
   int type;
   char *value;
}token_t;

