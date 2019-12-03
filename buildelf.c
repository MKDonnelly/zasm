#include <stdio.h>
#include <stdlib.h>

elfout_t *elfout_create(char *output_name){
   elfout_t *new_elfout = malloc(sizeof(elfout_t));
   new_elfout->output = fopen(output_name, "w");

   new_elfout->total_sections = 0;
}



