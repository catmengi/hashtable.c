#include <stdint.h>

typedef struct hashtable hashtable;
typedef struct ht_cont_llist ht_cont_llist;
typedef struct ht_ent_llist ht_ent_llist;
typedef struct ht_bucket ht_bucket;

typedef struct hashtable
{
    ht_bucket* bucket; //bucket array
    ht_cont_llist* cont_llist_head;   //linked list for intermediate data storage
    ht_cont_llist* cont_llist_tail;   //tail of intermediate data storage linked list;
    uint64_t size;                    //amount of buckets
    uint64_t free_buckets;            //amount of free buckets
    uint64_t coll_per_buck;
}hashtable;

typedef struct ht_bucket
{
    ht_ent_llist** ent_llists;
    uint64_t size;
    uint64_t used;
}ht_bucket;

typedef struct ht_ent_llist
{
    ht_cont_llist* parent;
    ht_ent_llist* next;
    ht_ent_llist* prev;
    char* key;
    uint64_t key_len;
    void* vptr;
}ht_ent_llist;
typedef struct ht_cont_llist
{
    uint64_t hash;
    ht_ent_llist* ent_llist;
    ht_cont_llist* next;
    ht_cont_llist* prev;
}ht_cont_llist;


/*functions for hashtable manipulations*/

int hashtable_rehash(hashtable* ht);
hashtable* hashtable_new();
int hashtable_coll_conf(hashtable* ht, uint64_t coll_per_buck);
int hashtable_add(hashtable* ht, char* key, void* vptr);
void* hashtable_get(hashtable* ht, const char* key);
int hashtable_free(hashtable* ht);
int _ht_cont_ll_remove(hashtable* ht, ht_cont_llist* ll_p);
int hashtable_remove(hashtable* ht, char* key);
char* strdup_s(const char* str);
