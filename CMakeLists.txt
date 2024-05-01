#
# MIT License
#
# Copyright 2024 PE Stanislav Yahniukov <pe@yahniukov.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this
# software and associated documentation files (the “Software”), to deal in the Software
# without restriction, including without limitation the rights to use, copy, modify, merge,
# publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
# to whom the Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all copies or
# substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
# FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

cmake_minimum_required(VERSION 3.10)

cmake_policy(SET CMP0079 NEW)

set(cryptodb_version 1.0.0)

project(cryptodb VERSION ${cryptodb_version} LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(CRYPTODB_BUILD_TESTS "Set to ON to build tests" ON)
option(CRYPTODB_FOR_WINDOWS "Set to ON when build for Windows with MINGW" OFF)

if(NOT DEFINED CRYPTODB_AS_SUBPROJECT)
    set(CRYPTODB_AS_SUBPROJECT ON)
    if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
        set(CRYPTODB_AS_SUBPROJECT OFF)
    endif()
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE Release)
endif()

include(CheckCCompilerFlag)

set(default_compiler_flags
    -Wall
    -Werror
    -Wpedantic
    -Wno-unused-variable
    -Wno-unused-but-set-variable
    -fPIC
    -fvisibility=hidden)
if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    list(APPEND default_compiler_flags -g)
endif()
if (CRYPTODB_AS_SUBPROJECT)
    add_definitions(-DCRYPTODB_EXPORT)
endif()

foreach(compiler_flag ${default_compiler_flags})
    string(REGEX REPLACE "[^a-zA-Z0-9]" "" current_variable ${compiler_flag})
    CHECK_C_COMPILER_FLAG(${compiler_flag} "FLAG_SUPPORTED_${current_variable}")
    if (FLAG_SUPPORTED_${current_variable})
        list(APPEND supported_compiler_flags)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${compiler_flag}")
    endif()
endforeach()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${supported_compiler_flags}")

include(CheckCXXCompilerFlag)

set(default_cxx_compiler_flags
    -Wall
    -Werror
    -Wpedantic
    -Wno-unused-variable
    -Wno-unused-but-set-variable
    -fPIC
    -fvisibility=hidden)
if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    list(APPEND default_cxx_compiler_flags -g)
endif()

foreach(compiler_flag ${default_cxx_compiler_flags})
    string(REGEX REPLACE "[^a-zA-Z0-9]" "" current_variable ${compiler_flag})
    CHECK_CXX_COMPILER_FLAG(${compiler_flag} "FLAG_SUPPORTED_${current_variable}")
    if (FLAG_SUPPORTED_${current_variable})
        list(APPEND supported_cxx_compiler_flags)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${compiler_flag}")
    endif()
endforeach()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${supported_cxx_compiler_flags}")

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    link_libraries("-s -O3")
endif()

# By default we build needed submodules ourselves unless user specified
# include directory and library locations for particular library:
# * CRC32C_INCLUDE_DIR
# * CRC32C_LIBRARY
# * LEVELDB_INCLUDE_DIR
# * LEVELDB_LIBRARY
# * MBEDCRYPTO_INCLUDE_DIR
# * MBEDCRYPTO_LIBRARY
# * CJSON_INCLUDE_DIR
# * CJSON_LIBRARY
# * SCPRNG_INCLUDE_DIR
# * SCPRNG_LIBRARY

# crc32c
if("${CRC32C_INCLUDE_DIR}" STREQUAL "")
    set(CRC32C_INSTALL OFF CACHE BOOL "")
    set(CRC32C_USE_GLOG OFF CACHE BOOL "")
    set(CRC32C_BUILD_TESTS OFF CACHE BOOL "")
    set(CRC32C_BUILD_BENCHMARKS OFF CACHE BOOL "")
    set(CRC32C_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/crc32c/include)
    set(CRC32C_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/external/crc32c/libcrc32c.a)
    add_library(crc32c::crc32c UNKNOWN IMPORTED)
    set_target_properties(crc32c::crc32c
        PROPERTIES
        IMPORTED_LOCATION ${CRC32C_LIBRARY}
        INTERFACE_INCLUDE_DIRECTORIES ${CRC32C_INCLUDE_DIR}
    )
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/crc32c)
    add_dependencies(crc32c::crc32c crc32c)
else()
    find_package(crc32c)
endif()

# leveldb
if("${LEVELDB_INCLUDE_DIR}" STREQUAL "")
    set(HAVE_CRC32C ON CACHE BOOL "")
    set(LEVELDB_INSTALL OFF CACHE BOOL "")
    set(LEVELDB_BUILD_TESTS OFF CACHE BOOL "")
    set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
    set(LEVELDB_BUILD_BENCHMARKS OFF CACHE BOOL "")
    if (CRYPTODB_FOR_WINDOWS)
        set(LEVELDB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/leveldb.windows/include)
        set(LEVELDB_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/external/leveldb.windows/libleveldb.dll)
    else()
        set(LEVELDB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/leveldb/include)
        set(LEVELDB_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/external/leveldb/libleveldb.so)
    endif()
    add_library(LevelDB::LevelDB UNKNOWN IMPORTED)
    set_target_properties(LevelDB::LevelDB
        PROPERTIES
        IMPORTED_LOCATION ${LEVELDB_LIBRARY}
        INTERFACE_INCLUDE_DIRECTORIES ${LEVELDB_INCLUDE_DIR}
    )
    if (CRYPTODB_FOR_WINDOWS)
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/leveldb.windows)
    else()
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/leveldb)
    endif()
    if (CMAKE_CROSSCOMPILING)
        target_link_libraries(leveldb -static-libgcc -static-libstdc++)
    endif()
    add_dependencies(leveldb crc32c::crc32c)
    target_link_libraries(leveldb crc32c::crc32c)
    add_dependencies(LevelDB::LevelDB leveldb)
