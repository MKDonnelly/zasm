#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

#define X86_INSTR_MAXLEN	15
typedef struct buildop{
   int len; //Increments down the opcode 
            //array as bytes are added
   char opcode[X86_INSTR_MAXLEN];
}buildop_t;

//parsed form of input line
typedef struct{
   char mnemonic[16];
   int total_operands;
   char operands[2][20];
}input_cmd_t;

//Uses to hold the template for forming
//an instruction of this kind
typedef struct{
   //name of instruction
   char name[16];
  
   int total_operands;
   //these are the symbolic
   //types of the operator.
   //(i.e. "r32", "i", etc) 
   char operands[2][5];

   //Not all instructions need
   //a rex prefix. uses_rex == 1
   //if one is required
   int uses_rex;
   char rex;

   int opcode_len;
   char opcode[1];

   //This can be 2 to 
   int uses_modrm;
   char modrm;
}opcode_format_t;

#define TYPE_REG64	0
#define TYPE_REG32	1
#define TYPE_IMM	2
#define TYPE_MEM	3
int type_of(char *str){
   if( strcmp( str, "rax" ) == 0 ||
       strcmp( str, "rbx" ) == 0 ||
       strcmp( str, "rcx" ) == 0 ||
       strcmp( str, "rdx" ) == 0 ||
       strcmp( str, "rdi" ) == 0 ||
       strcmp( str, "rsi" ) == 0 )
      return TYPE_REG64;

   if( strcmp( str, "eax" ) == 0 ||
       strcmp( str, "ebx" ) == 0 ||
       strcmp( str, "ecx" ) == 0 ||
       strcmp( str, "edx" ) == 0 ||
       strcmp( str, "edi" ) == 0 ||
       strcmp( str, "esi" ) == 0 )
      return TYPE_REG32;

   int is_number = 1;
   int i = 0;
   while( str[i] != 0 ){
      if( ! isdigit(str[i]) )
         is_number = 0;
      i++;
   }

   if( is_number )
      return TYPE_IMM;

   //If it isn't any of the above, it must be a memory
   //access (or a syntax error)
   return TYPE_MEM;
}

char *symbol_to_type(char *op){
   if( type_of(op) == TYPE_REG64 )
      return "r64";
   if( type_of(op) == TYPE_REG32 )
      return "r32";
   if( type_of(op) == TYPE_IMM )
      return "i";
}

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


#define MAX_OPCODES	100
int current_opcode = 0;
opcode_format_t opcodes[MAX_OPCODES];

#define MAX_LINE_LEN 1024
//Line format:
// <mnemonic> <src> <dest> <rex>  <opcode> <modr/m>
//   add       r64   r/m64  0x48   0x03       0x48
void read_opcodes(FILE *fd){
   size_t bytes = MAX_LINE_LEN;
   char *line = malloc(bytes + 1);

   int lines_left = 1;
   while( lines_left ){
      int bytes_read = getdelim(&line, &bytes, '\n', fd);

      if( bytes_read == -1 ){
         lines_left = 0;
      }else{
         opcodes[current_opcode].total_operands = 0;
 
         char *str = strtok(line, "\t");
         //First field is name
         strcpy( opcodes[current_opcode].name, str );
         str = strtok(NULL, "\t");

         //second field is first op
         if( strcmp( str, "none" ) != 0 ){
            int op_pos = opcodes[current_opcode].total_operands;
            strcpy( opcodes[current_opcode].operands[op_pos], str);
            opcodes[current_opcode].total_operands++;
         }
         str = strtok(NULL, "\t");

         //third field is second op (if present) 
         if( strcmp( str, "none" ) != 0 ){
            int op_pos = opcodes[current_opcode].total_operands;
            strcpy( opcodes[current_opcode].operands[op_pos], str );
            opcodes[current_opcode].total_operands++; 
         }
         str = strtok(NULL, "\t");

         //fourth field is rex
         int r = strcmp( "none", str );
         if( r == 0 ){
            opcodes[current_opcode].uses_rex = 0;
         }else{
            opcodes[current_opcode].uses_rex = 1;
            opcodes[current_opcode].rex = strtol(str, NULL, 16);
         }
         str = strtok(NULL, "\t");
   
         //fifth field is opcode
         opcodes[current_opcode].opcode_len = 1;
         opcodes[current_opcode].opcode[0] = strtol(str, NULL, 16);
         str = strtok(NULL, "\t");
   
         //sixth field is modrm. TODO strip off that '\n'
         if( strcmp( "none\n", str ) == 0 ){
            opcodes[current_opcode].uses_modrm = 0;
         }else if( str[0] == '+' && str[1] == 'r' ){
            //Uses for when the last 3 bits of the opcode
            //specifies the register, without the modrm
            opcodes[current_opcode].uses_modrm = 2;
         }else{
            opcodes[current_opcode].uses_modrm = 1;
            opcodes[current_opcode].modrm = strtol(str, NULL, 16);
         }

         current_opcode++;
      }
   }
}

