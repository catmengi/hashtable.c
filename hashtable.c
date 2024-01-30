#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#define DEFAULT_SIZE  32
#define MAX_ENT_PER_BUCKET 16    //lower values can easily enter infinity rehash state and very big memory consumption
char* strdup_s(const char* str)
{
    if(!str)
        return NULL;
    char* nstr = malloc(strlen(str)+1);
    if(!nstr)
        return NULL;
    strcpy(nstr,str);
    return nstr;
}

typedef struct hashtable hashtable;
typedef struct ht_cont_llist ht_cont_llist;
typedef struct ht_ent_llist ht_ent_llist;
typedef struct ht_bucket ht_bucket;

typedef struct hashtable
{
    ht_bucket* bucket;                //bucket array
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


int hashtable_rehash(hashtable* ht);
hashtable* hashtable_new();
int hashtable_coll_conf(hashtable* ht, uint64_t coll_per_buck);
int hashtable_add(hashtable* ht, char* key, void* vptr);
void* hashtable_get(hashtable* ht, const char* key);
int hashtable_free(hashtable* ht);
int _ht_cont_ll_remove(hashtable* ht, ht_cont_llist* ll_p);
int hashtable_remove(hashtable* ht, char* key);
char* strdup_s(const char* str);

uint64_t hash_func ( const char * key)
{
  uint64_t h = (525201411107845655ull);
  for (;*key;++key)
  {
    h ^= *key;
    h *= 0x5bd1e9955bd1e995;
    h ^= h >> 47;
  }
  return h;
}

hashtable* hashtable_new()
{
    hashtable* ht = calloc(1,sizeof(*ht));
    if(!ht)
        return NULL;
    ht->bucket = calloc(DEFAULT_SIZE,sizeof(*ht->bucket));
    if(!ht->bucket)
        return NULL;
    ht->size = DEFAULT_SIZE;
    ht->free_buckets = DEFAULT_SIZE;
    ht->cont_llist_head = calloc(1,sizeof(*ht->cont_llist_head));
    if(!ht->cont_llist_head)
        return NULL;
    ht->cont_llist_head->prev = NULL;
    ht->cont_llist_head->ent_llist = NULL;
    ht->cont_llist_tail = ht->cont_llist_head;
    for(int i = 0; i < DEFAULT_SIZE; i++)
    {
        ht->bucket[i].ent_llists = calloc(MAX_ENT_PER_BUCKET,sizeof(*ht->bucket[i].ent_llists));
        if(!ht->bucket[i].ent_llists)
            return NULL;
        ht->bucket[i].size = MAX_ENT_PER_BUCKET;
        ht->bucket[i].used = 0;
    }
    ht->coll_per_buck = MAX_ENT_PER_BUCKET;
    return ht;
}
int hashtable_coll_conf(hashtable* ht, uint64_t coll_per_buck)
{
    if(!ht)
        return 1;
    if(coll_per_buck < 8)
        coll_per_buck = 8;
    uint64_t old_coll = ht->coll_per_buck;
    ht->coll_per_buck = coll_per_buck;
    if(hashtable_rehash(ht) != 0)
    {
        ht->coll_per_buck = old_coll;
        if(hashtable_rehash(ht) != 0)
            return 1;
    }
    return 0;
}
uint64_t _ht_get_cont_len(hashtable* ht)
{
    if(!ht)
        return 0;
    uint64_t len = 0;
    ht_cont_llist* tmp_l = ht->cont_llist_head;
    while(tmp_l != NULL)
    {
        len++;
        tmp_l = tmp_l->next;
    }
    return len;
}
int hashtable_rehash(hashtable* ht)
{
    if(!ht)
        return 1;
    ht_cont_llist* tmp_l = ht->cont_llist_head;
    uint64_t old_size = ht->size;
    uint64_t len = _ht_get_cont_len(ht);
    ht->size += len;
    ht->free_buckets += len;
    for(int i = 0; i < old_size;i++)
    {
       free(ht->bucket[i].ent_llists);
    }
    free(ht->bucket);
    ht->bucket = calloc(ht->size,sizeof(*ht->bucket));
    if(!ht->bucket)
        return 1;
    for(int i = 0; i < ht->size; i++)
    {
        ht->bucket[i].ent_llists = calloc(ht->coll_per_buck,sizeof(*ht->bucket[i].ent_llists));
        if(!ht->bucket[i].ent_llists)
            return 1;
        ht->bucket[i].size = ht->coll_per_buck;
        ht->bucket[i].used = 0;
    }
    while(tmp_l != NULL)
    {
WRITE:
        //printf(",%p\n",tmp_l);
        uint64_t index = tmp_l->hash % ht->size;
        if(ht->bucket[index].used < (ht->bucket[index].size))
        {
            ht->bucket[index].ent_llists[ht->bucket[index].used] = tmp_l->ent_llist;
            ht->bucket[index].used++;
        }
        else
        {
            if(hashtable_rehash(ht) == 0)
            {
                goto WRITE;
            }
            else
                return 1;
        }
        tmp_l = tmp_l->next;
    }
    return 0;
}
int hashtable_add(hashtable* ht, char* key, void* vptr)
{
  if(!ht)
      return 1;
  if(!key)
      return 2;
  if(ht->free_buckets == 0)
      if(hashtable_rehash(ht) != 0)
          return 1;
START:
  uint64_t hash = hash_func(key);
  uint64_t index = hash % ht->size;
  if(ht->cont_llist_tail->hash == hash)
  {
      if(ht->bucket[index].used == ht->bucket[index].size)
      {
          if(hashtable_rehash(ht) != 0)
              return 1;
          else
              goto START;
      }
      ht->bucket[index].ent_llists[ht->bucket[index].used] = ht->cont_llist_tail->ent_llist;
      ht->bucket[index].used++;
  }
  if(ht->bucket[index].used == 0)
  {
    if(ht->cont_llist_tail->hash == 0)
    {
        ht->cont_llist_tail->hash = hash;
        ht->cont_llist_tail->ent_llist = malloc(sizeof(*ht->cont_llist_tail->ent_llist));
        ht->cont_llist_tail->ent_llist->prev = NULL;
        if(!ht->cont_llist_tail->ent_llist)
            return 3;
        ht->cont_llist_tail->ent_llist->key = strdup_s(key);
        if(!ht->cont_llist_tail->ent_llist->key)
            return 3;
        ht->cont_llist_tail->ent_llist->key_len = strlen(key);
        ht->cont_llist_tail->ent_llist->vptr = vptr;
        ht->cont_llist_tail->ent_llist->next = NULL;
        ht->cont_llist_tail->ent_llist->parent = ht->cont_llist_tail;
        ht->cont_llist_tail->next = NULL;
        ht->cont_llist_tail->prev = NULL;
        ht->bucket[index].ent_llists[ht->bucket[index].used] = ht->cont_llist_tail->ent_llist;
        ht->bucket[index].used++;
        ht->free_buckets--;
        return 0;
    }
    if(ht->cont_llist_tail->hash != 0)
    {
     //   printf("new ll");
        ht->cont_llist_tail->next = malloc(sizeof(*ht->cont_llist_tail->next));
        if(!ht->cont_llist_tail->next)
            return 3;
        ht->cont_llist_tail->next->prev = ht->cont_llist_tail;
        ht->cont_llist_tail = ht->cont_llist_tail->next;
        ht->cont_llist_tail->hash = hash;
        ht->cont_llist_tail->next = NULL;
        ht->cont_llist_tail->ent_llist = malloc(sizeof(*ht->cont_llist_tail->ent_llist));
        if(!ht->cont_llist_tail->ent_llist)
            return 3;
        ht->cont_llist_tail->ent_llist->key = strdup_s(key);
        if(!ht->cont_llist_tail->ent_llist->key)
            return 3;
        ht->cont_llist_tail->ent_llist->key_len = strlen(key);
        ht->cont_llist_tail->ent_llist->vptr = vptr;
        ht->cont_llist_tail->ent_llist->parent = ht->cont_llist_tail;
        ht->cont_llist_tail->ent_llist->prev = NULL;
        ht->cont_llist_tail->ent_llist->next = NULL;
        ht->bucket[index].ent_llists[ht->bucket[index].used] = ht->cont_llist_tail->ent_llist;
        ht->bucket[index].used++;
        ht->free_buckets--;
        return 0;
    }
  }
  if(ht->bucket[index].used != 0)
  {
    ht_cont_llist* tmp_l = NULL;
    for(int i = 0; i < ht->bucket[index].used; i++)
    {
      if(ht->bucket[index].ent_llists[i] != NULL)
        if(ht->bucket[index].ent_llists[i]->parent->hash == hash)
            tmp_l = ht->bucket[index].ent_llists[i]->parent;
    }
    if(tmp_l)
    {
        ht_ent_llist* tmp_e = tmp_l->ent_llist;
        if(!tmp_e)
            return 1;
        while(tmp_e->next != NULL && strcmp(tmp_e->key,key) != 0)
            tmp_e = tmp_e->next;
        if(strcmp(tmp_e->key,key) == 0)
        {
            tmp_e->vptr = vptr;
            return 0;
        }
        tmp_e->next = malloc(sizeof(*tmp_e->next));
        if(!tmp_e->next)
            return 3;
        tmp_e->next->prev = tmp_e;
        tmp_e = tmp_e->next;
        tmp_e->parent = tmp_l;
        tmp_e->key = strdup_s(key);
        tmp_e->key_len = strlen(key);
        tmp_e->vptr = vptr;
        tmp_e->next = NULL;
    }
    if(!tmp_l)
    {
      //printf("%p\n",ht->cont_llist_tail);
      ht->cont_llist_tail->next = malloc(sizeof(*ht->cont_llist_tail));
      if(!ht->cont_llist_tail->next)
          return 3;
      ht->cont_llist_tail->next->prev = ht->cont_llist_tail;
      ht->cont_llist_tail = ht->cont_llist_tail->next;
      ht->cont_llist_tail->ent_llist = malloc(sizeof(*ht->cont_llist_tail->ent_llist));
      if(!ht->cont_llist_tail->ent_llist)
          return 3;
      ht->cont_llist_tail->hash = hash;
      ht->cont_llist_tail->ent_llist->key = strdup_s(key);
      ht->cont_llist_tail->ent_llist->parent = ht->cont_llist_tail;
      ht->cont_llist_tail->ent_llist->vptr = vptr;
      ht->cont_llist_tail->ent_llist->key_len = strlen(key);
      ht->cont_llist_tail->ent_llist->next = NULL;
      ht->cont_llist_tail->ent_llist->prev = NULL;
      ht->cont_llist_tail->next = NULL;
      if(ht->bucket[index].used == ht->bucket[index].size)
      {
          if(hashtable_rehash(ht) != 0)
              return 1;
          else
              goto START;
      }
      ht->bucket[index].ent_llists[ht->bucket[index].used] = ht->cont_llist_tail->ent_llist;
      ht->bucket[index].used++;
      return 0;
    }

  }
  return 1;
}

void* hashtable_get(hashtable* ht, const char* key)
{
    if(!ht)
    {
        return NULL;
    }
    if(!key)
    {
        return NULL;
    }
    uint64_t hash = hash_func(key);
    uint64_t index = hash % ht->size;
    if(ht->bucket[index].used == 0)
    {
        return NULL;
    }
    if(ht->bucket[index].used == 1)
    {
        if(ht->bucket[index].ent_llists[0]->parent->hash == hash)
        {
            if(ht->bucket[index].ent_llists[0]->key != NULL)
            {
                if(strcmp(ht->bucket[index].ent_llists[0]->key,key) == 0)
                    return ht->bucket[index].ent_llists[0]->vptr;
                else
                {
                    return NULL;
                }
            }
            else
                return NULL;
        }
        return NULL;
    }
    if(ht->bucket[index].used > 1)
    {
        ht_cont_llist* tmp_l = NULL;
        for(int i = 0; i < ht->bucket[index].used; i++)
        {
          if(ht->bucket[index].ent_llists[i] != NULL)
            if(ht->bucket[index].ent_llists[i]->parent->hash == hash)
                tmp_l = ht->bucket[index].ent_llists[i]->parent;
        }
        if(tmp_l == NULL)
        {
            return NULL;
        }
        if(tmp_l != NULL)
        {
            ht_ent_llist* tmp_e = tmp_l->ent_llist;
            while(tmp_e->next != NULL && strcmp(tmp_e->key,key) != 0)
                tmp_e = tmp_e->next;
            if(tmp_e->key == NULL)
                return NULL;
            if(strcmp(tmp_e->key,key) == 0)
                return tmp_e->vptr;
            return NULL;
        }
    }
}
int hashtable_free(hashtable* ht)
{
    if(!ht)
        return 1;
    ht_cont_llist* tmp_l = ht->cont_llist_head;
    ht_ent_llist* tmp_e = NULL;
    for(int i = 0; i < ht->size; i++)
    {
        free(ht->bucket[i].ent_llists);
    }
    free(ht->bucket);
    ht_cont_llist* tmp_l_n;
    while(tmp_l != NULL)
    {
        ht_ent_llist* tmp_e_n;
        tmp_e = tmp_l->ent_llist;
        while(tmp_e != NULL)
        {
           if(tmp_e->key)
                free(tmp_e->key);
            tmp_e->parent = NULL;
            tmp_e_n = tmp_e->next;
            free(tmp_e);
            tmp_e = tmp_e_n;
        }
        tmp_l_n = tmp_l->next;
        free(tmp_l);
        tmp_l = tmp_l_n;
    }
    free(ht);
}
int _ht_cont_ll_remove(hashtable* ht, ht_cont_llist* ll_p)
{
    if(!ht)
        return 1;
    if(ll_p)
        return 1;
    if(ll_p->prev == NULL)
    {
        if(ll_p->next == NULL)
        {
            ll_p->hash = 0;
            ll_p->ent_llist = NULL;
            ll_p->next = NULL;
            return 0;
        }
        if(ll_p->next != NULL)
        {
            ht->cont_llist_tail = ll_p->next;
            ht->cont_llist_head = ll_p->next;
            ll_p->ent_llist = NULL;
            ll_p->hash = 0;
            return 0;
        }
    }
    if(ll_p->prev != NULL)
    {
        if(ll_p->next == NULL)
        {
                ll_p->prev->next = NULL;
                ht->cont_llist_tail = ll_p->prev;
                ll_p->ent_llist = NULL;
                ll_p->hash = 0;
                ll_p->prev = NULL;
                ll_p->next = NULL;
                free(ll_p);
                return 0;
        }
        if(ll_p->next != NULL)
        {
            ll_p->prev->next = ll_p->next;
            ll_p->next->prev = ll_p->prev;
            ll_p->next = NULL;
            ll_p->prev = NULL;
            ll_p->hash = 0;
            ll_p->ent_llist = NULL;
            free(ll_p);
            return 0;
        }
    }
    return 1;
}
int hashtable_remove(hashtable* ht, char* key)
{
    if(!ht)
        return 1;
    uint64_t hash = hash_func(key);
    uint64_t index = hash%ht->size;
    ht_ent_llist* tmp_e = NULL;
    ht_cont_llist* tmp_l = NULL;
    if(ht->bucket[index].used == 0)
        return 2;
    if(ht->bucket[index].used != 0 )
    {
        int i = 0;
        for(i = 0; i < ht->bucket[index].used; i++)
        {
          if(ht->bucket[index].ent_llists[i] != NULL)
          {
            if(ht->bucket[index].ent_llists[i]->parent->hash == hash)
            {
                tmp_l = ht->bucket[index].ent_llists[i]->parent;
                tmp_e = ht->bucket[index].ent_llists[i];
                if(ht->bucket[index].used - 1 == i)
                    ht->bucket[index].used--;
                break;
            }
          }
        }
        int nu = 0;
        for(int i = 0; i < ht->bucket[index].used;i++)
        {
            if(ht->bucket[index].ent_llists[i] == NULL)
                nu++;
        }
        if(nu == (ht->bucket[index].used - 1))
        {
            ht->bucket[index].used = 0;
            ht->free_buckets++;
        }
        if(tmp_l)
        {
                if(tmp_e->next == NULL)
                {
                    free(tmp_e->key);
                    //free(tmp_e);
                    tmp_e->key = NULL;
                    tmp_e->vptr = NULL;
                    _ht_cont_ll_remove(ht,tmp_l);
                    ht->bucket[index].ent_llists[i] = NULL;
                    if(ht->bucket[index].used == i+1 && ht->bucket[index].used >= 1)
                        ht->bucket[index].used--;
                    return 0;
                }
                if(tmp_e->next != NULL)
                {
                    while(tmp_e->next != NULL && strcmp(tmp_e->key,key) != 0)
                    {
                        tmp_e = tmp_e->next;
                    }
                    if(strcmp(tmp_e->key,key) == 0)
                    {
                        if(tmp_e->next != NULL)
                        {
                            if(tmp_e->prev != NULL)
                            {
                                tmp_e->prev->next = tmp_e->next;
                                tmp_e->next->prev = tmp_e->prev;
                            }
                            if(tmp_e->prev == NULL)
                            {
                                tmp_l->ent_llist = tmp_e->next;
                                ht->bucket[index].ent_llists[i] = tmp_l->ent_llist;
                                tmp_e->next->prev = NULL;
                            }
                        }
                        if(tmp_e->next == NULL)
                        {
                           if(tmp_e->prev != NULL)
                           {
                                tmp_e->prev->next = NULL;
                           }
                           if(tmp_e->prev == NULL)
                           {
                                _ht_cont_ll_remove(ht,tmp_l);
                                ht->bucket[index].ent_llists[i] = NULL;
                                if(ht->bucket[index].used == i+1)
                                    ht->bucket[index].used--;
                           }
                        }
                        free(tmp_e->key);
                        tmp_e->key = NULL;
                        tmp_e->vptr = NULL;
                        free(tmp_e);
                        tmp_e = NULL;
                        return 0;
                    }
                }
        }
        if(!tmp_l)
            return 2;
    }
}
int main(void){
    FILE* fp = fopen("words.txt","r");
    if(!fp)
    {
        perror("");
        exit(-1);
    }
    char buffer[512];
    hashtable* ht = hashtable_new();
    while(fgets(buffer,512,fp)){
        hashtable_add(ht,buffer,strdup(buffer));
    }
    fseek(fp,0,SEEK_SET);
    while(fgets(buffer,512,fp)){
    hashtable_get(ht,buffer);
    }
}
