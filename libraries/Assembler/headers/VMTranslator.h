#pragma once

#include <filesystem>
#include <iostream>
#include <vector>

#include "Utilities.h"

// Translate Hack.vm files to .asm

enum class VMCommandType { C_ARITHMETIC, C_PUSH, C_POP, C_LABEL, C_GOTO, C_IF, C_FUNCTION, C_RETURN, C_CALL, INVALID };
namespace VMTranslator
{
    class Translator
    {
    public:
        Translator(const std::string& fileName) : m_fileName{fileName}
        {
            m_fileName = fileName.substr(0, fileName.size()-3);
        }

        VMCommandType commandType(const std::string& line);

        std::string arg1(const std::string& line);

        int arg2(const std::string& line);

        int parse(const std::filesystem::path& inputFile);
        std::pair<std::string, std::string> parseCodeLine(const std::string& line, const bool addComment = true);

        int write(const std::filesystem::path& outputFile);
        void reset()
        {
            m_resultLines.clear();
            m_VMCodeCount = 0;
        }
        int incCodeCount() { return m_VMCodeCount++; }

    private:
        std::vector<std::string> m_resultLines;
        std::string m_fileName;
        int m_VMCodeCount{0};
    };
}