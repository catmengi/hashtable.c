#define ALLOC_ERR 2
#define ARG_ERR 1
#define ENOTFOUND 4
#define ENOSPACE 3
#include <sys/types.h>
#include <stdint.h>
typedef struct hashtable* hashtable_t;
int hashtable_create(hashtable_t* ht,size_t size,size_t coll_pbuck);                             /*create new hashtable with size and collisions per bucket and pass it to hashtable_t pointer*/         


int hashtable_add(hashtable_t ht,char* key,uint32_t keylen,void* vptr,uint64_t meta);            /*add new entry to hashtable*/


int hashtable_rehash(hashtable_t ht,uint64_t size);                                              /*change size of hashtable*/


int hashtable_get(hashtable_t ht, char* key, uint32_t keylen, void** out);                      //
                                                                                                //   get data/key from entry and pass pointer to it into out;
                                                                                                //   return error code
                                                                                                //
int hashtable_get_key(hashtable_t ht, char* key, uint32_t keylen, char** out);                                      

int hashtable_remove_entry(hashtable_t ht,char* key,uint32_t keylen);                                                       /*remove entry from hashtable dont affect data*/

void hashtable_free(hashtable_t ht);                                                                                        /*remove hashtable and free all keys, dont affect data*/

int hashtable_iterate(                                                                                                      /*iterate all hashtable items. If cmp returns 1 do_some is activated*/
                      hashtable_t ht,                                                                                       /*usrc and lenc(len of usr*) passed to cmp and usrd and lend passed to do_some*/
                      int (*cmp)(char* key,uint64_t keylen, void* data, uint64_t meta,void* usr, uint64_t len),
                      void (*do_some)(char* key,uint64_t keylen, void* data, uint64_t meta,void* usr, uint64_t len),
                      void* usrc,uint64_t lenc,void* usrd, uint64_t lend
                     );
