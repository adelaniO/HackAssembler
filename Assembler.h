#pragma once

#include <string>
#include <sstream>
#include <bitset>
#include <map>
#include <vector>
#include <filesystem>

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
        std::string trimSpaceAndComment(const std::string& line);
    private:
        std::map<std::string, std::bitset<16>> m_symbolToValue{initSymbols};
        unsigned long m_nextAvailableVariable{16};
    };

    void Assembler::reset()
    {
        m_nextAvailableVariable = 16;
        m_symbolToValue = initSymbols;
    }
    bool Assembler::startswith(const std::string& str, const std::string& cmp)
    {
        return str.compare(0, cmp.length(), cmp) == 0;
    }

    void Assembler::setBits(std::bitset<16>& bits, size_t start, const std::vector<bool>& values)
    {
        size_t end = start + values.size();
        size_t iVal{};
        for (size_t i = start; i < 15 && i < end; i++)
        {
            bits[i] = values[iVal++];
        }
    }

    std::string Assembler::parseSymbolLine(const std::string& line, unsigned long& symbolLine)
    {
        std::string error{};

        std::string lineString = line.substr(0, line.find("//"));
        if (lineString.empty())
            return error;
        const char& firstChar{ lineString[0] };
        if (firstChar == '(')
        {
            std::string symbol = line.substr(1, line.find(')') - 1);
            if (m_symbolToValue.find(symbol) == m_symbolToValue.end())
            {
                m_symbolToValue[symbol] = symbolLine--;
            }
            else
            {
                error = "Invalid User Symbol";
            }
        }
        symbolLine++;

        return error;
    }

    std::string Assembler::trimSpaceAndComment(const std::string& line)
    {
        std::string lineString = line.substr(0, line.find("//")); // Ignore everything after a comment
        char const* skip_set{ " \t\n" };
        const auto endPos = lineString.find_first_of(skip_set);
        if (endPos != std::string::npos)
            lineString = lineString.substr(0, endPos); // Trim trailing whitespace
        return lineString;
    }

    LineParseResult Assembler::parseCodeLine(const std::string& line)
    {
        LineParseResult result;
        std::string lineString = trimSpaceAndComment(line);
        const char& firstChar{ lineString[0] };
        if (firstChar == '(')
        {
            std::string symbol = line.substr(1, line.find(')'));
            if (!symbol.empty())
            {
                result.symbol = symbol;
            }
            else
            {
                result.error = "Invalid User Symbol";
            }
        }
        else if (firstChar == '@')
        {
            std::stringstream ss(lineString);
            // A Instruction
            bool hasVariable{};
            const auto symbol = lineString.substr(1);
            auto valueItr = m_symbolToValue.find(symbol);
            if (valueItr != m_symbolToValue.end())
            {
                result.bin = valueItr->second;
            }
            else if (symbol.find_first_not_of("0123456789") != std::string::npos)
            {
                m_symbolToValue[symbol] = m_nextAvailableVariable;
                result.bin = m_nextAvailableVariable++;
            }
            else
            {
                unsigned long val;
                char opener;
                ss >> opener;
                if (ss >> val)
                    result.bin = val;
                else
                {
                    m_symbolToValue[symbol] = m_nextAvailableVariable;
                    result.bin = m_nextAvailableVariable++;
                }
            }
            result.bin[15] = false;
            result.valid = true;
        }
        else if (firstChar)
        {
            // C Instruction
            result.bin[15] = true;
            result.bin[14] = true;
            result.bin[13] = true;

            auto destEnd = lineString.find('=');
            auto compEnd = lineString.find(';');
            if (destEnd == std::string::npos) destEnd = 0;
            const bool hasJump{ compEnd != std::string::npos };

            std::string dest = lineString.substr(0, destEnd);
            std::string comp = lineString.substr(destEnd ? destEnd + 1 : 0, compEnd);
            std::string jump = hasJump ? lineString.substr(compEnd + 1) : "";

            if (jump.size())
            {
                const auto jumpIterator = jumpMap.find(jump);
                if (jumpIterator != jumpMap.end())
                    setBits(result.bin, 0, jumpIterator->second);
                else
                    result.error = "Invalid jump";
            }
            if (dest.size())
            {
                const auto destIterator = destMap.find(dest);
                if (destIterator != destMap.end())
                    setBits(result.bin, 3, destIterator->second);
                else
                {
                    result.error = "Invalid destination";
                }
            }
            if (comp.size())
            {
                if (comp.find('M') != std::string::npos) result.bin[12] = true;
                else result.bin[12] = false;
                const auto compIterator = compMap.find(comp);
                if (compIterator != compMap.end())
                    setBits(result.bin, 6, compIterator->second);
                else
                {
                    result.error = "Invalid computation";
                }
            }
            result.valid = true;
        }
        return result;
    }

    int Assembler::parse(const std::filesystem::path& inputFile)
    {
        std::filesystem::path outputFile{ inputFile };
        outputFile.replace_extension("hack");
        std::cout << "Assembling " << inputFile << "\nOutputting to " << outputFile << '\n';

        // Open input
        std::ifstream input{ inputFile };
        if (!input)
        {
            std::cout << "Unable to open Input File\n";
            return 1;
        }

        // First Pass
        std::string lineString;
        unsigned long lineNumber{ 1 };
        unsigned long symbolLine{ 0 };
        while (input)
        {
            std::getline(input >> std::ws, lineString);
            std::string error = parseSymbolLine(lineString, symbolLine);
            if (!error.empty())
            {
                std::cout << "Error Ln-" << lineNumber << '\n';
                return 1;
            }
            lineNumber++;
        }

        // Second Pass
        input.clear();
        input.seekg(0);
        lineNumber = 1;
        std::vector<std::bitset<16>> resultLines;
        while (input)
        {
            lineString.clear();
            std::getline(input >> std::ws, lineString);
            if (lineString.empty()) continue;

            try
            {
                auto result = parseCodeLine(lineString);
                if (!result.error.empty())
                {
                    std::cout << "Error Ln-" << lineNumber << " : " << result.error << '\n';
                    input.close();
                    return 1;
                }
                else if (!result.symbol.empty())
                {
                }
                else if (result.valid)
                {
                    resultLines.push_back(result.bin);
                }
            }
            catch (...)
            {
                std::cout << "Error Parsing file\n";
                break;
            }

            std::cout << lineString << '\n';
            ++lineNumber;
        }

        input.close();

        if (!resultLines.empty())
        {
            std::ofstream outf{ outputFile };
            if (outf)
            {
                for (const auto line : resultLines)
                    outf << line << '\n';
            }
            else
            {
                std::cerr << "Unable to open output file for writing\n";
                return 1;
            }
        }
        else
        {
            std::cout << "Empty Assembly file\n";
        }

        return 0;
    }
}