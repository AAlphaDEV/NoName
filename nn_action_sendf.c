#include "nn_actions.h"

#include <errno.h>

#define PART_SZ 65536

int action_sendf(char *args[], client_t *cl) //name=action_sendf args1=path | result/action=\"action_sendf\"/<nb_parts>// [content]
{
    if(args[0] == NULL)
    {
        a_printf("action_sendf", "Need at least one arg.");
        send_failed(cl->s, "action_sendf", "Need at least one arg.");
        return 1;
    }

    char buffer[PART_SZ];
    char buffer2[512];
    FILE *file;

    file = fopen(args[0], "rb");
    if(file == NULL)
    {
        sprintf(buffer2, "Error with file '%s' : %s (errcode=%d)", args[0], strerror(errno), errno);
        a_printf("action_sendf", buffer2);

        send_failed(cl->s, "action_sendf", buffer2);
        return 2;
    }

    fseek(file, 0, SEEK_END);
    long int size = ftell(file);
    fseek(file, 0, SEEK_SET);

    sprintf(buffer2, "size : %ld", size);
    a_printf("action_sendf", buffer2);

    int nb_parts = (int) (size / PART_SZ);
    if((size % PART_SZ) > 0)
        nb_parts++;

    sprintf(buffer2, "nb_parts=%d (PART_SZ=%d)", nb_parts, PART_SZ);
    a_printf("action_sendf", buffer2);

    sprintf(buffer2, "result/action=\"action_sendf\"/%d//", nb_parts);
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
                sprintf(buffer2, "EOF - i=%d - c=%c (%d) - ftell()=%ld", j, c, c, ftell(file));
                a_printf("action_sendf", buffer2);
                send_sz = j;
                break;
            }

            buffer[j] = (char) c;
        }
        //sprintf(buffer2, "sending part %d - size : %d", i, send_sz);
        //a_printf("action_sendf", buffer2);
        if(safe_send(cl->s, buffer, send_sz, 0) == -1)
        {
            fclose(file);
            return 3;
        }
    }
    a_printf("action_sendf", "all part sent.");

    fclose(file);

    return 0;
}
