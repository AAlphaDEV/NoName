#include "nn_actions.h"

#include <errno.h>

int action_delf(char *args[], client_t *cl) //name="delf" - arg1=file-to-delete
{
    char buffer[512];

    if(args[0] == NULL)
    {
        a_printf("action_delf", "Need at least one arg.");
        send_failed(cl->s, "action_delf", "Need at least one arg.");
        return 1;
    }

    int r = remove(args[0]);
    if(r == -1)
    {
        sprintf(buffer, "Error with file '%s' : %s (errcode=%d)", args[0], strerror(errno), errno);
        a_printf("action_delf", buffer);

        send_failed(cl->s, "action_delf", buffer);
        return 2;
    }

    strcpy(buffer, "result/action=\"action_delf\"/\"File successfully removed.\"//");
    safe_send(cl->s, buffer, strlen(buffer), 0);

    return 0;
}
