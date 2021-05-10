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

    fs::path input{ argv[1] };
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
        const std::string dirName{argv[1]};
        fs::path output(dirName + '/' + input.filename() + ".asm");
        VMTranslator::Translator translator(output);
        std::vector<fs::path> inputs;
        auto dirEnt = readdir(dir);
        while(dirEnt)
        {
            if(dirEnt->d_type == DT_REG || dirEnt->d_type == DT_LNK)
            {
                fs::path curFile(dirName + "/" + dirEnt->d_name);
                if (curFile.extension() == ".vm")
                {
                    inputs.push_back(curFile);
                    std::cout << '\n' << curFile.filename();
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