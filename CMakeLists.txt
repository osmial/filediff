cmake_minimum_required(VERSION 3.5)

#========== Global Configurations =============#
#----------------------------------------------#
project(filediff LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# This makes C++20 a requirement and prevents a "decay" to C++98 when the compiler does not support C++20.
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Tell find_package() to first search using Config mode before falling back to Module mode (for conan)
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)

set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR} ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR} ${CMAKE_PREFIX_PATH})

#========== Find Packages =====================#

find_package(GTest REQUIRED)
find_package(Boost REQUIRED)
find_package(fmt REQUIRED)

#========== Targets Configurations ============#
# ==> Main target
add_executable(${PROJECT_NAME} main.cpp
                               adler32.cpp
                               signature.cpp
                               delta.cpp)

target_link_libraries(${PROJECT_NAME} Boost::program_options
                                      fmt::fmt)


# ==> Target for testing with GoogleTest
add_executable(tests tests/ut.cpp
                     adler32.cpp
                     signature.cpp
                     delta.cpp)

target_link_libraries(tests gtest::gtest
                            fmt::fmt)

add_test(UnitTests tests)
