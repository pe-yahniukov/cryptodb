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

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>

#include <cJSON.h>
#include <scprng.h>
#include <leveldb/c.h>
#include <mbedtls/aes.h>
#include <mbedtls/sha3.h>

#include <cryptodb.h>

#define CRYPTODB_AES_BLOCK_LEN (16)

#define CRYPTODB_UNUSED(var) ((void)var)

/**
 * PRIVATE API
 */

static void _cryptodb_comparator_destroy(void *arg)
{
    CRYPTODB_UNUSED(arg);
}

static int _cryptodb_comparator_compare(void *arg,
                                       const char *a, size_t alen,
                                       const char *b, size_t blen)
{
    CRYPTODB_UNUSED(arg);
    int n = (alen < blen) ? alen : blen;
    int r = memcmp(a, b, n);
    if (r == 0)
    {
        if (alen < blen)
            r = -1;
        else if (alen > blen)
            r = +1;
    }
    return r;
}

static const char * _cryptodb_comparator_name(void *arg)
{
    CRYPTODB_UNUSED(arg);
    return "cryptodb_comparator";
}

static inline int _leveldb_err_to_cryptodb_err(char *err)
{
    if      (strstr(err, "OK"))
        return CRYPTODB_ERR_OK;
    else if (strstr(err, "Invalid argument"))
        return CRYPTODB_ERR_WRONG_ARGUMENT;
    else
        return CRYPTODB_ERR_FAIL;
}

static char * _cryptodb_val_to_json(cryptodb_val_t valtype, void *val)
{
    char *cjson = NULL;
    cJSON *json = cJSON_CreateObject();
    if (json == NULL)
        return NULL;

    switch (valtype)
    {
    default:
        return NULL;
    case CRYPTODB_VAL_STRING:
        cJSON_AddStringToObject(json, "type", "string");
        cJSON_AddStringToObject(json, "val", (const char *)val);
        break;
    case CRYPTODB_VAL_NUM_INT:
        cJSON_AddStringToObject(json, "type", "int");
        cJSON_AddNumberToObject(json, "val", (double)*((int *)val));
        break;
    case CRYPTODB_VAL_NUM_DOUBLE:
        cJSON_AddStringToObject(json, "type", "double");
        cJSON_AddNumberToObject(json, "val", *((double *)val));
        break;
    }

    cjson = cJSON_Print(json);
    cJSON_Delete(json);
    if (cjson == NULL)
        return cjson;

    cJSON_Minify(cjson);

    return cjson;
}

static cryptodb_val_t _cryptodb_json_to_valtype(char *cjson, size_t *vallen)
{
    cryptodb_val_t result = CRYPTODB_VAL_UNKNOWN;

    cJSON *json = cJSON_Parse((const char *)cjson);
    if (json == NULL                       ||
        !cJSON_HasObjectItem(json, "type") ||
        !cJSON_HasObjectItem(json, "val"))
    {
        if (json)
            cJSON_Delete(json);
        return CRYPTODB_VAL_UNKNOWN;
    }

    char *type_str = cJSON_GetStringValue(cJSON_GetObjectItem(json, "type"));
    if      (!strcmp(type_str, "string"))
    {
        *vallen = strlen(cJSON_GetStringValue(cJSON_GetObjectItem(json, "val"))) + 1;
        result = CRYPTODB_VAL_STRING;
    }
    else if (!strcmp(type_str, "int"))
    {
        *vallen = sizeof(int);
        result = CRYPTODB_VAL_NUM_INT;
    }
    else if (!strcmp(type_str, "double"))
    {
        *vallen = sizeof(double);
        result = CRYPTODB_VAL_NUM_DOUBLE;
    }

    cJSON_Delete(json);

    return result;
}

