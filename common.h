#pragma once

typedef struct{
   char *buffer;
   int len;
}buffer_t;

char *strip_chars(char *input, char *remove);
void substr_replace(char *line, char *search_for, char *replace_with);
