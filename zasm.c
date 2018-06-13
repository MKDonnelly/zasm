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
   char mnemonic[20];
   int total_operands;
   char operands[2][20];
}input_cmd_t;

//Uses to hold the template for forming
//an instruction of this kind
typedef struct{
   //name of instruction
   char name[20];
  
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
   char opcode[3];

   int uses_modrm;
   char modrm;
}opcode_format_t;


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
//   addrmq    r64   r/m64  0x48   0x03       0x48
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
 
         char *str = strsep(&line, "\t");
         //First field is name
         strcpy( opcodes[current_opcode].name, str );
         str = strsep(&line, "\t");

         //second field is first op
         if( strcmp( str, "none" ) != 0 ){
            int op_pos = opcodes[current_opcode].total_operands;
            strcpy( opcodes[current_opcode].operands[op_pos], str);
            opcodes[current_opcode].total_operands++;
         }
         str = strsep(&line, "\t");

         //third field is second op (if present) 
         if( strcmp( str, "none" ) != 0 ){
            int op_pos = opcodes[current_opcode].total_operands;
            strcpy( opcodes[current_opcode].operands[op_pos], str );
            opcodes[current_opcode].total_operands++; 
         }
         str = strsep(&line, "\t");

         //fourth field is rex
         int r = strcmp( "none", str );
         if( r == 0 ){
            opcodes[current_opcode].uses_rex = 0;
         }else{
            opcodes[current_opcode].uses_rex = 1;
            opcodes[current_opcode].rex = strtol(str, NULL, 16);
         }
         str = strsep(&line, "\t");
   
         //fifth field is opcode
         char *op = strsep(&str, ",");
         while( op != NULL ){
            int pos = opcodes[current_opcode].opcode_len;
            opcodes[current_opcode].opcode[pos] = strtol(op, NULL, 16);
            opcodes[current_opcode].opcode_len++;
            op = strsep(&str, ",");
         }
         str = strsep(&line, "\t");
   
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

//Search for the format of an opcode given its name
opcode_format_t *search_opcode(input_cmd_t *line){
   for(int i = 0; i < current_opcode; i++){
      if( strcmp( opcodes[i].name, line->mnemonic ) == 0 )
            return &opcodes[i];
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
   int first_byte = buildop->len;
   for(int i = 0; i < fmt->opcode_len; i++){
      buildop->opcode[buildop->len] = fmt->opcode[i];
      buildop->len++;
   }

   //Does not use modrm, but uses the last 3 bits of the opcode
   if( fmt->uses_modrm == 2 ){
      buildop->opcode[first_byte] |= reg_to_num( line->operands[0] );
   }
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

void encode_immediate(buildop_t *buildop, input_cmd_t *line, opcode_format_t *fmt){
   //temporarily, I assume if an opcode as an imm, it must have 
   //2 operands and the imm is the second
   if( line->total_operands != 2 )
      return;
   //Immediate MUST be second operand (i.e. we can't mov to an imm)
   if( strcmp( "i", fmt->operands[1] ) == 0 ){
      if( strcmp("r32", fmt->operands[0]) == 0 ){
         uint32_t *ptr = &buildop->opcode[buildop->len];
         *ptr = atoi(line->operands[1]);
         buildop->len += 4;
     }else if( strcmp("r64", fmt->operands[0]) == 0){
         uint64_t *ptr = &buildop->opcode[buildop->len];
         *ptr = atol(line->operands[1]);
         buildop->len += 8;
     }
   }
}

buildop_t *create_opcode(input_cmd_t *line){
  
   opcode_format_t *fmt = search_opcode(line);
   if( fmt != NULL ){
      buildop_t *new_op = malloc(sizeof(buildop_t));
      new_op->len = 0;
      encode_rex( new_op, line, fmt );
      encode_op( new_op, line, fmt );
      encode_modrm( new_op, line, fmt ); 
      encode_immediate( new_op, line, fmt );
      return new_op;
   }
   return NULL;
}


//Given a string (i.e. "mov eax, ebx") encode it.
buildop_t *encode_line(char *line){
   input_cmd_t input;

   char *mnemonic = strsep(&line, " ");
   strcpy( input.mnemonic, mnemonic );
   input.total_operands = 0;
 
   char *first_op = strsep( &line, ",");
   if( first_op != NULL ){
      strcpy(input.operands[0], first_op);
      input.total_operands = 1;
   }

   char *second_op = strsep( &line, ",");
   if( second_op != NULL ){
      second_op++; //get rid of space
      strcpy( input.operands[1], second_op);
      input.total_operands = 2;
   }

   //generate the actual assembly codes
   buildop_t *op = create_opcode(&input);
   if( op == NULL ){
      printf("Opcode not found!\n");
      return -1;
   }
   return op;
}

int process_file(FILE *input_file, char *output){
   char *line = malloc(1024);
   size_t max = 1024;
   input_cmd_t input;
   int length = 0;
   int result = getline( &line, &max, input_file );
   line[result-1] = 0; //remove trailing newline
   while( result != -1 ){
      buildop_t *op = encode_line(line);
      for(int i = 0; i < op->len; i++, length++){
         output[length] = op->opcode[i];
      }
      result = getline( &line, &max, input_file );
      line[result-1] = 0; //remove trailing newline
   }
   return length;
}

void main(int argc, char **argv){
   FILE *ops = fopen("./opcode_table", "r");
   read_opcodes(ops);

   if( argc == 1 ){
      FILE *test = fopen("./test.asm", "r");
      char *output_asm = malloc(1024);
      int byte_length = process_file(test, output_asm);
   
      for(int i = 0; i < byte_length; i++){
         printf("%.2x ", output_asm[i] & 0xff);
      }
   }else{ 
      buildop_t *op = encode_line(argv[1]);      
      for(int i = 0; i < op->len; i++){
         printf("%.2x ", op->opcode[i] & 0xff);
      }
   }
}
