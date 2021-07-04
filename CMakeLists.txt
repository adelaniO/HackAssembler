cmake_minimum_required(VERSION 3.14...3.20)
project(
  HackAssembler 
  VERSION 0.1.0
  DESCRIPTION "Command line tool to convert the Jack language to Hack machine code"
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ======== UtilitiesLib ========
add_subdirectory(libraries/Utilities)
# ======== AssemblerLib ========
add_subdirectory(libraries/Assembler)
target_link_libraries(AssemblerLib PUBLIC UtilitiesLib)
target_include_directories(AssemblerLib PUBLIC libraries/Assembler libraries/Utilities)
# libraries/Assembler
# ======== CompilerLib ========
add_subdirectory(libraries/Compiler)
target_link_libraries(CompilerLib PUBLIC UtilitiesLib)
target_include_directories(CompilerLib PUBLIC libraries/Compiler libraries/Utilities)
#libraries/Compiler


# ======== AssemblerCLI ========
add_executable(AssemblerCLI applications/AssemblerCLI/main.cpp)
target_link_libraries(AssemblerCLI PRIVATE AssemblerLib UtilitiesLib)
target_include_directories(AssemblerCLI PUBLIC dependencies/dirent/include)

# ======== CompilerCLI ========
add_executable(CompilerCLI applications/CompilerCLI/main.cpp)
target_link_libraries(CompilerCLI PRIVATE CompilerLib UtilitiesLib)
target_include_directories(CompilerCLI PUBLIC dependencies/dirent/include)

# ======== Googletest ========
enable_testing()
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(dependencies/googletest)

# ======== UnitTester ========
add_executable(
  UnitTester
  tests/UnitTestMain.cpp
  tests/unitTests/TestUtilities.cpp
  tests/unitTests/TestCompiler.cpp
  tests/unittests/TestVMTranslator.cpp
  tests/unittests/TestAssembler.cpp
)
target_link_libraries(UnitTester PRIVATE gtest gmock AssemblerLib CompilerLib)
target_include_directories(UnitTester PUBLIC dependencies libraries/Utilities)
