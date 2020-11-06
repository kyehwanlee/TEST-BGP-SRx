

#include <uthash.h>
#include <stdbool.h>
#include "server/aspath_cache.h"
#include "util/log.h"

#define HDR "([0x%08X] AspathCache): "

typedef struct {

  UT_hash_handle    hh;            // The hash table where this entry is stored
  uint32_t          pathId;
  AC_PathListData   data;

} PathListCacheTable;


//
// TODO: to let main call this function to generate UT hash
bool createAspathCache(AspathCache* self)
{

  printf("%S\n", __FUNCTION__);
  
  if (!createRWLock(&self->tableLock))
  {
    RAISE_ERROR("Unable to setup the hash table r/w lock");
    return false;
  }
  // By default keep the hashtable null, it will be initialized with the first
  // element that will be added.
  self->aspathCacheTable = NULL;
  

 
  return true;
}


void releaseAspathCache(AspathCache* self)
{

  if (self != NULL)
  {
    releaseRWLock(&self->tableLock);
    emptyAspathCache(self);
  }

}

void emptyAspathCache(AspathCache* self)
{
  acquireWriteLock(&self->tableLock);
  self->aspathCacheTable = NULL;
  unlockWriteLock(&self->tableLock);

}

static void addAspath (AspathCache *self, PathListCacheTable *cacheTable)
{

  acquireWriteLock(&self->tableLock);
  HASH_ADD (hh, *((PathListCacheTable**)&self->aspathCacheTable), pathId, sizeof(uint32_t), cacheTable);
  unlockWriteLock(&self->tableLock);

}

static bool findAspath (AspathCache* self, uint32_t pathId, PathListCacheTable **p_cacheTable)
{
  acquireReadLock(&self->tableLock);
  HASH_FIND(hh, (PathListCacheTable*)&self->aspathCacheTable, &pathId, sizeof(uint32_t), (*p_cacheTable));
  unlockReadLock(&self->tableLock);
}


static void delAspath (AspathCache* self, PathListCacheTable *cacheTable)
{
  acquireWriteLock(&self->tableLock);
  HASH_DEL (*((PathListCacheTable**)&self->aspathCacheTable), cacheTable);
  unlockWriteLock(&self->tableLock);
}



































