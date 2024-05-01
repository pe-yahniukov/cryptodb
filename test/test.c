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

#include <math.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <pthread.h>

#include "cryptodb.h"

#define TEST_DB_FOLDER "db"
#define TEST_THREADS_COUNT (4)

typedef struct
{
    int id;
    int retval;
    char key[128];
    char value[128];
    cryptodb_t *cryptodb;
} test_thread_arg_t;

static int custom_user_kdf(void *cryptodb,
                           bool encrypt_decrypt,
                           uint8_t encryption_key[32],
                           uint8_t encryption_iv[16],
                           void *user_data)
{
    (void)cryptodb;
    (void)encrypt_decrypt;
    (void)user_data;
    memset(encryption_iv, 0, 16);
    memset(encryption_key, 0, 32);
    return CRYPTODB_SUCCESS;
}

static bool compare_double(double a, double b)
{
    double maxVal = fabs(a) > fabs(b) ? fabs(a) : fabs(b);
    return (fabs(a - b) <= maxVal * DBL_EPSILON);
}

static void * _test_thread_func(void *ptr)
{
    int ret = 0;
    char value[128] = "";
    test_thread_arg_t *args = (test_thread_arg_t *)ptr;

    args->retval = CRYPTODB_SUCCESS;

    ret = cryptodb_put_string(args->cryptodb,
                              args->key,
                              strlen(args->key) + 1,
                              args->value);
    if (CRYPTODB_SUCCESS != ret)
    {
        fprintf(stderr, "Thread #%d: ERROR: cryptodb_put_string(), error = %d\n", args->id,
                                                                                  (int)ret);
        args->retval = ret;
        return NULL;
    }

    ret = cryptodb_get(args->cryptodb,
                       args->key,
                       strlen(args->key) + 1,
                       CRYPTODB_VAL_STRING,
                       (void *)value);
    if (CRYPTODB_SUCCESS != ret)
    {
        fprintf(stderr, "Thread #%d: ERROR: cryptodb_get(), error = %d\n", args->id,
                                                                           (int)ret);
        args->retval = ret;
        return NULL;
    }
    if (strncmp(args->value, value, 128))
    {
        fprintf(stderr, "Thread #%d: ERROR: value is wrong\n", args->id);
        args->retval = CRYPTODB_ERR_FAIL;
        return NULL;
    }

    ret = cryptodb_delete(args->cryptodb,
                          args->key,
                          strlen(args->key) + 1);
    if (CRYPTODB_SUCCESS != ret)
    {
        fprintf(stderr, "Thread #%d: ERROR: cryptodb_delete(), error = %d\n", args->id,
                                                                              (int)ret);
        args->retval = ret;
        return NULL;
    }
    if (CRYPTODB_SUCCESS == cryptodb_get(args->cryptodb,
                                         args->key,
                                         strlen(args->key) + 1,
                                         CRYPTODB_VAL_STRING,
                                         (void *)value))
    {
        fprintf(stderr, "Thread #%d: ERROR: failed to remove key\n", args->id);
        args->retval = CRYPTODB_ERR_FAIL;
        return NULL;
    }

    return NULL;
}

