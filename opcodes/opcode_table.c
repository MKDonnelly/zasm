#include "opcode_table.h"
#include <string.h>
#include <stdio.h>

opcode_format_t ADD[] = {
   //r64, r64
   { .rex_prefix_needed = 1, 
     .rex_format = {0, 1, 0, 0, 1, 0, 0, 0},
     .opcode_len = 1,
     .opcode = { 0x89, 0, 0},
   }
};

#define TOTAL_MNEMONICS 1
//Used to retrieve a pointer to the formats for a given
//mnemonic family
mnemonic_map_t mnemonic_to_opfamily[] = {
    { "add", ADD },
};

//Searches through the ascii_to_formats_map table to 
//retrieve the family of opcodes associated with the given
//mnemonic. 
opcode_format_t *ascii_to_opformats(char *ascii){
   for(int i = 0; i < TOTAL_MNEMONICS; i++){
      if( strcmp(mnemonic_to_opfamily[i].mnemonic, ascii) == 0 ){
         return mnemonic_to_opfamily[i].opcode_family;
      }
   }
   return NULL;
}
