#include "lodis.h"
static lodis_index_t    *lodisIndex;
/*
 * inittialize memory for lodis_index and lodis_entry(s)
 * parameter:
 *      num:    number of total entrys
 * */
int lodis_init(int num)
{
    if ( num <= 0 ){
        return LODIS_INVALID_PARAM;
    }
    if ( lodisIndex ){
        return LODIS_EXISTS;
    }
    /* initialize global lodis_index */
    lodisIndex = (lodis_index_t *)malloc(sizeof(lodis_index_t));
    if ( lodisIndex == NULL ){
        return LODIS_MALLOC_FAIL;
    }
    lodisIndex->numOfEntry = num;
    lodisIndex->numOfKeys = 0;
    lodisIndex->logLevel = '0';
    lodisIndex->entryHead = (lodis_entry_t *)malloc(sizeof(lodis_entry_t)*num);
    if ( lodisIndex->entryHead == NULL ){
        return LODIS_MALLOC_FAIL;
    }
    memset( lodisIndex->entryHead , 0x00 , sizeof(lodis_entry_t) * num);
    return LODIS_SUCCESS;
}
/*
 * SET operation
 * parameter:
 *      key:        key string
 *      value:      value string
 *      length:     length of value string in bytes
 *      ttl:        time to live , set to 0 for permernent
 * */
int lodis_set(char *key , char *value , int length , int ttl)
{
    if ( lodisIndex == NULL )  return LODIS_NOT_INIT;
    /* parameter check */
    if ( key == NULL || strlen(key) == 0 || strlen(key) > MAX_KEY_LEN ||\
            value == NULL || length > MAX_VAL_LEN || length <= 0 ||\
            ttl < 0 )   return LODIS_INVALID_PARAM;
    /* locate entry */
    unsigned int    offset = _lodis_hash(key,lodisIndex->numOfEntry);
    lodis_entry_t   *entry = lodisIndex->entryHead + offset;
    lodis_cargo_t   *curcargo = entry->cargo;
    /* for debug */
    _lodis_debug("begin lodis_set(%s,%s,%d,%d)",key,value,length,ttl);
    _lodis_debug("entryHead address :%x" , lodisIndex->entryHead);
    _lodis_debug("entry offfset     :%u" , offset);
    _lodis_debug("entry address     :%x" , entry);
    _lodis_debug("entry numOfKeys   :%d" , entry->numOfCargo);
    _lodis_debug("curcargo address  :%x" , curcargo);
    /* first cargo in entry */
    if ( curcargo == NULL ){
        /* insert new cargo at head */
        _lodis_debug("insert first key[%s] into entry[%u]" , key , offset);
        curcargo = _lodis_insert_cargo(key,value,length,ttl);
        if ( curcargo == NULL ){
            return LODIS_MALLOC_FAIL;
        } else {
            entry->numOfCargo ++;
            entry->cargo = curcargo;
        }
    } else {
        /* there are existing cargos */
        while( curcargo ){
            if ( strcmp(curcargo->key , key) == 0 ) {
                _lodis_debug("key[%s] already exist" , key);
                /* update existing cargo */
                if ( _lodis_update_cargo(curcargo , key , value , length , ttl) ) 
                    return LODIS_MALLOC_FAIL;
            }
            if ( curcargo->next == NULL ){
                /* insert new cargo to the end */
                _lodis_debug("insert new cargo at the end of entry[%u]" , offset);
                curcargo->next = _lodis_insert_cargo(key,value,length,ttl);
                if ( curcargo == NULL )
                    return LODIS_MALLOC_FAIL;
                else
                    entry->numOfCargo ++;
                break;
            }
            curcargo = curcargo->next;
        }
    }
    lodisIndex->numOfKeys ++;
    return LODIS_SUCCESS;
}
/*
 * GET operation
 * parameter:
 *      key :       key string
 *      value:      receive value string
 *      length:     receive value length
 * */
