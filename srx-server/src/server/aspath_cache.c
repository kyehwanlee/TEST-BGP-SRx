

#include <uthash.h>
#include <stdbool.h>
#include "server/aspath_cache.h"
#include "shared/crc32.h"
#include "util/log.h"

#define HDR "([0x%08X] AspathCache): "

typedef struct {

  UT_hash_handle    hh;            // The hash table where this entry is stored
  uint32_t          pathId;
  AC_PathListData   data;
  uint8_t           aspaResult;
  AS_TYPE           asType;

} PathListCacheTable;


//
// TODO: to let main call this function to generate UT hash
bool createAspathCache(AspathCache* self, ASPA_DBManager* aspaDBManager)
{
  printf("[%s] AS path cache :%p \n", __FUNCTION__, self);
  
  if (!createRWLock(&self->tableLock))
  {
    RAISE_ERROR("Unable to setup the hash table r/w lock");
    return false;
  }
  // By default keep the hashtable null, it will be initialized with the first
  // element that will be added.
  self->aspathCacheTable = NULL;
  self->aspaDBManager = aspaDBManager;
 
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

static void add_AspathList (AspathCache *self, PathListCacheTable *cacheTable)
{

  acquireWriteLock(&self->tableLock);
  HASH_ADD (hh, *((PathListCacheTable**)&self->aspathCacheTable), pathId, sizeof(uint32_t), cacheTable);
  unlockWriteLock(&self->tableLock);

}

static bool find_AspathList (AspathCache* self, uint32_t pathId, PathListCacheTable **p_cacheTable)
{
  acquireReadLock(&self->tableLock);
  HASH_FIND(hh, (PathListCacheTable*)self->aspathCacheTable, &pathId, sizeof(uint32_t), (*p_cacheTable));
  unlockReadLock(&self->tableLock);

  return (*p_cacheTable != NULL);
}


static void del_AspathList (AspathCache* self, PathListCacheTable *cacheTable)
{
  acquireWriteLock(&self->tableLock);
  HASH_DEL (*((PathListCacheTable**)&self->aspathCacheTable), cacheTable);
  unlockWriteLock(&self->tableLock);
}


AS_PATH_LIST* newAspathListEntry (uint32_t length, uint32_t* pathData, AS_TYPE asType,  bool bBigEndian)
{
  if (!length)
  {
    printf("Error with no length\n");
    return NULL;
  }

  AS_PATH_LIST* pAspathList; 
  pAspathList               = (AS_PATH_LIST*)calloc(1, sizeof(AS_PATH_LIST));
  pAspathList->asPathLength = length;
  pAspathList->asPathList   = (uint32_t*)calloc(length, sizeof(uint32_t));
  pAspathList->asType       = asType;

  int i=0;
  for (i=0; i < length; i++)
  {
    if(bBigEndian)
    {
      pAspathList->asPathList[i] = ntohl(pathData[i]);
    }
    else
    {
      pAspathList->asPathList[i] = pathData[i];
    }
  }

  return pAspathList;
}

void printAsPathList(AS_PATH_LIST* aspl)
{
  printf("[%s] called \n", __FUNCTION__);
  if (aspl)
  {
    printf("\tpath ID           : %08X \n" , aspl->pathID);
    printf("\tlength            : %d \n"   , aspl->asPathLength);
    printf("\tValidation Result : %d \n"   , aspl->aspaValResult);
    printf("\tAS Path Type      : %d \n"   , aspl->asType);

    if(aspl->asPathList)
    {
      for(int i=0; i<aspl->asPathLength; i++)
      {
        printf("\tPath List[%d]: %d \n"   , i, aspl->asPathList[i]);
      }
    }
  }
  else
  {
    printf("\tNo path list\n");
  }
}


bool deleteAspathListEntry (AS_PATH_LIST* aspl)
{
  if (!aspl)
    return false;

  if (aspl->asPathList)
  {
    free(aspl->asPathList);
  }

  free(aspl);

  return true;
}
bool modifyAspaValidationResultToAspathCache(AspathCache *self, uint32_t pathId,
                      uint8_t modAspaResult, AS_PATH_LIST* pathlistEntry)
{
  bool retVal = true;
  PathListCacheTable *plCacheTable;

  if (!find_AspathList (self, pathId, &plCacheTable))
  {
    RAISE_SYS_ERROR("Does not exist in aspath list cache, can not modify it!");
    retVal = false;
  }
  else
  {
    if(modAspaResult != SRx_RESULT_DONOTUSE)
    {
      if(modAspaResult != plCacheTable->aspaResult)
      {
        plCacheTable->aspaResult = modAspaResult;
      }
    }
  }
  return retVal;
}

int storeAspathList (AspathCache* self, SRxDefaultResult* defRes, 
                      uint32_t pathId, AS_TYPE asType, AS_PATH_LIST* pathlistEntry)
{
  int retVal = 1; // by default report it worked

  PathListCacheTable *plCacheTable;
  
  if (find_AspathList (self, pathId, &plCacheTable))
  {
    LOG(LEVEL_WARNING, "Attempt to store an update that already exists in as path cache!");
    retVal = 0;
  }
  else
  {
    plCacheTable = (PathListCacheTable*) calloc(1, sizeof(PathListCacheTable));
    plCacheTable->pathId = pathId;
    plCacheTable->asType = asType;

    uint8_t length = pathlistEntry->asPathLength;
    plCacheTable->data.hops = length;

    // copy by value, NOT by reference.  Because path list Entry should be freed later
    //
    if ( length > 0 && pathlistEntry->asPathList)
    {
      plCacheTable->data.asPathList = (PATH_LIST*) calloc(length, sizeof(PATH_LIST));
      for(int i=0; i < length; i++)
      {
        plCacheTable->data.asPathList[i] = pathlistEntry->asPathList[i];
      }
    }
    //plCacheTable->data.asPathList = pathlistEntry->asPathList? 
    //                                pathlistEntry->asPathList: NULL;
    if (defRes != NULL)
    {
      plCacheTable->aspaResult = defRes->result.aspaResult;
    }

    add_AspathList(self, plCacheTable);
    printf("performed to add PathList Entry into As Path Cache\n");

  }

//#define TEST 
#ifdef TEST
  PathListCacheTable *tmp_plCacheTable;
  if (find_AspathList (self, pathId, &tmp_plCacheTable))
  {
    printf("Found in UThash\n");
  }
#endif

  return retVal;
}


// TODO: delete function for calling del_AspathList to remove the cache data
//
bool deleteAspathCache()
{
  return false;
}


// key : path id to find AS path cache record
// return: a new AS PATH LIST structure
//
AS_PATH_LIST* getAspathList (AspathCache* self, uint32_t pathId, SRxResult* srxRes)
{
  if (!pathId)
  {
    printf("Invalid path id\n");
    return NULL;
  }

  AS_PATH_LIST *aspl = NULL;
  PathListCacheTable *plCacheTable;
  
  if (find_AspathList (self, pathId, &plCacheTable))
  {
    aspl = (AS_PATH_LIST*)calloc(1, sizeof(AS_PATH_LIST));
    aspl->pathID        = plCacheTable->pathId;
    aspl->asPathLength  = plCacheTable->data.hops;
    aspl->asPathList    = plCacheTable->data.asPathList;
    aspl->aspaValResult = plCacheTable->aspaResult;
    aspl->asType        = plCacheTable->asType;

    if (srxRes->aspaResult != aspl->aspaValResult)
      srxRes->aspaResult  = aspl->aspaValResult;
  }
  else
  {
    srxRes->aspaResult = SRx_RESULT_UNDEFINED;
  }

  return aspl;
}


uint32_t makePathId (uint8_t asPathLength, PATH_LIST* asPathList, bool bBigEndian)
{
  uint32_t pathId=0;
  char* strBuf = NULL;

  if (!asPathList)
  {
    printf("as path list is NULL, making CRC failure\n");
    return 0;
  }

  int strSize = asPathLength * 4 *2 +1;  //  Path length * 4 byte, *2: hex string
  strBuf = (char*)calloc(strSize, sizeof(char));
  if (!strBuf)
  {
    printf("memory allocation error\n");
    return 0;
  }

  for (int i=0; i < asPathLength; i++)
  {
    if(bBigEndian)
      sprintf(strBuf + (i*4*2), "%08X", ntohl(asPathList[i]));
    else
      sprintf(strBuf + (i*4*2), "%08X", asPathList[i]);

  }

  pathId = crc32((uint8_t*)strBuf, strSize);
  printf("CRC: %08X strings: %s\n", pathId, strBuf);

  if (strBuf)
  {
    free(strBuf);
  }

  return pathId;
}

void printPathListCacheTableEntry(PathListCacheTable *cacheEntry)
{
  if (cacheEntry)
  {
    printf("path ID           : %08X \n" , cacheEntry->pathId);
    printf("length (hops)     : %d \n"   , cacheEntry->data.hops);
    printf("Validation Result : %d \n"   , cacheEntry->aspaResult);
    printf("\t(0:valid, 2:Invalid, 3:Undefined 5:Unknown, 6:Unverifiable)\n");
    printf("AS Path Type      : %d \n"   , cacheEntry->asType);

    if (cacheEntry->data.asPathList)
    {
      for(int i=0; i<cacheEntry->data.hops; i++)
      {
        printf("- Path List[%d]: %d \n", i, cacheEntry->data.asPathList[i]);
      }
    }
    else
    {
        printf("Path List: Doesn't exist \n");
    }
  }
  else
  {
    printf("No Entry exist\n");
  }
  printf("\n");
}



uint8_t getCountAsPathCache(AspathCache *self)
{
  uint8_t numRecords = 0;

  acquireWriteLock(&self->tableLock);
  numRecords = HASH_COUNT((PathListCacheTable*)self->aspathCacheTable);
  unlockWriteLock(&self->tableLock);

  return numRecords;
}

int idSort(PathListCacheTable *a, PathListCacheTable *b) 
{
  return (a->pathId - b->pathId);
}

void sortByPathId(AspathCache *self)
{
  acquireWriteLock(&self->tableLock);
  HASH_SORT(*((PathListCacheTable**)&self->aspathCacheTable), idSort);
  unlockWriteLock(&self->tableLock);
}


void printAllAsPathCache(AspathCache *self)
{
  PathListCacheTable *currCacheTable, *tmp;

  //sortByPathId(self);

  acquireWriteLock(&self->tableLock);
  HASH_ITER(hh, (PathListCacheTable*)self->aspathCacheTable, currCacheTable, tmp) 
  {
    printf("\n");
    printPathListCacheTableEntry(currCacheTable);
    printf("\n");
  }
  unlockWriteLock(&self->tableLock);
}











