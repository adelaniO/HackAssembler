#pragma once

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
        Translator(const fs::path output) : m_output{output}
        {}

        VMCommandType commandType(const std::string& line);

        std::string arg1(const std::string& line);

        int arg2(const std::string& line);

        int parse(const std::vector<fs::path>& inputs);
        int parseUnit(std::istream& input);
        std::pair<std::string, std::string> parseCodeLine(const std::string& line, const bool addComment = true);
        void init();

        int write(const std::string& outputFile);
        void reset()
        {
            m_resultLines.clear();
            m_id = 0;
        }
        int incID() { return m_id++; }
        void setCurrentFile(std::string file) { m_fileName = file; }

    private:
        std::vector<std::string> m_resultLines;
        std::string m_fileName;
        fs::path m_output;
        int m_id{0};
    };
}