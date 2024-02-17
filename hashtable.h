#define ALLOC_ERR 2
#define ARG_ERR 1
#define ENOTFOUND 4
#define ENOSPACE 3
#include <sys/types.h>
#include <stdint.h>
typedef struct hashtable* hashtable_t;
int hashtable_create(hashtable_t* ht,size_t size,size_t coll_pbuck);
int hashtable_rehash(hashtable_t ht,uint64_t size);
int hashtable_add(hashtable_t ht,char* key,uint32_t keylen,void* vptr,uint64_t meta);
int hashtable_rehash(hashtable_t ht,uint64_t size);
int hashtable_get(hashtable_t ht, char* key, uint32_t keylen, void** out);
int hashtable_get_key(hashtable_t ht, char* key, uint32_t keylen, char** out);
int hashtable_remove_entry(hashtable_t ht,char* key,uint32_t keylen);
void hashtable_free(hashtable_t ht);
int hashtable_iterate(
                      hashtable_t ht,
                      int (*cmp)(char* key,uint64_t keylen, void* data, uint64_t meta),
                      void (*do_some)(char* key,uint64_t keylen, void* data, uint64_t meta,void* usr, uint64_t len),
                      void* usr,uint64_t len
                     );
