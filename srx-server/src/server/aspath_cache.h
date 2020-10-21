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

//typedef void (*UpdateResultChanged)(SRxValidationResult* result);

/**
 * A single Update Cache.
 */
typedef struct {  

  void*           cacheTable;      // The cache data
  UpdateCache    *pUpdateCache;
} AspathCache;


/** This structure contains the BGPsec path information for both, BGP4 and 
 * BGPsec*/
typedef struct {
  u_int16_t              hops;
  u_int32_t*             listAsnPath;
} AC_PathData;



























#endif // __ASPATH_CACHE_H__ 
