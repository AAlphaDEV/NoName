#include "nn_server.h"

unsigned int th_size;

void server_startup(const char *ip, unsigned short port, unsigned int max_clients)
{
    printf("Creating server...\n");

    server = socket(AF_INET, SOCK_STREAM, 0);
    if(server == INVALID_SOCKET)
    {
        fatalWS("while creating socket ");
    }

    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);
    server_addr.sin_family = AF_INET;

    if(bind(server, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) != 0)
    {
        fatalWS("while binding server ");
    }

    printf("Server bound to %s:%hu.\n", ip, port);

    printf("Initializing actions...\n");
    init_actions();

    max_cl = max_clients;
    mx_max_cl = PTHREAD_MUTEX_INITIALIZER;

    cl_count = 0;
    mx_cl_count = PTHREAD_MUTEX_INITIALIZER;

    running = 1;
    mx_running = PTHREAD_MUTEX_INITIALIZER;

    listen(server, max_clients);

    th_size = sizeof(pthread_t);
    cl_clients = (client_t *) malloc(sizeof(client_t) * max_cl);
    int i;
    for(i = 0; i<max_cl; i++)
    {
        cl_clients[i].id = -1;
    }
    mx_cl_clients = PTHREAD_MUTEX_INITIALIZER;

    safe_pthread_create(&sv_main_th, NULL, server_main_thread, NULL);

    while(is_running())
    {
        Sleep(50);
    }

    return;
}

void server_stop()
{
    char buffer[512];

    pthread_cancel(sv_main_th);

    int i;
    for(i = 0; i<cl_count; i++)
    {
        sprintf(buffer, "closed/reason=\"closed by client\"//");
        send(cl_clients[i].s, buffer, strlen(buffer), 0);

        printf("Stopping thread : %d - cl_count=%d\n", i, cl_count);
        pthread_kill(cl_clients[i].cl_th, 0);
    }

    free(cl_clients);

    closesocket(server);

    printf("Server closed.\n");
}

void *server_main_thread(void *arg)
{
    SOCKET cl_new;
    int addr_size = sizeof(struct sockaddr);
    char buffer[1048];

    printf("Server started, waiting for clients...\n");

    while(1)
    {
        cl_new = accept(server, (struct sockaddr *) &server_addr, &addr_size);
        if(cl_new == INVALID_SOCKET)
        {
            continue;
        }

        if(sv_get_cl_count() >= max_cl)
        {
            printf("Limit of clients reached (cl_count>=max_cl). cl_count=%d - max_cl=%d\n", sv_get_cl_count(), sv_get_max_cl());

            strcpy(buffer, "refused/reason=\"limit of clients reached\"//");
            safe_send(cl_new, buffer, strlen(buffer), 0);

            closesocket(cl_new);
            continue;
        }

        pthread_t new_th;

        sv_cl_count_incr();

        int index = -1;
        int i;
        for(i = 0; i<sv_get_max_cl(); i++)
        {
            if(cl_clients[i].id == -1)
            {
                index = i;
                break;
            }
        }

        if(index == -1)
        {
            printf("Limit of clients reached (index=-1). cl_count=%d\n", sv_get_cl_count());

            strcpy(buffer, "refused/reason=\"limit of clients reached\"//");
            safe_send(cl_new, buffer, strlen(buffer), 0);

            closesocket(cl_new);
            sv_cl_count_decr();
            continue;
        }

        cl_clients[index].s = cl_new;
        cl_clients[index].id = index;

        safe_pthread_create(&new_th, NULL, cl_manager_thread, &cl_clients[index]);

        //printf("Adding thread at %d\n", index);

        pthread_mutex_lock(&mx_cl_clients);
        cl_clients[index].cl_th = new_th;
        pthread_mutex_unlock(&mx_cl_clients);

        Sleep(200);
    }

    pthread_exit(NULL);
    return NULL;
}

void *cl_manager_thread(void *arg)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

    client_t *client = (client_t *) arg;

    pthread_mutex_lock(&mx_cl_clients);
    SOCKET cl = client->s;
    int cl_id = client->id;
    pthread_mutex_unlock(&mx_cl_clients);

    char buffer[1024];
    int recv_s;

    printf("New client accepted, id : %d.\n", cl_id);

    sprintf(buffer, "accepted/id=%d/version=\"%d.%d\"//", cl_id, VERSION_MAJOR, VERSION_MINOR);
    safe_send(cl, buffer, strlen(buffer), 0); //SEND

    while(1)
    {
        recv_s = recv(cl, buffer, 1024, 0);
        if(recv_s == 0 || recv_s == -1)
        {
            Sleep(100);
            printf("Connection to client %d lost.\n", cl_id);
            sv_cl_count_decr();
            remove_th(cl_id);
            break;
        }

        buffer[recv_s] = '\0';

        printf("\tReceived from client %d : %s\n", cl_id, buffer);

        response(client, buffer, strlen(buffer));

        Sleep(50);
    }

    int ret = cl_id;

    pthread_exit(&ret);
    return NULL;
}

void response(client_t *cl, char* res, unsigned int len)
{
    char buffer[1024];
    char *action_name;
    char *args[3];
    char *token;

    token = strtok(res, "/");
    action_name = token;
    int i = 0;
    for(i = 0; i<3; i++)
    {
        args[i] = NULL;
    }

    i = 0;
    while(token != NULL)
    {
        token = strtok(NULL, "/");
        args[i] = token;

        i++;
    }

    //printf("Action name : %s\n", action_name);

    if(strcmp(action_name, "stop") == 0)
    {
        sprintf(buffer, "closed/reason=\"closed by client %d\"//", cl->id);
        safe_send(cl->s, buffer, strlen(buffer), 0);

        sv_request_stop();
        Sleep(100);
    } else
    {
        int r = do_action(action_name, args, cl);
        if(r == -1)
        {
            sprintf(buffer, "failed/action=\"no_action\"/reason=\"action not found : %s\"//", action_name);
            safe_send(cl->s, buffer, strlen(buffer), 0);
        }
    }
}

unsigned int is_running()
{
    pthread_mutex_lock(&mx_running);
    unsigned int r = running;
    pthread_mutex_unlock(&mx_running);

    return r;
}

void sv_request_stop()
{
    pthread_mutex_lock(&mx_running);
    running = 0;
    pthread_mutex_unlock(&mx_running);
}

void sv_request_start()
{
    pthread_mutex_lock(&mx_running);
    running = 1;
    pthread_mutex_unlock(&mx_running);
}

void sv_cl_count_decr()
{
    pthread_mutex_lock(&mx_cl_count);
    cl_count--;
    pthread_mutex_unlock(&mx_cl_count);
}
void sv_cl_count_incr()
{
    pthread_mutex_lock(&mx_cl_count);
    cl_count++;
    pthread_mutex_unlock(&mx_cl_count);
}
int sv_get_cl_count()
{
    pthread_mutex_lock(&mx_cl_count);
    unsigned int c = cl_count;
    pthread_mutex_unlock(&mx_cl_count);

    return c;
}

unsigned int sv_get_max_cl()
{
    pthread_mutex_lock(&mx_max_cl);
    unsigned int c = max_cl;
    pthread_mutex_unlock(&mx_max_cl);

    return c;
}

void remove_th(int index)
{
    pthread_mutex_lock(&mx_cl_clients);
    cl_clients[index].s = INVALID_SOCKET;
    cl_clients[index].id = -1;
    pthread_mutex_unlock(&mx_cl_clients);
}
