# corss-platform
set(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_STAGING_PREFIX /home/ribll2026/develop/crosstool/)
set(CMAKE_C_COMPILER arm-unknown-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-unknown-linux-gnueabihf-g++)
set(CMAKE_FIND_ROOT_PATH /home/ribll2026/x-tools/arm-unknown-linux-gnueabihf/arm-unknown-linux-gnueabihf/sysroot)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

link_libraries(atomic)
