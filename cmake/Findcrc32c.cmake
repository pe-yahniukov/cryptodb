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

find_path(CRC32C_INCLUDE_DIR NAMES "crc32c/crc32c.h")

find_library(CRC32C_LIBRARY NAMES libcrc32c)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(crc32c DEFAULT_MSG CRC32C_LIBRARY CRC32C_INCLUDE_DIR)

mark_as_advanced(CRC32C_LIBRARY CRC32C_INCLUDE_DIR)

if(crc32c_FOUND AND NOT (TARGET crc32c::crc32c))
    add_library(crc32c::crc32c UNKNOWN IMPORTED)
    set_target_properties(crc32c::crc32c
        PROPERTIES
        IMPORTED_LOCATION ${CRC32C_LIBRARY}
        INTERFACE_INCLUDE_DIRECTORIES ${CRC32C_INCLUDE_DIR})
endif()
