
typedef struct opcode{
   char name[16];
   int index_in_table;
}opcode_t;

typedef struct opcode_format{
   int rex_required; //1 for yes, 0 for no
   char rex_bits[8]; //ignored if rex_required == 0
                     //each field is either 0 or 1

   int opcode_len; //length in bytes of opcode
   char opcode[3]; //may be 1, 2, or 3 bytes

   int total_operands; //so far I am only supporting
                       //1 or 2 operands.
}opcode_format_t;

#define TOTAL_OPCODES	1
opcode_t opcodes_to_index[] = {
   {"add", 0},
};


//The actual opcode table
opcode_format_t all_opcodes[TOTAL_OPCODES] = {
   //Add 64 bit register to 64 bit register
   {1, 0, 1, 0, 0, 1, 0, 0, 0, 1, 89, 0, 0, 2},
}



