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

find_path(LEVELDB_INCLUDE_DIR NAMES "leveldb/c.h")

find_library(LEVELDB_LIBRARY NAMES libleveldb.so)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LevelDB DEFAULT_MSG LEVELDB_LIBRARY LEVELDB_INCLUDE_DIR)

mark_as_advanced(LEVELDB_LIBRARY LEVELDB_INCLUDE_DIR)

if(LevelDB_FOUND AND NOT (TARGET LevelDB::LevelDB))
    add_library(LevelDB::LevelDB UNKNOWN IMPORTED)
    set_target_properties(LevelDB::LevelDB
        PROPERTIES
        IMPORTED_LOCATION ${LEVELDB_LIBRARY}
        INTERFACE_INCLUDE_DIRECTORIES ${LEVELDB_INCLUDE_DIR})
endif()
