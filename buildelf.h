#pragma once
#include <elf.h>

//Code to incrementally build an elf object file
//For simplicity, this code assumes a lot of values

//Represents a section header and its contents
//We need to aggregate these all before we write
//them to the file, since the offsets will change
typedef struct section_pair{
   Elf64_Shdr header;
   char *content;
   size_t len;
}section_pair_t;

//Represents a section string table as it is being built
typedef struct shstrtab_out{
   //hard-code the 200 for simplicity
   char output[200];
   int len;
}shstrtab_out_t;

typedef struct elfout{
   FILE *output;
   //Hard-code the 10 for now to simplify things
   section_pair_t sections[10]; 
   int total_sections;

   shstrtab_out_t shstrtab;
}elfout_t;


elfout_t *elfout_create(char *output_name);

void elfout_make_generic_header(elfout_t *output);
void elfout_make_null_section(elfout_t *output);
void elfout_add_section(elfout_t *output, section_pair_t *section);

//Implicitly writes shstrtab to output. Also resolves offsets.
void elfout_commit(elfout_t *output);

section_pair_t *section_pair_create(elfout_t *p, char *name);
void section_pair_add_content(section_pair_t *p, char *data, size_t len);
void section_pair_add_flags(section_pair_t *p, int flags);
void section_pair_add_type(section_pair_t *p, int type);
