#include <iostream>

#include "dirent.h"
#include "Assembler.h"
#include "VMTranslator.h"

int main(int argc, char* argv[])
{
    if (argc <= 1)
    {
        std::cout << "Usage: <input file/directory>" << '\n';
        return 1;
    }
    
    std::string pathName{argv[1]};
    if(pathName.back() == '\\' || pathName.back() == '/')
        pathName = pathName.substr(0, pathName.size()-1);

    fs::path input{ pathName };
    if (input.extension() == ".asm")
    {
        Assembler::Assembler assembler;
        return assembler.parse(input);
    }
    else if (input.extension() == ".vm")
    {
        std::vector<fs::path> inputs = {input};
        fs::path output = input;
        output.replace_extension("asm");
        VMTranslator::Translator translator(output);
        return translator.parse(inputs);
    }

    if(DIR* dir = opendir(argv[1]))
    {
        const auto penultSlash = pathName.find_last_of("\\/", pathName.size() - 2);
        const auto outputFileName = pathName.substr(penultSlash != std::string::npos ? penultSlash + 1 : 0);
        fs::path output(pathName + '/' + outputFileName + ".asm");
        VMTranslator::Translator translator(output);
        std::vector<fs::path> inputs;
        auto dirEnt = readdir(dir);
        while(dirEnt)
        {
            if(dirEnt->d_type == DT_REG || dirEnt->d_type == DT_LNK)
            {
                fs::path curFile(pathName + "/" + dirEnt->d_name);
                if (curFile.extension() == ".vm")
                {
                    inputs.push_back(curFile);
                }
            }
            dirEnt = readdir(dir);
        }
        closedir(dir);
        return translator.parse(inputs);
    }
    else
    {
        std::cout << "Invalid InputFile Extension\n";
        return 1;
    }
}