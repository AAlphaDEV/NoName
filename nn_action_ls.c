#include "nn_actions.h"

#include <dirent.h>
#include <errno.h>

int action_ls(char *args[], client_t *cl) //name="ls" - arg1=<path>
{
    char *path = args[0];
    int buffer_len = 4096;
    char buffer[4096];
    char buffer2[4096];
    DIR *dir = NULL;
    struct dirent *child = NULL;
    int send_len = 0;

    if(path == NULL)
    {
        a_printf("action_ls", "failed");
        send_failed(cl->s, "action_ls", "at least one arg needed.");

        return 1;
    }

    sprintf(buffer, "path=\"%s\"", path);
    a_printf("action_ls", buffer);

    strcpy(buffer, "doing/action=\"action_ls\"//");
    safe_send(cl->s, buffer, strlen(buffer), 0);

    dir = opendir(path);
    if(dir == NULL)
    {
        sprintf(buffer, "Error with file '%s' : %s (errcode=%d)", path, strerror(errno), errno);
        a_printf("action_ls", buffer);

        send_failed(cl->s, "action_ls", buffer);

        return 2;
    }

    strcpy(buffer, "result/action=\"action_ls\"/subfiles=");
    send_len += strlen(buffer);
    while((child = readdir(dir)) != NULL)
    {
        memset(buffer2, 0, 4096);
        int l = sprintf(buffer2, "\"%s\";", child->d_name);

        if(strlen(buffer) + strlen(buffer2) > (buffer_len-20))
        {
            strcat(buffer, "/more");
            break;
        }
        send_len += l;
        strcat(buffer, buffer2);
    }

    strcpy(buffer2, "//");
    send_len += strlen(buffer2);
    strcat(buffer, buffer2);

    sprintf(buffer2, "send \"%s\"", buffer);
    a_printf("action_ls", buffer2);
    memset(buffer2, 0, 4096);

    //sprintf(buffer2, "len=%i", send_len);
    //a_printf("action_ls", buffer2);

    safe_send(cl->s, buffer, send_len, 0);

    if(closedir(dir) == -1)
    {
        strcpy(buffer, "error : ");
        strcat(buffer, strerror(errno));
        a_printf("action_ls", buffer);

        sprintf(buffer, "failed/action=\"action_ls\"/reason=\"%s\"//", strerror(errno));
        safe_send(cl->s, buffer, strlen(buffer), 0);

        return 2;
    }

    return 0;
}
