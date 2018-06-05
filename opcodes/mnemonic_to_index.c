#include "mnemonic_to_index.h"
#include <string.h>

//This table maps opcode names to an index. The index is
//used along with various aspects of the specific instruction
//(like what size the operands are) to provide three unique numbers.
//These three numbers are used in a 3D array to locate the specific
//format of a given command.
#define MNEMONIC_MAX_LEN	16
typedef struct{
   char mnemonic[MNEMONIC_MAX_LEN];
   int index;
}mnemonic_map_t;

#define MAX_OPCODES	1
mnemonic_map_t map[] = {
   {"add", 0 },
};

int mnemonic_to_index(char *mnemonic){
   for(int i = 0; i < MAX_OPCODES; i++){
      if( strcmp(mnemonic, map[i].mnemonic) == 0 )
         return map[i].index;
   }
}
