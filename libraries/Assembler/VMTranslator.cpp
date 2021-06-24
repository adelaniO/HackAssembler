#include "VMTranslator.h"
#include "VMTranslator.h"
#include <fstream>
#include "VMTranslator.h"

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
        else if (line == "function")
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

    int VMTranslator::Translator::parse(const std::vector<fs::path>& inputs)
    {
        if(inputs.empty())
        {
            std::cerr << "No VM files found\n";
            return 1;
        }

        // Call Sys.init and set Stack pointer to 256
        if(inputs.size() > 1) init();

        std::cout << "Directory -> " << m_output.directory() << '\n';
        std::cout << "__________________________\n";
        std::cout << "| #\t| File\n";
        std::cout << "__________________________\n";
        for (size_t i = 0; i < inputs.size(); i++)
        {
            // Open input
            const fs::path& inputFile = inputs[i];
            setCurrentFile(inputFile.filename());
            std::ifstream inputStream{ inputFile.fullFileName() };
            if (!inputStream)
            {
                std::cerr << "Unable to open Input File\n";
                return 1;
            }
            std::cout << "| " << std::to_string(i) + ":\t| " << inputFile.filename() << '\n';

            int retval = parseUnit(inputStream);
            inputStream.close();
            if (retval) return retval;
        }
        std::cout << "__________________________\n";
        std::cout << "Writing to -> " << m_output.fullFileName() << '\n';
        return write(m_output.fullFileName());
    }

    int Translator::parseUnit(std::istream& input)
    {
        std::string lineString;
        int lineNumber{ 1 };

        while (input)
        {
            std::getline(input >> std::ws, lineString);
            const auto parsed = parseCodeLine(lineString);
            const auto error = parsed.first;
            const auto result = parsed.second;
            if (!error.empty())
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
        return 0;
    }

    void VMTranslator::Translator::init()
    {
        const std::string result = "//Init\n@256\nD=A\n@SP\nM=D\n" + parseCodeLine("call Sys.init 0").second + "\n";
        m_resultLines.push_back(result);
    }

    std::pair<std::string, std::string> VMTranslator::Translator::parseCodeLine(const std::string& line, const bool addComment)
    {
        const auto codeLine = Utilities::trimComment(line);
        const auto splitCode = Utilities::splitBySpace(codeLine);
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
            const std::string id = "."+ std::to_string(incID());
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
        else if(cmdType == VMCommandType::C_GOTO)
        {
            if(splitCode.size() < 2)
                return {"C_GOTO: Insufficient instructions", ""};

            if(splitCode[0] == "goto")
                result += "@" + splitCode[1] + "\n0;JMP\n";
            else if(splitCode[0] == "if-goto")
                result += "@SP\nAM=M-1\nD=M\n@" + splitCode[1] + "\nD;JNE\n";
            else if(splitCode[0] == "label")
                result += "(" + splitCode[1] + ")\n";
        }
        else if(cmdType == VMCommandType::C_FUNCTION)
        {
            if (splitCode.size() < 3)
                return { "C_FUNCTION: Insufficient instructions", "" };

            result += "(" + splitCode[1] + ")\n@SP\nA=M\n";
            const int numVars = std::stoi(splitCode[2]);
            for (int i = 0; i < numVars; i++)
                result += "M=0\nA=A+1\n";
            result += "D=A\n@SP\nM=D\n";
        }
        else if(cmdType == VMCommandType::C_RETURN)
        {
            result += "@LCL\nD=M\n@R13\nM=D\n"; // LCL to temp (endFrame)
            result += "@5\nD=A\n@LCL\nA=M-D\nD=M\n@R14\nM=D\n"; // retAddr to temp
            result += "@SP\nA=M-1\nD=M\n@ARG\nA=M\nM=D\n"; // Move return value to arg0
            result += "@ARG\nD=M+1\n@SP\nM=D\n"; // Reposition SP
            result += "@1\nD=A\n@R13\nA=M-D\nD=M\n@THAT\nM=D\n"; // Restore caller THAT
            result += "@2\nD=A\n@R13\nA=M-D\nD=M\n@THIS\nM=D\n"; // Restore caller THIS
            result += "@3\nD=A\n@R13\nA=M-D\nD=M\n@ARG\nM=D\n"; // Restore caller ARG
            result += "@4\nD=A\n@R13\nA=M-D\nD=M\n@LCL\nM=D\n"; // Restore caller LCL
            result += "@R14\nA=M\n0;JMP\n";
        }
        else if(cmdType == VMCommandType::C_CALL)
        {
            if(splitCode.size() < 3)
                return { "C_CALL: Insufficient instructions", "" };

            const std::string id = "." + std::to_string(incID());
            // Push Return Address
            result += "@RETURN" + id + "\n" + "D=A\n@SP\nA=M\nM=D\n@SP\nM=M+1\n";
            // save Caller state by pushing to stack
            result += "@LCL\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n";
            result += "@ARG\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n";
            result += "@THIS\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n";
            result += "@THAT\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n";
            // set ARG to SP - 5 - numArgs
            result += "@5\nD=A\n@" + splitCode[2] + "\nD=D+A\n@SP\nD=M-D\n@ARG\nM=D\n";
            // set LCL to previous caller SP
            //result += "@5\nD=A\n@SP\nD=M-D\n@LCL\nM=D\n";
            result += "@SP\nD=M\n@LCL\nM=D\n";
            // go to function
            result += "@" + splitCode[1] + "\n0;JMP\n";
            // return address
            result += "(RETURN" + id + ")\n";
        }

        return {"", result};
    }

    int VMTranslator::Translator::write(const std::string& outputFile)
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
                outf.close();
                std::cerr << "Unable to open output file for writing\n";
                return 1;
            }
            outf.close();
        }
        else
        {
            std::cout << "Empty VM file\n";
        }
        return 0;
    }
}