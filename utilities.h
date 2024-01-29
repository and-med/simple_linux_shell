#ifndef UTILITIES_H
#define UTILITIES_H
#include<stdio.h>
#include<sys/types.h>

void unix_error(char *msg);
void app_error(char *msg);
char* Fgets(char *s, int size, FILE* stream);
pid_t Fork();

#endif