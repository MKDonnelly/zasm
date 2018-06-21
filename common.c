#include "common.h"
#include <string.h>


char *strip_chars(char *input, char *remove){
   if( input == NULL )
      return input;
   int non_ws = 0;
   int i = 0;
   while( input[i] != 0 ){
      int good = 1;
      for(int j = 0; remove[j] != 0; j++){
         if( remove[j] == input[i] )
            good = 0;
      }
      if( good ){
         input[non_ws] = input[i];
         non_ws++;
      }
      i++;
   }
   input[non_ws] = 0;
   return input;
}

//Line must have enough space to expand
void substr_replace(char *line, char *search_for, char *replace_with){
   int i = 0;
   int ti = 0;
   char temp[500];
   while( line[i] != 0 ){
      if( strncmp( &line[i], search_for, strlen(search_for) ) == 0 ){
         strcpy( &temp[ti], replace_with );
         ti += strlen(replace_with);
         i += strlen(search_for);
      }else{
         temp[ti] = line[i];
         ti++;
         i++;
      }
   }
   temp[ti] = 0;
   strcpy( line, temp );
}