static int _cryptodb_json_to_val(char *cjson, void *val)
{
    cJSON *json = cJSON_Parse((const char *)cjson);
    if (json == NULL                       ||
        !cJSON_HasObjectItem(json, "type") ||
        !cJSON_HasObjectItem(json, "val"))
    {
        if (json)
            cJSON_Delete(json);
        return CRYPTODB_ERR_FAIL;
    }
    if (cJSON_IsString(cJSON_GetObjectItem(json, "val")))
        strcpy((char *)val, cJSON_GetStringValue(cJSON_GetObjectItem(json, "val")));
    else
        *((double *)val) = cJSON_GetNumberValue(cJSON_GetObjectItem(json, "val"));
    cJSON_Delete(json);
    return CRYPTODB_ERR_OK;
}

static int _cryptodb_get_encryption_key_iv(cryptodb_t *cryptodb,
                                           bool encrypt_decrypt,
                                           uint8_t encryption_key[32],
                                           uint8_t encryption_iv[16])
{
    int result = CRYPTODB_SUCCESS;
    uint8_t sha512[64] = {0}, encryption[48] = {0};

    if (cryptodb->use_keys_instead_of_uniq_data)
    {
        memcpy(encryption_key, cryptodb->uniq_data, 32);
        memcpy(encryption_iv, cryptodb->uniq_data + 32, 16);
        return CRYPTODB_SUCCESS;
    }

    memset(sha512, 0, 64);
    memset(encryption, 0, 48);

    result = mbedtls_sha3(MBEDTLS_SHA3_512,
                          cryptodb->uniq_data,
                          cryptodb->uniq_data_len,
                          sha512, 64);
    if (result)
    {
        if (encrypt_decrypt)
            return CRYPTODB_ERR_ENCRYPTION_FAIL;
        else
            return CRYPTODB_ERR_DECRYPTION_FAIL;
    }

    result = scprng_rand_numbers((uint32_t *)encryption, 12, sha512);
    if (result)
    {
        if (encrypt_decrypt)
            return CRYPTODB_ERR_ENCRYPTION_FAIL;
        else
            return CRYPTODB_ERR_DECRYPTION_FAIL;
    }

    memcpy(encryption_key, encryption, 32);
    memcpy(encryption_iv, encryption + 32, 16);

    return CRYPTODB_SUCCESS;
}

static int _cryptodb_aes_256_cbc(char *in, char *out, size_t size,
                                 bool encrypt_decrypt,
                                 uint8_t encryption_key[32],
                                 uint8_t encryption_iv[16])
{
    uint8_t iv[16] = {0};
    mbedtls_aes_context ctx;
    int result = CRYPTODB_SUCCESS;

    mbedtls_aes_init(&ctx);

    if (encrypt_decrypt)
        result = mbedtls_aes_setkey_enc(&ctx,
                 (const unsigned char *)encryption_key,
                                        256);
    else
        result = mbedtls_aes_setkey_dec(&ctx,
                 (const unsigned char *)encryption_key,
                                        256);
    if (result)
    {
        mbedtls_aes_free(&ctx);
        if (encrypt_decrypt)
            return CRYPTODB_ERR_ENCRYPTION_FAIL;
        else
            return CRYPTODB_ERR_DECRYPTION_FAIL;
    }

    memcpy(iv, encryption_iv, 16);

    result = mbedtls_aes_crypt_cbc(&ctx,
                                   encrypt_decrypt ?
                                   MBEDTLS_AES_ENCRYPT :
                                   MBEDTLS_AES_DECRYPT,
                                   size,
                  (unsigned char *)iv,
            (const unsigned char *)in,
                  (unsigned char *)out);

    mbedtls_aes_free(&ctx);

    if (result)
    {
        if (encrypt_decrypt)
            return CRYPTODB_ERR_ENCRYPTION_FAIL;
        else
            return CRYPTODB_ERR_DECRYPTION_FAIL;
    }

    return CRYPTODB_SUCCESS;
}

