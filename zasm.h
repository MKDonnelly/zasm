#pragma once

#define DEFAULT_OPSIZE16        1

//The largest x86 instruction is 15 bytes long
#define MCODE_MAXLEN    15
typedef struct mcode{
   int len; //Increments down the bytes
            //array as bytes are added
   char bytes[MCODE_MAXLEN];
}mcode_t;

//This holds the parsed input line.
//For example, "movrrq rax, rbx" turns into
// .mnemonic = "movrrq",
// .total_operands = 2;
// .operands[0] = "rax"
// .operands[1] = "rbx"
#define MNEMONIC_MAX_LEN        20
#define MAX_OPERANDS            2
#define OPERANDS_MAX_LEN        20
typedef struct{
   char mnemonic[MNEMONIC_MAX_LEN];
   int total_operands;
   char operands[MAX_OPERANDS][OPERANDS_MAX_LEN];
}asm_input_t;