int main(int argc, char **argv)
{
    int ret = 0;
    DIR *dirdb = NULL;
    cryptodb_t cryptodb;
    int out_val_int = 0;
    char out_val[256] = "";
    double out_val_double = 0.0;
    pthread_t threads[TEST_THREADS_COUNT];
    uint8_t uniq_data[CRYPTODB_UNIQ_DATA_MAX_LEN] = {0};
    test_thread_arg_t threads_arg[TEST_THREADS_COUNT] = {0};

    // Was taken from https://www.poetryfoundation.org/poems/45236/thirteen-ways-of-looking-at-a-blackbird
    const char LARGE_TEXT[] = "I\
Among twenty snowy mountains,   \
The only moving thing   \
Was the eye of the blackbird.   \
\
II\
I was of three minds,   \
Like a tree   \
In which there are three blackbirds.   \
\
III\
The blackbird whirled in the autumn winds.   \
It was a small part of the pantomime.   \
\
IV\
A man and a woman   \
Are one.   \
A man and a woman and a blackbird   \
Are one.   \
\
V\
I do not know which to prefer,   \
The beauty of inflections   \
Or the beauty of innuendoes,  \
The blackbird whistling   \
Or just after.   \
\
VI\
Icicles filled the long window   \
With barbaric glass.   \
The shadow of the blackbird   \
Crossed it, to and fro.   \
The mood   \
Traced in the shadow   \
An indecipherable cause.   \
\
VII\
O thin men of Haddam,   \
Why do you imagine golden birds?   \
Do you not see how the blackbird   \
Walks around the feet   \
Of the women about you?   \
\
VIII\
I know noble accents   \
And lucid, inescapable rhythms;   \
But I know, too,   \
That the blackbird is involved   \
In what I know.   \
\
IX\
When the blackbird flew out of sight,   \
It marked the edge   \
Of one of many circles.   \
\
X\
At the sight of blackbirds   \
Flying in a green light,   \
Even the bawds of euphony   \
Would cry out sharply.   \
\
XI\
He rode over Connecticut   \
In a glass coach.   \
Once, a fear pierced him,   \
In that he mistook   \
The shadow of his equipage   \
For blackbirds.   \
\
XII\
The river is moving.   \
The blackbird must be flying.   \
\
XIII\
It was evening all afternoon.   \
It was snowing   \
And it was going to snow.   \
The blackbird sat   \
In the cedar-limbs.";
    char large_text_read[2048] = "";

    memset(&cryptodb, 0, sizeof(cryptodb_t));
    memset(uniq_data, 0, CRYPTODB_UNIQ_DATA_MAX_LEN);

    /**
     * Stress test: check return values with correct and incorrect arguments
     */

    if (strcmp(cryptodb_err_to_str(CRYPTODB_ERR_OK), "Success") ||
        strcmp(cryptodb_err_to_str(CRYPTODB_ERR_NULL_POINTER), "NULL pointer") ||
        strcmp(cryptodb_err_to_str(CRYPTODB_ERR_ALLOCATE_MEM), "Failed to allocate enough memory") ||
        strcmp(cryptodb_err_to_str(CRYPTODB_ERR_WRONG_ARGUMENT), "Wrong argument was provided") ||
        strcmp(cryptodb_err_to_str(CRYPTODB_ERR_ENCRYPTION_FAIL), "Encryption operation was failed") ||
        strcmp(cryptodb_err_to_str(CRYPTODB_ERR_DECRYPTION_FAIL), "Decryption operation was failed") ||
        strcmp(cryptodb_err_to_str(CRYPTODB_ERR_FAIL), "Fail") ||
        strcmp(cryptodb_err_to_str((cryptodb_err_t)(CRYPTODB_ERR_FAIL - 1)), "Unknown error"))
    {
        fprintf(stderr, "ERROR: cryptodb_err_to_str()\n");
        return -1;
    }
    if (strcmp(cryptodb_val_to_str(CRYPTODB_VAL_STRING), "String") ||
        strcmp(cryptodb_val_to_str(CRYPTODB_VAL_NUM_INT), "Integer number") ||
        strcmp(cryptodb_val_to_str(CRYPTODB_VAL_NUM_DOUBLE), "Double-precision floating-point number") ||
        strcmp(cryptodb_val_to_str(CRYPTODB_VAL_UNKNOWN), "Unknown value type"))
    {
        fprintf(stderr, "ERROR: cryptodb_val_to_str()\n");
        return -1;
    }
    if (cryptodb_open(NULL, NULL, 0, NULL, NULL, NULL, NULL) != CRYPTODB_ERR_NULL_POINTER ||
        cryptodb_open(TEST_DB_FOLDER, NULL, 0, NULL, NULL, NULL, NULL) != CRYPTODB_ERR_NULL_POINTER ||
        cryptodb_open(TEST_DB_FOLDER, NULL, 0, NULL, NULL, NULL, &cryptodb) != CRYPTODB_ERR_NULL_POINTER ||
        cryptodb_open(TEST_DB_FOLDER, uniq_data, 0, NULL, NULL, NULL, &cryptodb) != CRYPTODB_ERR_WRONG_ARGUMENT ||
        cryptodb_open(TEST_DB_FOLDER, uniq_data, CRYPTODB_UNIQ_DATA_MAX_LEN, NULL, NULL, NULL, &cryptodb) != CRYPTODB_SUCCESS)
    {
        fprintf(stderr, "ERROR: cryptodb_open()\n");
        return -1;
    }
    if (!cryptodb.db || !cryptodb.env || !cryptodb.cmp || !cryptodb.cache ||
        !cryptodb.options || !cryptodb.roptions || !cryptodb.woptions ||
        memcmp(cryptodb.uniq_data, uniq_data, CRYPTODB_UNIQ_DATA_MAX_LEN) ||
        cryptodb.uniq_data_len != CRYPTODB_UNIQ_DATA_MAX_LEN ||
        (dirdb = opendir(TEST_DB_FOLDER)) == NULL)
    {
        if (dirdb)
            (void)closedir(dirdb);
        fprintf(stderr, "ERROR: cryptodb_open()\n");
        return -1;
    }
    (void)closedir(dirdb);
    dirdb = NULL;

    cryptodb_close(&cryptodb);
    if (cryptodb.db || cryptodb.env || cryptodb.cmp || cryptodb.cache ||
        cryptodb.options || cryptodb.roptions || cryptodb.woptions ||
        cryptodb.uniq_data_len != 0)
    {
        fprintf(stderr, "ERROR: cryptodb_close()\n");
        return -1;
    }

    ret = cryptodb_open(TEST_DB_FOLDER, uniq_data, CRYPTODB_UNIQ_DATA_MAX_LEN, NULL, NULL, NULL, &cryptodb);
    if (CRYPTODB_SUCCESS != ret)
    {
        fprintf(stderr, "ERROR: cryptodb_open() #2\n");
        return -1;
    }

    if (cryptodb_put(NULL, NULL, 0, CRYPTODB_VAL_STRING, NULL) != CRYPTODB_ERR_NULL_POINTER ||
        cryptodb_put(&cryptodb, NULL, 0, CRYPTODB_VAL_STRING, NULL) != CRYPTODB_ERR_NULL_POINTER ||
        cryptodb_put(&cryptodb, "test_key", 0, CRYPTODB_VAL_STRING, NULL) != CRYPTODB_ERR_NULL_POINTER ||
        cryptodb_put(&cryptodb, "test_key", 0, CRYPTODB_VAL_STRING, "test_val") != CRYPTODB_ERR_WRONG_ARGUMENT ||
        cryptodb_put(&cryptodb, "test_key", strlen("test_key") + 1, CRYPTODB_VAL_STRING, "test_val") != CRYPTODB_SUCCESS)
    {
        cryptodb_close(&cryptodb);
        fprintf(stderr, "ERROR: cryptodb_put()\n");
        return -1;
    }

    if (cryptodb_get(NULL, NULL, 0, CRYPTODB_VAL_STRING, NULL) != CRYPTODB_ERR_NULL_POINTER ||
        cryptodb_get(&cryptodb, NULL, 0, CRYPTODB_VAL_STRING, NULL) != CRYPTODB_ERR_NULL_POINTER ||
        cryptodb_get(&cryptodb, "test_key", 0, CRYPTODB_VAL_STRING, NULL) != CRYPTODB_ERR_NULL_POINTER ||
        cryptodb_get(&cryptodb, "test_key", 0, CRYPTODB_VAL_STRING, out_val) != CRYPTODB_ERR_WRONG_ARGUMENT ||
        cryptodb_get(&cryptodb, "test_key", strlen("test_key") + 1, CRYPTODB_VAL_STRING, out_val) != CRYPTODB_SUCCESS ||
        strcmp("test_val", out_val))
    {
        cryptodb_close(&cryptodb);
        fprintf(stderr, "ERROR: cryptodb_get()\n");
        return -1;
    }

    if (cryptodb_delete(NULL, NULL, 0) != CRYPTODB_ERR_NULL_POINTER ||
        cryptodb_delete(&cryptodb, NULL, 0) != CRYPTODB_ERR_NULL_POINTER ||
        cryptodb_delete(&cryptodb, "test_key", 0) != CRYPTODB_ERR_WRONG_ARGUMENT ||
        cryptodb_delete(&cryptodb, "test_key", strlen("test_key") + 1) != CRYPTODB_SUCCESS ||
        cryptodb_get(&cryptodb, "test_key", strlen("test_key") + 1, CRYPTODB_VAL_STRING, out_val) == CRYPTODB_SUCCESS)
    {
        cryptodb_close(&cryptodb);
        fprintf(stderr, "ERROR: cryptodb_delete()\n");
        return -1;
    }

    if (cryptodb_put_integer(&cryptodb, "test_key_int", strlen("test_key_int") + 1, 42) != CRYPTODB_SUCCESS ||
        cryptodb_get(&cryptodb, "test_key_int", strlen("test_key_int") + 1, CRYPTODB_VAL_NUM_INT, &out_val_int) != CRYPTODB_SUCCESS ||
        out_val_int != 42)
    {
        cryptodb_close(&cryptodb);
        fprintf(stderr, "ERROR: cryptodb_get #2()\n");
        return -1;
    }

    if (cryptodb_put_double(&cryptodb, "test_key_double", strlen("test_key_double") + 1, 42.42) != CRYPTODB_SUCCESS ||
        cryptodb_get(&cryptodb, "test_key_double", strlen("test_key_double") + 1, CRYPTODB_VAL_NUM_DOUBLE, &out_val_double) != CRYPTODB_SUCCESS ||
        !compare_double(out_val_double, 42.42))
    {
        cryptodb_close(&cryptodb);
        fprintf(stderr, "ERROR: cryptodb_get #3()\n");
        return -1;
    }

    cryptodb_close(&cryptodb);

    if (cryptodb_destroy(NULL, NULL) != CRYPTODB_ERR_NULL_POINTER ||
        cryptodb_destroy(TEST_DB_FOLDER, NULL) != CRYPTODB_SUCCESS ||
        (dirdb = opendir(TEST_DB_FOLDER)) != NULL)
    {
        if (dirdb)
            (void)closedir(dirdb);
        fprintf(stderr, "ERROR: cryptodb_destroy()\n");
        return -1;
    }

    /**
     * Options test: check with cryptodb_options_t
     */

    cryptodb_options_t options = {0};
    memset(&options, 0, sizeof(cryptodb_options_t));

    options.block_size = CRYPTODB_OPT_DEFAULT_BLOCK_SIZE;
    options.max_open_files = CRYPTODB_OPT_DEFAULT_MAX_FILES;
    options.cache_capacity = CRYPTODB_OPT_DEFAULT_CACHE_SIZE;
    options.max_file_size = CRYPTODB_OPT_DEFAULT_MAX_FILE_SIZE;
    options.write_buffer_size = CRYPTODB_OPT_DEFAULT_WR_BUF_SIZE;
    options.block_restart_interval = CRYPTODB_OPT_DEFAULT_BLOCK_RE_INT;

    ret = cryptodb_open(TEST_DB_FOLDER, uniq_data, CRYPTODB_UNIQ_DATA_MAX_LEN, &options, NULL, NULL, &cryptodb);
    if (CRYPTODB_SUCCESS != ret)
    {
        fprintf(stderr, "ERROR: cryptodb_open() options\n");
        return -1;
    }

    ret = cryptodb_put_double(&cryptodb, "test_key", strlen("test_key") + 1, 42.4242);
    if (CRYPTODB_SUCCESS != ret)
    {
        cryptodb_close(&cryptodb);
        fprintf(stderr, "ERROR: cryptodb_put() options\n");
        return -1;
    }

    ret = cryptodb_get(&cryptodb, "test_key", strlen("test_key") + 1, CRYPTODB_VAL_NUM_DOUBLE, &out_val_double);
    if (CRYPTODB_SUCCESS != ret || !compare_double(out_val_double, 42.4242))
    {
        cryptodb_close(&cryptodb);
        fprintf(stderr, "ERROR: cryptodb_get() options\n");
        return -1;
    }

    ret = cryptodb_delete(&cryptodb, "test_key", strlen("test_key") + 1);
    if (CRYPTODB_SUCCESS != ret || CRYPTODB_SUCCESS == cryptodb_get(&cryptodb, "test_key", strlen("test_key") + 1, CRYPTODB_VAL_NUM_DOUBLE, &out_val_double))
    {
        cryptodb_close(&cryptodb);
        fprintf(stderr, "ERROR: cryptodb_delete() options\n");
        return -1;
    }

    cryptodb_close(&cryptodb);

    if (cryptodb_destroy(TEST_DB_FOLDER, &options) != CRYPTODB_SUCCESS ||
        (dirdb = opendir(TEST_DB_FOLDER)) != NULL)
    {
        if (dirdb)
            (void)closedir(dirdb);
        fprintf(stderr, "ERROR: cryptodb_destroy() options\n");
        return -1;
    }

    /**
     * Thread-safety test
     */

    ret = cryptodb_open(TEST_DB_FOLDER, uniq_data, CRYPTODB_UNIQ_DATA_MAX_LEN, NULL, NULL, NULL, &cryptodb);
    if (CRYPTODB_SUCCESS != ret)
    {
        fprintf(stderr, "ERROR: cryptodb_open() thread-safety\n");
        return -1;
    }

    for (int i = 0; i < TEST_THREADS_COUNT; ++i)
    {
        threads_arg[i].id = i;
        threads_arg[i].retval = 0;
        threads_arg[i].cryptodb = &cryptodb;
        snprintf(threads_arg[i].key, 128, "key%d", i);
        snprintf(threads_arg[i].value, 128, "value%d", i);
    }
    for (int i = 0; i < TEST_THREADS_COUNT; ++i)
    {
        if (pthread_create(&threads[i], NULL, _test_thread_func, (void *)&threads_arg[i]))
        {
            cryptodb_close(&cryptodb);
            fprintf(stderr, "ERROR: pthread_create() %d thread-safety\n", i);
            return -1;
        }
    }
    for (int i = 0; i < TEST_THREADS_COUNT; ++i)
    {
        if (pthread_join(threads[i], NULL))
        {
            cryptodb_close(&cryptodb);
            fprintf(stderr, "ERROR: pthread_join() %d thread-safety\n", i);
            return -1;
        }
        if (threads_arg[i].retval != CRYPTODB_SUCCESS)
        {
            cryptodb_close(&cryptodb);
            fprintf(stderr, "ERROR: thread #%d fail\n", threads_arg[i].id);
            return -1;
        }
    }

    cryptodb_close(&cryptodb);
    cryptodb_destroy(TEST_DB_FOLDER, NULL);

    /**
     * Large text test
     */

    ret = cryptodb_open(TEST_DB_FOLDER, uniq_data, CRYPTODB_UNIQ_DATA_MAX_LEN, NULL, NULL, NULL, &cryptodb);
    if (CRYPTODB_SUCCESS != ret)
    {
        fprintf(stderr, "ERROR: cryptodb_open() large text\n");
        return -1;
    }

    ret = cryptodb_put_string(&cryptodb, "test_key", strlen("test_key") + 1, LARGE_TEXT);
    if (CRYPTODB_SUCCESS != ret)
    {
        cryptodb_close(&cryptodb);
        fprintf(stderr, "ERROR: cryptodb_put() large text\n");
        return -1;
    }

    ret = cryptodb_get(&cryptodb, "test_key", strlen("test_key") + 1, CRYPTODB_VAL_STRING, &large_text_read);
    if (CRYPTODB_SUCCESS != ret || strcmp(large_text_read, LARGE_TEXT))
    {
        cryptodb_close(&cryptodb);
        fprintf(stderr, "ERROR: cryptodb_get() large text\n");
        return -1;
    }

    ret = cryptodb_delete(&cryptodb, "test_key", strlen("test_key") + 1);
    if (CRYPTODB_SUCCESS != ret || CRYPTODB_SUCCESS == cryptodb_get(&cryptodb, "test_key", strlen("test_key") + 1, CRYPTODB_VAL_STRING, &large_text_read))
    {
        cryptodb_close(&cryptodb);
        fprintf(stderr, "ERROR: cryptodb_delete() large text\n");
        return -1;
    }

    cryptodb_close(&cryptodb);

    if (cryptodb_destroy(TEST_DB_FOLDER, NULL) != CRYPTODB_SUCCESS ||
        (dirdb = opendir(TEST_DB_FOLDER)) != NULL)
    {
        if (dirdb)
            (void)closedir(dirdb);
        fprintf(stderr, "ERROR: cryptodb_destroy() large text\n");
        return -1;
    }

    /**
     * Large text test but with ready keys
     */

    uint8_t key[32] = {0}, iv[16] = {0};
    memset(key, 0, 32);
    memset(iv, 0, 16);

    ret = cryptodb_open_with_keys(TEST_DB_FOLDER, key, iv, NULL, &cryptodb);
    if (CRYPTODB_SUCCESS != ret)
    {
        fprintf(stderr, "ERROR: cryptodb_open() large text\n");
        return -1;
    }

    ret = cryptodb_put_string(&cryptodb, "test_key", strlen("test_key") + 1, LARGE_TEXT);
    if (CRYPTODB_SUCCESS != ret)
    {
        cryptodb_close(&cryptodb);
        fprintf(stderr, "ERROR: cryptodb_put() large text\n");
        return -1;
    }

    ret = cryptodb_get(&cryptodb, "test_key", strlen("test_key") + 1, CRYPTODB_VAL_STRING, &large_text_read);
    if (CRYPTODB_SUCCESS != ret || strcmp(large_text_read, LARGE_TEXT))
    {
        cryptodb_close(&cryptodb);
        fprintf(stderr, "ERROR: cryptodb_get() large text\n");
        return -1;
    }

    ret = cryptodb_delete(&cryptodb, "test_key", strlen("test_key") + 1);
    if (CRYPTODB_SUCCESS != ret || CRYPTODB_SUCCESS == cryptodb_get(&cryptodb, "test_key", strlen("test_key") + 1, CRYPTODB_VAL_STRING, &large_text_read))
    {
        cryptodb_close(&cryptodb);
        fprintf(stderr, "ERROR: cryptodb_delete() large text\n");
        return -1;
    }

    cryptodb_close(&cryptodb);

    if (cryptodb_destroy(TEST_DB_FOLDER, NULL) != CRYPTODB_SUCCESS ||
        (dirdb = opendir(TEST_DB_FOLDER)) != NULL)
    {
        if (dirdb)
            (void)closedir(dirdb);
        fprintf(stderr, "ERROR: cryptodb_destroy() large text\n");
        return -1;
    }

    /**
     * User KDF test
     */

    ret = cryptodb_open(TEST_DB_FOLDER, uniq_data, CRYPTODB_UNIQ_DATA_MAX_LEN, NULL, custom_user_kdf, NULL, &cryptodb);
    if (CRYPTODB_SUCCESS != ret)
    {
        fprintf(stderr, "ERROR: cryptodb_open() user_kdf\n");
        return -1;
    }

    ret = cryptodb_put_double(&cryptodb, "test_key", strlen("test_key") + 1, 42.4242);
    if (CRYPTODB_SUCCESS != ret)
    {
        cryptodb_close(&cryptodb);
        fprintf(stderr, "ERROR: cryptodb_put() user_kdf\n");
        return -1;
    }

    ret = cryptodb_get(&cryptodb, "test_key", strlen("test_key") + 1, CRYPTODB_VAL_NUM_DOUBLE, &out_val_double);
    if (CRYPTODB_SUCCESS != ret || !compare_double(out_val_double, 42.4242))
    {
        cryptodb_close(&cryptodb);
        fprintf(stderr, "ERROR: cryptodb_get() user_kdf\n");
        return -1;
    }

    ret = cryptodb_delete(&cryptodb, "test_key", strlen("test_key") + 1);
    if (CRYPTODB_SUCCESS != ret || CRYPTODB_SUCCESS == cryptodb_get(&cryptodb, "test_key", strlen("test_key") + 1, CRYPTODB_VAL_NUM_DOUBLE, &out_val_double))
    {
        cryptodb_close(&cryptodb);
        fprintf(stderr, "ERROR: cryptodb_delete() user_kdf\n");
        return -1;
    }

    cryptodb_close(&cryptodb);

    if (cryptodb_destroy(TEST_DB_FOLDER, &options) != CRYPTODB_SUCCESS ||
        (dirdb = opendir(TEST_DB_FOLDER)) != NULL)
    {
        if (dirdb)
            (void)closedir(dirdb);
        fprintf(stderr, "ERROR: cryptodb_destroy() user_kdf\n");
        return -1;
    }

    /**
     * Options test with disabled keys encryption
     */

    memset(&options, 0, sizeof(cryptodb_options_t));

    options.block_size = CRYPTODB_OPT_DEFAULT_BLOCK_SIZE;
    options.max_open_files = CRYPTODB_OPT_DEFAULT_MAX_FILES;
    options.cache_capacity = CRYPTODB_OPT_DEFAULT_CACHE_SIZE;
    options.max_file_size = CRYPTODB_OPT_DEFAULT_MAX_FILE_SIZE;
    options.write_buffer_size = CRYPTODB_OPT_DEFAULT_WR_BUF_SIZE;
    options.block_restart_interval = CRYPTODB_OPT_DEFAULT_BLOCK_RE_INT;

    options.disable_keys_encryption = 1;

    ret = cryptodb_open(TEST_DB_FOLDER, uniq_data, CRYPTODB_UNIQ_DATA_MAX_LEN, &options, NULL, NULL, &cryptodb);
    if (CRYPTODB_SUCCESS != ret)
    {
        fprintf(stderr, "ERROR: cryptodb_open() options\n");
        return -1;
    }

    ret = cryptodb_put_double(&cryptodb, "test_key", strlen("test_key") + 1, 42.4242);
    if (CRYPTODB_SUCCESS != ret)
    {
        cryptodb_close(&cryptodb);
        fprintf(stderr, "ERROR: cryptodb_put() options\n");
        return -1;
    }

    ret = cryptodb_get(&cryptodb, "test_key", strlen("test_key") + 1, CRYPTODB_VAL_NUM_DOUBLE, &out_val_double);
    if (CRYPTODB_SUCCESS != ret || !compare_double(out_val_double, 42.4242))
    {
        cryptodb_close(&cryptodb);
        fprintf(stderr, "ERROR: cryptodb_get() options\n");
        return -1;
    }

    ret = cryptodb_delete(&cryptodb, "test_key", strlen("test_key") + 1);
    if (CRYPTODB_SUCCESS != ret || CRYPTODB_SUCCESS == cryptodb_get(&cryptodb, "test_key", strlen("test_key") + 1, CRYPTODB_VAL_NUM_DOUBLE, &out_val_double))
    {
        cryptodb_close(&cryptodb);
        fprintf(stderr, "ERROR: cryptodb_delete() options\n");
        return -1;
    }

    cryptodb_close(&cryptodb);

    if (cryptodb_destroy(TEST_DB_FOLDER, &options) != CRYPTODB_SUCCESS ||
        (dirdb = opendir(TEST_DB_FOLDER)) != NULL)
    {
        if (dirdb)
            (void)closedir(dirdb);
        fprintf(stderr, "ERROR: cryptodb_destroy() options\n");
        return -1;
    }

    fprintf(stdout, "PASS\n");

    return 0;
}
