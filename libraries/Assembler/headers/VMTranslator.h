#pragma once

#include <string>
#include <filesystem>
#include <iostream>

// Translate Hack.vm files to .asm

enum class VMCommandType { C_ARITHMETIC, C_PUSH, C_POP, C_LABEL, C_GOTO, C_IF, C_FUNCTION, C_RETURN, C_CALL };
namespace VMTranslator
{
    class Translator
    {
    public:
        Translator() {}

        VMCommandType commandType();

        std::string arg1();

        int arg2();

        int parse(const std::filesystem::path& inputFile)
        {
            if (inputFile.extension() != ".vm")
            {
                std::cout << "Invalid InputFile Extension\n";
                return 1;
            }

            return 0;
        }

        int write(const std::filesystem::path& outputFile);

    private:
    };
}