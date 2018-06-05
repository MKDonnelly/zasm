#include <stdio.h>
#include <stdlib.h>
#include "./opcodes/opcode_table.h"

void main(){
   //printf("%d\n", mnemonic_to_index("add"));
   opcode_format_t *table = ascii_to_opformats("add");
   if( table != NULL ){
      printf("Got %x\n", table);
      opcode_format_t add = table[0];
      printf("%x\n", 0x000000ff & add.opcode[0]);
   }
}
