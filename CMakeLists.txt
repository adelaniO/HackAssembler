cmake_minimum_required(VERSION 3.14.0...3.20)
project(
  HackAssembler 
  VERSION 0.1.0
  DESCRIPTION "Command line tool to convert the Jack language to Hack machine code"
)

include(CTest)
enable_testing()
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ======== Project CLI ========
add_executable(AssemblerCLI main.cpp Assembler.h VMTranslator.h)

# ======== Googletest ========
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(dependencies/googletest)

# ======== UnitTester ========
add_executable(
  UnitTester
  tests/UnitTestMain.cpp
  tests/unitTests/TestExample.cpp
  tests/unittests/TestVMTranslator.cpp
  tests/unittests/TestAssembler.cpp
)
target_include_directories(UnitTester PRIVATE . dependencies)
target_link_libraries(UnitTester PRIVATE gtest)
