#ifndef NN_SERVER_H
#define NN_SERVER_H

#include <winsock2.h>

#include "utils.h"
#include "nn_actions.h"

/**** Global Variables ****/
SOCKET server;
struct sockaddr_in server_addr;

pthread_mutex_t mx_max_cl;
unsigned int max_cl;

pthread_mutex_t mx_cl_count;
int cl_count;

pthread_t sv_main_th;
pthread_mutex_t mx_cl_clients;
client_t *cl_clients;

pthread_mutex_t mx_running;
unsigned int running;
/*****************************/

void sv_request_start();
void sv_request_stop();
unsigned int is_running();

void sv_cl_count_decr();
void sv_cl_count_incr();
int sv_get_cl_count();

unsigned int sv_get_max_cl();

void server_startup(const char *ip, unsigned short port, unsigned int max_clients);
void server_stop();

void response(struct client_t *, char* res, unsigned int len);

void remove_th(int index);

void *server_main_thread(void *arg);
void *cl_manager_thread(void *arg);

#endif // NN_SERVER_H
