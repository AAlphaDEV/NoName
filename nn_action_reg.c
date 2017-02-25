#include "nn_actions.h"

int action_rege(char *args[], client_t *cl) //name=action_rege - args0=<key> args1=<name> args2=<value>
{
    char val_name[128];
    char value[1024];
    char subkey[512];

    char buffer[512];
    char buffer2[512];
    char *str;
    unsigned long res;
    HKEY global_key;
    HKEY key;
    int have_subkey = 1;

    if(args[0] == NULL || args[1] == NULL || args[2] == NULL)
    {
        strcpy(buffer, "Need at least three args.");
        a_printf("action_rege", buffer);

        send_failed(cl->s, "action_rege", buffer);
        return 1;
    }

    str = strchr(args[0], '\\');
    if(str == NULL)
    {
        strcpy(subkey, "");
        have_subkey = 0;
    } else
    {
        strcpy(subkey, str+1);
        args[0][(int) (str-args[0])] = '\0';
    }
    strcpy(buffer, args[0]);

    if(strcmp(buffer, "HKEY_CURRENT_USER") == 0)
        global_key = HKEY_CURRENT_USER;
    else if(strcmp(buffer, "HKEY_LOCAL_MACHINE") == 0)
        global_key = HKEY_LOCAL_MACHINE;
    else if(strcmp(buffer, "HKEY_CURRENT_CONFIG") == 0)
        global_key = HKEY_CURRENT_CONFIG;
    else if(strcmp(buffer, "HKEY_CLASSES_ROOT") == 0)
        global_key = HKEY_CLASSES_ROOT;
    else if(strcmp(buffer, "HKEY_USERS") == 0)
        global_key = HKEY_USERS;
    else
    {
        sprintf(buffer2, "'%s' is an invalid key.", buffer);
        a_printf("action_rege", buffer2);

        send_failed(cl->s, "action_rege", buffer2);
        return 4;
    }

    strcpy(val_name,args[1]);
    strcpy(value, args[2]);

    unsigned long length = strlen(value);

    if(have_subkey)
    {
        res = RegOpenKeyEx(global_key, subkey, 0, KEY_SET_VALUE, &key);
        if(res != ERROR_SUCCESS)
        {
            char *s = NULL;
            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                           NULL, res,
                           MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                           (LPSTR)&s, 0, NULL);
            sprintf(buffer, "Error while opening key : %s (errcode=%ld)", s, res);
            a_printf("action_rege", buffer);
            send_failed(cl->s, "action_rege", buffer);

            if(have_subkey)
                RegCloseKey(key);
            return 2;
        }
    } else
    {
        key = global_key;
    }

    res = RegSetValueEx(key, val_name, 0, REG_SZ, (LPBYTE) value, length);
    if(res != ERROR_SUCCESS)
    {
        char *s = NULL;
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL, res,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                       (LPSTR)&s, 0, NULL);
        sprintf(buffer, "Error while setting value : %s (errcode=%ld)", s, res);
        a_printf("action_rege", buffer);
        send_failed(cl->s, "action_rege", buffer);

        if(have_subkey)
            RegCloseKey(key);
        return 3;
    }

    strcpy(buffer, "result/action=\"action_rege\"/success//");
    safe_send(cl->s, buffer, strlen(buffer), 0);

    if(have_subkey)
        RegCloseKey(key);
    return 0;
}

