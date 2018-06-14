#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "opcode_table.h"

//The largest x86 instruction is 15 bytes long
#define MCODE_MAXLEN	15
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
#define MNEMONIC_MAX_LEN	20
#define MAX_OPERANDS		2
#define OPERANDS_MAX_LEN	20
typedef struct{
   char mnemonic[MNEMONIC_MAX_LEN];
   int total_operands;
   char operands[MAX_OPERANDS][OPERANDS_MAX_LEN];
}asm_input_t;


//Given the name of a register (i.e. "rax") convert it
//into its number. The number is used when referencing the
//register in machine code.
char reg_to_num(char *reg){
   if( strcmp(reg, "rax") == 0 || strcmp(reg, "eax") == 0)
      return 0b0000;
   if( strcmp(reg, "rbx") == 0 || strcmp(reg, "ebx") == 0)
      return 0b0011;
   if( strcmp(reg, "rcx") == 0 || strcmp(reg, "ecx") == 0)
      return 0b0001;
   if( strcmp(reg, "rdx") == 0 || strcmp(reg, "edx") == 0)
      return 0b0010;
   if( strcmp(reg, "rsi") == 0 || strcmp(reg, "esi") == 0)
      return 0b0110;
   if( strcmp(reg, "rdi") == 0 || strcmp(reg, "edi") == 0)
      return 0b0111;
}

//Search for the format of a machine code given 
//the mnemonic.
mcode_fmt_t *search_format(char *mnemonic){
   for(int i = 0; i < TOTAL_FORMATS; i++){
      if( strcmp( formats[i].name, mnemonic ) == 0 )
            return &formats[i];
   }
   return NULL;
}


//Encodes the rex prefix, if needed.
void encode_rex(mcode_t *output, asm_input_t *line, mcode_fmt_t *fmt){
   if( fmt != 0 && fmt->rex != -1 ){
      output->bytes[output->len] = (char)fmt->rex;

      //Add the two extra bits that extend the rex prefix
      //We only need the highest bit (bit 3) since the rex prefix
      //only includes that. So we and with 0b1000 and then shift right.
      //destination
      int op1_extension = (reg_to_num( line->operands[0] ) & 0b1000) >> 3;
      //source
      int op2_extension = (reg_to_num( line->operands[1] ) & 0b1000) >> 3;
      output->bytes[output->len] |= (op2_extension << 3) | op1_extension;
      output->len++;
   }
}


void encode_op(mcode_t *output, asm_input_t *line, mcode_fmt_t *fmt){
   int opcode_byte = output->len;
   for(int i = 0; fmt->opcode[i] != -1; i++){
      output->bytes[output->len] = fmt->opcode[i];
      output->len++;
   }

   //Does not use modrm, to specify the registers, but uses
   //the last 3 bits of the opcode
   if( fmt->modrm == -2 ){
      output->bytes[opcode_byte] |= reg_to_num( line->operands[0] );
   }
}

void encode_modrm(mcode_t *output, asm_input_t *line, mcode_fmt_t *fmt){
   if( fmt->modrm >= 0 ){
      //reg_to_num returns a 4 bit value (the upper bit is used by
      //the rex prefix, if required). we only need the lower 3.
      //destination
      int op1 = reg_to_num(line->operands[0]) & 0b111;
      //source
      int op2 = reg_to_num(line->operands[1]) & 0b111;
      output->bytes[output->len++] = ((char)fmt->modrm) | (op2 << 3) | op1;
   }
}

void encode_immediate(mcode_t *output, asm_input_t *line, 
                      mcode_fmt_t *fmt){
   //Immediate MUST be second operand (i.e. we can't mov to an imm)
   if( fmt->operands[0] == 0 || fmt->operands[1] == 0 )
      return;
   if( strcmp( "i", fmt->operands[1] ) == 0 ){
      if( strcmp("r32", fmt->operands[0]) == 0 ){
         uint32_t *ptr = &output->bytes[output->len];
         *ptr = atoi(line->operands[1]);
         output->len += 4;
     }else if( strcmp("r64", fmt->operands[0]) == 0){
         uint64_t *ptr = &output->bytes[output->len];
         *ptr = atol(line->operands[1]);
         output->len += 8;
     }
   }
}

//Given a string (i.e. "mov eax, ebx") encode it.
mcode_t *assemble_line(char *line){
   asm_input_t input;

   //Git the first part of the string: the mnemonic
   char *mnemonic = strsep(&line, " ");
   strcpy( input.mnemonic, mnemonic );
   input.total_operands = 0;
 
   //first operand (destination)
   char *first_op = strsep( &line, ",");
   if( first_op != NULL ){
      strcpy(input.operands[0], first_op);
      input.total_operands = 1;
   }

   //second operand (source)
   char *second_op = strsep( &line, ",");
   if( second_op != NULL ){
      second_op++; //get rid of space. TODO get rid of this
      strcpy( input.operands[1], second_op);
      input.total_operands = 2;
   }

   //generate the actual assembly codes

   //get the format for the given mnemonic
   mcode_fmt_t *fmt = search_format(input.mnemonic);

   //allocate space for the new machine code to generate
   mcode_t *new_mcode = malloc(sizeof(mcode_t));
   if( fmt != NULL ){
      new_mcode->len = 0;
      encode_rex( new_mcode, &input, fmt );
      encode_op( new_mcode, &input, fmt );
      encode_modrm( new_mcode, &input, fmt ); 
      encode_immediate( new_mcode, &input, fmt );
   }else{
      //Format was null. Was is not in the opcode table?
      free(new_mcode);
      printf("ERROR: Opcode not found: %s\n", input.mnemonic);
   }

   //return our new machine code chunk.
   return new_mcode;
}

int process_file(FILE *input_file, char *output_mcode){
   char *line = malloc(1024);
   size_t max = 1024;

   //We will want to return the size of the machine code we
   //placed in output_mcode. There is no other way for the caller
   //to know this.
   int length = 0;
   int result = getline( &line, &max, input_file );
   line[result-1] = 0; //remove trailing newline

   while( result != -1 ){
      mcode_t *mcode = assemble_line(line);
      //copy the generated machine code out
      for(int i = 0; i < mcode->len; i++, length++){
         output_mcode[length] = mcode->bytes[i];
      }
      //get the next line, and remove the newline
      result = getline( &line, &max, input_file );
      line[result-1] = 0;
   }

   //Return number of bytes placed in output_mcode
   return length;
}

void main(int argc, char **argv){
   //FILE *ops = fopen("./opcode_table", "r");
   //read_opcodes(ops);

   //Open and assembly "test.asm"
   if( argc == 1 ){
      FILE *test = fopen("./test.asm", "r");
      char *output_asm = malloc(1024);
      int byte_length = process_file(test, output_asm);
   
      for(int i = 0; i < byte_length; i++){
         printf("%.2x ", output_asm[i] & 0xff);
      }
   }else{ 
      //Assembly CLI argument 1 
      mcode_t *mcode = assemble_line(argv[1]);      
      for(int i = 0; i < mcode->len; i++){
         printf("%.2x ", mcode->bytes[i] & 0xff);
      }
   }
}