int lodis_get(char *key , char *value , int length)
{
    if ( lodisIndex == NULL )  return LODIS_NOT_INIT;
    /* parameter check */
    if ( key == NULL || strlen(key) == 0 || strlen(key) > MAX_KEY_LEN ||\
            value == NULL ) return LODIS_INVALID_PARAM;
    /* update statistics */
    lodisIndex->numOfTotalGet ++;
    /* locate entry */
    unsigned int    offset = _lodis_hash(key,lodisIndex->numOfEntry);
    lodis_entry_t   *entry = lodisIndex->entryHead + offset;
    lodis_cargo_t   *curcargo = entry->cargo;
    lodis_cargo_t   *precargo = entry->cargo;
    /* for debug */
    _lodis_debug("begin lodis_get(%s,%d)",key,length);
    _lodis_debug("entryHead address :%x" , lodisIndex->entryHead);
    _lodis_debug("entry offfset     :%u" , offset);
    _lodis_debug("entry address     :%x" , entry);
    _lodis_debug("entry numOfKeys   :%d" , entry->numOfCargo);
    while( curcargo ){
        /* found key */
        if ( strcmp(curcargo->key , key) == 0 ) {
            /* check ttl */
            if ( curcargo->ttl > 0 && time(NULL) > curcargo->ttl ){
                /* key time out , delete current key */
                _lodis_debug("key[%s] found in entry[%u] but time out" , key , offset);
                free(curcargo->value);
                precargo->next = curcargo->next;
                free(curcargo);
                /* update statistics */
                if ( --entry->numOfCargo == 0 )
                    entry->cargo = NULL;
                lodisIndex->numOfKeys --;
                lodisIndex->numOfTotalMiss ++;

                return LODIS_KEY_TIMEOUT;
            } else {
                /* return value normally */
                _lodis_debug("key[%s] found in entry[%d]" , key , offset);
                if ( curcargo->lenOfValue <= length )
                    memcpy( value , curcargo->value , curcargo->lenOfValue);
                else
                    memcpy( value , curcargo->value , length);
                lodisIndex->numOfTotalHit ++;
                return LODIS_SUCCESS;
            }
        } else {
            precargo = curcargo;
            curcargo = curcargo->next;
        }
    }
    _lodis_debug("key[%s] not found in entry[%d]" , key , offset);
    lodisIndex->numOfTotalMiss ++;
    return LODIS_KEY_NOTFOUND;
}
/*
 * DEL operation
 * paramater:
 *      key:            key string to delete
 * */
int lodis_del(char *key)
{
    if ( lodisIndex == NULL )  return LODIS_NOT_INIT;
    /* parameter check */
    if ( key == NULL || strlen(key) == 0 || strlen(key) > MAX_KEY_LEN )
        return LODIS_INVALID_PARAM;
    /* locate entry */
    unsigned int    offset = _lodis_hash(key,lodisIndex->numOfEntry);
    lodis_entry_t   *entry = lodisIndex->entryHead + offset;
    lodis_cargo_t   *curcargo = entry->cargo;
    lodis_cargo_t   *precargo = entry->cargo;
    /* for debug */
    _lodis_debug("begin lodis_del(%s)",key);
    _lodis_debug("entryHead address :%x" , lodisIndex->entryHead);
    _lodis_debug("entry offfset     :%u" , offset);
    _lodis_debug("entry address     :%x" , entry);
    _lodis_debug("entry numOfKeys   :%d" , entry->numOfCargo);
    _lodis_debug("curcargo address  :%x" , curcargo);
    while( curcargo ){
        /* found key */
        if ( strcmp(curcargo->key , key) == 0 ) {
            /* key time out , delete current key */
            _lodis_debug("begin to delete key[%s] found in entry[%u]" , key , offset);
            free(curcargo->value);
            precargo->next = curcargo->next;
            free(curcargo);
            /* update statistics */
            if ( --entry->numOfCargo == 0 )
                entry->cargo = NULL;
            lodisIndex->numOfKeys --;
            return LODIS_SUCCESS;
        } else {
            precargo = curcargo;
            curcargo = curcargo->next;
        }
    }
    return LODIS_KEY_NOTFOUND;
}
/*
 * FLUSH operation
 * */
