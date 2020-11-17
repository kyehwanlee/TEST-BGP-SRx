#ifndef __ASPA_TRIE_H__
#define __ASPA_TRIE_H__

#include <stdint.h>

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
  TrieNode *tableRoot;

} ASPA_DBManager;


TrieNode* newAspaTrie(void);
TrieNode* make_trienode(char data, char* userData );
void free_trienode(TrieNode* node);
TrieNode* insert_trie(TrieNode* root, char* word, char* userData);
int search_trie(TrieNode* root, char* word);
void print_trie(TrieNode* root);
bool initializeAspaTupleManager(ASPA_DBManager* aspaDBManager);





#endif // __ASPA_TRIE_H__ 
