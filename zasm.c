#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

typedef struct{
   char name[16];
   
   char first_op[10];
   char second_op[10];

   int uses_rex;
   char rex;

   int opcode_len;
   char opcode[1];

   int uses_modrm;
   char modrm;
}opcode_format_t;

#define MAX_OPCODES	100
int current_opcode = 0;
opcode_format_t opcodes[MAX_OPCODES];

#define MAX_LINE_LEN 1024
#define LINES	2
//Line format:
// <mnemonic> <src> <dest> <rex>  <opcode> <modr/m>
//   add       r64   r/m64  0x48   0x03       0x48
void read_opcodes(FILE *fd){
   size_t bytes = MAX_LINE_LEN;
   char *line = malloc(bytes + 1);

   for(int i = 0; i < LINES; i++){
      int bytes_read = getdelim(&line, &bytes, '\n', fd);

      int field = 0;
      char *str = strtok(line, "\t");
      while( str != NULL ){
         if( field == 0 ){
            strcpy( opcodes[current_opcode].name, str );
         }else if( field == 1 ){
            strcpy( opcodes[current_opcode].first_op, str);
         }else if( field == 2){
            strcpy( opcodes[current_opcode].second_op, str );
         }else if( field == 3){
            if( strcmp( "none", str ) == 0 ){
               opcodes[current_opcode].uses_rex = 0;
            }else{
               opcodes[current_opcode].uses_rex = 1;
               opcodes[current_opcode].rex = strtol(str, NULL, 16);
            }
         }else if( field == 4){
            opcodes[current_opcode].opcode_len = 1;
            opcodes[current_opcode].opcode[0] = strtol(str, NULL, 16);
         }else if( field == 5 ){
            if( strcmp( "none", str ) == 0 ){
               opcodes[current_opcode].uses_modrm = 0;
            }else{
               opcodes[current_opcode].uses_modrm = 1;
               opcodes[current_opcode].modrm = strtol(str, NULL, 16);
            }
         }
         str = strtok(NULL, "\t");
         field++;
      }
      current_opcode++;
   }
}

opcode_format_t *search_opcode(char *name, char *first, char *second){
   for(int i = 0; i < current_opcode; i++){
      if( strcmp( opcodes[i].name, name ) == 0 &&
          strstr( opcodes[i].first_op, first ) != NULL &&
          strstr( opcodes[i].second_op, second) != NULL )
         return &opcodes[i];
   }
}

#define TYPE_REG64	0
#define TYPE_MEM64	1
int type_of(char *str){
   if( strcmp( str, "rax" ) == 0 ||
       strcmp( str, "rbx" ) == 0 )
      return TYPE_REG64;
}

char reg_to_num(char *reg){
   if( strcmp(reg, "rax") == 0 )
      return 0b0000;
   if( strcmp(reg, "rbx") == 0 )
      return 0b0011;
   if( strcmp(reg, "rcx") == 0 )
      return 0b0001;
   if( strcmp(reg, "rdx") == 0 )
      return 0b0010;
   if( strcmp(reg, "rsi") == 0 )
      return 0b0110;
   if( strcmp(reg, "rdi") == 0 )
      return 0b0111;
}

#define PRINT_BYTE( val ) ((unsigned char *)(val))[0]
void main(){
   FILE *ops = fopen("./opcode_table", "r");
   read_opcodes(ops);


   char *line[] = {"add", "rbx", "rax"};
   char *name = line[0];
   char *op1 = malloc(100);
   char *op2 = malloc(100);
   if( type_of(line[1]) == TYPE_REG64 ){
      strcpy(op1, "r64");
   }
   if( type_of(line[2]) == TYPE_REG64){
      strcpy(op2, "r64");
   }
   
   opcode_format_t *f = search_opcode(name, op1, op2);
   char opcode_buf[15];
   int pos = 0;
   if( f->uses_rex ){
      uint8_t op1_extension = (reg_to_num(line[1]) & 0b1000) >> 3;
      uint8_t op2_extension = (reg_to_num(line[2]) & 0b1000) >> 3;
      printf("%d %d\n", op1_extension, op2_extension);
      opcode_buf[pos] = f->rex | (op1_extension<<1) | (op2_extension << 2);
      pos++;
   }
   opcode_buf[pos++] = f->opcode[0];
   if( f->uses_modrm ){
      uint8_t op1_num = reg_to_num(line[1]) & 0b111;
      uint8_t op2_num = reg_to_num(line[2]) & 0b111;
      printf("Op1: %d, op2:%d\n", op1_num, op2_num);
      printf("%d\n", op2_num << 3);
      opcode_buf[pos++] = f->modrm | (op1_num | (op2_num << 3));
   }
   
   printf("Final encoding: ");
   for(int i = 0; i < pos; i++){
      printf("%x, ", opcode_buf[i] & 0x0000ff);
   }
}
