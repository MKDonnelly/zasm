#pragma once
#include <stdint.h>
#include "common.h"

void generate_elf_header(buffer_t *buf, uint64_t vaddr);
void build_elfheader(buffer_t *buf, uint64_t code_offset);
void build_pheader(buffer_t *buf);