static int _cryptodb_open(const char *path,
                          uint8_t uniq_data[CRYPTODB_UNIQ_DATA_MAX_LEN],
                          size_t uniq_data_len,
                          cryptodb_options_t *options,
                          cryptodb_user_kdf user_kdf,
                          void *kdf_user_data,
                          cryptodb_t *cryptodb,
                          bool use_keys_instead_of_uniq_data)
{
    int result = CRYPTODB_SUCCESS;

    char *err = NULL;
    leveldb_t *db = NULL;
    leveldb_env_t *env = NULL;
    leveldb_cache_t *cache = NULL;
    leveldb_comparator_t *cmp = NULL;
    leveldb_options_t *dboptions = NULL;
    leveldb_readoptions_t *roptions = NULL;
    leveldb_writeoptions_t *woptions = NULL;

    if (path == NULL || cryptodb == NULL || uniq_data == NULL)
        return CRYPTODB_ERR_NULL_POINTER;
    if (!strlen(path) || !uniq_data_len)
        return CRYPTODB_ERR_WRONG_ARGUMENT;

    cryptodb_close(cryptodb);

    dboptions = leveldb_options_create();
    roptions  = leveldb_readoptions_create();
    env       = leveldb_create_default_env();
    woptions  = leveldb_writeoptions_create();
    cmp       = leveldb_comparator_create(NULL,
                                _cryptodb_comparator_destroy,
                                _cryptodb_comparator_compare,
                                _cryptodb_comparator_name);
    cache     = leveldb_cache_create_lru(options ?
                                options->cache_capacity :
                                CRYPTODB_OPT_DEFAULT_CACHE_SIZE);
    if (!dboptions || !roptions || !env || !woptions || !cache || !cmp)
    {
        if (env)
            leveldb_env_destroy(env);
        if (cache)
            leveldb_cache_destroy(cache);
        if (cmp)
            leveldb_comparator_destroy(cmp);
        if (dboptions)
            leveldb_options_destroy(dboptions);
        if (roptions)
            leveldb_readoptions_destroy(roptions);
        if (woptions)
            leveldb_writeoptions_destroy(woptions);
        return CRYPTODB_ERR_ALLOCATE_MEM;
    }

    leveldb_options_set_env(dboptions, env);
    leveldb_options_set_cache(dboptions, cache);
    leveldb_options_set_info_log(dboptions, NULL);
    leveldb_options_set_comparator(dboptions, cmp);
    leveldb_options_set_paranoid_checks(dboptions, 1);
    leveldb_options_set_create_if_missing(dboptions, 1);
    leveldb_options_set_compression(dboptions, leveldb_no_compression);

    leveldb_options_set_block_size(dboptions,
                                   options ?
                                   options->block_size :
                                   CRYPTODB_OPT_DEFAULT_BLOCK_SIZE);
    leveldb_options_set_max_open_files(dboptions,
                                       options ?
                                       options->max_open_files :
                                       CRYPTODB_OPT_DEFAULT_MAX_FILES);
    leveldb_options_set_max_file_size(dboptions,
                                      options ?
                                      options->max_file_size :
                                      CRYPTODB_OPT_DEFAULT_MAX_FILE_SIZE);
    leveldb_options_set_write_buffer_size(dboptions,
                                          options ?
                                          options->write_buffer_size :
                                          CRYPTODB_OPT_DEFAULT_WR_BUF_SIZE);
    leveldb_options_set_block_restart_interval(dboptions,
                                               options ?
                                               options->block_restart_interval :
                                               CRYPTODB_OPT_DEFAULT_BLOCK_RE_INT);

    leveldb_writeoptions_set_sync(woptions, 1);
    leveldb_readoptions_set_fill_cache(roptions, 1);
    leveldb_readoptions_set_verify_checksums(roptions, 1);

    db = leveldb_open(dboptions, path, &err);
    if ((db == NULL) || err)
    {
        if (err)
        {
            result = _leveldb_err_to_cryptodb_err(err);
            leveldb_free(err);
        }
        if (result == CRYPTODB_ERR_OK && !db)
            result = CRYPTODB_ERR_FAIL;
        if (result != CRYPTODB_ERR_OK)
        {
            if (db)
                leveldb_close(db);
            leveldb_env_destroy(env);
            leveldb_cache_destroy(cache);
            leveldb_comparator_destroy(cmp);
            leveldb_options_destroy(dboptions);
            leveldb_readoptions_destroy(roptions);
            leveldb_writeoptions_destroy(woptions);
        }
    }

    cryptodb->db = db;
    cryptodb->env = env;
    cryptodb->cmp = cmp;
    cryptodb->cache = cache;
    cryptodb->options = dboptions;
    cryptodb->roptions = roptions;
    cryptodb->woptions = woptions;
    cryptodb->user_kdf = user_kdf;
    cryptodb->kdf_user_data = kdf_user_data;
    cryptodb->uniq_data_len = uniq_data_len;
    cryptodb->use_keys_instead_of_uniq_data = use_keys_instead_of_uniq_data;
    cryptodb->disable_keys_encryption = options ?
                                        options->disable_keys_encryption : 0;
    memcpy(cryptodb->uniq_data, uniq_data, uniq_data_len);

    return result; 
}

