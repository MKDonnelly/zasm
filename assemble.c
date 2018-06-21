#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "zasm.h"


#define DATA_START_ADDR 0x400000
int code_start_addr = DATA_START_ADDR;

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
      gsymtab.entries[gsymtab.total_entries].data_offset = output_buf->len;
      gsymtab.total_entries++;
   
      output_buf->buffer[output_buf->len] = (char)atoi(arg);
      output_buf->len++;
   }
}

void encode_datarefs(parsed_asm_t *line, symtab_t *symtab){
   for(int i = 0; i < symtab->total_entries; i++){
      if( strcmp(line->operands[0], symtab->entries[i].refstring) == 0 ){
         char buffer[30];
         sprintf(buffer, "%d", symtab->entries[i].data_offset);
         substr_replace(line->operands[0], symtab->entries[i].refstring, buffer);
      }
      if(strcmp(line->operands[1], symtab->entries[i].refstring) == 0 ){
         sprintf(line->operands[1], "%d", symtab->entries[i].data_offset);
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
      printf("Buffer is %s\n", buf);
      assemble_data(buf, &mcode);
      result = getline(&buf, &max, input);
      i++;
   }

   //Increment past the start of the code
   code_start_addr += mcode.len;
   printf("Start: %x\n", code_start_addr);

   result = getline(&buf, &max, input);
   while( result != -1 ){
      printf("Code: %s\n", buf);
      assemble_line(parse_line(buf), &mcode);
      result = getline(&buf, &max, input);
   }

   printf("Assembly:\n");
   for(int i = 0; i < mcode.len; i++){
      printf("%.2x ", mcode.buffer[i] & 0xff);
   }

   buffer_t hdr;
   hdr.len = 0;
   hdr.buffer = malloc(900);
   generate_elf_header(&hdr, code_start_addr);
   printf("Header:\n");
   for(int i = 0; i < hdr.len; i++){
      printf("%.2x ", hdr.buffer[i] & 0xff);
   }

   FILE *output = fopen("output.elf", "w");
   if( output == NULL )
      printf("output");
   fwrite(hdr.buffer, sizeof(char), (size_t)hdr.len, output);
   fwrite(mcode.buffer, sizeof(char), (size_t)mcode.len, output);

   //printf("%s %s, %s\n", p->mnemonic, p->operands[0], p->operands[1]);
}
