
#ifndef _SRXCRYPTOAPI_H
#define _SRXCRYPTOAPI_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <sys/param.h>
#include <stdarg.h>
#include <stdbool.h>

/** The SKI length is defined in the protocol specification. */
#define SKI_LENGTH     20
/** Twice the length of the SKI_LENGTH in binary form */
#define SKI_HEX_LENGTH 40

////////////////////////////////////////////////////////////////////////////////
// BGPSEC Path Structures
////////////////////////////////////////////////////////////////////////////////

/** BGPSEC PathSegment data in abstracted form, no limit is placed on length.
 * This structure traverses the path from the destination to the origin with
 * the next pointer pointing to the root (origin) 
 */
typedef struct PathSegment_t
{
  /** Pointer to the next path segment which is the direction to the origin. */
  struct PathSegment_t*  next;
  /** The AS number of this segment*/
  u_int32_t     as;
  /** The pCunter for this segment. >= 1 */
  u_int8_t      pCount;
  /** Flags - See Specification */
  u_int8_t      flags;
} PathSegment;

typedef struct SigSegment_t
{
  /** Pointer to the next signature segment. */
  struct SigSegment_t * next;
  /** The SKI (20 Bytes according to the SPEC)*/
  u_char*        ski;
  /** Length of the signature in bytes */
  u_int16_t      sigLen;
  /** The signature */
  u_char*        signature;
} SigSegment;

typedef struct SigBlock_t
{
  /** Internal pointer to the next signature block */
  struct SigBlock_t*   next;
  /** Length of the signature block in bytes */
  u_int16_t     sigBlockLen;
  /** The ID  of the algorithm suite */
  u_int8_t      algoSuiteId;
  /** Pointer to all signature segments. */
  SigSegment*   sigSegments;
} SigBlock;

/* BGPSEC path may be include some BGPSEC Segments.  */
typedef struct
{
  // @TODO: replace size_t with proper size
  /** Length of the secure path (2 bytes) represents the total length in bytes 
   * of the path including the 2 bytes of this field. */
  size_t       securePathLen;

  /** Pointer to the path segments. */
  PathSegment* pathSegments;
  /** Pointer to the signature blocks. */
  SigBlock*    sigBlocks;
} BgpsecPathAttr;

// Crypto API types
/** The BGPSec Key wrapper. The stored key structure is in DER format.*/
typedef struct
{
  /** The id of the used algorithm suite (See RFC)*/
  u_int8_t  algoID;
  /** The ASN that uses the Key */
  u_int32_t asn;
  /** The SKI of the key */
  u_int8_t  ski[SKI_LENGTH];
  /** The length of the key byte stream. */
  u_int16_t keyLength;
  /** The key in DER format */
  u_int8_t* keyData;
} BGPSecKey;

/* Parameter structure for using validation request and Signing request.
 * It will be used for structured data which includes the hash information
 * this structural information will be used for verification and signing
 * function parameter
 */
typedef struct
{
  u_int16_t dataLength;
  u_int8_t* data;       // the data buffer (message) to sign over.
  u_int8_t* ski;
  u_int8_t  algoID;
  u_int16_t sigLen;
  u_int8_t* signature;
} BGPSecSignData;


extern unsigned long term_debug;

#define MAX_CFGFILE_NAME 255