/**
 * PUBLIC API
 */

const char * cryptodb_err_to_str(cryptodb_err_t err)
{
    switch (err)
    {
    default:
        break;
    case CRYPTODB_ERR_OK:
        return "Success";
    case CRYPTODB_ERR_NULL_POINTER:
        return "NULL pointer";
    case CRYPTODB_ERR_ALLOCATE_MEM:
        return "Failed to allocate enough memory";
    case CRYPTODB_ERR_WRONG_ARGUMENT:
        return "Wrong argument was provided";
    case CRYPTODB_ERR_ENCRYPTION_FAIL:
        return "Encryption operation was failed";
    case CRYPTODB_ERR_DECRYPTION_FAIL:
        return "Decryption operation was failed";
    case CRYPTODB_ERR_FAIL:
        return "Fail";
    }

    return "Unknown error";
}

const char * cryptodb_val_to_str(cryptodb_val_t val)
{
    switch (val)
    {
    default:
        break;
    case CRYPTODB_VAL_STRING:
        return "String";
    case CRYPTODB_VAL_NUM_INT:
        return "Integer number";
    case CRYPTODB_VAL_NUM_DOUBLE:
        return "Double-precision floating-point number";
    }

    return "Unknown value type";
}

int cryptodb_open(const char *path,
                  uint8_t uniq_data[CRYPTODB_UNIQ_DATA_MAX_LEN],
                  size_t uniq_data_len,
                  cryptodb_options_t *options,
                  cryptodb_user_kdf user_kdf,
                  void *kdf_user_data,
                  cryptodb_t *cryptodb)
{
    return _cryptodb_open(path, uniq_data, uniq_data_len, options,
                          user_kdf, kdf_user_data, cryptodb, false);
}

int cryptodb_open_with_keys(const char *path,
                            uint8_t encryption_key[32],
                            uint8_t encryption_iv[16],
                            cryptodb_options_t *options,
                            cryptodb_t *cryptodb)
{
    uint8_t uniq_data[CRYPTODB_UNIQ_DATA_MAX_LEN];

    if (encryption_key == NULL || encryption_iv == NULL)
        return CRYPTODB_ERR_NULL_POINTER;

    memset(uniq_data, 0, CRYPTODB_UNIQ_DATA_MAX_LEN);
    memcpy(uniq_data, encryption_key, 32);
    memcpy(uniq_data + 32, encryption_iv, 16);

    return _cryptodb_open(path, uniq_data, 48, options, NULL, NULL, cryptodb, true);
}

void cryptodb_close(cryptodb_t *cryptodb)
{
    if (cryptodb)
    {
        if (cryptodb->db)
        {
            leveldb_close(cryptodb->db);
            cryptodb->db = NULL;
        }
        if (cryptodb->env)
        {
            leveldb_env_destroy(cryptodb->env);
            cryptodb->env = NULL;
        }
        if (cryptodb->cmp)
        {
            leveldb_comparator_destroy(cryptodb->cmp);
            cryptodb->cmp = NULL;
        }
        if (cryptodb->cache)
        {
            leveldb_cache_destroy(cryptodb->cache);
            cryptodb->cache = NULL;
        }
        if (cryptodb->options)
        {
            leveldb_options_destroy(cryptodb->options);
            cryptodb->options = NULL;
        }
        if (cryptodb->roptions)
        {
            leveldb_readoptions_destroy(cryptodb->roptions);
            cryptodb->roptions = NULL;
        }
        if (cryptodb->woptions)
        {
            leveldb_writeoptions_destroy(cryptodb->woptions);
            cryptodb->woptions = NULL;
        }
        cryptodb->uniq_data_len = 0;
        memset(cryptodb->uniq_data, 0, CRYPTODB_UNIQ_DATA_MAX_LEN);
    }
}

