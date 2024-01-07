# hashtable.c
hashtable realization using two different collision handling methods
error codes:

  -return 0 : no error
  
  -return 1 : general error (eg null argument)
  
  -return 2 : not found
  
  -return 3 : malloc errors
  
  -return NULL : error
  
  hashtable* hashtable_new();                              //create new hashtable
  
 int hashtable_add(hashtable* ht, char* key, void* vptr); //add key with void* as data, if key exist this will change his data
 
 void* hashtable_get(hashtable* ht, const char* key);     //get key's data, if not exist or some error return NULL
 
 int hashtable_free(hashtable* ht);                       //free hashtable, dont affect data
 
 int hashtable_remove(hashtable* ht, char* key);          //remove key from hashtable
 
