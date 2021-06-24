#include <fstream>
#include "Assembler.h"

namespace Assembler
{
    void Assembler::reset()
    {
        m_nextAvailableVariable = 16;
        m_symbolToValue = initSymbols;
        m_resultLines.clear();
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

    LineParseResult Assembler::parseCodeLine(const std::string& line)
    {
        LineParseResult result;
        std::string lineString = Utilities::trimSpaceAndComment(line);
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

    int Assembler::parse(const fs::path& inputFile)
    {
        fs::path outputFile{ inputFile };
        outputFile.replace_extension("hack");

        // Open input
        std::ifstream input{ inputFile.fullFileName() };
        if (!input)
        {
            std::cout << "Unable to open Input File\n";
            return 1;
        }
        std::cout << "Assembling " << inputFile.fullFileName() << "\nOutputting to " << outputFile.fullFileName() << '\n';

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
                    m_resultLines.push_back(result.bin);
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
        return write(outputFile.fullFileName());
    }

    int Assembler::write(const std::string& outputFile)
    {
        if (!m_resultLines.empty())
        {
            std::ofstream outf{ outputFile };
            if (outf)
            {
                for (const auto line : m_resultLines)
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