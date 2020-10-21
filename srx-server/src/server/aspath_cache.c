

#include <uthash.h>
#include <stdbool.h>
#include "server/aspath_cache.h"

#define HDR "([0x%08X] AspathCache): "

typedef struct {

  uint32_t id;
  UT_hash_handle    hh;            // The hash table where this entry is stored
  AC_PathData       data;

} PathListCacheTable;


//
// TODO: to let main call this function to generate UT hash
bool createAspathCache(AspathCache* self)
{

  // By default keep the hashtable null, it will be initialized with the first
  // element that will be added.
  self->cacheTable= NULL;

  return true;
}


void releaseAspathCache(AspathCache* self)
{


}


























