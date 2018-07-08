#pragma once

enum { TYPE_MNEMONIC, TYPE_REGISTER, TYPE_MEMORY, TYPE_IMM, TYPE_COMMA,
       TYPE_PRAGMA, TYPE_SYNOP, TYPE_VAR, TYPE_NL, TYPE_STR, TYPE_LABEL };

#define is_operand( token ) \
     ( (token).type == TYPE_REGISTER ||  \
       (token).type == TYPE_MEMORY   ||  \
       (token).type == TYPE_IMM      ||  \
       (token).type == TYPE_VAR      ||  \
       (token).type == TYPE_STR )

typedef struct{
   int type;
   char *value;
}token_t;

typedef struct{
   //The number of operands that the mnemonic
   //accepts as well as the line type. The line
   //type dictated what sould process this: 
   //if it is an assembly line ("movrrl #rax, #rbx")
   //then it is passed to the assembler. If its is
   //data ("db $var, !1") it is passed to the data
   //assembler. If it is a pragma ("pragma section data")
   //it is passed to the pragma function.
   #define LT_CODE	0
   #define LT_DATA	1
   int total_operands, linetype;
   token_t *mnemonic;
   token_t *operands[3];
}asmline_t;

//Given an asmline_t, retrieve data from it
static inline char *asmline_get_mnemonic(asmline_t *line){
   return line->mnemonic->value;
}

static inline char *asmline_get_op(asmline_t *line, int op){
   if( line->total_operands > op )
      return line->operands[op]->value;
   else
      return NULL;
}

static inline int asmline_get_optype(asmline_t *line, int op){
   if( line->total_operands > op )
      return line->operands[op]->type;
   else
      return -1;
}

//When uncommented, these make the program segfault.
//Why?
token_t *lexer(char *input, int *total);
asmline_t *parse_line(token_t **tokens);