int cryptodb_put(cryptodb_t *cryptodb,
                 const char* key, size_t keylen,
                 cryptodb_val_t valtype, void *val)
{
    uint8_t encryption_key[32] = {0}, encryption_iv[16] = {0};
    int result = CRYPTODB_SUCCESS, encrypt_len = 0, encrypt_key_len = 0;
    char *err = NULL, *cjson = NULL, *encrypt = NULL, *encrypt_key = NULL;

    if (cryptodb == NULL || key == NULL || val == NULL ||
        cryptodb->db == NULL || cryptodb->woptions == NULL)
        return CRYPTODB_ERR_NULL_POINTER;
    if (!keylen)
        return CRYPTODB_ERR_WRONG_ARGUMENT;

    switch (valtype)
    {
    default:
        return CRYPTODB_ERR_WRONG_ARGUMENT;
    case CRYPTODB_VAL_STRING:
    case CRYPTODB_VAL_NUM_INT:
    case CRYPTODB_VAL_NUM_DOUBLE:
        cjson = _cryptodb_val_to_json(valtype, val);
        if (cjson == NULL)
            return CRYPTODB_ERR_FAIL;
        break;
    }

    if (cryptodb->user_kdf)
        result = cryptodb->user_kdf(cryptodb,
                                    true,
                                    encryption_key,
                                    encryption_iv,
                                    cryptodb->kdf_user_data);
    else
        result = _cryptodb_get_encryption_key_iv(cryptodb,
                                                 true,
                                                 encryption_key,
                                                 encryption_iv);
    if (result != CRYPTODB_ERR_OK)
    {
        cJSON_free(cjson);
        return result;
    }

    encrypt_len = strlen((const char *)cjson) + 1;
    while (encrypt_len % CRYPTODB_AES_BLOCK_LEN != 0)
        ++encrypt_len;

    encrypt = (char *)calloc(encrypt_len, sizeof(char));
    if (encrypt == NULL)
    {
        cJSON_free(cjson);
        return CRYPTODB_ERR_ALLOCATE_MEM;
    }
    memcpy(encrypt, cjson, strlen((const char *)cjson) + 1);
    cJSON_free(cjson);

    result = _cryptodb_aes_256_cbc(encrypt, encrypt,
                                   encrypt_len,
                                   true,
                                   encryption_key,
                                   encryption_iv);
    if (result != CRYPTODB_ERR_OK)
        return result;

    if (!cryptodb->disable_keys_encryption)
    {
        encrypt_key_len = keylen;
        while (encrypt_key_len % CRYPTODB_AES_BLOCK_LEN != 0)
            ++encrypt_key_len;

        encrypt_key = (char *)calloc(encrypt_key_len, sizeof(char));
        if (encrypt_key == NULL)
        {
            free(encrypt);
            return CRYPTODB_ERR_ALLOCATE_MEM;
        }
        memcpy(encrypt_key, key, keylen);

        result = _cryptodb_aes_256_cbc(encrypt_key,
                                       encrypt_key,
                                       encrypt_key_len,
                                       true,
                                       encryption_key,
                                       encryption_iv);
        if (result != CRYPTODB_ERR_OK)
        {
            free(encrypt);
            free(encrypt_key);
            return result;
        }
    }

    leveldb_put(cryptodb->db,
                cryptodb->woptions,
                cryptodb->disable_keys_encryption ?
                key : encrypt_key,
                cryptodb->disable_keys_encryption ?
                keylen : encrypt_key_len,
                (const char *)encrypt,
                (size_t)encrypt_len,
                &err);
    free(encrypt);
    if (!cryptodb->disable_keys_encryption)
        free(encrypt_key);
    if (err)
    {
        result = _leveldb_err_to_cryptodb_err(err);
        leveldb_free(err);
    }

    return result;
}

