#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <bitset>
#include <map>
#include <vector>
#include <filesystem>

#include "Utilities.h"

namespace Assembler
{
    const std::map<std::string, std::vector<bool>> compMap{
        {"0",   { 0, 1, 0, 1, 0, 1, 0 }},
        {"1",   { 1, 1, 1, 1, 1, 1, 0 }},
        {"-1",  { 0, 1, 0, 1, 1, 1, 0 }},
        {"D",   { 0, 0, 1, 1, 0, 0, 0 }},
        {"A",   { 0, 0, 0, 0, 1, 1, 0 }},
        {"!D",  { 1, 0, 1, 1, 0, 0, 0 }},
        {"!A",  { 1, 0, 0, 0, 1, 1, 0 }},
        {"-D",  { 1, 1, 1, 1, 0, 0, 0 }},
        {"-A",  { 1, 1, 0, 0, 1, 1, 0 }},
        {"D+1", { 1, 1, 1, 1, 1, 0, 0 }},
        {"A+1", { 1, 1, 1, 0, 1, 1, 0 }},
        {"D-1", { 0, 1, 1, 1, 0, 0, 0 }},
        {"A-1", { 0, 1, 0, 0, 1, 1, 0 }},
        {"D+A", { 0, 1, 0, 0, 0, 0, 0 }},
        {"D-A", { 1, 1, 0, 0, 1, 0, 0 }},
        {"A-D", { 1, 1, 1, 0, 0, 0, 0 }},
        {"D&A", { 0, 0, 0, 0, 0, 0, 0 }},
        {"D|A", { 1, 0, 1, 0, 1, 0, 0 }},
        {"M",   { 0, 0, 0, 0, 1, 1, 1 }},
        {"!M",  { 1, 0, 0, 0, 1, 1, 1 }},
        {"-M",  { 1, 1, 0, 0, 1, 1, 1 }},
        {"M+1", { 1, 1, 1, 0, 1, 1, 1 }},
        {"M-1", { 0, 1, 0, 0, 1, 1, 1 }},
        {"D+M", { 0, 1, 0, 0, 0, 0, 1 }},
        {"D-M", { 1, 1, 0, 0, 1, 0, 1 }},
        {"M-D", { 1, 1, 1, 0, 0, 0, 1 }},
        {"D&M", { 0, 0, 0, 0, 0, 0, 1 }},
        {"D|M", { 1, 0, 1, 0, 1, 0, 1 }}
    };
    const std::map<std::string, std::vector<bool>> destMap{
        {"null", { 0, 0, 0 }},
        {"M",    { 1, 0, 0 }},
        {"D",    { 0, 1, 0 }},
        {"MD",   { 1, 1, 0 }},
        {"A",    { 0, 0, 1 }},
        {"AM",   { 1, 0, 1 }},
        {"AD",   { 0, 1, 1 }},
        {"AMD",  { 1, 1, 1 }}
    };
    const std::map<std::string, std::vector<bool>> jumpMap{
        {"null", { 0, 0, 0 }},
        {"JGT",  { 1, 0, 0 }},
        {"JEQ",  { 0, 1, 0 }},
        {"JGE",  { 1, 1, 0 }},
        {"JLT",  { 0, 0, 1 }},
        {"JNE",  { 1, 0, 1 }},
        {"JLE",  { 0, 1, 1 }},
        {"JMP",  { 1, 1, 1 }}
    };

    const std::map<std::string, std::bitset<16>> initSymbols{
         {"R0",0}
        ,{"R1",1}
        ,{"R2",2}
        ,{"R3",3}
        ,{"R4",4}
        ,{"R5",5}
        ,{"R6",6}
        ,{"R7",7}
        ,{"R8",8}
        ,{"R9",9}
        ,{"R10",10}
        ,{"R11",11}
        ,{"R12",12}
        ,{"R13",13}
        ,{"R14",14}
        ,{"R15",15}
        ,{"SCREEN",16384}
        ,{"KBD",24576}
        ,{"SP",0}
        ,{"LCL",1}
        ,{"ARG",2}
        ,{"THIS",3}
        ,{"THAT",4}
    };

    struct LineParseResult
    {
        std::bitset<16> bin{};
        std::string error{}, symbol{};
        bool valid{};
    };

    class Assembler
    {
    public:
        int parse(const std::filesystem::path& inputFile);
        void reset();
        std::string parseSymbolLine(const std::string& line, unsigned long& symbolLine);
        LineParseResult parseCodeLine(const std::string& line);
    private:
        bool startswith(const std::string& str, const std::string& cmp);
        void setBits(std::bitset<16>& bits, size_t start, const std::vector<bool>& values);
        int write(const std::filesystem::path& outputFile);
    private:
        std::map<std::string, std::bitset<16>> m_symbolToValue{initSymbols};
        unsigned long m_nextAvailableVariable{16};
        std::vector<std::bitset<16>> m_resultLines;
    };
}