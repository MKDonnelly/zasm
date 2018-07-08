#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "elf64.h"
#include "elfheader.h"

#define START_VADDR	0x400000
#define PROG_VADDR	(START_VADDR + sizeof(Elf64_Ehdr) + sizeof(Elf64_Phdr)) 

void generate_elf_header(buffer_t *buf, uint64_t vaddr){
   build_elfheader(buf, vaddr);
   build_pheader(buf);
}

//Build standard header
void build_elfheader(buffer_t *buf, uint64_t code_offset){

   Elf64_Ehdr header;

   //Magic number
   header.e_ident[0] = 0x7f;
   header.e_ident[1] = 'E';
   header.e_ident[2] = 'L';
   header.e_ident[3] = 'F';

   //Set class (2 = 64 bit)
   header.e_ident[4] = 2;

   //Set Little endian (1 = little endian)
   header.e_ident[5] = 1;

   //set to 1 for original elf version
   header.e_ident[6] = 1;

   //9 bytes of padding
   for(int i = 0; i < 9; i++){
      header.e_ident[7+i] = 0;
   }

   //Set type as executable (2)
   header.e_type = 2;

   //Set machine as x86_64 (0x3E)
   header.e_machine = 0x3E;

   //Set to 1 for original ELF version
   header.e_version = 1;

   //Set entry point vaddr
   header.e_entry = PROG_VADDR + code_offset;

   //Set program header offset 
   header.e_phoff = 64;

   //Set section header offset (none, so 0)
   header.e_shoff = 0;

   //Ignored in x86_64
   header.e_flags = 0;
   
   //Size of this header, in bytes
   header.e_ehsize = 64;

   //Size of program header
   header.e_phentsize = 56;

   //Number of entries in program header table
   header.e_phnum = 1;
 
   //Contains size of section entry (0 since there are none)
   header.e_shentsize = 0;

   //Number of section entries (0 since there is not section part)
   header.e_shnum = 0;

   //Index of section header string
   header.e_shstrndx = 0;

   //Write header buffer to file
   memcpy(&buf->buffer[buf->len], (char*)&header, sizeof(Elf64_Ehdr));
   buf->len += sizeof(Elf64_Ehdr);
}

void build_pheader(buffer_t *buf){
   Elf64_Phdr header;

   //Set type to 1 (LOAD)
   header.p_type = 1;

   //Set flags to 7 (TODO why?)
   header.p_flags = 7;

   //Set offset within file 
   //THIS IS THE OFFSET ADDED TO THE ENTRY ADDRESS!
   //If this is zero, we can directly jump to the entry.
   header.p_offset = 0;

   //Setup virtual address of segment
   header.p_vaddr = START_VADDR;

   //physical address. not used
   header.p_paddr = 0;

   //File size
   header.p_filesize = 0x1000;

   //size in memory
   header.p_memsize = 0x1000;

   //alignment
   header.p_align = 0x1000;

   //Write header to file
   memcpy(&buf->buffer[buf->len], (char*)&header, sizeof(Elf64_Phdr));
   buf->len += sizeof(Elf64_Phdr);
}
