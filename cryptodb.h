/**
 * MIT License
 *
 * Copyright 2024 PE Stanislav Yahniukov <pe@yahniukov.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the “Software”), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
 * to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
*/

#pragma once

#include <stdint.h>

#ifndef CRYPTODB_EXPORT
#if defined(_WIN32) && _WIN32
    #define CRYPTODB_EXPORT __declspec(dllexport)
#elif defined(WIN32) && WIN32
    #define CRYPTODB_EXPORT __declspec(dllexport)
#else
    #define CRYPTODB_EXPORT __attribute ((visibility ("default")))
#endif
#endif

#define CRYPTODB_VER_MAJOR    (1)
#define CRYPTODB_VER_MINOR    (0)
#define CRYPTODB_VER_REVISION (0)

#define CRYPTODB_UNIQ_DATA_MAX_LEN (512)

#define CRYPTODB_OPT_DEFAULT_CACHE_SIZE    (8 << 20)
#define CRYPTODB_OPT_DEFAULT_WR_BUF_SIZE   (4 * 1024 * 1024)
#define CRYPTODB_OPT_DEFAULT_MAX_FILES     (1000)
#define CRYPTODB_OPT_DEFAULT_BLOCK_SIZE    (4 * 1024)
#define CRYPTODB_OPT_DEFAULT_BLOCK_RE_INT  (16)
#define CRYPTODB_OPT_DEFAULT_MAX_FILE_SIZE (2 * 1024 * 1024)

typedef enum {
    CRYPTODB_ERR_OK  = 0,
    CRYPTODB_SUCCESS = CRYPTODB_ERR_OK,

    CRYPTODB_ERR_NULL_POINTER    = -1,
    CRYPTODB_ERR_ALLOCATE_MEM    = -2,
    CRYPTODB_ERR_WRONG_ARGUMENT  = -3,
    CRYPTODB_ERR_ENCRYPTION_FAIL = -4,
    CRYPTODB_ERR_DECRYPTION_FAIL = -5,
    // <-- New error types should be added here

    CRYPTODB_ERR_FAIL = -1024 // always last
} cryptodb_err_t;

typedef enum {
    CRYPTODB_VAL_STRING = 1,
    CRYPTODB_VAL_NUM_INT = 2,    // types: int, int32_t
    CRYPTODB_VAL_NUM_DOUBLE = 3, // types: double
    // <-- New value types should be added here

    CRYPTODB_VAL_UNKNOWN // always last
} cryptodb_val_t;

/**
 * User defined KDF (Key Derivation Function) function. User can
 * optionally specify it in cryptodb_open.
 * If not specified, default _cryptodb_get_encryption_key_iv()
 * in cryptodb.c file will be used.
 *
 * It should return cryptodb_err_t. See _cryptodb_get_encryption_key_iv()
 * as example.
 *
 * "void *cryptodb" should be a pointer to cryptodb_t handler (see below)
 */
typedef int (*cryptodb_user_kdf)(void *cryptodb,
                                 bool encrypt_decrypt,
                                 uint8_t encryption_key[32],
                                 uint8_t encryption_iv[16],
                                 void *user_data);

typedef struct {
    void *db;
    void *env;
    void *cmp;
    void *cache;
    void *options;
    void *roptions;
    void *woptions;
    uint8_t uniq_data[CRYPTODB_UNIQ_DATA_MAX_LEN];
    size_t uniq_data_len;
    bool use_keys_instead_of_uniq_data;
    cryptodb_user_kdf user_kdf;
    void *kdf_user_data;
    int disable_keys_encryption; // See cryptodb_options_t below
} cryptodb_t;

/**
 * cryptodb_options_t
 *
 * See default values above (CRYPTODB_OPT_DEFAULT_*)
 */
