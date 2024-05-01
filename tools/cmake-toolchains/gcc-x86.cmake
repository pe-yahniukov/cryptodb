set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR X86)
set(CMAKE_CROSSCOMPILING TRUE)

# cross compilers to use for C, C++ and Fortran
set(CMAKE_C_COMPILER gcc -m32)
set(CMAKE_CXX_COMPILER g++ -m32)
set(CMAKE_Fortran_COMPILER gfortran)
set(CMAKE_RC_COMPILER gcc)

# target environment on the build host system
set(CMAKE_FIND_ROOT_PATH /usr)

# modify default behavior of FIND_XXX() commands
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)