inline int cryptodb_put_string(cryptodb_t *cryptodb,
                               const char* key, size_t keylen, const char *val)
{
    return cryptodb_put(cryptodb, key, keylen, CRYPTODB_VAL_STRING, (void *)val);
}

inline int cryptodb_put_integer(cryptodb_t *cryptodb,
                                const char* key, size_t keylen, int val)
{
    return cryptodb_put(cryptodb, key, keylen, CRYPTODB_VAL_NUM_INT, (void *)&val);
}

inline int cryptodb_put_double(cryptodb_t *cryptodb,
                               const char* key, size_t keylen, double val)
{
    return cryptodb_put(cryptodb, key, keylen, CRYPTODB_VAL_NUM_DOUBLE, (void *)&val);
}

int cryptodb_get(cryptodb_t *cryptodb,
                 const char* key, size_t keylen,
                 cryptodb_val_t valtype, void *val)
{
    int val_int = 0;
    char *val_str = NULL;
    double val_double = 0;

    void *cval = NULL;
    size_t vallen = 0, cvallen = 0;
    cryptodb_val_t cvaltype = CRYPTODB_VAL_UNKNOWN;
    uint8_t encryption_key[32] = {0}, encryption_iv[16] = {0};
    int result = CRYPTODB_SUCCESS, decrypt_len = 0, encrypt_key_len = 0;
    char *err = NULL, *str = NULL, *decrypt = NULL, *encrypt_key = NULL;

    if (cryptodb == NULL || key == NULL || val == NULL ||
        cryptodb->db == NULL || cryptodb->roptions == NULL)
        return CRYPTODB_ERR_NULL_POINTER;
    if (!keylen)
        return CRYPTODB_ERR_WRONG_ARGUMENT;

    if (cryptodb->user_kdf)
        result = cryptodb->user_kdf(cryptodb,
                                    false,
                                    encryption_key,
                                    encryption_iv,
                                    cryptodb->kdf_user_data);
    else
        result = _cryptodb_get_encryption_key_iv(cryptodb,
                                                 false,
                                                 encryption_key,
                                                 encryption_iv);
    if (result != CRYPTODB_ERR_OK)
        return result;

    if (!cryptodb->disable_keys_encryption)
    {
        encrypt_key_len = keylen;
        while (encrypt_key_len % CRYPTODB_AES_BLOCK_LEN != 0)
            ++encrypt_key_len;

        encrypt_key = (char *)calloc(encrypt_key_len, sizeof(char));
        if (encrypt_key == NULL)
            return CRYPTODB_ERR_ALLOCATE_MEM;
        memcpy(encrypt_key, key, keylen);

        result = _cryptodb_aes_256_cbc(encrypt_key,
                                       encrypt_key,
                                       encrypt_key_len,
                                       true,
                                       encryption_key,
                                       encryption_iv);
        if (result != CRYPTODB_ERR_OK)
        {
            free(encrypt_key);
            return result;
        }
    }

    str = leveldb_get(cryptodb->db,
                      cryptodb->roptions,
                      cryptodb->disable_keys_encryption ?
                      key : encrypt_key,
                      cryptodb->disable_keys_encryption ?
                      keylen : encrypt_key_len,
                      &vallen, &err);
    if (!cryptodb->disable_keys_encryption)
        free(encrypt_key);
    if ((str == NULL) || err)
    {
        if (err)
        {
            result = _leveldb_err_to_cryptodb_err(err);
            leveldb_free(err);
        }
        if (result == CRYPTODB_ERR_OK && !str)
            result = CRYPTODB_ERR_FAIL;
        if (result != CRYPTODB_ERR_OK && str)
            leveldb_free(str);
    }
    if (result != CRYPTODB_ERR_OK)
        return result;
    if (!vallen)
    {
        leveldb_free(str);
        return CRYPTODB_ERR_FAIL;
    }

    decrypt_len = vallen;
    if (decrypt_len % CRYPTODB_AES_BLOCK_LEN != 0)
    {
        leveldb_free(str);
        return CRYPTODB_ERR_FAIL;
    }
    decrypt = (char *)calloc(decrypt_len, sizeof(char));
    if (decrypt == NULL)
    {
        leveldb_free(str);
        return CRYPTODB_ERR_ALLOCATE_MEM;
    }

    result = _cryptodb_aes_256_cbc(str,
                                   decrypt,
                                   decrypt_len,
                                   false,
                                   encryption_key,
                                   encryption_iv);
    leveldb_free(str);
    if (result != CRYPTODB_ERR_OK)
    {
        free(decrypt);
        return result;
    }

    cvaltype = _cryptodb_json_to_valtype(decrypt, &cvallen);
    if (cvaltype == CRYPTODB_VAL_UNKNOWN || !cvallen)
    {
        free(decrypt);
        return CRYPTODB_ERR_FAIL;
    }
    if (cvaltype != valtype)
    {
        free(decrypt);
        return (int)cvaltype;
    }
    if (cvaltype == CRYPTODB_VAL_STRING)
    {
        val_str = (char *)calloc(cvallen, sizeof(char));
        if (val_str == NULL)
        {
            free(decrypt);
            return CRYPTODB_ERR_ALLOCATE_MEM;
        }
    }

    switch (cvaltype)
    {
    default:
        free(decrypt);
        return CRYPTODB_ERR_FAIL;
    case CRYPTODB_VAL_STRING:
        result = _cryptodb_json_to_val(decrypt, (void *)val_str);
        if (result != CRYPTODB_ERR_OK)
        {
            free(val_str);
            free(decrypt);
            return result;
        }
        cval = (void *)val_str;
        break;
    case CRYPTODB_VAL_NUM_INT:
        result = _cryptodb_json_to_val(decrypt, (void *)&val_double);
        if (result != CRYPTODB_ERR_OK)
        {
            free(decrypt);
            return result;
        }
        val_int = (int)val_double;
        cval = (void *)&val_int;
        break;
    case CRYPTODB_VAL_NUM_DOUBLE:
        result = _cryptodb_json_to_val(decrypt, (void *)&val_double);
        if (result != CRYPTODB_ERR_OK)
        {
            free(decrypt);
            return result;
        }
        cval = (void *)&val_double;
        break;
    }

    memcpy(val, cval, cvallen);

    free(decrypt);
    if (val_str)
        free(val_str);

    return result;
}

