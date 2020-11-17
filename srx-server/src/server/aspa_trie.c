
#include <stdio.h> /* printf */
#include <stdlib.h> /* exit */
#include <string.h>
#include <stdbool.h>
#include "server/aspa_trie.h"

bool initializeAspaTupleManager(ASPA_DBManager* aspaDBManager) 
{
   aspaDBManager->tableRoot = newAspaTrie();

  return true;
}

TrieNode* newAspaTrie(void) 
{
  TrieNode *rootNode = make_trienode('\0', NULL);
  return rootNode;
}

ASPA_Object* newASPAObject(uint32_t cusAsn, uint16_t pAsCount, uint32_t* provAsns, uint16_t afi)
{
  ASPA_Object *obj = (ASPA_Object*)calloc(1, sizeof(ASPA_Object));
  obj->customerAsn = cusAsn;
  obj->providerAsCount = pAsCount;
  obj->providerAsns = (uint32_t*) calloc(pAsCount, sizeof(uint32_t));
  
  if (obj->providerAsns && provAsns)
  {
    int i=0;
    for(i=0; i< pAsCount; i++)
    {
      obj->providerAsns[i] = provAsns[i];
    }
  }
  obj->afi = afi;

  return obj;

}


TrieNode* make_trienode(char data, char* userData ) {
    // Allocate memory for a TrieNode
    TrieNode* node = (TrieNode*) calloc (1, sizeof(TrieNode));
    int i=0;
    for (i=0; i<N; i++)
        node->children[i] = NULL;
    node->is_leaf = 0;
    node->data = data;
    node->userData = userData? userData:NULL;
    return node;
}

void free_trienode(TrieNode* node) {
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

TrieNode* insert_trie(TrieNode* root, char* word, char* userData) {
    // Inserts the word onto the Trie
    // ASSUMPTION: The word only has lower case characters
    TrieNode* temp = root;
    int i=0;
    for (i=0; word[i] != '\0'; i++) {
        // Get the relative position in the alphabet list
        int idx = (int) word[i] - '0';
        //printf("index: %02x(%d), word[%d]: %c  \n", idx, idx, i, word[i]);
        if (temp->children[idx] == NULL) {
            // If the corresponding child doesn't exist,
            // simply create that child!
            temp->children[idx] = make_trienode(word[i], userData);
        }
        else {
            // Do nothing. The node already exists
        }
        // Go down a level, to the child referenced by idx
        // since we have a prefix match
        temp = temp->children[idx];
    }
    // At the end of the word, mark this node as the leaf node
    temp->is_leaf = 1;
    return root;
}


int search_trie(TrieNode* root, char* word)
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

void print_trie(TrieNode* root) {
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
}








#if 0
int main() {
    // Driver program for the Trie Data Structure Operations
    TrieNode* root = make_trienode('\0');
    root = insert_trie(root, "61500");
    root = insert_trie(root, "61301");

    printf(" --- searching --- \n");
    print_search(root, "61500");
    print_search(root, "61300");
    print_search(root, "60000");

    print_trie(root);
    free_trienode(root);
    printf("\n");
    return 0;
}
#endif









