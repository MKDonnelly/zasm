#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "assemble.h"
#include "opcode_table.h"

//Given the name of a register (i.e. "rax") convert it
//into its number. The number is used when referencing the
//register in machine code.
char reg_to_num(char *reg){
   if( strcmp(reg, "#rax") == 0 || strcmp(reg, "#eax") == 0)
      return 0b0000;
   if( strcmp(reg, "#rbx") == 0 || strcmp(reg, "#ebx") == 0)
      return 0b0011;
   if( strcmp(reg, "#rcx") == 0 || strcmp(reg, "#ecx") == 0)
      return 0b0001;
   if( strcmp(reg, "#rdx") == 0 || strcmp(reg, "#edx") == 0)
      return 0b0010;
   if( strcmp(reg, "#rsi") == 0 || strcmp(reg, "#esi") == 0)
      return 0b0110;
   if( strcmp(reg, "#rdi") == 0 || strcmp(reg, "#edi") == 0)
      return 0b0111;
}

//Search for the format of a machine code given 
//the mnemonic. The table is in opcode_table.h
mcode_fmt_t *search_format(char *mnemonic){
   for(int i = 0; i < total_formats(); i++){
      if( strcmp( formats[i].name, mnemonic ) == 0 )
            return &formats[i];
   }
   return NULL;
}

//Determine the type of memory reference being made
//This is primarily done by finding the number after
//the closing brace, if it exists. This differentiates
//the following three cases.
// 1 - [<reg>]
// 2 - [<reg>]+disp8
// 3 - [<reg>]+disp32
int memref_type(char *memref){
   if( strstr(memref, "+") == NULL ){
      //No plus, so it must be 1
      return 0b00; //Mod == 00
   }else{
      char *number_start = strstr(memref, "+");
      number_start++; //skip over "+"
      int number = atoi(number_start);
      if( number < 255 ){
         //number can be represented as disp8
         return 0b01;
      }else{
         return 0b10;
      }
   }
}


void encode_memref(buffer_t *output, asmline_t *line, 
                   mcode_fmt_t *fmt){
   //NOTE: I presume all instructions have the memory as the second 
   //operator.
   char *memref;
   char modrm = fmt->modrm;
   
   if( fmt->operands[1][0] == 'm'){
      memref = asmline_get_op(line, 1);
   }else{
       return;
   }

   int mod = memref_type(memref);
   char *reg = strip_chars(memref, "[]");
 
   if( mod == 0 ){
      modrm |= mod << 6 | ((reg_to_num(asmline_get_op(line, 0)) & 0b111) << 3) 
               | (reg_to_num(reg) & 0b111);
   }else if( mod == 0b01 ){

   }else if( mod == 0b10 ){

   }

   buffer_append(output, modrm);
}

void encode_prefixes(buffer_t *output, asmline_t *line, 
                     mcode_fmt_t *fmt){
   if( DEFAULT_OPSIZE16 && 
     (strcmp(fmt->operands[0], "m32") == 0 || 
      strcmp(fmt->operands[1], "m32") == 0 )){
      //Legacy prefix needed
      buffer_append(output, LPREFIX_MEM_OVERRIDE);
   }
}

//Encodes the rex prefix, if needed.
void encode_rex(buffer_t *output, asmline_t *line, 
                mcode_fmt_t *fmt){
   if( fmt != 0 && fmt->rex != -1 ){
      //Add the two extra bits that extend the rex prefix
      //We only need the highest bit (bit 3) since the rex prefix
      //only includes that. So we and with 0b1000 and then shift right.
      //destination
      int op1_extension = (reg_to_num(asmline_get_op(line, 0)) & 0b1000) >> 3;
      //source
      int op2_extension = (reg_to_num(asmline_get_op(line, 1) ) & 0b1000) >> 3;
      buffer_append(output, (char)fmt->rex | (op2_extension << 3) 
                            | op1_extension);
   }
}


void encode_op(buffer_t *output, asmline_t *line, 
               mcode_fmt_t *fmt){
   int opcode_byte = output->len;
   for(int i = 0; fmt->opcode[i] != -1; i++){
      buffer_append(output, (char)fmt->opcode[i]);
   }

   //Does not use modrm, to specify the registers, but uses
   //the last 3 bits of the opcode
   if( fmt->modrm == -2 ){
      output->buffer[opcode_byte] |= reg_to_num(asmline_get_op(line,0));
   }
}

void encode_modrm(buffer_t *output, asmline_t *line, 
                  mcode_fmt_t *fmt){
   //Catch memory references; they are pretty complicated
   if( fmt->operands[0][0] == 'm' || fmt->operands[1][0] == 'm'){
      encode_memref(output, line, fmt);
   }else if( fmt->modrm >= 0 ){
      //reg_to_num returns a 4 bit value (the upper bit is used by
      //the rex prefix, if required). we only need the lower 3.
      //destination
      int op1 = reg_to_num(asmline_get_op(line,0)) & 0b111;
      //source
      int op2 = reg_to_num(asmline_get_op(line,1)) & 0b111;
      buffer_append(output, ((char)fmt->modrm) | (op2 << 3) | op1);
   }
}


void encode_immediate(buffer_t *output, asmline_t *line, 
                      mcode_fmt_t *fmt){
   //Immediate MUST be second operand (i.e. we can't mov to an imm)
   if( fmt->operands[0] == 0 || fmt->operands[1] == 0 )
      return;
   if( strcmp( "i", fmt->operands[1] ) == 0 ){
      if( strcmp("r32", fmt->operands[0]) == 0 ){
         uint32_t *ptr = (uint32_t*)&output->buffer[output->len];
         char *imm = asmline_get_op(line, 1);
         imm++; //increment to remove prefixed "!"
         *ptr = atoi(imm);
         output->len += 4;
     }else if( strcmp("r64", fmt->operands[0]) == 0){
         uint64_t *ptr = (uint64_t*)&output->buffer[output->len];
         char *imm = asmline_get_op(line, 1);
         imm++;
         *ptr = atol(imm);
         output->len += 8;
     }
   }
}


//Given a string (i.e. "mov eax, ebx") encode it.
void assemble_line(asmline_t *parsed, buffer_t *output_mcode){
   //get the format for the given mnemonic
   mcode_fmt_t *fmt = search_format(asmline_get_mnemonic(parsed));

   if( fmt != NULL ){
      encode_prefixes(output_mcode, parsed, fmt);
      encode_rex(output_mcode, parsed, fmt );
      encode_op(output_mcode, parsed, fmt );
      encode_modrm(output_mcode, parsed, fmt ); 
      encode_immediate(output_mcode, parsed, fmt );
   }else{
      //Format was null. Was is not in the opcode table?
      printf("ERROR: Opcode not found: %s\n", parsed->mnemonic);
   }
}
