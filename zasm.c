#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "assemble.h"
#include "elfheader.h"

typedef struct stentry{
   int data_offset;
   char *refstring;
}stentry_t;

typedef struct symtab{
   stentry_t entries[100];
   int total_entries;
}symtab_t;

symtab_t gsymtab;

void assemble_data(asmline_t *line, buffer_t *output_buf){
   //Separate line
   char *asm_direct = asmline_get_mnemonic(line);

   char *name = asmline_get_op(line, 0);

   if( strcmp(asm_direct, "db") == 0 ){
      gsymtab.entries[gsymtab.total_entries].refstring = 
                                                 malloc(strlen(name)+1);
      strcpy(gsymtab.entries[gsymtab.total_entries].refstring, name);
      gsymtab.entries[gsymtab.total_entries].data_offset = 
                                             output_buf->len + 0x400078;
      gsymtab.total_entries++;

      //place data
      char *data = asmline_get_op(line, 1);
      data++;
      buffer_append(output_buf, (char)atoi(data));
   }
}

void encode_datarefs(asmline_t *line){
   for(int i = 0; i < gsymtab.total_entries; i++){
      for(int j = 0; j < line->total_operands; j++)
         if( strcmp(asmline_get_op(line, j), 
             gsymtab.entries[i].refstring) == 0 )
            sprintf(asmline_get_op(line, j), "%d", 
                  gsymtab.entries[i].data_offset);
   }
}

void process_line(asmline_t *line, buffer_t *buf){
}

void main(int argc, char **argv){
   FILE *input;

   if( argc > 1 ){
      input = fopen(argv[1], "r");
   }else{
      printf("Usage: %s <file>\n", argv[0]);
      exit(1);
   }

   size_t max = 1024;
   char *buf = malloc(max);
   int result = getline( &buf, &max, input);
   buffer_t *mcode = create_buffer(200);
   buffer_t *data = create_buffer(200);

   while( result != -1 ){
      int t;
      token_t *toks = lexer(buf, &t);
      asmline_t *l = parse_line(&toks);
   
      if( l->linetype == LT_CODE ){
         encode_datarefs(l);
         assemble_line(l, mcode);
      }else if( l->linetype == LT_DATA ){
         assemble_data(l, data);
      }

      result = getline(&buf, &max, input);
   }

   buffer_t *hdr = create_buffer(900);
   generate_elf_header(hdr, data->len);

   FILE *output = fopen("output.elf", "w");
   fwrite(hdr->buffer, sizeof(char), (size_t)hdr->len, output);
   fwrite(data->buffer, sizeof(char), (size_t)data->len, output);
   fwrite(mcode->buffer, sizeof(char), (size_t)mcode->len, output);
}
