/*
 Include Headers
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "trie.h"
#include <pthread.h>

#define SIZE 26                                     //Number of child nodes
#define INDEX(c) ((int)c - (int)'a')                //Macro to calculate index of an alphabet

//Initialize Trie
trie_t init_trie(void){

  trie_t trie;
  trie = malloc(sizeof(_trie_t));                   //Malloc trie structure           
  
  if(trie == NULL)                                  //If trie allocation failed, then return
  {
    printf("Cannot create trie\n");
    return NULL;
  }

  trie->head = malloc(sizeof(_trie_node_t));        //Malloc head node of trie
  
  if(trie->head == NULL)                            //If head allocation failed, then return
  {
    printf("Cannot create head\n");
    free(trie);                                     //Free trie
    return NULL;                                    
  }

  trie->head->is_end = false;                       //Set is_end for head to be false

  for(int i = 0;i < SIZE; i++)                      //Set all child nodes of head to be NULL
  {
    trie->head->children[i] = NULL;
  }

  #ifndef _NO_HOH_LOCK_TRIE                         //Initialize HOH lock
    pthread_mutex_init(&trie->head->node_lock,NULL);
    // printf("HOH Lock Initialized\n");
  #endif

  #ifdef _NO_HOH_LOCK_TRIE                          //Initialize Single lock
    #ifdef _S_LOCK_TRIE
      pthread_mutex_init(&trie->s_lock,NULL);
      // printf("Single Lock Initialized\n");
    #endif
  #endif

  #ifndef _S_LOCK_TRIE                              //Initialize RW lock
    #ifdef _NO_HOH_LOCK_TRIE
      pthread_rwlock_init(&trie->rw_lock,NULL);
      // printf("RW Lock Initialized\n");
    #endif
  #endif

return trie;                                        //Return trie after allocation
}

//Create node helper function for Insert
struct node* create_node()
{
  struct node* node = malloc(sizeof(_trie_node_t)); //Malloc new node

  if(node == NULL)                                  //If node cannot be created, return
  {
    printf("Cannot create node\n");
    return node;
  }
  node->is_end = false;                             //Set is_end for node to be false

  for(int i = 0;i < SIZE; i++)                      //Initialize child nodes to be NULL
  {
    node->children[i] = NULL;
  }

  #ifndef _NO_HOH_LOCK_TRIE                         //Initialize HOH node lock if defined
  pthread_mutex_init(&node->node_lock,NULL);
  #endif

return node;                                        //Return new node
}

/* 
    Insert function puts the given key,value pair in the trie.
    It overwrites the value if the key already exists.
*/
void insert(trie_t trie, char* key, int value){

  #ifdef _S_LOCK_TRIE                               //Acquire Single Lock if defined
    #ifdef _NO_HOH_LOCK_TRIE
      pthread_mutex_lock(&trie->s_lock);
    #endif
  #endif

  #ifndef _S_LOCK_TRIE                              //Acquire RW Lock if defined
    #ifdef _NO_HOH_LOCK_TRIE
      pthread_rwlock_wrlock(&trie->rw_lock);
    #endif
  #endif

  struct node* root = trie->head;

  #ifndef _NO_HOH_LOCK_TRIE                         //Acquire HOH node Lock if defined
    pthread_mutex_lock(&root->node_lock);
    struct node* prev = root;                       //Set a previous node to trie->head
  #endif

  /*
    Insert each letter of the "key" given into the trie 
    alongwith the value specified.
    The loop is run for the length of "key" times.
  */
  for(int i = 0; i < strlen(key); i++)              
  {
    /*
      Find the index of the current letter in key using 
      the Macro.
    */
    int index = INDEX(key[i]);                      

    /*
      If the node doesn't have a child for the given index
      create a new node for that index.
    */
    if(root->children[index] == NULL){               
      root->children[index] = create_node();
    }

    #ifndef _NO_HOH_LOCK_TRIE                       //Set previous node to current node for HOH lock case
      prev = root;
    #endif

    root = root->children[index];                   //Traverse down the trie to insert next key,value

    #ifndef _NO_HOH_LOCK_TRIE                       //Acquire the next lock and release the lock of previous node
      pthread_mutex_lock(&root->node_lock);
      pthread_mutex_unlock(&prev->node_lock);
    #endif
  }

  /*
    Set the value of new node and mark is_end to be true.
  */
  root->value = value;                        
  root->is_end = true;

  #ifndef _NO_HOH_LOCK_TRIE                           //Release HOH node lock if defined
    pthread_mutex_unlock(&root->node_lock);
  #endif

  #ifdef _S_LOCK_TRIE                                 //Release Single lock if defined
    #ifdef _NO_HOH_LOCK_TRIE
      pthread_mutex_unlock(&trie->s_lock);
    #endif
  #endif

  #ifndef _S_LOCK_TRIE                                 //Release RW lock if defined
    #ifdef _NO_HOH_LOCK_TRIE
      pthread_rwlock_unlock(&trie->rw_lock);
    #endif
  #endif

}

