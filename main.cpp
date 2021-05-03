#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>

#include "Assembler.h"
#include "VMTranslator.h"

int main(int argc, char* argv[])
{
    if (argc <= 1)
    {
        std::cout << "Usage: <input file>" << '\n';
        return 1;
    }

    std::filesystem::path inputFile{ argv[1] };
    if (inputFile.extension() == ".asm")
    {
        Assembler::Assembler assembler;
        return assembler.parse(inputFile);
    }
    if (inputFile.extension() == ".asm")
    {
        VMTranslator::Translator translator;
        return translator.parse(inputFile);
    }
    else
    {
        std::cout << "Invalid InputFile Extension\n";
        return 1;
    }
}