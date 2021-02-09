#ifndef __ASPA_TRIE_H__
#define __ASPA_TRIE_H__

#include <stdint.h>
#include "shared/srx_defs.h"
#include "server/configuration.h"
#include "util/mutex.h"
#include "util/rwlock.h"

// The number of children for each node
// We will construct a N-ary tree and make it a Trie
#define N 10

typedef struct {
  uint32_t customerAsn; 
  uint16_t providerAsCount;
  uint32_t *providerAsns;
  uint16_t afi;
} ASPA_Object;


typedef struct TrieNode TrieNode;
struct TrieNode {
    // The Trie Node Structure
    // Each node has N children, starting from the root
    // and a flag to check if it's a leaf node
    char data; // Storing for printing purposes only
    TrieNode* children[N];
    int is_leaf;
    void *userData;
    ASPA_Object *aspaObjects;
};

typedef struct {
  TrieNode*         tableRoot;
  uint32_t          countAspaObj;
  Configuration*    config;  // The system configuration
  RWLock            tableLock;
} ASPA_DBManager;


static TrieNode* newAspaTrie(void);
static TrieNode* make_trienode(char data, char* userData, ASPA_Object* );
static void free_trienode(TrieNode* node);
TrieNode* insertAspaObj(ASPA_DBManager* self, char* word, char* userData, ASPA_Object* obj);
static int search_trie(TrieNode* root, char* word);
void print_trie(TrieNode* root);
bool initializeAspaDBManager(ASPA_DBManager* aspaDBManager, Configuration* config);
static void emptyAspaDB(ASPA_DBManager* self);
ASPA_Object* findAspaObject(ASPA_DBManager* self, char* word);
void print_search(TrieNode* root, char* word);
bool deleteASPAObject(ASPA_DBManager* self, ASPA_Object *obj);
ASPA_Object* newASPAObject(uint32_t cusAsn, uint16_t pAsCount, uint32_t* provAsns, uint16_t afi);
ASPA_ValidationResult ASPA_DB_lookup(ASPA_DBManager* self, uint32_t customerAsn, uint32_t providerAsn, uint8_t afi);





#endif // __ASPA_TRIE_H__ 
