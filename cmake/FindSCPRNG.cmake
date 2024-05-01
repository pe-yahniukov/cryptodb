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

find_path(SCPRNG_INCLUDE_DIR NAMES "scprng.h")

find_library(SCPRNG_LIBRARY NAMES libscprng)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SCPRNG DEFAULT_MSG SCPRNG_LIBRARY SCPRNG_INCLUDE_DIR)

mark_as_advanced(SCPRNG_LIBRARY SCPRNG_INCLUDE_DIR)

if(SCPRNG_FOUND AND NOT (TARGET SCPRNG::SCPRNG))
    add_library(SCPRNG::SCPRNG UNKNOWN IMPORTED)
    set_target_properties(SCPRNG::SCPRNG
        PROPERTIES
        IMPORTED_LOCATION ${SCPRNG_LIBRARY}
        INTERFACE_INCLUDE_DIRECTORIES ${SCPRNG_INCLUDE_DIR})
endif()