int cryptodb_delete(cryptodb_t *cryptodb,
                    const char* key, size_t keylen)
{
    char *err = NULL, *encrypt_key = NULL;
    int result = CRYPTODB_SUCCESS, encrypt_key_len = 0;
    uint8_t encryption_key[32] = {0}, encryption_iv[16] = {0};

    if (cryptodb == NULL     || key == NULL ||
        cryptodb->db == NULL || cryptodb->woptions == NULL)
        return CRYPTODB_ERR_NULL_POINTER;
    if (!keylen)
        return CRYPTODB_ERR_WRONG_ARGUMENT;

    if (cryptodb->user_kdf)
        result = cryptodb->user_kdf(cryptodb,
                                    true,
                                    encryption_key,
                                    encryption_iv,
                                    cryptodb->kdf_user_data);
    else
        result = _cryptodb_get_encryption_key_iv(cryptodb,
                                                 true,
                                                 encryption_key,
                                                 encryption_iv);
    if (result != CRYPTODB_ERR_OK)
        return result;

    if (!cryptodb->disable_keys_encryption)
    {
        encrypt_key_len = keylen;
        while (encrypt_key_len % CRYPTODB_AES_BLOCK_LEN != 0)
            ++encrypt_key_len;

        encrypt_key = (char *)calloc(encrypt_key_len, sizeof(char));
        if (encrypt_key == NULL)
            return CRYPTODB_ERR_ALLOCATE_MEM;
        memcpy(encrypt_key, key, keylen);

        result = _cryptodb_aes_256_cbc(encrypt_key,
                                       encrypt_key,
                                       encrypt_key_len,
                                       true,
                                       encryption_key,
                                       encryption_iv);
        if (result != CRYPTODB_ERR_OK)
        {
            free(encrypt_key);
            return result;
        }
    }

    leveldb_delete(cryptodb->db,
                   cryptodb->woptions,
                   cryptodb->disable_keys_encryption ?
                   key : encrypt_key,
                   cryptodb->disable_keys_encryption ?
                   keylen : encrypt_key_len,
                   &err);
    if (!cryptodb->disable_keys_encryption)
        free(encrypt_key);
    if (err)
    {
        result = _leveldb_err_to_cryptodb_err(err);
        leveldb_free(err);
    }

    return result;
}

