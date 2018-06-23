#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "zasm.h"

typedef struct stentry{
   int data_offset;
   char *refstring;
}stentry_t;

typedef struct symtab{
   stentry_t entries[100];
   int total_entries;
}symtab_t;

symtab_t gsymtab;

void assemble_data(char *line, buffer_t *output_buf){
   //Separate line
   char *asm_direct = strsep(&line, " ");

   char *name = strsep(&line, " ");

   char *arg = strsep(&line, " ");
   strip_chars(arg, " ");

   if( strcmp(asm_direct, "db") == 0 ){
      gsymtab.entries[gsymtab.total_entries].refstring = malloc(100);
      strcpy(gsymtab.entries[gsymtab.total_entries].refstring, name);
      gsymtab.entries[gsymtab.total_entries].data_offset = output_buf->len + 0x400078;
      gsymtab.total_entries++;
   
      output_buf->buffer[output_buf->len] = (char)atoi(arg);
      output_buf->len++;
   }
}

void encode_datarefs(parsed_asm_t *line){
   for(int i = 0; i < gsymtab.total_entries; i++){
      if( strcmp(line->operands[0], gsymtab.entries[i].refstring) == 0 )
         sprintf(line->operands[0], "%d", gsymtab.entries[i].data_offset);

      if(strcmp(line->operands[1], gsymtab.entries[i].refstring) == 0 ){
         sprintf(line->operands[1], "%d", gsymtab.entries[i].data_offset);
      }
      
   }
}

parsed_asm_t *parse_line(char *line){
   parsed_asm_t *input = malloc(sizeof(parsed_asm_t));
   line = strip_chars(line, "\n");

   //Git the first part of the string: the mnemonic
   char *mnemonic = strsep(&line, " ");
   strcpy( input->mnemonic, mnemonic );
   input->total_operands = 0;
 
   //first operand (destination)
   char *first_op = strsep( &line, ",");
   strip_chars(first_op, " \t");
   if( first_op != NULL ){
      strcpy(input->operands[0], first_op);
      input->total_operands = 1;
   }

   //second operand (source)
   char *second_op = strsep( &line, ",");
   strip_chars(second_op, " \t");
   if( second_op != NULL ){
      strcpy( input->operands[1], second_op);
      input->total_operands = 2;
   }

   return input;
}


void main(){
   FILE *input = fopen("./test.zasm", "r");
   size_t max = 1024;
   char *buf = malloc(max);
   int result = getline( &buf, &max, input);
   buffer_t mcode;
   mcode.len = 0;
   mcode.buffer = malloc(200);   

   int i = 0;
   while( result != -1 && strncmp(buf, "code", 4) != 0 ){
      assemble_data(buf, &mcode);
      result = getline(&buf, &max, input);
      i++;
   }

   //Increment past the start of the data to get to the code
   int code_offset = mcode.len;

   result = getline(&buf, &max, input);
   while( result != -1 ){
      parsed_asm_t *l = parse_line(buf);
      encode_datarefs(l);
      assemble_line(l, &mcode);
      result = getline(&buf, &max, input);
   }

   buffer_t hdr;
   hdr.len = 0;
   hdr.buffer = malloc(900);
   generate_elf_header(&hdr, code_offset);

   //Write final binary
   FILE *output = fopen("output.elf", "w");
   fwrite(hdr.buffer, sizeof(char), (size_t)hdr.len, output);
   fwrite(mcode.buffer, sizeof(char), (size_t)mcode.len, output);
}
