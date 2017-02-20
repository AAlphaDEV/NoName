#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#include "utils.h"
#include "nn_server.h"

#define VERSION_MAJOR 0
#define VERSION_MINOR 1

int runevery();

int LOG = 1;
int ERRLOG = 1;
int RUNEVERY = 0;
int HIDE_CONSOLE = 0;

FILE *log_file;
FILE *errlog_file;

int main(int argc, char *argv[])
{
    if(HIDE_CONSOLE)
    {
        HWND console = GetConsoleWindow();
        ShowWindow(console, SW_HIDE);
    }

    WSADATA wsa;

    start_log();

    if(WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        fatalWS("while initializing winsock ");
    }

    if(RUNEVERY)
        runevery();

    printf("*********************\n");
    printf("***  NoName_v%d.%d  ***\n", VERSION_MAJOR, VERSION_MINOR);
    printf("*********************\n");
    server_startup("127.0.0.1", 33334, 5);

    server_stop();
    WSACleanup();

    close_log();

    return 0;
}

void start_log()
{
    if(LOG)
    {
        log_file = freopen("log\\winsec.log", "a", stdout);
        if(log_file == NULL)
        {
            fprintf(stderr, "failed to open log file");
            LOG = 0;
        }
    }
    if(ERRLOG)
    {
        errlog_file = freopen("log\\error.log", "a", stderr);
        if(errlog_file == NULL)
        {
            fprintf(stderr, "failed to open error log file");
            ERRLOG = 0;
        }
    }
}

void close_log()
{
    if(LOG)
    {
        fclose(log_file);
    }
    if(ERRLOG)
    {
        fclose(errlog_file);
    }
}

int runevery()
{
    char val_name[128];
    char value[1024];

    char buffer[512];
    unsigned long res;

    res = GetModuleFileName(NULL, buffer, 512);
    if(res == 0)
    {
        fatalWinAPI("while retrieving program's name ", GetLastError());
        return -1;
    }

    strcpy(val_name, "winsec");
    sprintf(value, "\"%s\"", buffer);

    unsigned long length = strlen(value);

    HKEY key;
    res = RegOpenKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &key);
    if(res != ERROR_SUCCESS)
    {
        fatalWinAPI("while opening subkey ", res);
        return -1;
    }

    res = RegSetValueEx(key, val_name, 0, REG_SZ, (LPBYTE) value, length);

    return 0;
}
