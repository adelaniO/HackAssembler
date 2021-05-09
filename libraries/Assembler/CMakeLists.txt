set(
  HEADER_LIST
  headers/Assembler.h
  headers/VMTranslator.h
  headers/Utilities.h
)

add_library(AssemblerLib ${HEADER_LIST} Assembler.cpp VMTranslator.cpp Utilities.cpp)
