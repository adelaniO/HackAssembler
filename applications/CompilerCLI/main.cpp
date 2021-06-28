#include <iostream>

#include "dirent.h"
#include "Utilities.h"
#include "Tokenizer.h"
#include "CompilationEngine.h"

int main(int argc, char* argv[])
{
    if (argc <= 1)
    {
        std::cout << "Usage: <input file/directory>" << '\n';
        return 1;
    }
    
    std::string pathName{argv[1]};
    if(pathName.back() == '\\' || pathName.back() == '/')
        pathName = pathName.substr(0, pathName.size()-1);

    fs::path input{ pathName };
    if (input.extension() == ".jack")
    {
        Compiler::Tokenizer tokenizer;
        tokenizer.parse(input);
        Compiler::CompilationEngine compiler{&tokenizer};
        return 0;
    }
    else
    {
        std::cout << "Invalid InputFile Extension\n";
        return 1;
    }
}