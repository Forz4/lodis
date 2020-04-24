#ifndef  _LODIS_H_
#define  _LODIS_H_
#define  MAX_KEY_LEN     512
#define  MAX_VAL_LEN     4096
#define  MAX_LOG_LEN     200
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include "lodis_api.h"

typedef struct lodis_index{
    int                 numOfEntry;
    int                 numOfKeys;
    int                 numOfTotalGet;
    int                 numOfTotalHit;
    int                 numOfTotalMiss;
    char                logLevel;
    struct lodis_entry  *entryHead;
}lodis_index_t;

typedef struct lodis_entry{
    int                 numOfCargo;
    struct lodis_cargo  *cargo;
}lodis_entry_t;

typedef struct lodis_cargo{
    char                key[MAX_KEY_LEN];
    char                *value;
    int                 lenOfValue;
    time_t              ttl;
    struct lodis_cargo  *next;
}lodis_cargo_t;

static unsigned int     _lodis_hash(char *key , int size);
static lodis_cargo_t*   _lodis_insert_cargo( char *key , char *value , int length , int ttl);
static int              _lodis_update_cargo( lodis_cargo_t *cargo , char *key , char *value , int length , int ttl);
static void             _lodis_debug(char *format , ... );



#endif
