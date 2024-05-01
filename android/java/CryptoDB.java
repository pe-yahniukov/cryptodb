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

package com.yahniukov.cryptodb;

public final class CryptoDB {

    static {
        System.loadLibrary("cryptodb-android");
    }

    private static final int ENCRYPTION_KEY_IV = 16;
    private static final int ENCRYPTION_KEY_SIZE = 32;

    private static long CRYPTODB_UNIQ_DATA_MAX_LEN = 0;

    private static native long getUniqDataMaxLen();
    private static native int getVersionMajor();
    private static native int getVersionMinor();
    private static native int getVersionRevision();
    private static native String errorToStr(int err);
    private static native void setDBOptions(long cache_capacity,
                                            long write_buffer_size,
                                            int max_open_files,
                                            long block_size,
                                            int block_restart_interval,
                                            long max_file_size,
                                            int disable_keys_encryption);
    private static native void setDBOptionsToDefault();
    private static native int open(String path,
                                   byte[] uniq_data,
                                   long uniq_data_len);
    private static native int openWithKeys(String path,
                                           byte[] encryption_key,
                                           byte[] encryption_iv);
    private static native int destroy(String path);
    private static native void close();
    private static native int putString(String key, String val);
    private static native int putInteger(String key, int val);
    private static native int putDouble(String key, double val);
    private static native String getString(String key, int expected_max_length);
    private static native int getInteger(String key);
    private static native double getDouble(String key);
    private static native int delete(String key);

    public enum Error
    {
        OK,
        Null,
        AllocateMemory,
        WrongArgument,
        EncryptionFail,
        DecryptionFail,
        // <-- New errors should be added here
        Fail // always last
    }

    private static Error nativeErrToJavaErr(int err)
    {
        switch (err) {
            default:
                break;
            case 0:
                return Error.OK;
            case -1:
                return Error.Null;
            case -2:
                return Error.AllocateMemory;
            case -3:
                return Error.WrongArgument;
            case -4:
                return Error.EncryptionFail;
            case -5:
                return Error.DecryptionFail;
            // <-- New errors should be added here
        }
        return Error.Fail;
    }

    private static int javaErrtoNativeErr(Error err)
    {
        switch (err) {
            default:
            case Fail:
                break;
            case OK:
                return 0;
            case Null:
                return -1;
            case AllocateMemory:
                return -2;
            case WrongArgument:
                return -3;
            case EncryptionFail:
                return -4;
            case DecryptionFail:
                return -5;
        }
        return -1024;
    }

    public static class Options
    {
        public long cache_capacity;
        public long write_buffer_size;
        public int max_open_files;
        public long block_size;
        public int block_restart_interval;
        public long max_file_size;
        public int disable_keys_encryption;
    }

    public static class Version
    {
        public int major;
        public int minor;
        public int revision;
    }

    public static class GetStringResult
    {
        public Error err;
        public String val;
    }

    public static class GetIntegerResult
    {
        public Error err;
        public int val;
    }

    public static class GetDoubleResult
    {
        public Error err;
        public double val;
    }

    public static long GetUniqDataMaxLen()
    {
        if (CRYPTODB_UNIQ_DATA_MAX_LEN == 0)
            CRYPTODB_UNIQ_DATA_MAX_LEN = getUniqDataMaxLen();
        return CRYPTODB_UNIQ_DATA_MAX_LEN;
    }

    public static Version GetVersion()
    {
        Version v = new Version();
        v.major    = getVersionMajor();
        v.minor    = getVersionMinor();
        v.revision = getVersionRevision();
        return v;
    }

    public static String ErrorToString(Error err)
    {
        return errorToStr(javaErrtoNativeErr(err));
    }

    public static void SetOptions(Options options)
    {
        setDBOptions(
                options.cache_capacity,
                options.write_buffer_size,
                options.max_open_files,
                options.block_size,
                options.block_restart_interval,
                options.max_file_size,
                options.disable_keys_encryption);
    }

    public static void SetDefaultOptions()
    {
        setDBOptionsToDefault();
    }

    public static Error Open(String path,
                             byte[] uniq_data)
    {
        if (uniq_data.length > CRYPTODB_UNIQ_DATA_MAX_LEN)
            return Error.WrongArgument;
        return nativeErrToJavaErr(open(path, uniq_data, (long)uniq_data.length));
    }

    public static Error OpenWithKeys(String path,
                                     byte[] encryption_key,
                                     byte[] encryption_iv)
    {
        if (encryption_key.length != ENCRYPTION_KEY_SIZE ||
            encryption_iv.length  != ENCRYPTION_KEY_IV)
            return Error.WrongArgument;
        return nativeErrToJavaErr(openWithKeys(path, encryption_key, encryption_iv));
    }

    public static Error Destroy(String path)
    {
        return nativeErrToJavaErr(destroy(path));
    }

    public static void Close()
    {
        close();
    }

    public static Error PutString(String key,
                                  String val)
    {
        return nativeErrToJavaErr(putString(key, val));
    }

    public static Error PutInteger(String key,
                                   int val)
    {
        return nativeErrToJavaErr(putInteger(key, val));
    }

    public static Error PutDouble(String key,
                                  double val)
    {
        return nativeErrToJavaErr(putDouble(key, val));
    }

    public static GetStringResult GetString(String key,
                                            int expected_max_length)
    {
        GetStringResult result = new GetStringResult();
        result.val = getString(key, expected_max_length);
        if (result.val.isEmpty())
            result.err = Error.Fail;
        else
            result.err = Error.OK;
        return result;
    }

    public static GetIntegerResult GetInteger(String key)
    {
        GetIntegerResult result = new GetIntegerResult();
        result.val = getInteger(key);
        if (result.val < Error.OK.ordinal())
            result.err = nativeErrToJavaErr(result.val);
        else
            result.err = Error.OK;
        return result;
    }

    public static GetDoubleResult GetDouble(String key)
    {
        GetDoubleResult result = new GetDoubleResult();
        result.val = getDouble(key);
        if ((int)result.val < Error.OK.ordinal())
            result.err = nativeErrToJavaErr((int)result.val);
        else
            result.err = Error.OK;
        return result;
    }

    public static Error Delete(String key)
    {
        return nativeErrToJavaErr(delete(key));
    }
}
