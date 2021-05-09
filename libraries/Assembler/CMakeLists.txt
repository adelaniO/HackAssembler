set(
  HEADER_LIST
  Assembler.h
  VMTranslator.h
  Utilities.h
)

add_library(AssemblerLib ${HEADER_LIST} Assembler.cpp VMTranslator.cpp Utilities.cpp)
