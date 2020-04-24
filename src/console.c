#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lodis_api.h"

struct command{
    char op[100];
    char key[100];
    char value[200];
    char ttl[10];
};

void parseCommand(struct command *cmd , char *buf)
{
    int i = 0;  
    int j = 0;
    for ( i = 0 ; i < strlen(buf) ; i ++){
        while ( i < strlen(buf) &&  buf[i] != ' ' ){
            cmd->op[j ++] = buf[i++];
        }
        if ( i == strlen( buf) )    break;
        i ++;

        j = 0;
        while ( i < strlen(buf) &&  buf[i] != ' ' ){
            cmd->key[j ++] = buf[i++];
        }
        if ( i == strlen( buf) )    break;
        i ++;

        j = 0;
        while ( i < strlen(buf) &&  buf[i] != ' ' ){
            cmd->value[j ++] = buf[i++];
        }
        if ( i == strlen( buf) )    break;
        i ++;

        j = 0;
        while ( i < strlen(buf) &&  buf[i] != ' ' ){
            cmd->ttl[j ++] = buf[i++];
        }
        break;
    }
    return;
}

int main()
{
    char buf[2000];
    int  ret = 0;
    struct command cmd;
    char   value[2000];
    while (1){
        printf(">");
        memset( buf , 0x00 , sizeof(buf));
        memset( &cmd , 0x00 , sizeof(cmd));
        gets(buf);
        parseCommand( &cmd , buf);
        if ( memcmp( cmd.op , "init" , 4 ) == 0 ){
            if ( (ret = lodis_init(atoi(cmd.key))) != 0 ){
                printf(">init error , ret[%d]\n" , ret);
            } else {
                printf(">ok\n");
            }
        } else if ( memcmp( cmd.op , "set" , 3) == 0 ){
            if ( (ret=lodis_set(cmd.key , cmd.value , strlen(cmd.value) , atoi(cmd.ttl))) != 0 ){
                printf(">set error , ret[%d]\n" , ret);
            } else {
                printf(">ok\n");
            }
        } else if ( memcmp( cmd.op , "get" , 3) == 0 ){
            memset(value , 0x00 , sizeof(value));
            if ( (ret = lodis_get(cmd.key , value , sizeof(value)) ) != 0 ){
                printf(">get error , ret[%d]\n" , ret);
            } else {
                printf(">ok\n");
                printf(">value = %s\n" , value);
            }
        } else if ( memcmp( cmd.op , "del" , 3) == 0 ){
            if ( (ret = lodis_del(cmd.key) ) != 0 ) {
                printf(">del error , ret[%d]\n" , ret);
            } else {
                printf(">ok\n");
            }
        } else if ( memcmp( cmd.op , "flush" , 5) == 0 ){
            if ( ( ret = lodis_flushAll() ) != 0 ) {
                printf(">flush error , ret[%d]\n" , ret);
            } else {
                printf(">ok\n");
            }
        } else if ( memcmp( cmd.op , "logon" , 5) == 0 ){
            lodis_logon();
        } else if ( memcmp( cmd.op , "logoff" , 6) == 0 ){
            lodis_logoff();
        } else if ( memcmp( cmd.op , "close" , 5) == 0 ){
            if ( ( ret = lodis_close() ) != 0 ) {
                printf(">close error , ret[%d]\n" , ret);
            } else {
                printf(">ok\n");
            }
            break;
        } 
    }
    return 0;
}