/* 
    Find function returns -1 if the key is not found. 
    Otherwise, it returns 0 and puts the value in the variable pointed to by val_ptr.
*/
int find(trie_t trie,char* key, int* val_ptr){

  if(trie->head == NULL)                                //Return if the head of trie is NULL
  {
    return -1;
  }

  #ifdef _S_LOCK_TRIE                                   //Acquire Single lock lock if defined
    #ifdef _NO_HOH_LOCK_TRIE
      pthread_mutex_lock(&trie->s_lock);
    #endif
  #endif

  #ifndef _S_LOCK_TRIE                                   //Acquire RW lock lock if defined
    #ifdef _NO_HOH_LOCK_TRIE
      pthread_rwlock_rdlock(&trie->rw_lock);
    #endif
  #endif

  trie_node_t root = trie->head;

  #ifndef _NO_HOH_LOCK_TRIE                              //Acquire HOH node lock lock if defined
    pthread_mutex_lock(&root->node_lock);
    trie_node_t prev = root;
  #endif

  int len = strlen(key);
  int index;

  /*
    Run the loop for length of key times.
    If found return 0 and its value pointed else 
    return -1.
  */
  for(int i = 0; i < len; i++)
  {
    /*
      Find the index of the current letter in key using 
      the Macro.
    */
    index = INDEX(key[i]);

    if(root->children[index] != NULL)                   //If the child is present for th index
    {
      #ifndef _NO_HOH_LOCK_TRIE                         //Set previous node to current in case of HOH lcoking
        prev = root;
      #endif

      root = root->children[index];                     //Traverse down the trie

      #ifndef _NO_HOH_LOCK_TRIE                       //Acquire the next lock and release the lock of previous node
        pthread_mutex_lock(&root->node_lock);
        pthread_mutex_unlock(&prev->node_lock);
      #endif
    }
    else                                              //If no child is present for given index, return -1
    {
      #ifdef _S_LOCK_TRIE                             //Release Single lock if defined
        #ifdef _NO_HOH_LOCK_TRIE
          pthread_mutex_unlock(&trie->s_lock);
        #endif
      #endif

      #ifndef _S_LOCK_TRIE                             //Release RW lock if defined
        #ifdef _NO_HOH_LOCK_TRIE
          pthread_rwlock_unlock(&trie->rw_lock);
        #endif
      #endif

      #ifndef _NO_HOH_LOCK_TRIE                        //Release HOH node lock if defined
        pthread_mutex_unlock(&root->node_lock);
      #endif

      return -1;
      }
  }

  /*
    If the key if found, return 0 and store
    the value pointed by it in val_ptr.
  */
  if(root != NULL && root->is_end)
  {
    *val_ptr = root->value;

    #ifdef _S_LOCK_TRIE                             //Release Single lock if defined
      #ifdef _NO_HOH_LOCK_TRIE
        pthread_mutex_unlock(&trie->s_lock);
      #endif
    #endif

    #ifndef _S_LOCK_TRIE                             //Release RW lock if defined
      #ifdef _NO_HOH_LOCK_TRIE
        pthread_rwlock_unlock(&trie->rw_lock);
      #endif
    #endif

    #ifndef _NO_HOH_LOCK_TRIE                         //Release HOH node lock if defined
      pthread_mutex_unlock(&root->node_lock);
    #endif

    return 0;
  }

  #ifdef _S_LOCK_TRIE                             //Release Single lock if defined
    #ifdef _NO_HOH_LOCK_TRIE
      pthread_mutex_unlock(&trie->s_lock);
    #endif
  #endif

  #ifndef _S_LOCK_TRIE                             //Release RW lock if defined
    #ifdef _NO_HOH_LOCK_TRIE
      pthread_rwlock_unlock(&trie->rw_lock);
    #endif
  #endif

  #ifndef _NO_HOH_LOCK_TRIE                        //Release HOH node lock if defined
    pthread_mutex_unlock(&root->node_lock);
  #endif

return -1;
}

