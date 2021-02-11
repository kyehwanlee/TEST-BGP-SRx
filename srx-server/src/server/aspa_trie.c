
#include <stdio.h> /* printf */
#include <stdlib.h> /* exit */
#include <string.h>
#include <stdbool.h>
#include "server/aspa_trie.h"
#include "util/log.h"

static uint32_t countTrieNode =0;

// API for initialization
//
bool initializeAspaDBManager(ASPA_DBManager* aspaDBManager, Configuration* config) 
{
   aspaDBManager->tableRoot = newAspaTrie();
   aspaDBManager->countAspaObj = 0;
   aspaDBManager->config = config;
  
   if (!createRWLock(&aspaDBManager->tableLock))
   {
     RAISE_ERROR("Unable to setup the aspa object db r/w lock");
     return false;
   }

  return true;
}

// delete all db
//
static void emptyAspaDB(ASPA_DBManager* self)
{
  acquireWriteLock(&self->tableLock);
  free_trienode(self->tableRoot);
  self->tableRoot = NULL;
  self->countAspaObj = 0;
  unlockWriteLock(&self->tableLock);
}


// external api for release db
//
void releaseAspaDBManager(ASPA_DBManager* self)
{
  if (self != NULL)
  {
    releaseRWLock(&self->tableLock);
    emptyAspaDB(self);
  }
}


// generate trie node
//
static TrieNode* newAspaTrie(void) 
{
  TrieNode *rootNode = make_trienode('\0', NULL, NULL);
  return rootNode;
}


// external api for creating db object
//
ASPA_Object* newASPAObject(uint32_t cusAsn, uint16_t pAsCount, uint32_t* provAsns, uint16_t afi)
{
  ASPA_Object *obj = (ASPA_Object*)calloc(1, sizeof(ASPA_Object));
  obj->customerAsn = cusAsn;
  obj->providerAsCount = pAsCount;
  obj->providerAsns = (uint32_t*) calloc(pAsCount, sizeof(uint32_t));
  
  if (obj->providerAsns && provAsns)
  {
    for(int i=0; i< pAsCount; i++)
    {
      obj->providerAsns[i] = provAsns[i];
    }
  }
  obj->afi = afi;

  return obj;

}

// delete aspa object
//
bool deleteASPAObject(ASPA_DBManager* self, ASPA_Object *obj)
{
  if(obj)
  {
    if (obj->providerAsns)
    {
      free(obj->providerAsns);
    }
    free (obj);
    self->countAspaObj--;
    return true;
  }
  return false;
}


// create trie node
//
static TrieNode* make_trienode(char data, char* userData, ASPA_Object* obj) {
    // Allocate memory for a TrieNode
    TrieNode* node = (TrieNode*) calloc (1, sizeof(TrieNode));
    int i=0;
    for (i=0; i<N; i++)
        node->children[i] = NULL;
    node->is_leaf = 0;
    node->data = data;
    node->userData = NULL;
    node->aspaObjects = NULL;
    return node;
}

// free node
static void free_trienode(TrieNode* node) {
    // Free the trienode sequence
    int i=0;
    for(i=0; i<N; i++) {
        if (node->children[i] != NULL) {
            free_trienode(node->children[i]);
        }
        else {
            continue;
        }
    }
    free(node);
}

//  new value insert and substitution 
//
TrieNode* insertAspaObj(ASPA_DBManager* self, char* word, char* userData, ASPA_Object* obj) {
    TrieNode* temp = self->tableRoot;
    acquireWriteLock(&self->tableLock);

    for (int i=0; word[i] != '\0'; i++) {
        int idx = (int) word[i] - '0';
        //printf("index: %02x(%d), word[%d]: %c  \n", idx, idx, i, word[i]);
        if (temp->children[idx] == NULL) {
            // If the corresponding child doesn't exist, simply create that child!
            temp->children[idx] = make_trienode(word[i], userData, obj);
        }
        else {
            // Do nothing. The node already exists
        }
        // Go down a level, to the child referenced by idx
        temp = temp->children[idx];
    }
    // At the end of the word, mark this node as the leaf node
    temp->is_leaf = 1;
    temp->userData =  userData;

    // substitution
    if (temp->aspaObjects && temp->aspaObjects != obj)
    {
      deleteASPAObject(self, temp->aspaObjects);
    }
    temp->aspaObjects = obj;
    countTrieNode++;
    self->countAspaObj++;

    unlockWriteLock(&self->tableLock);

    return temp;
}

// get total count
//
uint32_t getCountTrieNode(void)
{
  return countTrieNode;
}

