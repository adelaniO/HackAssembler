#include <iostream>
#include <fstream>

#include "dirent.h"
#include "Utilities.h"
#include "Tokenizer.h"
#include "CompilationEngine.h"

int compileJackFile(const fs::path& input, bool outputXML)
{
    try
    {
        std::cout << "Compiling " << input.filename() << '\n';
        // Tokenize
        Compiler::Tokenizer tokenizer;
        tokenizer.parse(input);
        // Compile
        fs::path outputVM = input;
        outputVM.replace_extension("vm");
        std::ofstream vmFile(outputVM.fullFileName());
        Compiler::VMWriter vmWriter{&vmFile};
        Compiler::CompilationEngine compiler(&tokenizer, &vmWriter);
        compiler.startCompilation();
        if(outputXML)
        {
            // Write Tokens XML
            std::string tokensFileName = input.directory() + '/' + input.filename() + "T.xml";
            fs::path outputXml{ tokensFileName };
            std::ofstream tokenstFile(outputXml.fullFileName());
            tokenizer.printTokens(tokenstFile);
            tokenstFile.close();

            // Write Compiler XML
            outputXml = input;
            outputXml.replace_extension("xml");
            std::ofstream compilerFile(outputXml.fullFileName());
            compiler.print(compilerFile);
            compilerFile.close();
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what();
        return 1;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    bool outputXML{};
    if (argc <= 1)
    {
        std::cout << "Usage: <input file/directory>" << '\n';
        return 1;
    }
    else if(argc > 2)
    {
        outputXML = argv[2] == "-outputXML";
    }
    
    std::string pathName{argv[1]};
    if(pathName.back() == '\\' || pathName.back() == '/')
        pathName = pathName.substr(0, pathName.size()-1);

    fs::path input{ pathName };
    if (input.extension() == ".jack")
    {
        return compileJackFile(input, outputXML);
    }
    else if(DIR* dir = opendir(argv[1]))
    {
        auto dirEnt = readdir(dir);
        int result = 0;
        while(dirEnt)
        {
            if(dirEnt->d_type == DT_REG || dirEnt->d_type == DT_LNK)
            {
                fs::path curFile(pathName + "/" + dirEnt->d_name);
                if (curFile.extension() == ".jack")
                {
                    int compilerResult = compileJackFile(curFile, outputXML);
                    result = compilerResult > result ? compilerResult : result;
                }
            }
            dirEnt = readdir(dir);
        }
        closedir(dir);
        return result;
    }
    else
    {
        std::cerr << "Invalid InputFile Extension\n";
        return 1;
    }
}