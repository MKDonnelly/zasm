#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "../opcode_table.h"
#include "../common.h"

static int is_mnemonic(char *str){
   for(int i = 0; i < total_formats(); i++){
      if( strcmp(str, formats[i].name) == 0 )
         return 1;
   }
   return 0;
}

static char *regs[] = { 
   "rax", "rbx", "rcx", 
   "eax", "ebx", "ecx",
};
                    
int is_register(char *str){
   for(int i = 0; i < sizeof(regs); i++){
      if( strcmp(str, regs[i]) == 0 )
         return 1;
   }
   return 0;
}

static int is_number(char *str){
   for(int i = 0; i < strlen(str); i++){
      if( !isdigit(str[i]) )
         return 0;
   }
   return 1;
}

static int is_identifier(char *str){
   for(int i = 0; i < strlen(str); i++){
      if( !isalpha(str[i]) )
         return 0;
   }
   return 1;
}

static int type_of(char *str){
   if( is_mnemonic(str) )
      return TYPE_MNEMONIC;
   else if( is_register(str) )
      return TYPE_REGISTER;
   else if( is_number(str) )
      return TYPE_NUMBER;
   else if( is_identifier(str) )
      return TYPE_IDENTIFIER;
}

token_t *lexer(char *input, int *total){
   
}
