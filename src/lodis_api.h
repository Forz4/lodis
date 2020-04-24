#ifndef _LODIS_API_H_
#define _LODIS_API_H_

#define LODIS_KEY_TIMEOUT    2
#define LODIS_KEY_NOTFOUND   1
#define LODIS_SUCCESS        0
#define LODIS_INVALID_PARAM  -1
#define LODIS_EXISTS         -2
#define LODIS_NOT_INIT       -3
#define LODIS_MALLOC_FAIL    -4

int     lodis_init(int num);
int     lodis_set(char *key , char *value , int length , int ttl);
int     lodis_get(char *key , char *value , int length);
int     lodis_del(char *key);
void    lodis_logon();
void    lodis_logoff();
int     lodis_flushAll();
int     lodis_close();

#endif