typedef struct {
    size_t cache_capacity; // A Cache is an interface that maps keys to values. It has internal
                           // synchronization and may be safely accessed concurrently from
                           // multiple threads.
    size_t write_buffer_size; // Amount of data to build up in memory (backed by an unsorted log
                              // on disk) before converting to a sorted on-disk file.
                              // Larger values increase performance, especially during bulk loads.
                              // Up to two write buffers may be held in memory at the same time,
                              // so you may wish to adjust this parameter to control memory usage.
                              // Also, a larger write buffer will result in a longer recovery time
                              // the next time the database is opened.
    int max_open_files; // Number of open files that can be used by the DB.  You may need to
                        // increase this if your database has a large working set (budget
                        // one open file per 2MB of working set).
    size_t block_size; // Approximate size of user data packed per block.  Note that the
                       // block size specified here corresponds to uncompressed data.  The
                       // actual size of the unit read from disk may be smaller if
                       // compression is enabled.  This parameter can be changed dynamically.
    int block_restart_interval; // Number of keys between restart points for delta encoding of keys.
                                // This parameter can be changed dynamically.  Most clients should
                                // leave this parameter alone.
    size_t max_file_size; // Leveldb will write up to this amount of bytes to a file before
                          // switching to a new one.
                          // Most clients should leave this parameter alone.  However if your
                          // filesystem is more efficient with larger files, you could
                          // consider increasing the value.  The downside will be longer
                          // compactions and hence longer latency/performance hiccups.
                          // Another reason to increase this parameter might be when you are
                          // initially populating a large database.
    int disable_keys_encryption; // If not 0, entry keys will not be encrypted, only values
} cryptodb_options_t;

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief      Return string representation of cryptodb_err_t
 *
 * @param[in]  err   See cryptodb_err_t
 *
 * @return     String representation of cryptodb_err_t
 */
CRYPTODB_EXPORT const char * cryptodb_err_to_str(cryptodb_err_t err);

/**
 * @brief      Return string representation of cryptodb_val_t
 *
 * @param[in]  val   See cryptodb_val_t
 *
 * @return     String representation of cryptodb_val_t
 */
CRYPTODB_EXPORT const char * cryptodb_val_to_str(cryptodb_val_t val);

/**
 * @brief      Open database that is located in specified "path" folder.
 *             The database will be created if not exist.
 *
 * @param[in]  path           The full database folder path
 * @param[in]  uniq_data      Uniq client data that will be used
 *                            to create encryption/decryption keys
 * @param[in]  uniq_data_len  "uniq_data" length
 * @param[in]  options        (Optional, can be NULL)
 *                            See cryptodb_options_t. If NULL, default
 *                            fields will be used.
 * @param[in]  user_kdf       (Optional, can be NULL)
 *                            See cryptodb_user_kdf. If NULL, default
 *                            _cryptodb_get_encryption_key_iv() will be
 *                            used.
 * @param[in]  kdf_user_data  (Optional, can be NULL)
 *                            user_kdf client data, see cryptodb_user_kdf
 * @param[out] cryptodb       Database handler
 *
 * @return     See cryptodb_err_t
 */
CRYPTODB_EXPORT int cryptodb_open(const char *path,
                                  uint8_t uniq_data[CRYPTODB_UNIQ_DATA_MAX_LEN],
                                  size_t uniq_data_len,
                                  cryptodb_options_t *options,
                                  cryptodb_user_kdf user_kdf,
                                  void *kdf_user_data,
                                  cryptodb_t *cryptodb);

/**
 * @brief      Open database that is located in specified "path" folder.
 *             The database will be created if not exist.
 *             It's alternative to cryptodb_open() function. Use this
 *             if you want to use ready secret encryption key and IV.
 *
 * @param[in]  path            The full database folder path
 * @param[in]  encryption_key  Secret encryption key for AES-256 CBC encryption
 * @param[in]  encryption_iv   Secret initialization vector for AES-256 CBC encryption
 * @param[in]  options         (Optional, can be NULL)
 *                             See cryptodb_options_t. If NULL, default
 *                             fields will be used.
 * @param[out] cryptodb        Database handler
 *
 * @return     See cryptodb_err_t
 */
CRYPTODB_EXPORT int cryptodb_open_with_keys(const char *path,
                                            uint8_t encryption_key[32],
                                            uint8_t encryption_iv[16],
                                            cryptodb_options_t *options,
                                            cryptodb_t *cryptodb);