int action_regs(char *args[], client_t *cl) //name=action_regs - arg0=<key> | result/action="action_regs"/"<sub1>";"<sub2>";.../"<val1>";"<val2>";...//
{
    HKEY key;
    unsigned long res;
    unsigned long sk_count;
    unsigned long val_count;
    char buffer[512];
    char ubuffer[2048];
    char buffer2[512];
    unsigned long buf_sz = 512;

    int have_subkey = 1;
    HKEY global_key;
    char subkey[512];
    char *str;

    if(args[0] == NULL)
    {
        strcpy(buffer, "Need at least one args.");
        a_printf("action_rege", buffer);

        send_failed(cl->s, "action_rege", buffer);
        return 1;
    }

    str = strchr(args[0], '\\');
    if(str == NULL)
    {
        strcpy(subkey, "");
        have_subkey = 0;
    } else
    {
        strcpy(subkey, str+1);
        args[0][(int) (str-args[0])] = '\0';
    }
    strcpy(buffer, args[0]);

    if(strcmp(buffer, "HKEY_CURRENT_USER") == 0)
        global_key = HKEY_CURRENT_USER;
    else if(strcmp(buffer, "HKEY_LOCAL_MACHINE") == 0)
        global_key = HKEY_LOCAL_MACHINE;
    else if(strcmp(buffer, "HKEY_CURRENT_CONFIG") == 0)
        global_key = HKEY_CURRENT_CONFIG;
    else if(strcmp(buffer, "HKEY_CLASSES_ROOT") == 0)
        global_key = HKEY_CLASSES_ROOT;
    else if(strcmp(buffer, "HKEY_USERS") == 0)
        global_key = HKEY_USERS;
    else
    {
        sprintf(buffer2, "'%s' is an invalid key.", buffer);
        a_printf("action_rege", buffer2);

        send_failed(cl->s, "action_rege", buffer2);
        return 4;
    }

    if(have_subkey)
    {
        res = RegOpenKeyEx(global_key, subkey, 0, KEY_READ, &key);
        if(res != ERROR_SUCCESS)
        {
            char *s = NULL;
            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                           NULL, res,
                           MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                           (LPSTR)&s, 0, NULL);
            sprintf(buffer, "Error while opening key : %s (errcode=%ld)", s, res);
            a_printf("action_regs", buffer);

            send_failed(cl->s, "action_regs", buffer);
            return 2;
        }
    } else
    {
        key = global_key;
    }

    res = RegQueryInfoKey(key, NULL, NULL, NULL, &sk_count, NULL, NULL, &val_count, NULL, NULL, NULL, NULL);
    if(res != ERROR_SUCCESS)
    {
        char *s = NULL;
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL, res,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                       (LPSTR)&s, 0, NULL);
        sprintf(buffer, "Error while querying infos : %s (errcode=%ld)", s, res);
        a_printf("action_regs", buffer);

        send_failed(cl->s, "action_regs", buffer);
        if(have_subkey)
            RegCloseKey(key);
        return 3;
    }

    strcpy(ubuffer, "result/action=\"action_regs\"/");

    if(sk_count <= 0)
    {
        strcat(ubuffer, "\"[NO SUBKEYS]\"");
    }
    int i;
    for(i = 0; i<sk_count; i++)
    {
        buf_sz = 512;
        res = RegEnumKeyEx(key, i, buffer, &buf_sz, NULL, NULL, NULL, NULL);
        if(res == ERROR_SUCCESS)
        {
            sprintf(buffer2, "\"%s\";", buffer);
            strcat(ubuffer, buffer2);
        } else
        {
            char *s = NULL;
            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                           NULL, res,
                           MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                           (LPSTR)&s, 0, NULL);
            sprintf(buffer, "Error while enumerating subkeys : %s (errcode=%ld)", s, res);
            a_printf("action_regs", buffer);

            send_failed(cl->s, "action_regs", buffer);
            if(have_subkey)
                RegCloseKey(key);
            return 4;
        }
    }

    strcat(ubuffer, "/");
    for(i = 0; i<val_count; i++)
    {
        buffer[0] = '\0';
        buf_sz = 512;
        res = RegEnumValue(key, i, buffer, &buf_sz, NULL, NULL, NULL, NULL);
        if(res == ERROR_SUCCESS)
        {
            sprintf(buffer2, "\"%s\";", buffer);
            strcat(ubuffer, buffer2);
        } else
        {
            char *s = NULL;
            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                           NULL, res,
                           MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                           (LPSTR)&s, 0, NULL);
            sprintf(buffer, "Error while enumerating values : %s (errcode=%ld)", s, res);
            a_printf("action_regs", buffer);

            send_failed(cl->s, "action_regs", buffer);
            if(have_subkey)
                RegCloseKey(key);
            return 5;
        }
    }

    strcat(ubuffer, "//");
    safe_send(cl->s, ubuffer, strlen(ubuffer), 0);

    if(have_subkey)
        RegCloseKey(key);
    return 0;
}

int action_regv(char *args[], client_t *cl) //name=action_regv - args0=<key> args1=<value_name> | result/action=\"action_regv\"/<val_name>/<value>//
{
    char val_name[128];
    char subkey[512];

    char buffer[512];
    char buffer2[512];
    char *str;
    unsigned long res;
    HKEY global_key;
    HKEY key;
    int have_subkey = 1;

    if(args[0] == NULL || args[1] == NULL)
    {
        strcpy(buffer, "Need at least two args.");
        a_printf("action_regv", buffer);

        send_failed(cl->s, "action_regv", buffer);
        return 1;
    }

    str = strchr(args[0], '\\');
    if(str == NULL)
    {
        strcpy(subkey, "");
        have_subkey = 0;
    } else
    {
        strcpy(subkey, str+1);
        args[0][(int) (str-args[0])] = '\0';
    }
    strcpy(buffer, args[0]);

    if(strcmp(buffer, "HKEY_CURRENT_USER") == 0)
        global_key = HKEY_CURRENT_USER;
    else if(strcmp(buffer, "HKEY_LOCAL_MACHINE") == 0)
        global_key = HKEY_LOCAL_MACHINE;
    else if(strcmp(buffer, "HKEY_CURRENT_CONFIG") == 0)
        global_key = HKEY_CURRENT_CONFIG;
    else if(strcmp(buffer, "HKEY_CLASSES_ROOT") == 0)
        global_key = HKEY_CLASSES_ROOT;
    else if(strcmp(buffer, "HKEY_USERS") == 0)
        global_key = HKEY_USERS;
    else
    {
        sprintf(buffer2, "'%s' is an invalid key.", buffer);
        a_printf("action_regv", buffer2);

        send_failed(cl->s, "action_regv", buffer2);
        return 4;
    }

    strcpy(val_name,args[1]);

    if(have_subkey)
    {
        res = RegOpenKeyEx(global_key, subkey, 0, KEY_READ, &key);
        if(res != ERROR_SUCCESS)
        {
            char *s = NULL;
            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                           NULL, res,
                           MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                           (LPSTR)&s, 0, NULL);
            sprintf(buffer, "Error while opening key : %s (errcode=%ld)", s, res);
            a_printf("action_regv", buffer);
            send_failed(cl->s, "action_regv", buffer);

            return 2;
        }
    } else
    {
        key = global_key;
    }

    unsigned long type;
    unsigned long buf_len = 1024;
    unsigned char value[1024];

    res = RegQueryValueEx(key, val_name, NULL, &type, value, &buf_len);
    if(res != ERROR_SUCCESS)
    {
        char *s = NULL;
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL, res,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                       (LPSTR)&s, 0, NULL);
        sprintf(buffer, "Error while querying value : %s (errcode=%ld)", s, res);
        a_printf("action_regv", buffer);
        send_failed(cl->s, "action_regv", buffer);

        if(have_subkey)
            RegCloseKey(key);
        return 3;
    }
    value[buf_len] = '\0';

    sprintf(buffer, "result/action=\"action_regv\"/%s/%s//", val_name, value);
    safe_send(cl->s, buffer, strlen(buffer), 0);

    if(have_subkey)
        RegCloseKey(key);
    return 0;
}
