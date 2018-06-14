#pragma once

//Uses to hold the template for forming
//an instruction of this kind
typedef struct{
   //name of instruction
   char name[20];
  
   //holds the SYMBOLIC TYPES of operands
   //i.e. rax is "r64", 100 is "i"
   //This can be iterated through, until
   //operands[i] == 0, at which point, there
   //are not more operands.
   char operands[3][7];

   //Not all instructions need
   //a rex prefix. rex == -1 if none
   //is required
   char rex;

   //The length of an opcode, in bytes.
   //if opcode[i] == -1, there are not more opcodes
   int opcode[4];

   //modrm == -1 - no modrm used
   //modrm == -2 - no modrm used, but the last 3 bits opcode are
   //modrm >= 0 - This is the modrm value
   int modrm;
}mcode_fmt_t;

#define MAKE_OPCODE(_name, _operands, _rex, _opcode, _modrm)	\
{.name = _name, .operands = {_operands}, .rex = _rex, 		\
   .opcode = {_opcode}, .modrm = _modrm}

#define MAX_FORMATS	100
#define TOTAL_FORMATS	(sizeof(formats) / sizeof(formats[0]))
#define NO_REX	-1
#define PLUS_REG	-2
#define NO_MODRM	-1
#define PROTECT(...) __VA_ARGS__
//Format:
//(<name>, PROTECT(<param types>, 0), <rex>, 
//PROTECT(<opcode(s)>, -1), <modrm>)
mcode_fmt_t formats[MAX_FORMATS] = {
   MAKE_OPCODE("movrrq", PROTECT("r64", "r64", 0), 0x48, 
               PROTECT(0x8B, -1), 0xC0),

   MAKE_OPCODE("movril", PROTECT("r32", "i", 0), NO_REX,
               PROTECT(0xB8, -1), PLUS_REG ),

   MAKE_OPCODE("addrrl", PROTECT("r32", "r32", 0), NO_REX,
               PROTECT(0x01, -1), 0xC0),

   MAKE_OPCODE("movrrl", PROTECT("r32", "r32", 0), NO_REX,
               PROTECT(0x8B, -1), 0xC0),

   MAKE_OPCODE("movriq", PROTECT("r64", "i", 0), 0x48,
               PROTECT(0xB8, -1), PLUS_REG),

   MAKE_OPCODE("nop", PROTECT(0), NO_REX,
               PROTECT(0x90, -1), NO_MODRM),

   MAKE_OPCODE("andrrq", PROTECT("r64", "r64", 0), 0x48,
               PROTECT(0x21, -1), 0xC0),

   MAKE_OPCODE("xorrrq", PROTECT("r64", "r64", 0), 0x48,
               PROTECT(0x31, -1), 0xC0),

   MAKE_OPCODE("syscall", PROTECT(0), NO_REX,
               PROTECT(0x0F, 0x05, -1), NO_MODRM),
};
