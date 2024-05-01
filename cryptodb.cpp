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

#include <cstring>

#include "cryptodb.hpp"

namespace cryptodb {

const std::string CryptoDB::ErrorToStr(cryptodb_err_t err)
{
    return (const std::string)std::string((char *)cryptodb_err_to_str(err));
}

const std::string CryptoDB::ValueToStr(cryptodb_val_t val)
{
    return (const std::string)std::string((char *)cryptodb_val_to_str(val));
}

int CryptoDB::Open(std::string path,
                   uint8_t uniq_data[CRYPTODB_UNIQ_DATA_MAX_LEN],
                   size_t uniq_data_len,
                   cryptodb_options_t *options,
                   cryptodb_user_kdf user_kdf,
                   void *kdf_user_data,
                   CryptoDB** dbptr)
{
    *dbptr = nullptr;

    CryptoDB *db = new CryptoDB();

    int err = cryptodb_open(path.c_str(),
                            uniq_data,
                            uniq_data_len,
                            options,
                            user_kdf,
                            kdf_user_data,
                            &db->db);
    if (CRYPTODB_SUCCESS != err)
    {
        delete db;
        return err;
    }

    *dbptr = db;

    return CRYPTODB_SUCCESS;
}

int CryptoDB::OpenWithKeys(std::string path,
                           uint8_t encryption_key[32],
                           uint8_t encryption_iv[16],
                           cryptodb_options_t *options,
                           CryptoDB** dbptr)
{
    *dbptr = nullptr;

    CryptoDB *db = new CryptoDB();

    int err = cryptodb_open_with_keys(path.c_str(),
                                      encryption_key,
                                      encryption_iv,
                                      options,
                                      &db->db);
    if (CRYPTODB_SUCCESS != err)
    {
        delete db;
        return err;
    }

    *dbptr = db;

    return CRYPTODB_SUCCESS;
}

int CryptoDB::Destroy(std::string path,
                      cryptodb_options_t *options)
{
    return cryptodb_destroy(path.c_str(), options);
}

void CryptoDB::Close(void)
{
    cryptodb_close(&this->db);
}

int CryptoDB::PutString(std::string key, std::string val)
{
    return cryptodb_put_string(&this->db,
                               key.c_str(),
                               strlen(key.c_str()) + 1,
                               val.c_str());
}

int CryptoDB::PutInteger(std::string key, int val)
{
    return cryptodb_put_integer(&this->db,
                                key.c_str(),
                                strlen(key.c_str()) + 1,
                                val);
}

int CryptoDB::PutDouble(std::string key, double val)
{
    return cryptodb_put_double(&this->db,
                               key.c_str(),
                               strlen(key.c_str()) + 1,
                               val);
}

int CryptoDB::GetString(std::string key,
                        int expected_max_length,
                        std::string **val)
{
    int err = 0;
    char *value = NULL;

    if (expected_max_length <= 0)
        return CRYPTODB_ERR_WRONG_ARGUMENT;

    value = (char *)calloc(expected_max_length, sizeof(char));
    if (value == NULL)
        return CRYPTODB_ERR_ALLOCATE_MEM;

    *val = nullptr;

    err = cryptodb_get(&this->db,
                       key.c_str(),
                       strlen(key.c_str()) + 1,
                       CRYPTODB_VAL_STRING,
                       (void *)value);
    if (CRYPTODB_SUCCESS != err)
    {
        free(value);
        return err;
    }

    *val = new std::string(value);

    free(value);

    return CRYPTODB_SUCCESS;
}

int CryptoDB::GetInteger(std::string key, int **val)
{
    int err = 0, value = 0;

    *val = nullptr;

    err = cryptodb_get(&this->db,
                       key.c_str(),
                       strlen(key.c_str()) + 1,
                       CRYPTODB_VAL_NUM_INT,
                       (void *)&value);
    if (CRYPTODB_SUCCESS != err)
        return err;

    *val = new int(value);

    return CRYPTODB_SUCCESS;
}

int CryptoDB::GetDouble(std::string key, double **val)
{
    int err = 0;
    double value = 0.0;

    *val = nullptr;

    err = cryptodb_get(&this->db,
                       key.c_str(),
                       strlen(key.c_str()) + 1,
                       CRYPTODB_VAL_NUM_DOUBLE,
                       (void *)&value);
    if (CRYPTODB_SUCCESS != err)
        return err;

    *val = new double(value);

    return CRYPTODB_SUCCESS;
}

int CryptoDB::Delete(std::string key)
{
    return cryptodb_delete(&this->db,
                           key.c_str(),
                           strlen(key.c_str()) + 1);
}

} // namespace cryptodb
