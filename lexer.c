#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "opcode_table.h"
#include "common.h"


int type_of(char *str){
   for(int i = 0; i < total_formats(); i++){
      if( strcmp(str, formats[i].name) == 0)
         return TYPE_MNEMONIC;
   }

   if( strcmp(str, "db") == 0 )
      return TYPE_SYNOP;

   if( strstr(str, "#") != NULL )
      return TYPE_REGISTER;
   if( strstr(str, "@") != NULL )
      return TYPE_MEMORY;
   if( strstr(str, "!") != NULL )
      return TYPE_IMM;
   if( strstr(str, "$") != NULL )
      return TYPE_VAR;

   if( strcmp(str, "section_code") == 0 )
      return TYPE_PRAGMA;


   return -1;
}

token_t *lexer(char *input, int *total){
   token_t *tokens = malloc(sizeof(token_t) * 10);
   int tokens_index = 0;

   int i = 0;
   char *tok = input;
   while(input[i] != '\0'){
      while( input[i] == ' ' ){
         tok++;
         i++;
      }

      while( input[i] != ' ' && input[i] != ',' && input[i] != '\n' )
         i++;

      char delim = input[i];
      input[i] = '\0';

      tokens[tokens_index].type = type_of(tok);
      tokens[tokens_index].value = tok;
      tokens_index++;

      if( delim == ',' ){
         tokens[tokens_index].type = TYPE_COMMA;
         tokens_index++;
      }else if( delim == '\n') {
         tokens[tokens_index].type = TYPE_NL;
         tokens_index++;
      }

      i++;
      tok = &input[i];
   }

   *total = tokens_index;
   return tokens;
}


asmline_t *parse_line(token_t **tokens){
   if( *tokens == NULL )
      return NULL;
 
   asmline_t *parsed = malloc(sizeof(asmline_t));
   parsed->total_operands = 0;

   //We iterate down the chain of tokens, shifting 
   //forward as we parse the tokens. We only deal with
   //the first element: think of tokens as a stack.
   while( (*tokens)->type != TYPE_NL ){
      if( (*tokens)->type == TYPE_MNEMONIC ){
         parsed->mnemonic = *tokens;
         parsed->linetype = LT_CODE;
      }else if( (*tokens)->type == TYPE_SYNOP ){
         parsed->mnemonic = *tokens;
         parsed->linetype = LT_DATA;
      }else if( is_operand(**tokens) ){
         parsed->operands[parsed->total_operands] = *tokens;
         parsed->total_operands++;
      }

      //Start at the next token, since the one we dealt with 
      //has been asigned.
      (*tokens)++;
   }

   //Move over newline
   (*tokens)++;

   return parsed;
}

/*void main(){
   int t;
   char line[] = "movril #rax, #rcx\n \
                  andrrq $test, !123 \n";
   token_t *ret = lexer(line, &t);
   
   asmline_t *p = parse_line(&ret);
   asmline_t *s = parse_line(&ret);
}*/
