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

find_path(MBEDCRYPTO_INCLUDE_DIR NAMES "mbedtls/aes.h")

find_library(MBEDCRYPTO_LIBRARY NAMES libmbedcrypto)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MbedCrypto DEFAULT_MSG MBEDCRYPTO_LIBRARY MBEDCRYPTO_INCLUDE_DIR)

mark_as_advanced(MBEDCRYPTO_LIBRARY MBEDCRYPTO_INCLUDE_DIR)

if(MbedCrypto_FOUND AND NOT (TARGET MbedCrypto::MbedCrypto))
    add_library(MbedCrypto::MbedCrypto UNKNOWN IMPORTED)
    set_target_properties(MbedCrypto::MbedCrypto
        PROPERTIES
        IMPORTED_LOCATION ${MBEDCRYPTO_LIBRARY}
        INTERFACE_INCLUDE_DIRECTORIES ${MBEDCRYPTO_INCLUDE_DIR})
endif()