/**
 * @brief      Close database that associated with specific handler
 *
 * @param[in]  cryptodb  Database handler
 */
CRYPTODB_EXPORT void cryptodb_close(cryptodb_t *cryptodb);

/**
 * @brief      Put the "key-value" entry in the database.
 *             The value length will be determined in the following way:
 *             * if valtype = CRYPTODB_VAL_STRING     - strlen((const char *)val) + 1
 *             * if valtype = CRYPTODB_VAL_NUM_INT    - sizeof(int)
 *             * if valtype = CRYPTODB_VAL_NUM_DOUBLE - sizeof(double)
 *
 * @param[in]  cryptodb  Database handler
 * @param[in]  key       Database entry key
 * @param[in]  keylen    Database entry key length
 * @param[in]  valtype   See cryptodb_val_t
 * @param[in]  val       Pointer to entry value
 *
 * @return     See cryptodb_err_t
 */
CRYPTODB_EXPORT int cryptodb_put(cryptodb_t *cryptodb,
                                 const char* key, size_t keylen,
                                 cryptodb_val_t valtype, void *val);

/**
 * @brief      "cryptodb_put" wrapper where valtype == CRYPTODB_VAL_STRING
 *
 * @return     See cryptodb_err_t
 */
CRYPTODB_EXPORT int cryptodb_put_string(cryptodb_t *cryptodb,
                                        const char* key, size_t keylen, const char *val);

/**
 * @brief      "cryptodb_put" wrapper where valtype == CRYPTODB_VAL_NUM_INT
 *
 * @return     See cryptodb_err_t
 */
CRYPTODB_EXPORT int cryptodb_put_integer(cryptodb_t *cryptodb,
                                         const char* key, size_t keylen, int val);

/**
 * @brief      "cryptodb_put" wrapper where valtype == CRYPTODB_VAL_NUM_DOUBLE
 *
 * @return     See cryptodb_err_t
 */
CRYPTODB_EXPORT int cryptodb_put_double(cryptodb_t *cryptodb,
                                        const char* key, size_t keylen, double val);

/**
 * @brief      Get value of the entry that is assosiated with specified key.
 *             The value length should be determined in the following way:
 *             * if valtype = CRYPTODB_VAL_STRING     - strlen((const char *)val) + 1
 *             * if valtype = CRYPTODB_VAL_NUM_INT    - sizeof(int)
 *             * if valtype = CRYPTODB_VAL_NUM_DOUBLE - sizeof(double)
 *             If actual value type is not same as was specified in "valtype"
 *             the function doesn't touch "val" pointer and returns actual
 *             value type - cryptodb_val_t
 *
 * @param[in]   cryptodb  Database handler
 * @param[in]   key       Database entry key
 * @param[in]   keylen    Database entry key length
 * @param[in]   valtype   See cryptodb_val_t
 * @param[out]  val       Pointer to entry value
 *
 * @return     cryptodb_err_t or cryptodb_val_t, see @brief
 */
CRYPTODB_EXPORT int cryptodb_get(cryptodb_t *cryptodb,
                                 const char* key, size_t keylen,
                                 cryptodb_val_t valtype, void *val);

/**
 * @brief      Delete entry with specified key from the database
 *
 * @param[in]  cryptodb  Database handler
 * @param[in]  key       Database entry key
 * @param[in]  keylen    Database entry key length
 *
 * @return     See cryptodb_err_t
 */
CRYPTODB_EXPORT int cryptodb_delete(cryptodb_t *cryptodb,
                                    const char* key, size_t keylen);

/**
 * @brief      Destroy database that is located in specified "path" folder.
 *
 * @param[in]  path     The full database folder path
 * @param[in]  options  (Optional, can be NULL)
 *                      See cryptodb_options_t. If NULL, default
 *                      fields will be used.
 *
 * @return     See cryptodb_err_t
 */
CRYPTODB_EXPORT int cryptodb_destroy(const char *path,
                                     cryptodb_options_t *options);

#ifdef __cplusplus
}
#endif
