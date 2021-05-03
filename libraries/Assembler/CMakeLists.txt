set(HEADER_LIST headers/Assembler.h headers/VMTranslator.h)
add_library(AssemblerLib Assembler.cpp VMTranslator.cpp ${HEADER_LIST})
