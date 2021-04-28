cmake_minimum_required(VERSION 3.0.0)
project(HackAssembler VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(HackAssembler main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set_target_properties(HackAssembler PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS OFF)
include(CPack)
