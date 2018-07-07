#include "opcode_table.h"

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

int total_formats(){
   return sizeof(formats) / sizeof(formats[0]);
}

