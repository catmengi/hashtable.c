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
    ht_ent_llist** ent_llists;    //ent_llist store pointer to ht_ent_llist struct containing keys and data, used to retriev data from bucket
    uint64_t size;                //max ammount of ent_llist, internal usage
    uint64_t used;                //ammount of current used ent_llists, internal usage
}ht_bucket;

typedef struct ht_ent_llist
{
    ht_cont_llist* parent;         //pointer to parent ht_cont_llist struct internal usage
    ht_ent_llist* next;            //pointer to next key internal usage
    ht_ent_llist* prev;            // pointer to previus key, internal usage
    char* key;                     //key for acces, normal null terminated string
    uint64_t key_len;              //unused currently
    void* vptr;                    //data of key
}ht_ent_llist;
typedef struct ht_cont_llist
{
    uint64_t hash;
    ht_ent_llist* ent_llist;
    ht_cont_llist* next;
    ht_cont_llist* prev;
}ht_cont_llist;


/*functions for hashtable manipulations*/

/*int hashtable_rehash(hashtable* ht); internal used function!*/
hashtable* hashtable_new();                              //create new hashtable
int hashtable_add(hashtable* ht, char* key, void* vptr); //add key with void* as data, if key exist this will change his data
void* hashtable_get(hashtable* ht, const char* key);     //get key's data, if not exist or some error return NULL
int hashtable_free(hashtable* ht);                       //free hashtable, dont affect data
int hashtable_remove(hashtable* ht, char* key);          //remove key from hashtable
char* strdup_s(const char* str);                         //strdup but it checking return value of malloc
