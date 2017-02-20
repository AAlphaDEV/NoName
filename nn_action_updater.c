#include "nn_actions.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define PART_SZ 4096

int action_update(char *args[], client_t *cl) //name="up" - arg1=file-to-update - arg2=nb_part - arg3=restart
{
    char buffer[512];
    char filename[64];
    char tmp_filename[64];
    int nb_parts;
    int restart;

    char ult_buffer[PART_SZ];
    FILE *file;

    if(args[0] == NULL || args[1] == NULL || args[2] == NULL)
    {
        a_printf("action_up", "need at least three args.");

        send_failed(cl->s, "action_up", "need at least three args.");
        return 1;
    }

    strcpy(filename, args[0]);
    sprintf(tmp_filename, "%s.tmp", filename);
    nb_parts = atoi(args[1]);
    restart = atoi(args[2]);

    sprintf(buffer, "filename=\"%s\"", filename);
    a_printf("action_up", buffer);

    sprintf(buffer, "tmp_filename=\"%s\"", tmp_filename);
    a_printf("action_up", buffer);

    sprintf(buffer, "nb_parts=%d", nb_parts);
    a_printf("action_up", buffer);

    file = fopen(tmp_filename, "wb");
    if(file == NULL)
    {
        strcpy(buffer, "error : ");
        strcat(buffer, strerror(errno));
        a_printf("action_up", buffer);

        sprintf(buffer, "failed/action=\"action_up\"/reason=\"%s\"", strerror(errno));
        safe_send(cl->s, buffer, strlen(buffer), 0);
    }

    int p;
    for(p = 0; p<nb_parts; p++)
    {
        int rsize = recv(cl->s, ult_buffer, PART_SZ, 0);
        sprintf(buffer, "writing part %d - rsize=%d", p, rsize);
        a_printf("action_up", buffer);

        int i;
        for(i = 0; i<rsize; i++)
        {
            fputc(ult_buffer[i], file);
        }
    }

    strcpy(buffer, "all parts received.");
    a_printf("action_up", buffer);

    fclose(file);

    sprintf(buffer, "\"%s\";\"%s\";%d", filename, tmp_filename, restart);
    char *proc_args = malloc(strlen(buffer) * sizeof(char));
    if(proc_args == NULL)
    {
        fprintf(stderr, "Failed to allocate memory, error : %s\n", strerror(errno));

        strcpy(buffer, "failed/action=\"action_up\"/\"failed to allocate memory\"//");
        safe_send(cl->s, buffer, strlen(buffer), 0);
        return 2;
    }
    strncpy(proc_args, buffer, strlen(buffer));

    Sleep(50);
    strcpy(buffer, "result/action=\"action_up\"/\"success - restarting server...\"//");
    safe_send(cl->s, buffer, strlen(buffer), 0);

    /***** New process *****/
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    int result = CreateProcess("changer.exe",   // the path
        proc_args,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory
        &si,            // Pointer to STARTUPINFO structure
        &pi             // Pointer to PROCESS_INFORMATION structure
    );

    if(result == 0)
    {
        char error[128];
        strcpy(error, "[!!] Failed to open process : ");

        int lasterror = GetLastError();
        char err_id[10];
        sprintf(err_id, "[%d] ", lasterror);
        strcat(error, err_id);

        char *s = NULL;
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL, lasterror,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                       (LPSTR)&s, 0, NULL);
        strcat(error, s);
        LocalFree(s);

        fprintf(stderr, "%s", error);
    }
    /***********************/

    sv_request_stop();

    Sleep(100);

    return 0;
}
