# hashtable.c
hashtable realization using two different collision handling methods

**error codes:**

   666 unknown code error;
   
   ENOTFOUND(int 4) not found

   ALLOC_ERR(int 2) malloc failed

   ENOSPACE(int 3) hastable_rehash cannot fit in given sizes (data in hashtable is UNSTABLE AND CANNOT WORK PROPERLY rehash until succeful rehash)

   ARG_ERR(int 1)  some of args that cant accept NULL is NULL
 
