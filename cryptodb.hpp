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

#include <string>

#include "cryptodb.h"

namespace cryptodb {

class CRYPTODB_EXPORT CryptoDB
{
public:
    CryptoDB()  = default;
    ~CryptoDB() = default;

    /**
     * @brief      Returns a string representation of an CryptoDB error.
     *             C++ analogue of the cryptodb_err_to_str().
     *
     * @param[in]  err   See cryptodb_err_t
     *
     * @return     String representation of the CryptoDB error.
     */
    static const std::string ErrorToStr(cryptodb_err_t err);

    /**
     * @brief      Returns a string representation of an CryptoDB value type.
     *             C++ analogue of the cryptodb_val_to_str().
     *
     * @param[in]  val   See cryptodb_val_t
     *
     * @return     String representation of the CryptoDB value type.
     */
    static const std::string ValueToStr(cryptodb_val_t val);

    /**
     * @brief      Open database that is located in specified "path" folder.
     *             The database will be created if not exist.
     *             C++ analogue of the cryptodb_open().
     *
     * @param[in]   path           The full database folder path
     * @param[in]   uniq_data      Uniq client data that will be used
     *                             to create encryption/decryption keys
     * @param[in]   uniq_data_len  "uniq_data" length
     * @param[in]   options        See cryptodb_options_t
     * @param[in]   user_kdf       (Optional, can be NULL)
     *                             See cryptodb_user_kdf. If NULL, default
     *                             _cryptodb_get_encryption_key_iv() will be
     *                             used.
     * @param[in]   kdf_user_data  (Optional, can be NULL)
     *                             user_kdf client data, see cryptodb_user_kdf
     * @param[out]  dbptr          Output db instance, should be nullptr
     *
     * @return     See cryptodb_err_t
     */
    static int Open(std::string path,
                    uint8_t uniq_data[CRYPTODB_UNIQ_DATA_MAX_LEN],
                    size_t uniq_data_len,
                    cryptodb_options_t *options,
                    cryptodb_user_kdf user_kdf,
                    void *kdf_user_data,
                    CryptoDB** dbptr);

    /**
     * @brief      Open database that is located in specified "path" folder.
     *             The database will be created if not exist.
     *             C++ analogue of the cryptodb_open_with_keys().
     *
     * @param[in]   path            The full database folder path
     * @param[in]   encryption_key  Secret encryption key for AES-256 CBC encryption
     * @param[in]   encryption_iv   Secret initialization vector for AES-256 CBC encryption
     * @param[in]   options         See cryptodb_options_t
     * @param[out]  dbptr           Output db instance, should be nullptr
     *
     * @return     See cryptodb_err_t
     */
    static int OpenWithKeys(std::string path,
                            uint8_t encryption_key[32],
                            uint8_t encryption_iv[16],
                            cryptodb_options_t *options,
                            CryptoDB** dbptr);

    /**
     * @brief      Destroy database that is located in specified "path" folder.
     *             C++ analogue of the cryptodb_destroy().
     *
     * @param[in]  path     The full database folder path
     * @param[in]  options  See cryptodb_options_t
     *
     * @return     See cryptodb_err_t
     */
    static int Destroy(std::string path,
                       cryptodb_options_t *options);

    /**
     * @brief      Close database
     *             C++ analogue of the cryptodb_close().
     */
    void Close(void);

    /**
     * @brief      Put the "key-value" entry in the database
     *             where "value" is string.
     *             C++ analogue of the cryptodb_put_string().
     *
     * @param[in]  key   The entry key
     * @param[in]  val   The entry string value
     *
     * @return     See cryptodb_err_t
     */
    int PutString(std::string key, std::string val);

    /**
     * @brief      Put the "key-value" entry in the database
     *             where "value" is integer number.
     *             C++ analogue of the cryptodb_put_integer().
     *
     * @param[in]  key   The entry key
     * @param[in]  val   The entry integer number value
     *
     * @return     See cryptodb_err_t
     */
    int PutInteger(std::string key, int val);

    /**
     * @brief      Put the "key-value" entry in the database
     *             where "value" is double-precision floating-point
     *             number.
     *             C++ analogue of the cryptodb_put_double().
     *
     * @param[in]  key   The entry key
     * @param[in]  val   The entry double-precision floating-point number value
     *
     * @return     See cryptodb_err_t
     */
    int PutDouble(std::string key, double val);

    /**
     * @brief      Get string value of the entry that is assosiated
     *             with specified key.
     *             In case if the key exists but value is not string,
     *             *val will be nullptr and function returns the
     *             actual value type - cryptodb_val_t.
     *
     * @param[in]   key                  The entry key
     * @param[in]   expected_max_length  Expected maximum length that
     *                                   string value should have.
     * @param[out]  val                  The value, should be nullptr
     *
     * @return     cryptodb_err_t or cryptodb_val_t, see @brief
     */
    int GetString(std::string key,
                  int expected_max_length,
                  std::string **val);

    /**
     * @brief      Get integer number value of the entry that is assosiated
     *             with specified key.
     *             In case if the key exists but value is not integer,
     *             *val will be nullptr and function returns the
     *             actual value type - cryptodb_val_t.
     *
     * @param[in]   key  The entry key
     * @param[out]  val  The value, should be nullptr
     *
     * @return     cryptodb_err_t or cryptodb_val_t, see @brief
     */
    int GetInteger(std::string key, int **val);

    /**
     * @brief      Get double-precision floating-point number value of
     *             the entry that is assosiated with specified key.
     *             In case if the key exists but value is not
     *             double-precision floating-point, *val will be nullptr
     *             and function returns the actual value type - cryptodb_val_t.
     *
     * @param[in]   key  The entry key
     * @param[out]  val  The value, should be nullptr
     *
     * @return     cryptodb_err_t or cryptodb_val_t, see @brief
     */
    int GetDouble(std::string key, double **val);

    /**
     * @brief      Delete entry with specified key from the database.
     *             C++ analogue of the cryptodb_delete().
     *
     * @param[in]  key   The key
     *
     * @return     See cryptodb_err_t
     */
    int Delete(std::string key);

private:
    cryptodb_t db;
};

} // namespace cryptodb
