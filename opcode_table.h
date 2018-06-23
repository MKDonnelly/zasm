#pragma once

//Default operand size when making memory references.
#define DEFAULT_OPSIZE16        1

//The legacy prefix needed when doing 32-bit mem references
//when the default operand size is 16 bits
#define LPREFIX_MEM_OVERRIDE	0x67

//Uses to hold the template for forming
//an instruction of this kind
typedef struct{
   //name of instruction
   char name[20];
  
   //holds the SYMBOLIC TYPES of operands
   //i.e. rax is "r64", 100 is "i"
   //This can be iterated through, until
   //operands[i] == -1, at which point, there
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

#define MAKE_OPCODE(_name, _operands,  _rex, _opcode, _modrm) \
{.name = _name, .operands = {_operands}, \
   .rex = _rex, .opcode = {_opcode}, .modrm = _modrm}

#define TOTAL_FORMATS	(sizeof(formats) / sizeof(formats[0]))
#define PLUS_REG	-2
#define END		-1
#define PROTECT(...) __VA_ARGS__
//Format:
//(<name>, PROTECT(<param types>, 0), PROTECT(<lprefixes>,-1), <rex>, 
//PROTECT(<opcode(s)>, -1), <modrm>)
mcode_fmt_t formats[] = {
   MAKE_OPCODE("movrrq", PROTECT("r64", "r64", END), 0x48, 
               PROTECT(0x8B, END), 0xC0),

   MAKE_OPCODE("movril", PROTECT("r32", "i", END), END,
               PROTECT(0xB8, END), PLUS_REG ),

   MAKE_OPCODE("addrrl", PROTECT("r32", "r32", END), END,
               PROTECT(0x01, END), 0xC0),

   MAKE_OPCODE("movrrl", PROTECT("r32", "r32", END), END,
               PROTECT(0x8B, END), 0xC0),

   MAKE_OPCODE("movriq", PROTECT("r64", "i", END), 0x48,
               PROTECT(0xB8, END), PLUS_REG),

   MAKE_OPCODE("nop", END, END, 
               PROTECT(0x90, END), END),

   MAKE_OPCODE("andrrq", PROTECT("r64", "r64", END), 0x48,
               PROTECT(0x21, END), 0xC0),

   MAKE_OPCODE("xorrrq", PROTECT("r64", "r64", END), 0x48,
               PROTECT(0x31, END), 0xC0),

   MAKE_OPCODE("syscall", END, END, 
               PROTECT(0x0F, 0x05, END), END),

   MAKE_OPCODE("movrml", PROTECT("r32", "m32", END), END,
               PROTECT(0x8B, END), 0x00),
};
