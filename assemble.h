#pragma once
#include "common.h"
#include "lexer.h"

void assemble_line(asmline_t *parsed, buffer_t *output_mcode);
