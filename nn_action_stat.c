#include "nn_actions.h"

#include <unistd.h>
#include <sys/stat.h>

int action_stat(char *args[], client_t *cl) // name=stat - arg1=<path> | <size-of-stat>// [bytes-of-stat]
{
    if(args[0] == NULL)
    {
        a_printf("action_stat", "Need at least one arg.");
        send_failed(cl->s, "action_stat", "Need at least one arg.");
        return 1;
    }

    char buffer[512];
    char path[512];
    struct stat file_stat;
    int stat_sz = sizeof(struct stat);

    strcpy(path, args[0]);

    int r = stat(path, &file_stat);
    if(r == -1)
    {
        sprintf(buffer, "Error with file '%s' : %s (errcode=%d)", path, strerror(errno), errno);
        a_printf("action_stat", buffer);

        send_failed(cl->s, "action_stat", buffer);
        return 2;
    }

    sprintf(buffer, "Sending stat for : \"%s\".", path);
    a_printf("action_stat", buffer);

    sprintf(buffer, "%d//", stat_sz);
    char *ptr = (char *) &file_stat;

    safe_send(cl->s, buffer, strlen(buffer), 0);

    Sleep(50);

    char buffer2[stat_sz];
    int i;
    for(i = 0; i<stat_sz; i++)
    {
        buffer2[i] = (char) *(ptr+i);
    }
    safe_send(cl->s, buffer2, stat_sz, 0);

    return 0;
}
