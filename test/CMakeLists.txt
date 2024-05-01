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

add_executable(cryptodb_test ${CMAKE_CURRENT_SOURCE_DIR}/test.c)
add_dependencies(cryptodb_test cryptodb)
target_include_directories(cryptodb_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(cryptodb_test
    LevelDB::LevelDB
    cryptodb
)

add_executable(cryptodb_cxx_test ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)
add_dependencies(cryptodb_cxx_test cryptodbcxx)
target_include_directories(cryptodb_cxx_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(cryptodb_cxx_test
    LevelDB::LevelDB
    cryptodb
    cryptodbcxx
)