int cryptodb_destroy(const char *path,
                     cryptodb_options_t *options)
{
    int result = CRYPTODB_SUCCESS;

    char *err = NULL;
    leveldb_env_t *env = NULL;
    leveldb_cache_t *cache = NULL;
    leveldb_comparator_t *cmp = NULL;
    leveldb_options_t *dboptions = NULL;
    leveldb_readoptions_t *roptions = NULL;
    leveldb_writeoptions_t *woptions = NULL;

    if (path == NULL)
        return CRYPTODB_ERR_NULL_POINTER;
    if (!strlen(path))
        return CRYPTODB_ERR_WRONG_ARGUMENT;

    dboptions = leveldb_options_create();
    roptions  = leveldb_readoptions_create();
    env       = leveldb_create_default_env();
    woptions  = leveldb_writeoptions_create();
    cmp       = leveldb_comparator_create(NULL,
                                _cryptodb_comparator_destroy,
                                _cryptodb_comparator_compare,
                                _cryptodb_comparator_name);
    cache     = leveldb_cache_create_lru(options ?
                                options->cache_capacity :
                                CRYPTODB_OPT_DEFAULT_CACHE_SIZE);
    if (!dboptions || !roptions || !env || !woptions || !cache || !cmp)
    {
        if (env)
            leveldb_env_destroy(env);
        if (cache)
            leveldb_cache_destroy(cache);
        if (cmp)
            leveldb_comparator_destroy(cmp);
        if (dboptions)
            leveldb_options_destroy(dboptions);
        if (roptions)
            leveldb_readoptions_destroy(roptions);
        if (woptions)
            leveldb_writeoptions_destroy(woptions);
        return CRYPTODB_ERR_ALLOCATE_MEM;
    }

    leveldb_options_set_env(dboptions, env);
    leveldb_options_set_cache(dboptions, cache);
    leveldb_options_set_info_log(dboptions, NULL);
    leveldb_options_set_comparator(dboptions, cmp);
    leveldb_options_set_paranoid_checks(dboptions, 1);
    leveldb_options_set_create_if_missing(dboptions, 1);
    leveldb_options_set_compression(dboptions, leveldb_no_compression);

    leveldb_options_set_block_size(dboptions,
                                   options ?
                                   options->block_size :
                                   CRYPTODB_OPT_DEFAULT_BLOCK_SIZE);
    leveldb_options_set_max_open_files(dboptions,
                                       options ?
                                       options->max_open_files :
                                       CRYPTODB_OPT_DEFAULT_MAX_FILES);
    leveldb_options_set_max_file_size(dboptions,
                                      options ?
                                      options->max_file_size :
                                      CRYPTODB_OPT_DEFAULT_MAX_FILE_SIZE);
    leveldb_options_set_write_buffer_size(dboptions,
                                          options ?
                                          options->write_buffer_size :
                                          CRYPTODB_OPT_DEFAULT_WR_BUF_SIZE);
    leveldb_options_set_block_restart_interval(dboptions,
                                               options ?
                                               options->block_restart_interval :
                                               CRYPTODB_OPT_DEFAULT_BLOCK_RE_INT);

    leveldb_destroy_db(dboptions, path, &err);
    if (err)
    {
        result = _leveldb_err_to_cryptodb_err(err);
        leveldb_free(err);
    }

    leveldb_env_destroy(env);
    leveldb_cache_destroy(cache);
    leveldb_comparator_destroy(cmp);
    leveldb_options_destroy(dboptions);
    leveldb_readoptions_destroy(roptions);
    leveldb_writeoptions_destroy(woptions);

    return result;
}
