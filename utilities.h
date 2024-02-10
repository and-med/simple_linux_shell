#ifndef UTILITIES_H
#define UTILITIES_H
#include<stdio.h>
#include<sys/types.h>

#ifdef DEBUG
#define DEBUG_LOG printf("%s:%d\n", __FILE__, __LINE__); 
#else
#define DEBUG_LOG /*foo*/
#endif

void unix_error(char *msg);
void app_error(char *msg);
char* Fgets(char *s, int size, FILE* stream);
pid_t Fork();
pid_t reap_child(pid_t pid, int options);

typedef void handler_t(int);

handler_t *register_signal_handler(int signum, handler_t *handler);

#endif