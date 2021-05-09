#include <fstream>
#include "headers/VMTranslator.h"

namespace VMTranslator
{
    VMCommandType VMTranslator::Translator::commandType(const std::string& line)
    {
        if (line == "add"
            || line == "sub"
            || line == "neg"
            || line == "eq"
            || line == "gt"
            || line == "lt"
            || line == "and"
            || line == "or"
            || line == "not"
            )
        {
            return VMCommandType::C_ARITHMETIC;
        }
        else if(line == "pop")
        {
            return VMCommandType::C_POP;
        }
        else if(line == "push")
        {
            return VMCommandType::C_PUSH;
        }
        else if (line == "label" || line == "goto" || line == "if-goto")
        {
            return VMCommandType::C_GOTO;
        }
        else if (line == "fucntion")
        {
            return VMCommandType::C_FUNCTION;
        }
        else if (line == "return")
        {
            return VMCommandType::C_RETURN;
        }
        else if (line == "call")
        {
            return VMCommandType::C_CALL;
        }
        return VMCommandType::INVALID;
    }

    std::string Translator::arg1(const std::string& line)
    {
        return std::string();
    }

    int Translator::arg2(const std::string& line)
    {
        try
        {
            return std::stoi(line);
        }
        catch(...)
        {
            return -1;
        }
    }

    int VMTranslator::Translator::parse(const std::filesystem::path& inputFile)
    {
        std::filesystem::path outputFile{ inputFile };
        outputFile.replace_extension("asm");

        // Open input
        std::ifstream input{ inputFile };
        if (!input)
        {
            std::cerr << "Unable to open Input File\n";
            return 1;
        }
        std::cout << "Translating " << inputFile << "\nOutputting to " << outputFile << '\n';

        // First Pass
        std::string lineString;
        int lineNumber{ 1 };
        int m_currentVMCode = 0;

        while (input)
        {
            std::getline(input >> std::ws, lineString);
            const auto [error, result] = parseCodeLine(lineString);
            if(!error.empty())
            {
                std::cerr << "ln-" + std::to_string(lineNumber) + ": " + error;
                return 1;
            }
            if (!result.empty())
            {
                m_resultLines.push_back(result);
            }
            lineNumber++;
            lineString.clear();
        }
        return write(outputFile);
    }

