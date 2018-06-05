#pragma once

#include <stdint.h>

//Program address
typedef long long int Elf64_Addr;

//File offset
typedef long long int Elf64_Off;

//Unsigned medium integer
typedef uint16_t Elf64_Half;

//Unsigned integer
typedef uint32_t Elf64_Word;

//Unsigned long integer
typedef unsigned long long int Elf64_Xword;

//Number of bytes of identification in the elf header
#define ELF_NIDENT 16

enum Elf_Ident{
   EI_CLASS    = 4, //Arch (32/64 bit)
   EI_DATA     = 5, //Little/Big endian
   EI_VERSION  = 6, //ELF Version (always 1)
   EI_OSABI    = 7,
   EI_ABIVERSION = 8,
   EI_PAD      = 9  //Goes up to 15
};

//ELF signature (in e_ident)
#define ELF_MAGIC "\177ELF"
#define ELF_MAGIC_LEN 4

typedef struct{

/*    e_ident content
   index 0..3: Magic bits (0x7F, 'E', 'L', 'F')
   index 4: Machine class (1 for 32 bit, 2 for 64 bit)
   index 5: Little/big endianness (1 for little, 2 for big)
            Since this is for the x86 arch, this should always be 1
   index 6: Set to 1 for original ELF version
   index 7..15: Padding 
*/
   #define ELFDATA2LSB 1
   #define ELFCLASS32  1
   unsigned char e_ident[ELF_NIDENT];

/*   type content
   Specifies the object type
    0 - None/Unknown
    1 - Relocatable
    2 - Executable
    3 - Shared
    4 - Core Dump
*/
   #define ET_NONE 0
   #define ET_REL  1
   #define ET_EXEC 2
   #define ET_SHARE 3
   Elf64_Half e_type;

/*   machine content
   Specifies target ISA (some are)
      0x0 - No specific ISA
      0x03 - x86
      0x3E - x86_64
*/
   #define MACHINE_X86 0x3
   Elf64_Half e_machine;

   //Set to 1 for orginal ELF version
   Elf64_Word e_version;

   //Specifies the entry point for the binary.
   //This will always be a VIRTUAL address when
   //we have laid the program out in memory.
   Elf64_Addr e_entry;

   //Points to the start of the program header table.
   Elf64_Off e_phoff;

   //Points to the head of the section header table.
   Elf64_Off e_shoff;

   //Implementation depends on target arch
   //This may be ignored for x86
   Elf64_Word e_flags;

   //Contains the size of THIS header. This should be
   //64 bytes for 64 bit arch, or 52 bytes for 32 bit arch
   Elf64_Half e_ehsize;

   //Contains the size of the program header table.
   Elf64_Half e_phentsize;

   //Contains the number of entries in the program header 
   //table
   Elf64_Half e_phnum;

   //Contains the size of an entry within the section header table
   Elf64_Half e_shentsize;

   //Contains the number of entries in the section header
   Elf64_Half e_shnum;
 
   //Contains index of the section header for translating sections
   //to their given names. 
   Elf64_Half e_shstrndx;
}Elf64_Ehdr;

/*
//Represents a section within the elf file
typedef struct{
   //This represents an offset within the .shstrtab section
   //giving the name of this section.
   #define SHN_UNDEF 0x00
   Elf32_Word sh_name;

   #define SHT_NULL 0 //Null segment
   #define SHT_PROGBITS 1 //Program data
   #define SHT_SYMTAB 2 //Symbol table
   #define SHT_STRTAB 3 //String table
   #define SHT_RELA 4 //Relocation w/ addend
   #define SHT_NOBITS 8 //Not present in file
   #define SHT_REL 9 //Relocation w/ no addend
   Elf32_Word sh_type;

   //Identifies attributes for section
   #define SHF_WRITE 0x1 //Writeable
   #define SHF_ALLOC 0x2 //Exists in memory
   Elf32_Word sh_flags;

   //Virtual address of section in memory, for loaded sections
   Elf32_Addr sh_addr;

   //Offset of section in elf file
   Elf32_Off sh_offset;

   //Size of section in elf file
   Elf32_Word sh_size;

   //Contains the section index of an associated section.
   Elf32_Word sh_link;
   
   //Contains extra info about the section.
   Elf32_Word sh_info;

   //Contains the alignment required for this section.
   //Must be power of 2
   Elf32_Word sh_addralign;

   //Contains the size in bytes for sections that contain
   //fixed-size entries.
   Elf32_Word sh_entsize;
}Elf32_Shdr;*/


//This tells the OS how to create a process image. There will
//be an array of these starting at e_phoff. There will be a total
//of e_phnum of these with a size of e_phentsize for each. 
typedef struct{
   //Identifies the type of the segment
   #define PT_NULL 0
   #define PT_LOAD 0x1
   #define PT_DYNAMIC 0x2
   #define PT_INTERP 0x3
   Elf64_Word p_type;

   //Segment attributes
   Elf64_Word p_flags;
   
   //This contains the offset of the segment
   //within the file
   Elf64_Off p_offset;

   //This contains the virtual address of the segment
   //within memory
   Elf64_Addr p_vaddr;
 
   //For a system where the physical address is important,
   //this is used. On x86, this DOES NOT MATTER and is undefined
   //for the System V ABI.
   Elf64_Addr p_paddr;

   //Size of this segment. We can take this with p_offset to
   //get a descriptor for the segment.
   Elf64_Xword p_filesize;

   //Size in bytes when the segment is in memory.
   Elf64_Xword p_memsize;

   //0 and 1 specify no alignment. Should be integral power of 2.
   Elf64_Xword p_align;
} Elf64_Phdr;