/*
    Helper function to find if a node has
    no children. 
    Return 1 if it has any child, else 0.
*/
int isEmpty(trie_node_t root)
{
  for(int i = 0;i < SIZE; i++)
  {
    if(root->children[i])
      return 0;
  }
  return 1;
}

/*
    Helper function to delete key, value pair 
    from trie.
    In case of HOH locking:
      The function recurses down by locking the path to be 
      deleted and releases them while moving bottom up after
      deleting the required node.
*/
trie_node_t delete_kv_helper(trie_node_t root, char *key)
{
  if(root == NULL){                                   //If current node is NULL, return 
    return NULL;
  }

  #ifndef _NO_HOH_LOCK_TRIE                           //Acquire HOH node lock if defined
	  pthread_mutex_lock(&root->node_lock); 
	#endif

  /* 
    Recurse down the trie if "key" is still present.
    Pass the pointer to next letter of key while recursing.
  */
  if(*key)
  {
    int index = INDEX(*key);
    root->children[index] = delete_kv_helper(root->children[index], key+1);
       
    /*
      If the current node is a leaf node and has is_end false,
      release the HOH node lock and destroy it.
      Free the node and set it to NULL.
    */
    if(isEmpty(root) && root->is_end == 0){
      #ifndef _NO_HOH_LOCK_TRIE
        pthread_mutex_unlock(&root->node_lock);
        pthread_mutex_destroy(&root->node_lock);
      #endif 

      free(root);
      root = NULL;

      return root;
    }

    #ifndef _NO_HOH_LOCK_TRIE                             //Release HOH node lock if defined
      pthread_mutex_unlock(&root->node_lock);
    #endif 

    return root;
  }

  /*
    If "key" is not present, need to delete this node.
    Set is_end to false if a non-leaf node.
    Else delete the node.
  */
  if(*key == '\0')
  {
    root->is_end = false;

    /*
      If the node is a leaf node, release HOH node lock
      and destroy it.
      Free the node and set it to NULL.
    */
    if(isEmpty(root))
    {
      #ifndef _NO_HOH_LOCK_TRIE
        pthread_mutex_unlock(&root->node_lock);
        pthread_mutex_destroy(&root->node_lock);
      #endif

      free(root);
      root = NULL;

      return root;
    }

    #ifndef _NO_HOH_LOCK_TRIE                             //Release HOH node lock if defined
      pthread_mutex_unlock(&root->node_lock);
    #endif

    return root;
  }
  
return root;
}

/*
    Delete_kv function deletes the key and its value from 
    the trie if it is present.
*/
void delete_kv(trie_t trie, char* key){

  #ifdef _S_LOCK_TRIE                                 //Acquire Single lock if defined
    #ifdef _NO_HOH_LOCK_TRIE
      pthread_mutex_lock(&trie->s_lock);
    #endif
  #endif

  #ifndef _S_LOCK_TRIE                                 //Acquire RW lock if defined
    #ifdef _NO_HOH_LOCK_TRIE
      pthread_rwlock_wrlock(&trie->rw_lock);
    #endif
  #endif

  #ifndef _NO_HOH_LOCK_TRIE                            //Acquire HOH node lock if defined
    pthread_mutex_lock(&trie->head->node_lock);
  #endif

  struct node* node = trie->head;

  /*
    Find the index of the current letter in key using 
    the Macro.
    Send the respective path to helper to delete the key, value.
  */
  for(int i = 0; i < SIZE; i++){
    if(i == INDEX(*key)){
      if(node->children[i] != NULL){
        node->children[i] = delete_kv_helper(node->children[i], key+1);
      }
    } 
  }

  #ifdef _S_LOCK_TRIE                             //Release Single lock if defined
    #ifdef _NO_HOH_LOCK_TRIE
      pthread_mutex_unlock(&trie->s_lock);
    #endif
  #endif

  #ifndef _S_LOCK_TRIE                             //Release RW lock if defined
    #ifdef _NO_HOH_LOCK_TRIE
      pthread_rwlock_unlock(&trie->rw_lock);
    #endif
  #endif

  #ifndef _NO_HOH_LOCK_TRIE                         //Release HOH node lock if defined
    pthread_mutex_unlock(&trie->head->node_lock);
  #endif

return;
}