    std::pair<std::string, std::string> VMTranslator::Translator::parseCodeLine(const std::string& line, const bool addComment)
    {
        auto codeLine = Utilities::trimComment(line);
        auto splitCode = Utilities::splitBySpace(codeLine);
        if(codeLine.empty() || splitCode.empty()) return {};
        std::string result{};
        if(addComment)
        {
            result += "//";
            for(const auto str : splitCode)
                result += ' ' + str;
            result += '\n';
        }

        auto cmdType = commandType(splitCode[0]);
        if(cmdType == VMCommandType::C_PUSH)
        {
            if(splitCode.size() < 3)
                return {"C_PUSH: Insufficient instructions", ""};

            const std::string valToD = '@'+splitCode[2]+"\nD=A\n";
            const std::string staticVal = '@'+m_fileName+'.'+splitCode[2]+'\n';
            const std::string tempToA = '@'+std::to_string(5+std::stoi(splitCode[2]))+'\n';
            // Set D to constant or located memory value
            if(splitCode[1] == "constant")
                result += valToD;
            else if(splitCode[1] == "local")
                result += valToD + "@LCL\nA=D+M\nD=M\n";
            else if(splitCode[1] == "argument")
                result += valToD + "@ARG\nA=D+M\nD=M\n";
            else if(splitCode[1] == "this")
                result += valToD + "@THIS\nA=D+M\nD=M\n";
            else if(splitCode[1] == "that")
                result += valToD + "@THAT\nA=D+M\nD=M\n";
            else if(splitCode[1] == "static")
                result += valToD + staticVal + "D=M\n";
            else if(splitCode[1] == "temp")
                result += tempToA + "D=M\n";
            else if(splitCode[1] == "pointer" && splitCode[2] == "0")
                result += "@THIS\nD=M\n";
            else if(splitCode[1] == "pointer" && splitCode[2] == "1")
                result += "@THAT\nD=M\n";
            else
                return {"C_POP: Invalid instruction", ""};
            
            // Add to stack and increment stack pointer
            result += "@SP\nA=M\nM=D\n@SP\nM=M+1\n";
        }
        else if(cmdType == VMCommandType::C_POP)
        {
            if(splitCode.size() < 3)
                return {"C_POP: Insufficient instructions", ""};

            const std::string valToD = '@'+splitCode[2]+"\nD=A\n";
            const std::string staticVal = '@'+m_fileName+'.'+splitCode[2]+'\n';
            const std::string tempToA = '@'+std::to_string(5+std::stoi(splitCode[2]))+'\n';
            const std::string decSP = "@SP\nM=M-1\n";
            if(splitCode[1] == "local")
                result += valToD + "@LCL\nM=D+M\n@SP\nAM=M-1\nD=M\n@LCL\nA=M\nM=D\n" + valToD + "@LCL\nM=M-D\n";
            else if(splitCode[1] == "argument")
                result += valToD + "@ARG\nM=D+M\n@SP\nAM=M-1\nD=M\n@ARG\nA=M\nM=D\n" + valToD + "@ARG\nM=M-D\n";
            else if(splitCode[1] == "this")
                result += valToD + "@THIS\nM=D+M\n@SP\nAM=M-1\nD=M\n@THIS\nA=M\nM=D\n" + valToD + "@THIS\nM=M-D\n";
            else if(splitCode[1] == "that")
                result += valToD + "@THAT\nM=D+M\n@SP\nAM=M-1\nD=M\n@THAT\nA=M\nM=D\n" + valToD + "@THAT\nM=M-D\n";
            else if(splitCode[1] == "static")
                result += "@SP\nAM=M-1\nD=M\n" + staticVal+"M=D\n";
            else if(splitCode[1] == "pointer" && splitCode[2] == "0")
                result += "@SP\nAM=M-1\nD=M\n@THIS\nM=D\n" ;
            else if(splitCode[1] == "pointer" && splitCode[2] == "1")
                result += "@SP\nAM=M-1\nD=M\n@THAT\nM=D\n";
            else if(splitCode[1] == "temp")
                result += "@SP\nAM=M-1\nD=M\n" + tempToA + "M=D\n";
            else
                return {"C_POP: Invalid instruction", ""};
            //result += decSP;
        }
        else if(cmdType == VMCommandType::C_ARITHMETIC)
        {
            const std::string id = "."+ std::to_string(m_VMCodeCount);
            if(splitCode[0] == "add")
                result += "@SP\nM=M-1\nA=M\nD=M\nM=0\nA=A-1\nM=D+M\n";
            else if(splitCode[0] == "sub")
                result += "@SP\nM=M-1\nA=M\nD=M\nM=0\nA=A-1\nM=M-D\n";
            else if(splitCode[0] == "neg")
                result += "@SP\nA=M-1\nM=-M\n";
            else if (splitCode[0] == "eq")
                result +=
                "@SP\nAM=M-1\nD=M\nM=0\nA=A-1\nD=M-D\n"
                "@EQ" + id +
                "\nD;JEQ\n@SP\nA=M-1\nM=0\n"
                "@EQEND" + id +
                "\n0;JMP\n"
                "(EQ" + id + ")"
                "\n@SP\nA=M-1\nM=-1\n"
                "(EQEND" + id + ")\n";
            else if (splitCode[0] == "gt")
                result +=
                "@SP\nAM=M-1\nD=M\nM=0\nA=A-1\nD=M-D\n"
                "@GT" + id +
                "\nD;JGT\n@SP\nA=M-1\nM=0\n"
                "@GTEND" + id +
                "\n0;JMP\n"
                "(GT" + id + ")"
                "\n@SP\nA=M-1\nM=-1\n"
                "(GTEND" + id + ")\n";
            else if (splitCode[0] == "lt")
                result +=
                "@SP\nAM=M-1\nD=M\nM=0\nA=A-1\nD=M-D\n"
                "@LT" + id +
                "\nD;JLT\n@SP\nA=M-1\nM=0\n"
                "@LTEND" + id +
                "\n0;JMP\n"
                "(LT" + id + ")"
                "\n@SP\nA=M-1\nM=-1\n"
                "(LTEND" + id + ")\n";
            else if (splitCode[0] == "and")
                result += "@SP\nAM=M-1\nD=M\nA=A-1\nM=D&M\n";
            else if(splitCode[0] == "or" )
                result += "@SP\nAM=M-1\nD=M\nA=A-1\nM=D|M\n";
            else if(splitCode[0] == "not")
                result += "@SP\nA=M-1\nM=!M\n";
            else
                return {"C_ARITHMETIC: Invalid instruction", ""};
        }

        incCodeCount();
        return {"", result};
    }

    int VMTranslator::Translator::write(const std::filesystem::path& outputFile)
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
            std::cout << "Empty VM file\n";
        }
        return 0;
    }
}