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

#include <jni.h>

#include <mutex>
#include <cstdlib>
#include <cstdint>
#include <cstdbool>

#include "cryptodb.hpp"

using namespace std;
using namespace cryptodb;

static mutex db_mutex;
static CryptoDB *db = nullptr;
static cryptodb_options_t db_options;

static bool is_dboptions_initialized(void)
{
    return db_options.block_size &&
           db_options.max_file_size &&
           db_options.max_open_files &&
           db_options.cache_capacity &&
           db_options.write_buffer_size &&
           db_options.block_restart_interval;
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_yahniukov_cryptodb_CryptoDB_getUniqDataMaxLen(
        JNIEnv* env,
        jclass thiz) {
    return (jlong)CRYPTODB_UNIQ_DATA_MAX_LEN;
}

extern "C" JNIEXPORT jint JNICALL
        Java_com_yahniukov_cryptodb_CryptoDB_getVersionMajor(
                JNIEnv* env,
                jclass thiz) {
    return (jint)CRYPTODB_VER_MAJOR;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_yahniukov_cryptodb_CryptoDB_getVersionMinor(
        JNIEnv* env,
        jclass thiz) {
    return (jint)CRYPTODB_VER_MINOR;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_yahniukov_cryptodb_CryptoDB_getVersionRevision(
        JNIEnv* env,
        jclass thiz) {
    return (jint)CRYPTODB_VER_REVISION;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_yahniukov_cryptodb_CryptoDB_errorToStr(
        JNIEnv* env,
        jclass thiz,
        jint err) {
    return env->NewStringUTF(CryptoDB::ErrorToStr((cryptodb_err_t)err).c_str());
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_yahniukov_cryptodb_CryptoDB_valueToStr(
        JNIEnv* env,
        jclass thiz,
        jint val) {
    return env->NewStringUTF(CryptoDB::ValueToStr((cryptodb_val_t)val).c_str());
}

extern "C" JNIEXPORT void JNICALL
Java_com_yahniukov_cryptodb_CryptoDB_setDBOptions(
        JNIEnv* env,
        jclass thiz,
        jlong cache_capacity,
        jlong write_buffer_size,
        jint max_open_files,
        jlong block_size,
        jint block_restart_interval,
        jlong max_file_size,
        jint disable_keys_encryption) {
    db_options.block_size = (size_t)block_size;
    db_options.max_open_files = (int)max_open_files;
    db_options.max_file_size = (size_t)max_file_size;
    db_options.cache_capacity = (size_t)cache_capacity;
    db_options.write_buffer_size = (size_t)write_buffer_size;
    db_options.block_restart_interval = (int)block_restart_interval;
    db_options.disable_keys_encryption = (int)disable_keys_encryption;
}

extern "C" JNIEXPORT void JNICALL
Java_com_yahniukov_cryptodb_CryptoDB_setDBOptionsToDefault(
        JNIEnv* env,
        jclass thiz) {
    db_options.block_size = (size_t)CRYPTODB_OPT_DEFAULT_BLOCK_SIZE;
    db_options.max_open_files = (int)CRYPTODB_OPT_DEFAULT_MAX_FILES;
    db_options.cache_capacity = (size_t)CRYPTODB_OPT_DEFAULT_CACHE_SIZE;
    db_options.max_file_size = (size_t)CRYPTODB_OPT_DEFAULT_MAX_FILE_SIZE;
    db_options.write_buffer_size = (size_t)CRYPTODB_OPT_DEFAULT_WR_BUF_SIZE;
    db_options.block_restart_interval = (int)CRYPTODB_OPT_DEFAULT_BLOCK_RE_INT;
    db_options.disable_keys_encryption = 0;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_yahniukov_cryptodb_CryptoDB_open(
        JNIEnv* env,
        jclass thiz,
        jstring path,
        jbyteArray uniq_data,
        jlong uniq_data_len) {
    if (!db_mutex.try_lock() || !is_dboptions_initialized())
        return (jint)CRYPTODB_ERR_FAIL;

    char *path_p = (char *)env->GetStringUTFChars(path, 0);

    jint result = (jint)CryptoDB::Open(path_p,
                                       (uint8_t *)uniq_data,
                                       (size_t)uniq_data_len,
                                       &db_options,
                                       NULL,
                                       NULL,
                                       &db);

    env->ReleaseStringUTFChars(path, path_p);

    return result;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_yahniukov_cryptodb_CryptoDB_openWithKeys(
        JNIEnv* env,
        jclass thiz,
        jstring path,
        jbyteArray encryption_key,
        jbyteArray encryption_iv) {
    if (!db_mutex.try_lock() || !is_dboptions_initialized())
        return (jint)CRYPTODB_ERR_FAIL;

    char *path_p = (char *)env->GetStringUTFChars(path, 0);

    jint result = (jint)CryptoDB::OpenWithKeys(path_p,
                                               (uint8_t *)encryption_key,
                                               (uint8_t *)encryption_iv,
                                               &db_options,
                                               &db);

    env->ReleaseStringUTFChars(path, path_p);

    return result;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_yahniukov_cryptodb_CryptoDB_destroy(
        JNIEnv* env,
        jclass thiz,
        jstring path) {
    if (!db_mutex.try_lock() || !is_dboptions_initialized())
        return (jint)CRYPTODB_ERR_FAIL;

    char *path_p = (char *)env->GetStringUTFChars(path, 0);

    jint result = (jint)CryptoDB::Destroy(path_p, &db_options);

    env->ReleaseStringUTFChars(path, path_p);

    return result;
}

extern "C" JNIEXPORT void JNICALL
Java_com_yahniukov_cryptodb_CryptoDB_close(
        JNIEnv* env,
        jclass thiz) {
    if (db != nullptr) {
        db->Close();
        delete db;
        db = nullptr;
    }
    db_mutex.unlock();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_yahniukov_cryptodb_CryptoDB_putString(
        JNIEnv* env,
        jclass thiz,
        jstring key,
        jstring val) {
    char *key_p = (char *)env->GetStringUTFChars(key, 0);
    char *val_p = (char *)env->GetStringUTFChars(val, 0);

    jint result = (jint)db->PutString(string(key_p),
                                      string(val_p));

    env->ReleaseStringUTFChars(key, key_p);
    env->ReleaseStringUTFChars(val, val_p);

    return result;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_yahniukov_cryptodb_CryptoDB_putInteger(
        JNIEnv* env,
        jclass thiz,
        jstring key,
        jint val) {
    char *key_p = (char *)env->GetStringUTFChars(key, 0);

    jint result = (jint)db->PutInteger(string(key_p), (int)val);

    env->ReleaseStringUTFChars(key, key_p);

    return result;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_yahniukov_cryptodb_CryptoDB_putDouble(
        JNIEnv* env,
        jclass thiz,
        jstring key,
        jdouble val) {
    char *key_p = (char *)env->GetStringUTFChars(key, 0);

    jint result = (jint)db->PutDouble(string(key_p), (double)val);

    env->ReleaseStringUTFChars(key, key_p);

    return result;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_yahniukov_cryptodb_CryptoDB_getString(
        JNIEnv* env,
        jclass thiz,
        jstring key,
        jint expected_max_length) {
    string *val = nullptr;

    char *key_p = (char *)env->GetStringUTFChars(key, 0);

    int err = db->GetString(string(key_p),
                            (int)expected_max_length,
                            &val);

    env->ReleaseStringUTFChars(key, key_p);

    if (err != CRYPTODB_SUCCESS || val == nullptr) {
        if (val != nullptr)
            delete val;
        return env->NewStringUTF("");
    }

    jstring result = env->NewStringUTF((*val).c_str());

    delete val;

    return result;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_yahniukov_cryptodb_CryptoDB_getInteger(
        JNIEnv* env,
        jclass thiz,
        jstring key) {
    int *val = nullptr;

    char *key_p = (char *)env->GetStringUTFChars(key, 0);

    int err = db->GetInteger(string(key_p), &val);

    env->ReleaseStringUTFChars(key, key_p);

    if (err != CRYPTODB_SUCCESS || val == nullptr) {
        if (val != nullptr)
            delete val;
        return (jint)err;
    }

    jint result = (jint)*val;

    delete val;

    return result;
}

extern "C" JNIEXPORT jdouble JNICALL
Java_com_yahniukov_cryptodb_CryptoDB_getDouble(
        JNIEnv* env,
        jclass thiz,
        jstring key) {
    double *val = nullptr;

    char *key_p = (char *)env->GetStringUTFChars(key, 0);

    int err = db->GetDouble(string(key_p), &val);

    env->ReleaseStringUTFChars(key, key_p);

    if (err != CRYPTODB_SUCCESS || val == nullptr) {
        if (val != nullptr)
            delete val;
        return (jint)err;
    }

    jdouble result = (jdouble)*val;

    delete val;

    return result;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_yahniukov_cryptodb_CryptoDB_delete(
        JNIEnv* env,
        jclass thiz,
        jstring key) {
    char *key_p = (char *)env->GetStringUTFChars(key, 0);

    int result = db->Delete(string(key_p));

    env->ReleaseStringUTFChars(key, key_p);

    return result;
}