/*
    Helper function to get the pointer to next character
    in the passed string, to get the prefix string.
*/
char* get_prefix(char* key, int i)
{
  char *str = malloc(sizeof(char)*(strlen(key) + 4));
  char part[2];
  part[0] = (char)(i+97);
  part[1] = '\0';
  strcpy(str,key);
  strcat(str,part);

  return str;
}

/*
    Traverse Trie Helper function for keys_with_prefix
*/
void traverse(trie_node_t root, char* key, char **list, int* count)
{
  /*
    If node has is_end set to true, then insert this key into the prefix
    list.
    Increase the count by one.
  */
  if(root->is_end)
  {
    list[*count] = malloc((strlen(key)+2)*sizeof(char));
    strcpy(list[*count],key);
    (*count)++;
  }

  /*
    Return if the node is a leaf node.
  */
  if(isEmpty(root))
  {
    return;
  }

  /*
    Recurse in all the children of the given node, to find prefixes.
  */
  for(int i = 0; i < SIZE; i++)
  {
    if(root->children[i] != NULL)
    {
      //Get the new prefix string 
      char * str = get_prefix(key, i);
      
      /*
        Acquire the HOH node lock of the child along the path
      */
      if(root != NULL && root->children[i] != NULL)
      {
        #ifndef _NO_HOH_LOCK_TRIE
          pthread_mutex_lock(&root->children[i]->node_lock);
        #endif
      }
      
      //Traverse down the trie
      traverse(root->children[i], str , list, count);

      /*
        Release the HOH node lock of child
      */
      if(root != NULL && root->children[i] != NULL)
      {
        #ifndef _NO_HOH_LOCK_TRIE
          pthread_mutex_unlock(&root->children[i]->node_lock);
        #endif
      }

      //Free the malloc'd prefix string
      free(str);
    }
  }
}