else()
    find_package(LevelDB REQUIRED)
endif()

# MbedCrypto
if("${MBEDCRYPTO_INCLUDE_DIR}" STREQUAL "")
    set(ENABLE_TESTING OFF CACHE BOOL "")
    set(ENABLE_PROGRAMS OFF CACHE BOOL "")
    set(MBEDCRYPTO_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/mbedtls/include)
    set(MBEDCRYPTO_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/external/mbedtls/library/libmbedcrypto.a)
    add_library(MbedCrypto::MbedCrypto UNKNOWN IMPORTED)
    set_target_properties(MbedCrypto::MbedCrypto
        PROPERTIES
        IMPORTED_LOCATION ${MBEDCRYPTO_LIBRARY}
        INTERFACE_INCLUDE_DIRECTORIES ${MBEDCRYPTO_INCLUDE_DIR}
    )
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/mbedtls)
    add_dependencies(MbedCrypto::MbedCrypto lib)
else()
    find_package(MbedCrypto REQUIRED)
endif()

# cJSON
if("${CJSON_INCLUDE_DIR}" STREQUAL "")
    if (CRYPTODB_FOR_WINDOWS)
        set(ENABLE_STACK_PROTECTION_STRONG OFF CACHE BOOL "")
    endif()
    set(ENABLE_CJSON_TEST OFF CACHE BOOL "")
    set(ENABLE_TARGET_EXPORT OFF CACHE BOOL "")
    set(ENABLE_PUBLIC_SYMBOLS OFF CACHE BOOL "")
    set(ENABLE_CJSON_UNINSTALL OFF CACHE BOOL "")
    set(CJSON_BUILD_SHARED_LIBS OFF CACHE BOOL "")
    set(CJSON_OVERRIDE_BUILD_SHARED_LIBS ON CACHE BOOL "")
    set(CJSON_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/cJSON)
    set(CJSON_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/external/cJSON/libcjson.a)
    add_library(cJSON::cJSON UNKNOWN IMPORTED)
    set_target_properties(cJSON::cJSON
        PROPERTIES
        IMPORTED_LOCATION ${CJSON_LIBRARY}
        INTERFACE_INCLUDE_DIRECTORIES ${CJSON_INCLUDE_DIR}
    )
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/cJSON)
    add_dependencies(cJSON::cJSON cjson)
else()
    find_package(cJSON REQUIRED)
endif()

# scprng
if("${SCPRNG_INCLUDE_DIR}" STREQUAL "")
    set(SCPRNG_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/scprng)
    set(SCPRNG_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/external/scprng/libscprng.a)
    add_library(SCPRNG::SCPRNG UNKNOWN IMPORTED)
    set_target_properties(SCPRNG::SCPRNG
        PROPERTIES
        IMPORTED_LOCATION ${SCPRNG_LIBRARY}
        INTERFACE_INCLUDE_DIRECTORIES ${SCPRNG_INCLUDE_DIR}
    )
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/scprng)
    add_dependencies(scprng-static MbedCrypto::MbedCrypto)
    add_dependencies(SCPRNG::SCPRNG scprng-static)
else()
    find_package(SCPRNG REQUIRED)
endif()

add_library(cryptodb SHARED ${CMAKE_CURRENT_SOURCE_DIR}/cryptodb.c)

add_dependencies(cryptodb
    cJSON::cJSON
    SCPRNG::SCPRNG
    LevelDB::LevelDB
    MbedCrypto::MbedCrypto
)

target_include_directories(cryptodb
    PRIVATE
    ${CJSON_INCLUDE_DIR}
    ${SCPRNG_INCLUDE_DIR}
    ${LEVELDB_INCLUDE_DIR}
    ${MBEDCRYPTO_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(cryptodb
    cJSON::cJSON
    SCPRNG::SCPRNG
    LevelDB::LevelDB
    MbedCrypto::MbedCrypto
)

add_library(cryptodbcxx SHARED ${CMAKE_CURRENT_SOURCE_DIR}/cryptodb.cpp)

target_include_directories(cryptodbcxx
    PRIVATE
    ${LEVELDB_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_dependencies(cryptodbcxx
    cJSON::cJSON
    SCPRNG::SCPRNG
    LevelDB::LevelDB
    MbedCrypto::MbedCrypto
    cryptodb
)

target_link_libraries(cryptodbcxx
    cJSON::cJSON
    SCPRNG::SCPRNG
    LevelDB::LevelDB
    MbedCrypto::MbedCrypto
    cryptodb
)

if (CMAKE_CROSSCOMPILING)
    target_link_libraries(cryptodb -static-libgcc)
    target_link_libraries(cryptodbcxx -static-libstdc++)
endif()

if ((NOT CRYPTODB_AS_SUBPROJECT) AND CRYPTODB_BUILD_TESTS)
    add_subdirectory(test)
endif()
