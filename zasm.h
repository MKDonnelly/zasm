#pragma once

#include <stdint.h>

//For opcode
/*
   //d = 0 if adding from register TO memory
   //d = 1 if adding from memory TO register
   //s = 0 for 8 bit operands
   //s = 1 for 16 of 32 bit operands (depending on setup GDT)
   uint8_t s_bit : 1;
   uint8_t d_bit : 1;*/

typedef struct rex{
   uint8_t fixed_bit_pattern : 4; //Always 0100
   uint8_t w_bit : 1; //0 = operand size determined by CS.D
                      //1 = 64 bit operand
   //NOTE: The following bits are prefix to the normal ModR/m.
   //For example, the r_bit is prefixed to the ModR/M field to allow
   //16 registers to be encoded
   uint8_t r_bit : 1; //extension for ModR/M reg field
   uint8_t x_bit : 1; //extension for SIB index field
   uint8_t b_bit : 1; //extension for ModR/M r/m field, SIB base, or opcode
}rex_t;

//8 bits long. specifies what the two arguments
//are (registers or memory)
typedef struct modrm{
   //mod values:
   //  00 - Register indirect addressing mode/SIB without displacement/
   //       displacement only addressing mode
   //  01 - One-byte signed displacement follows addressing mode bytes
   //  10 - Four-byte signed displacement follows addressing mode bytes
   //  11 - register addressing mode
   uint8_t mod : 2;

   //Source/destination register, depending on default operation length
   // 000 - eax/ax/al
   // 001 - ecx/cx/al
   // 010 - edx/dx/dl
   // 011 - ebx/bx/bl
   // 100 - esp/sp/
   // 101 - ebp/bp/
   // 110 - esi/si 
   // 111 - edi/di
   uint8_t reg : 3;  //source
   uint8_t rm : 3;   //destination
}modrm_t;

//SIB byte, used when doing stuff like [eax+4*rbx]
//Not every instruction requires this, and I may not
//support it initially.
//Format: [(base) + (index)*(scale)]
//index can be a scalar value of 1,2,4, or 8
//(base) and (scale) must be registers
typedef struct sib{
   //scale bits:
   //   00 - 1 byte scale (i.e. [eax+1*ebx]
   //   01 - 2 byte scale (i.e. [eax+2*ebx]
   //   10 - 4 byte scale (i.e. [eax+4*ebx]
   //   11 - 8 byte scale (i.e. [eax+8*ebx]
   uint8_t scale_bits : 2;

   //Register to use as index
   uint8_t index_bits : 3;

   //Register to use as base
   uint8_t base_bits : 3;
}sib_t;

//Maps mnemonic opcode names to opcode values
typedef struct opcode{
   char opcode_name[16];
   uint16_t opcode_val;
   int opcode_bits;
   int operands;
}opcode_t;

