set(
  HEADER_LIST
  Tokenizer.h
)

add_library(CompilerLib ${HEADER_LIST} Tokenizer.cpp "CompilationEngine.h" "CompilationEngine.cpp")