int lodis_flushAll()
{
    if ( lodisIndex == NULL )  return LODIS_NOT_INIT;
    _lodis_debug("begin flush all keys");
    lodis_entry_t   *entry = NULL;
    lodis_cargo_t   *curcargo = NULL;
    lodis_cargo_t   *nexcargo = NULL;
    int i = 0;
    for ( i = 0 ; i < lodisIndex->numOfEntry ; i ++){
        entry = lodisIndex->entryHead + i;
        curcargo = entry->cargo;
        while( curcargo ){
            nexcargo = curcargo->next;
            free(curcargo->value);
            free(curcargo);
            curcargo = nexcargo;
        }
        entry->cargo = NULL;
        entry->numOfCargo = 0;
    }
    lodisIndex->numOfKeys = 0;
    return LODIS_SUCCESS;
}
/*
 * CLOSE operation
 * */
int lodis_close()
{
    if ( lodisIndex == NULL )  return LODIS_NOT_INIT;
    _lodis_debug("begin lodis_close");
    lodis_flushAll();
    free(lodisIndex->entryHead);
    free(lodisIndex);
    lodisIndex = NULL;
    return LODIS_SUCCESS;
}
/*
 * turn on debug info output
 * */
void lodis_logon()
{
    if ( lodisIndex )
        lodisIndex->logLevel = '1';
    return;
}
/*
 * turn off debug info output
 * */
void lodis_logoff()
{
    if ( lodisIndex )
        lodisIndex->logLevel = '0';
    return;
}
/*
 * hash function 
 * */
static unsigned int _lodis_hash(char *key , int size)
{
    
    _lodis_debug("begin hash(%s,%d)" , key ,size);
    unsigned int seed = 131;
    unsigned int value = 0;
    while ( *key ){
        value = value * seed + (*key++);
    }
    return (value & 0x7FFFFFFF) % size;
}
/*
 * create a new cargo struct
 * */
static lodis_cargo_t* _lodis_insert_cargo( char *key , char *value , int length , int ttl)
{
    lodis_cargo_t* cargo = (lodis_cargo_t *)malloc(sizeof(lodis_cargo_t));
    if ( cargo == NULL ){
        return NULL;
    }
    memset(cargo , 0x00 , sizeof(lodis_cargo_t));
    cargo->value = (char *)malloc(length);
    if ( cargo->value == NULL ){
        return NULL;
    }
    memset(cargo->value , 0x00 , length);
    memcpy(cargo->key , key , strlen(key));
    memcpy(cargo->value , value , length);
    cargo->next = NULL;
    cargo->lenOfValue = length;
    if ( ttl > 0 )
        cargo->ttl = time(NULL) + ttl;
    else
        cargo->ttl = 0;
    _lodis_debug("_lodis_insert_cargo finished");
    return cargo;
}
/*
 * update an existing cargo struct
 * */
static int _lodis_update_cargo( lodis_cargo_t *cargo , char *key , char *value , int length , int ttl)
{
    /* value changes */
    if ( length != cargo->lenOfValue || memcmp( cargo->value , value , length) != 0 ){
        free(cargo->value);
        cargo->value = (char *)malloc(length);
        if ( cargo->value == NULL ){
            return -1;
        }
        memcpy(cargo->value , value , length);
        cargo->lenOfValue = length;
    }
    if ( ttl > 0 )
        cargo->ttl = time(NULL) + ttl;
    _lodis_debug("_lodis_update_cargo finished");
    _lodis_debug("cargo->key      :%s",cargo->key);
    _lodis_debug("cargo->value    :%s",cargo->value);
    _lodis_debug("cargo->length   :%d",cargo->lenOfValue);
    _lodis_debug("cargo->ttl      :%u",cargo->ttl);
    return 0;
}
/*
 * debug print
 * */
static void _lodis_debug(char *fmt , ... )
{
    if ( lodisIndex->logLevel != '1' )  return;
    va_list         ap;
    time_t          t;
    struct tm       *timeinfo;
    struct timeval  tv_now;
	char            message[1000];
    int             sz = 0;

	memset(message , 0x00 , sizeof(message));
    gettimeofday(&tv_now , NULL);
	time(&t);
	timeinfo = localtime(&t);
	sz = sprintf(message , \
			"[%02d:%02d:%02d:%06d][PID<%-8d>]" ,\
			timeinfo->tm_hour , \
			timeinfo->tm_min, \
			timeinfo->tm_sec, \
			tv_now.tv_usec, \
			getpid());
	va_start(ap , fmt);
	vsnprintf(message + sz , MAX_LOG_LEN - sz , fmt , ap);
	va_end(ap);

    printf( "%s\n" , message);
    return;
}
