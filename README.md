# CryptoDB

## [LevelDB](https://github.com/google/leveldb.git) with AES encryption powered by [mbedcrypto](https://github.com/Mbed-TLS/mbedtls.git)
## Local lightweight, fast, and encrypted persistent key-value storage that is distributed as C and C++ libraries suitable for embedded systems. Also, there is a Java JNI wrapper for Android in an 'android' folder.

## In short, this is LevelDB wrapper with built-in AES-256 CBC encryption of both keys (optional) and values, and the possibility to store not only strings but also integer and floating-point numbers. The CryptoDB library is thread-safe and without memory leaks.

## Maintained target platforms
* Linux (x86, x86_64, arm32, arm64)
* Windows (x86, x86_64)
* Android (arm64-v8a, armeabi-v7a, x86, x86_64)

Only a little-endian version was tested with the platforms.

**If you don't see your platform here it doesn't mean the library can't be compiled on it. The list above contains only targets where the library was tested.**

## Interface

See the C library interface in "cryptodb.h" and the C++ library interface in "cryptodb.hpp". It's a simple and usual "key-value" database API. Common operations: put(key, value), get(key), delete(key).

The most unusual thing you can see there is an "uniq_data" parameter in cryptodb_open(). This data is used to secretly generate a key and initialization vector for AES-256 CBC encryption. Thus, even the application itself doesn't know this secret data. The user can specify his own KDF (Key Derivation Function) in the "user_kdf" parameter of the same function.

It's recommended to specify some really unique device data in this parameter, e.g. MAC-address or manufacturer device ID. Better preliminary XORed with some user's passphrase. It's good to use [advertisingIdentifier](https://developer.apple.com/documentation/adsupport/asidentifiermanager/advertisingidentifier) on iOS, and [AdvertisingIdClient](https://developers.google.com/android/reference/com/google/android/gms/ads/identifier/AdvertisingIdClient.Info#getId()) on Android. It was designed in a way that a database that was created on one device cannot be decrypted on the other device, even with the same library.

In case you want to specify the encryption key and IV yourself, use cryptodb_open_with_keys() function.

## Pre-requirements

* Linux distribution OS
* [make](https://www.gnu.org/software/make/)
* [cmake](https://cmake.org/download/)
* [cppcheck](https://cppcheck.sourceforge.io) - Optional, for pre-commit hook
* [valgrind](https://valgrind.org/downloads/) - Optional, for pre-commit hook
* [Android NDK](https://developer.android.com/ndk) - Optional, for Java JNI wrapper for Android
* [Docker](https://docs.docker.com/engine/install/) - Optional, for cross-compiling using "tools/make_distr.sh" script

## Preparing the repository

Please execute the below command in the root directory of the repository every time in a new terminal:
```console
$ source ./setup_env.sh
```

## Building

```console
$ mkdir build && cd build && cmake .. && make
```
### Build for all supported platforms at once
```console
$ ./tools/make_distr.sh
```
You will find C and C++ libraries distribution in a "distr" folder in the root directory of the repository.

### Used versions of external submodules during the building

* [cJSON](https://github.com/DaveGamble/cJSON.git) v1.7.17
* [crc32c](https://github.com/google/crc32c.git) v1.1.2
* [mbedtls](https://github.com/Mbed-TLS/mbedtls.git) v3.6.0
* [leveldb](https://github.com/google/leveldb.git) v1.23
* [scprng](https://github.com/pe-yahniukov/scprng.git) v2.0.2

By default, if cmake command was "cmake ..", the build system will build the above submodules itself.

If you want to build cryptodb with your version of the submodules, you need to specify some of the below options in "cmake" command:
* CRC32C_INCLUDE_DIR
* CRC32C_LIBRARY
* LEVELDB_INCLUDE_DIR
* LEVELDB_LIBRARY
* MBEDCRYPTO_INCLUDE_DIR
* MBEDCRYPTO_LIBRARY
* CJSON_INCLUDE_DIR
* CJSON_LIBRARY
* SCPRNG_INCLUDE_DIR
* SCPRNG_LIBRARY

See CMakeLists.txt for the details.

## Testing

```console
$ ./build/test/cryptodb_test && ./build/test/cryptodb_cxx_test
```
OR

```console
$ valgrind ./build/test/cryptodb_test && valgrind ./build/test/cryptodb_cxx_test
```
### Testing ARM versions
See "tools/qemu-arm/README.md" for the details how to test ARM versions of the library.

## License

MIT License. See LICENSE in the root directory of the repository.

Also, see submodules licenses in "external/licenses" folder.

## Contribution

Please see "CONTRIBUTION.md" for the details.
