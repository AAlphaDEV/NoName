#ifndef UTILS_H
#define UTILS_H


#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

struct client_t {
    SOCKET s;
    int id;
    pthread_t cl_th;
};

typedef struct client_t client_t;

void copy_file(FILE *source, FILE *dest);

void fatal(char *err_msg);
void fatalWS(char *err_msg);
void fatalWinAPI(char *err_msg, int errcode);

void safe_pthread_create(pthread_t *th, const pthread_attr_t *attr, void *(* func)(void *), void *arg);
void safe_pthread_join(pthread_t t, void **res);

int safe_send(SOCKET s, const char* b, int len, int flags);

void close_log();
void start_log();

#endif // UTILS_H