opcode_format_t *search_opcode(input_cmd_t *line){
   for(int i = 0; i < current_opcode; i++){
      int result = strcmp( opcodes[i].name, line->mnemonic );
      if( result == 0 ){
         int valid = 1;
         for(int j = 0; j < line->total_operands; j++){
            int r = strcmp( opcodes[i].operands[j], symbol_to_type(line->operands[j]));
            if( r != 0 )
               valid = 0;
         }
         if( valid )
            return &opcodes[i];
      }
   }
   return NULL;
}


//Encodes the rex prefix, if needed.
void encode_rex(buildop_t *buildop, input_cmd_t *line, 
                opcode_format_t *fmt){
   if( fmt != 0 && fmt->uses_rex ){
      buildop->opcode[buildop->len] = fmt->rex;

      //Add the two extra bits that extend the rex prefix
      //destination
      int op1_extension = (reg_to_num( line->operands[0] ) & 0b1000) >> 3;
      //source
      int op2_extension = (reg_to_num( line->operands[1] ) & 0b1000) >> 3;
      buildop->opcode[buildop->len] |= 
                                     (op2_extension << 3) | op1_extension;
      buildop->len++;
   }
}

void encode_op(buildop_t *buildop, input_cmd_t *line, 
               opcode_format_t *fmt){
   buildop->opcode[buildop->len] = fmt->opcode[0];
   //Does not use modrm, but uses the last 3 bits of the opcode
   if( fmt->uses_modrm == 2 ){
      buildop->opcode[buildop->len] |= reg_to_num( line->operands[0] );
   }
   buildop->len++;
}

void encode_modrm(buildop_t *buildop, input_cmd_t *line, 
                  opcode_format_t *fmt){
   if( fmt->uses_modrm == 1 ){
      char modrm = fmt->modrm;
      //destination
      int op1 = reg_to_num(line->operands[0]) & 0b111;
      //source
      int op2 = reg_to_num(line->operands[1]) & 0b111;
      modrm |= (op2 << 3) | op1;
      buildop->opcode[buildop->len++] = modrm;
   }
}

void encode_immediate(buildop_t *buildop, input_cmd_t *line){
   //Immediate MUST be second operand (i.e. we can't mov to an imm)
   char *type = symbol_to_type(line->operands[1]);
   if( strcmp( "i", type ) == 0 ){
      char *reg_type = symbol_to_type(line->operands[0]);
      //TODO why is the normal strcmp failing here?
      if( reg_type[0] == 'r' && reg_type[1] == '3' && reg_type[2] == '2' ){
         uint32_t *ptr = &buildop->opcode[buildop->len];
         *ptr = atoi(line->operands[1]);
         buildop->len += 4;
     }else if( reg_type[0] == 'r' && reg_type[1] == '6' && reg_type[2] == '4'){
         uint64_t *ptr = &buildop->opcode[buildop->len];
         *ptr = atol(line->operands[1]);
         buildop->len += 8;
     }
   }
}

void main(){
   FILE *ops = fopen("./opcode_table", "r");
   read_opcodes(ops);

   input_cmd_t line;
   strcpy(line.mnemonic, "nop");
   strcpy(line.operands[0], "");
   strcpy(line.operands[1], "");
   line.total_operands = 0;

   buildop_t op;
   op.len = 0;
   opcode_format_t *fmt = search_opcode( &line );
   if( fmt == NULL ){
      printf("Opcode not found!\n");
      exit(1);
   }

   encode_rex(&op, &line, fmt);
   encode_op(&op, &line, fmt);
   encode_modrm(&op, &line, fmt);
   encode_immediate(&op, &line);

   for(int i = 0; i < op.len; i++){
      printf("%.2x ", op.opcode[i] & 0xff);
   }
}
