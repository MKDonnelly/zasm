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

symtab_t gsymtab = {0, 0};

typedef struct buffer{
   char *buf;
   int bytes;
}buffer_t;

void assemble_data(char *line, buffer_t *output_buf){
   //Separate line
   char *asm_direct = strsep(&line, " ");

   char *name = strsep(&line, " ");

   char *arg = strsep(&line, " ");
   strip_chars(arg, " ");

   if( strcmp(asm_direct, "db") == 0 ){
      gsymtab.entries[gsymtab.total_entries].refstring = malloc(100);
      strcpy(gsymtab.entries[gsymtab.total_entries].refstring, name);
      gsymtab.entries[gsymtab.total_entries].data_offset = output_buf->bytes;
      gsymtab.total_entries++;
   
      output_buf->buf[output_buf->bytes] = (char)atoi(arg);
      output_buf->bytes++;
   }
}

void encode_datarefs(char *line, symtab_t *symtab){
   for(int i = 0; i < symtab->total_entries; i++){
      printf("Searching for %s\n", symtab->entries[i].refstring);
      if( strstr(line, symtab->entries[i].refstring) != NULL ){
         char buffer[30];
         sprintf(buffer, "%d", symtab->entries[i].data_offset);
         substr_replace(line, symtab->entries[i].refstring, buffer);
      }
   }
}

void main(){
   FILE *input = fopen("./test.zasm", "r");
   size_t max = 1024;
   char *buf = malloc(max);
   int result = getline( &buf, &max, input);
   buffer_t output;
   output.bytes = 0;
   output.buf = malloc(200);

//   TODO why does *str not work when declaring?
//   char str[] = "one two       ";
//   substr_replace(str, "two", "three");

   while( result != -1 ){
      assemble_data(buf, &output);
      result = getline(&buf, &max, input);
   }

   char str[] = "movrmq rax, test_again             ";
   encode_datarefs(str, &gsymtab);
   printf("Now: %s\n", str);
}
