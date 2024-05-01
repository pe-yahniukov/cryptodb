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

#include <cmath>
#include <cfloat>
#include <cstring>
#include <iostream>

#include "cryptodb.hpp"

using namespace std;
using namespace cryptodb;

#define TEST_DB_FOLDER "db"

static bool compare_double(double a, double b)
{
    double maxVal = fabs(a) > fabs(b) ? fabs(a) : fabs(b);
    return (fabs(a - b) <= maxVal * DBL_EPSILON);
}

int main(int argc, char **argv)
{
    CryptoDB *db = nullptr;
    double *test_double = nullptr;
    std::string *test_str = nullptr;
    int err = 0, *test_int = nullptr;
    uint8_t uniq_data[CRYPTODB_UNIQ_DATA_MAX_LEN] = {0};

    memset(uniq_data, 0, CRYPTODB_UNIQ_DATA_MAX_LEN);

    /**
     * Main tests are already done in C version.
     * The main purpose of C++ test is to check
     * if the C++ interface works fine.
     */

    if (CryptoDB::ErrorToStr(CRYPTODB_ERR_FAIL).compare("Fail") ||
        CryptoDB::ErrorToStr(CRYPTODB_ERR_ALLOCATE_MEM).compare("Failed to allocate enough memory"))
    {
        cerr << "ERROR: ErrorToStr()" << endl;
        return -1;
    }

    if (CryptoDB::ValueToStr(CRYPTODB_VAL_NUM_INT).compare("Integer number") ||
        CryptoDB::ValueToStr(CRYPTODB_VAL_NUM_DOUBLE).compare("Double-precision floating-point number"))
    {
        cerr << "ERROR: ValueToStr()" << endl;
        return -1;
    }

    /**
     * With unique data test
     */

    err = CryptoDB::Open(TEST_DB_FOLDER,
                         uniq_data,
                         CRYPTODB_UNIQ_DATA_MAX_LEN,
                         NULL, NULL, NULL, &db);
    if (CRYPTODB_SUCCESS != err || db == nullptr)
    {
        if (db != nullptr)
            delete db;
        cerr << "ERROR: Open()" << endl;
        return -1;
    }

    err = db->PutString("test_key", "test_val");
    if (CRYPTODB_SUCCESS != err)
    {
        db->Close();
        delete db;
        cerr << "ERROR: PutString()" << endl;
        return -1;
    }

    err = db->GetString("test_key",
                        strlen("test_val") + 1,
                        &test_str);
    if (CRYPTODB_SUCCESS != err ||
        test_str == nullptr     ||
        test_str->compare("test_val"))
    {
        delete test_str;
        db->Close();
        delete db;
        cerr << "ERROR: GetString()" << endl;
        return -1;
    }

    delete test_str;

    err = db->GetInteger("test_key", &test_int);
    if (err != CRYPTODB_VAL_STRING || test_int != nullptr)
    {
        db->Close();
        delete db;
        cerr << "ERROR: PutString()" << endl;
        return -1;
    }

    err = db->Delete("test_key");
    if (CRYPTODB_SUCCESS != err)
    {
        db->Close();
        delete db;
        cerr << "ERROR: Delete()" << endl;
        return -1;
    }
    err = db->GetString("test_key",
                        strlen("test_val") + 1,
                        &test_str);
    if (CRYPTODB_SUCCESS == err)
    {
        delete test_str;
        db->Close();
        delete db;
        cerr << "ERROR: Delete()" << endl;
        return -1;
    }

    delete test_str;

    err = db->PutInteger("test_key", 42);
    if (CRYPTODB_SUCCESS != err)
    {
        db->Close();
        delete db;
        cerr << "ERROR: PutInteger()" << endl;
        return -1;
    }

    err = db->GetInteger("test_key", &test_int);
    if (CRYPTODB_SUCCESS != err || test_int == nullptr || *test_int != 42)
    {
        delete test_int;
        db->Close();
        delete db;
        cerr << "ERROR: PutInteger()" << endl;
        return -1;
    }

    delete test_int;

    err = db->Delete("test_key");
    if (CRYPTODB_SUCCESS != err)
    {
        db->Close();
        delete db;
        cerr << "ERROR: Delete() #2" << endl;
        return -1;
    }

    err = db->PutDouble("test_key", 42.42);
    if (CRYPTODB_SUCCESS != err)
    {
        db->Close();
        delete db;
        cerr << "ERROR: PutDouble()" << endl;
        return -1;
    }

    err = db->GetDouble("test_key", &test_double);
    if (CRYPTODB_SUCCESS != err || test_double == nullptr || !compare_double(*test_double, 42.42))
    {
        delete test_double;
        db->Close();
        delete db;
        cerr << "ERROR: PutDouble()" << endl;
        return -1;
    }

    delete test_double;

    err = db->Delete("test_key");
    if (CRYPTODB_SUCCESS != err)
    {
        db->Close();
        delete db;
        cerr << "ERROR: Delete() #3" << endl;
        return -1;
    }

    db->Close();
    delete db;
    db = nullptr;

    err = CryptoDB::Destroy(TEST_DB_FOLDER, NULL);
    if (CRYPTODB_SUCCESS != err)
    {
        cerr << "ERROR: Destroy()" << endl;
        return -1;
    }

    /**
     * With ready encryption key and IV
     */

    uint8_t key[32] = {0}, iv[16] = {0};
    memset(key, 0, 32);
    memset(iv, 0, 16);

    err = CryptoDB::OpenWithKeys(TEST_DB_FOLDER,
                                 key, iv, NULL, &db);
    if (CRYPTODB_SUCCESS != err || db == nullptr)
    {
        if (db != nullptr)
            delete db;
        cerr << "ERROR: OpenWithKeys()" << endl;
        return -1;
    }

    err = db->PutDouble("test_key", 42.42);
    if (CRYPTODB_SUCCESS != err)
    {
        db->Close();
        delete db;
        cerr << "ERROR: PutDouble() #2" << endl;
        return -1;
    }

    test_double = nullptr;

    err = db->GetDouble("test_key", &test_double);
    if (CRYPTODB_SUCCESS != err || test_double == nullptr || !compare_double(*test_double, 42.42))
    {
        delete test_double;
        db->Close();
        delete db;
        cerr << "ERROR: PutDouble()" << endl;
        return -1;
    }

    delete test_double;

    err = db->Delete("test_key");
    if (CRYPTODB_SUCCESS != err)
    {
        db->Close();
        delete db;
        cerr << "ERROR: Delete() #4" << endl;
        return -1;
    }

    db->Close();
    delete db;
    db = nullptr;

    err = CryptoDB::Destroy(TEST_DB_FOLDER, NULL);
    if (CRYPTODB_SUCCESS != err)
    {
        cerr << "ERROR: Destroy() #2" << endl;
        return -1;
    }

    cout << "PASS" << endl;

    return 0;
}
