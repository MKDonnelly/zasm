#pragma once

#define DEFAULT_OPSIZE16        1

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
}parsed_asm_t;