// search method
//
static int search_trie(TrieNode* root, char* word)
{
    // Searches for word in the Trie
    TrieNode* temp = root;
    int i=0;
    for(i=0; word[i]!='\0'; i++)
    {
        int position = word[i] - '0';
        if (temp->children[position] == NULL)
            return 0;
        temp = temp->children[position];
    }
    if (temp != NULL && temp->is_leaf == 1)
        return 1;
    return 0;
}

// external api for searching trie
//
ASPA_Object* findAspaObject(ASPA_DBManager* self, char* word)
{
    ASPA_Object *obj;
    TrieNode* temp = self->tableRoot; 

    for(int i=0; word[i]!='\0'; i++)
    {
        int position = word[i] - '0';
        if (temp->children[position] == NULL)
            return NULL;
        temp = temp->children[position];
    }
    if (temp != NULL && temp->is_leaf == 1)
    {
        obj = temp->aspaObjects;
        return obj;
    }
    return NULL;
}

//
//  print all nodes
//
TrieNode* printAllLeafNode(TrieNode *node)
{
  TrieNode* leaf = NULL;
  uint8_t count=0;

  if (node->is_leaf == 1)
  {
    leaf = node;
    return leaf;
  }

  for (int i=0; i<N; i++) 
  {
    if(node->children[i])
    {
      leaf = printAllLeafNode(node->children[i]);
      if (leaf)
      {
        //printf("++ count: %d i:%d digit: %c user data: %s\n", ++count, i, leaf->data, leaf->userData);
        printf("\n++ count: %d, user data: %s, ASPA object:%p \n", 
            ++count, leaf->userData, leaf->aspaObjects);

        ASPA_Object *obj = leaf->aspaObjects;
        if (obj)
        {
          printf("++ customer ASN: %d\n", obj->customerAsn);
          printf("++ providerAsCount : %d\n", obj->providerAsCount);
          printf("++ Address: provider asns : %p\n", obj->providerAsns);
          if (obj->providerAsns)
          {
            for(int i=0; i< obj->providerAsCount; i++)
              printf("++ providerAsns[%d]: %d\n", i, obj->providerAsns[i]);
          }
          printf("++ afi: %d\n", obj->afi);
        }
      }
    }
  }

  return NULL;
}



void print_trie(TrieNode* root) {/*{{{*/
    // Prints the nodes of the trie
    if (!root)
        return;
    TrieNode* temp = root;
    printf("%c -> ", temp->data);
    int i=0;
    for (i=0; i<N; i++) {
        print_trie(temp->children[i]);
    }
}

void print_search(TrieNode* root, char* word) {
    printf("Searching for %s: ", word);
    if (search_trie(root, word) == 0)
        printf("Not Found\n");
    else
        printf("Found!\n");
}/*}}}*/

// 
// external API for db loopkup
//
ASPA_ValidationResult ASPA_DB_lookup(ASPA_DBManager* self, uint32_t customerAsn, uint32_t providerAsn, uint8_t afi )
{
  LOG(LEVEL_INFO, FILE_LINE_INFO " called");

  char strCusAsn[6] = {};
  sprintf(strCusAsn, "%d", customerAsn);  

  acquireWriteLock(&self->tableLock);
  ASPA_Object *obj = findAspaObject(self, strCusAsn);
  unlockWriteLock(&self->tableLock);

  if (!obj) // if there is no object item
  {
    LOG(LEVEL_INFO, "[db] No customer ASN exist -- Unknown");
    return ASPA_RESULT_UNKNOWN;
  }
  else // found object
  {
    LOG(LEVEL_INFO, "[db] customer ASN: %d", obj->customerAsn);
    LOG(LEVEL_INFO, "[db] providerAsCount : %d", obj->providerAsCount);
    LOG(LEVEL_INFO, "[db] Address: provider asns : %p", obj->providerAsns);
    LOG(LEVEL_INFO, "[db] afi: %d", obj->afi);

    if (obj->providerAsns)
    {
      for(int i=0; i< obj->providerAsCount; i++)
      {
        LOG(LEVEL_INFO, "[db] providerAsns[%d]: %d", i, obj->providerAsns[i]);
        if (obj->providerAsns[i] == providerAsn && obj->afi == afi)
        {
          LOG(LEVEL_INFO, "[db] Matched -- Valid");
          return ASPA_RESULT_VALID;
        }
      }
  
      LOG(LEVEL_INFO, "[db] No Matched -- Invalid");
      return ASPA_RESULT_INVALID;
    }
  }
  return ASPA_RESULT_UNDEFINED;

}