/* The SRxCryptoAPI wrapper object.*/
typedef struct
{
  /** The Library Handle. */
  void* libHandle;
  /** The configuration file name. */
  char* configFile;

  /**
   * Perform BGPSEC path validation. This function uses the list of keys
   * provided by the caller. Internally stored keys are NOT used. This function
   * returns the validation state or -1 for an error. The caller manages the
   * memory andMUST assure the memory is intact until the function returns. The
   * implementation itself DOES NOT modify the given data.
   *
   * @param bgpsec_path pointer to the BGPSEC path attribute.
   * @param number_keys The number of keys passed within the key array.
   * @param keys an array containing the keys.
   * @param prefix pointer to the prefix.
   * @param localAS the callers local AS number.
   *
   * @return -1: error, 0: invalid, 1: valid
   */
  int (*validate)(BgpsecPathAttr* bgpsec_path, u_int16_t number_keys,
                  BGPSecKey** keys, void *prefix, u_int32_t localAS );

  /**
   * Perform BGPSEC path validation. (Optional) This function uses the list of
   * registered public keys and returns the validation state or -1 for an error.
   * The caller manages the memory and MUST assure the memory is intact until
   * the function returns. The implementation itself DOES NOT modify the given
   * data.
   *
   * @param bgpsec_path pointer to the BGPSEC path attribute.
   * @param prefix pointer to the prefix.
   * @param localAS the callers local AS number.
   * @param extCode contains more information in case the validation value is
   *                invalid. 0: validation failed, 1: key not found.
   *
   * @return -1: error, 0: invalid, 1: valid
   */
  int (*extValidate)(BgpsecPathAttr* bgpsec_path, void *prefix,
                     u_int32_t localAS, u_int8_t* extCode);

  /**
   * Sign the given BGPSecSign data using the given key. This method fills the
   * key into the BGPSecSignData object.
   *
   * @param bgpsec_data The data object to be signed. This also includes the
   *                    generated signature.
   * @param key The BGPSec key.
   *
   * @return 0: failure, 1: success
   */
  int (*sign_with_key)(BGPSecSignData* bgpsec_data, BGPSecKey* key);

  /**
   * Sign the given data using the given pre-registered key. This method fills
   * the into the signature. The sigLen will contain the length of the
   * used signature space.
   *
   * @param dataLength Size of the buffer containing the data to be signed.
   * @param data The data buffer to be signed
   * @param keyID The pre-registered private key to be used
   * @param sigLen The length in bytes of the signature buffer
   * @param signature The signature buffer itself
   *
   * @return 0: failure, > 0: length of the signature
   */
  int (*sign_with_id)(u_int16_t dataLength, u_int8_t* data, u_int8_t keyID,
                      u_int16_t sigLen, u_int8_t* signature);

  /**
   * Register the private key (Optional). This method allows to register the
   * private key with the API object. The return value is an 8 bit integer.
   * The key must be internally copied. The memory is NOT shared for longer than
   * the registration execution cycle.
   * NOTE: The key information MUST be copied within the API.
   *
   * @param key The key to be registered. The BGPSecKey Structure contains all
   *            needed key information.
   *
   * @return 0: registration failed, 1-255: The ID of the key
   */
  u_int8_t (*registerPrivateKey)(BGPSecKey* Key);

  /**
   * Remove the registration of a given key with the specified key ID.
   * (Optional)
   *
   * @param keyID The id of the key
   *
   * @return 0: failure, 1: success
   */
  u_int8_t (*unregisterPrivateKey)(u_int8_t keyID);

  /**
   * Register the public key. (Optional)
   * In case the API provides public key management the keys can be
   * pre-registered. This will allow to call the verification without the need
   * to determine the public keys. The API will get the key information itself.
   * NOTE: The key information MUST be copied within the API.
   *
   * @param key The key itself.
   *
   * @return 0: failure, 1: success
   */
  u_int8_t (*registerPublicKey)(BGPSecKey* key);

  /**
   * Remove the registered key with the same ski and asn. (Optional)
   * This method allows to remove a particular key that is registered for the
   * given SKI and ASN.
   *
   * @param key The key itself.
   *
   * @return 0: failure, 1: success
   */
  int (*unregisterPublicKey)(BGPSecKey* key);

  /**
   * This method determines if the API provides the extended public key
   * management. In this case the extended validation method extValidate can be
   * called.
   *
   * @return 0: Does NOT provide the extended method. 1: does provide extended
   *         functionality
   */
  int (*isExtended)();

  /**
   * Return 1 if this API allows the storage of private keys, otherwise 0.
   *
   * @return 0: Does not provide private key storage, 1: Does provide key
   *         private storage
   */
  int (*isPrivateKeyStorage)();
} SRxCryptoAPI;

/* Function Declaration */

/**
 * This function initialized the SRxCrypto API. the SRxCryptoAPI object must be
 * created and released by the user of the API. In case the configuration is not
 * set the default API located in ./ will be loaded.
 *
 * The return value will be
 */
int srxCryptoInit(SRxCryptoAPI* api);

/**
 * This function unloads the library that is loaded and attached to the
 * srcCryuptoAPI.
 *
 * @return 0
 */
int srxCryptoUnbind();

////////////////////////////////////////////////////////////////////////////////
// Helper Functions
////////////////////////////////////////////////////////////////////////////////

/**
 * This function sets the key path.
 *
 * @param key_path the path to the keys (\0 terminated).
 *
 * @return 0 error, 1 success
 *
 */
int sca_SetKeyPath (char* key_path);

/**
 * This method generates a filename out of the given SKI.
 *
 * @param filenamebuf The pre-allocated filename buffer which will be filled
 *                    with the filename.
 * @param filenamebufLen Maximum length of the buffer.
 * @param ski       The SKI where the filename will be generated from
 *
 * @return The filename buffer that was handed over.
 */
char* sca_FindDirInSKI (char* filenamebuf, size_t filenamebufLen, u_int8_t* ski);

/**
 * Load the key from the key volt location configured within the API. The key
 * needs the SKI specified in binary format.
 * The returned key is in DER format. The parameter fPrivate is used to
 * indicate if the private or public key will be returned. This is of importance
 * in case both keys exist. Both keys will have the same SKI.
 *
 * @param key Pre-allocated memory where the ley will be loaded into.
 * @param fPrivate indicates if the key is private or public.
 *
 * @return 1 if key was loaded successfully, 0 otherwise
 */
int sca_loadKey(BGPSecKey* key, bool fPrivate);

/**
 * Writes the loging information.
 *
 * @param level The logging level
 * @param format The format of the logging info
 * @param ...
 */
void sca_debugLog( int level, const char *format, ...);

#endif /* _SRXCRYPTOAPI_H*/
