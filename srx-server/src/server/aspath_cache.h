/**
 * This software was developed at the National Institute of Standards and
 * Technology by employees of the Federal Government in the course of
 * their official duties. Pursuant to title 17 Section 105 of the United
 * States Code this software is not subject to copyright protection and
 * is in the public domain.
 * 
 * NIST assumes no responsibility whatsoever for its use by other parties,
 * and makes no guarantees, expressed or implied, about its quality,
 * reliability, or any other characteristic.
 * 
 * We would appreciate acknowledgment if the software is used.
 * 
 * NIST ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" CONDITION AND
 * DISCLAIM ANY LIABILITY OF ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING
 * FROM THE USE OF THIS SOFTWARE.
 * 
 * 
 * This software might use libraries that are under GNU public license or
 * other licenses. Please refer to the licenses of all libraries required 
 * by this software.
 *
 * Aspath Cache.
 *
 * @version 0.5.0.0
 *
 * Changelog:
 * -----------------------------------------------------------------------------
 *
 */

#ifndef __ASPATH_CACHE_H__
#define __ASPATH_CACHE_H__ 

#include <stdio.h>
#include "server/configuration.h"
#include "shared/srx_defs.h"
#include "shared/srx_packets.h"
#include "util/mutex.h"
#include "util/rwlock.h"
#include "util/slist.h"
#include "server/update_cache.h"

typedef uint32_t as_t;

// AS Path List structure
typedef struct {
  uint32_t  pathID;
  uint8_t   asPathLength;
  uint32_t  *asPathList;
  uint8_t   aspaValResult;

} AS_PATH_LIST;




typedef struct {
  uint16_t              hops;
  uint32_t*             asPathList;
} AC_PathListData;



/**
 * A single Update Cache.
 */
typedef struct {  

  UpdateCache       *linkUpdateCache;
  void              *aspathCacheTable;
  RWLock            tableLock;

} AspathCache;


bool createAspathCache(AspathCache* self);
void releaseAspathCache(AspathCache* self);
void emptyAspathCache(AspathCache* self);




















#endif // __ASPATH_CACHE_H__ 
