cmake_minimum_required(VERSION 3.5)

#========== Global Configurations =============#
#----------------------------------------------#
project(filediff LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============= Conan Bootstrap =============================#

# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
 message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
 file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.13/conan.cmake"
               "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_run(REQUIRES

              # Libraries to be installed locally
              gtest/cci.20210126

              BASIC_SETUP
              CMAKE_TARGETS
              BUILD missing
              GENERATORS cmake_find_package
              )

set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR} ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR} ${CMAKE_PREFIX_PATH})

#========== Find Packages =====================#

find_package(GTest REQUIRED)

#========== Targets Configurations ============#
# ==> Main target
add_executable(filediff main.cpp)

# ==> Target for testing with GoogleTest
add_executable(tests tests/ut.cpp
                     adler32.cpp)

target_link_libraries(tests GTest::GTest)

add_test(UnitTests tests)

install(TARGETS tests DESTINATION ./bin)