/*
    Keys_with_prefix function returns an array of strings with the given prefix.
    Last element of the array is NULL.
    If no key matches the prefix, the array has a single NULL value.
*/
char** keys_with_prefix(trie_t trie, char* prefix){

  /*
    Initialize the list and the count of entries in list
  */
  int *count = malloc(sizeof(int));
  *count = 0;

  char** list = malloc(2048*sizeof(char*));

  //If list cannot be initialized, return
  if(list == NULL)
  {
    printf("Cannot initialize list\n");
    free(count);
    return list;
  }
  
  //Set first index of list to NULL
  list[*count] = NULL;

  #ifdef _S_LOCK_TRIE                           //Acquire Single lock if defined
    #ifdef _NO_HOH_LOCK_TRIE
      pthread_mutex_lock(&trie->s_lock);
    #endif
  #endif

  #ifndef _S_LOCK_TRIE                           //Acquire RW lock if defined
    #ifdef _NO_HOH_LOCK_TRIE
      pthread_rwlock_rdlock(&trie->rw_lock);
    #endif
  #endif

  int index;

  trie_node_t root = trie->head;

  #ifndef _NO_HOH_LOCK_TRIE                        //Acquire HOH node lock if defined
    pthread_mutex_lock(&root->node_lock);
    trie_node_t prev = root;
  #endif

  /*
    Iterate for length of the prefix string
  */
  for(int i = 0; i < strlen(prefix); i++)
  {
    /*
      Traverse down the trie if the prefix string has some 
      characters.
    */
    index = INDEX(prefix[i]);
    if(root->children[index] != NULL)
    {
      #ifndef _NO_HOH_LOCK_TRIE
        prev = root;
      #endif

      root = root->children[index];

      /*
        Acquire the child's node lock and release the parent's
        lock.
      */
      #ifndef _NO_HOH_LOCK_TRIE
        pthread_mutex_lock(&root->node_lock);
        pthread_mutex_unlock(&prev->node_lock);
      #endif
    }
    else
    {
      /*
        If the prefix is not contained in the trie, return
      */
      #ifdef _S_LOCK_TRIE                           //Release Single lock if defined
        #ifdef _NO_HOH_LOCK_TRIE
          pthread_mutex_unlock(&trie->s_lock);
        #endif
      #endif

      #ifndef _S_LOCK_TRIE                           //Release RW lock if defined
        #ifdef _NO_HOH_LOCK_TRIE
          pthread_rwlock_unlock(&trie->rw_lock);
        #endif
      #endif

      #ifndef _NO_HOH_LOCK_TRIE                       //Release HOH node lock if defined
        pthread_mutex_unlock(&root->node_lock);
      #endif

      free(count);
      return list;
    }
  }

  /*
    If the prefix string is exhausted and the current node
    is a leaf, add the prefix to the list and return.
  */
  if(root->is_end && isEmpty(root))
  {

    list[*count] = malloc((strlen(prefix)+2)*sizeof(char));

    strcpy(list[*count], prefix);
    (*count)++;
    list[*count] = NULL;

    #ifdef _S_LOCK_TRIE                           //Release Single lock if defined
      #ifdef _NO_HOH_LOCK_TRIE
        pthread_mutex_unlock(&trie->s_lock);
      #endif
    #endif

    #ifndef _S_LOCK_TRIE                           //Release RW lock if defined
      #ifdef _NO_HOH_LOCK_TRIE
        pthread_rwlock_unlock(&trie->rw_lock);
      #endif
    #endif

    #ifndef _NO_HOH_LOCK_TRIE                       //Release HOH node lock if defined
      pthread_mutex_unlock(&root->node_lock);
    #endif

    free(count);
    return list;
  }

  /*
    If the prefix string is exhausted but the current
    node is not a leaf node, recurse down the trie to find
    prefixes.
  */
  if(!isEmpty(root))
  {
    traverse(root, prefix, list, count);
    list[*count] = NULL;

    #ifdef _S_LOCK_TRIE                           //Release Single lock if defined
      #ifdef _NO_HOH_LOCK_TRIE
        pthread_mutex_unlock(&trie->s_lock);
      #endif
    #endif

    #ifndef _S_LOCK_TRIE                           //Release RW lock if defined
      #ifdef _NO_HOH_LOCK_TRIE
        pthread_rwlock_unlock(&trie->rw_lock);
      #endif
    #endif

    #ifndef _NO_HOH_LOCK_TRIE                      //Release HOH node lock if defined
      pthread_mutex_unlock(&root->node_lock);
    #endif

    free(count);
    return list;
  }
  
  #ifdef _S_LOCK_TRIE                           //Release Single lock if defined
    #ifdef _NO_HOH_LOCK_TRIE
      pthread_mutex_unlock(&trie->s_lock);
    #endif
  #endif

  #ifndef _S_LOCK_TRIE                           //Release RW lock if defined
    #ifdef _NO_HOH_LOCK_TRIE
      pthread_rwlock_unlock(&trie->rw_lock);
    #endif
  #endif

  #ifndef _NO_HOH_LOCK_TRIE                      //Release HOH node lock if defined
    pthread_mutex_unlock(&root->node_lock);
  #endif

free(count);
return list;
}

/*
  Helper function to delete trie
*/
void delete_trie_helper(trie_node_t root)
{
  //Return if the current node is NULL
  if(root == NULL)
    return;

  /* 
    Recurse for all children of current node.
  */
  for(int i = 0; i < SIZE; i++)
  { 
    if(root->children[i] != NULL)
    {
      delete_trie_helper(root->children[i]);
    }
  }
  
  /*
    If the current node is a leaf node, destroy
    its node lock and delete it.
  */
  if(isEmpty(root) || root != NULL)
  {
    #ifndef _NO_HOH_LOCK_TRIE
      pthread_mutex_destroy(&root->node_lock);
    #endif

    free(root);
    root = NULL;
    return;
  }
}

/*
    Delete Trie function clears the entire trie from memory. 
*/
void delete_trie(trie_t trie){

  //If the trie is NULL, return
  if(trie == NULL){
    return;
  }

  /*
    If the trie head is a leaf node, destroy its
    node lock and delete it.
  */
  if(isEmpty(trie->head))
  {
    #ifndef _NO_HOH_LOCK_TRIE
      pthread_mutex_destroy(&trie->head->node_lock);
    #endif

    free(trie->head);
    trie->head = NULL;

    free(trie);
    trie = NULL;
    
    return;
  }

  delete_trie_helper(trie->head);

  free(trie);
  trie = NULL;

return;
}
