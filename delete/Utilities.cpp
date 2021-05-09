#include "Utilities.h"

namespace Utilities
{
    std::string trimSpaceAndComment(const std::string& line)
    {
        std::string lineString = line.substr(0, line.find("//")); // Ignore everything after a comment
        char const* skip_set{ " \t\n" };
        const auto endPos = lineString.find_first_of(skip_set);
        if (endPos != std::string::npos)
            lineString = lineString.substr(0, endPos); // Trim trailing whitespace
        return lineString;
    }
    std::string trimComment(const std::string& line)
    {
        return line.substr(0, line.find("//")); // Ignore everything after a comment
    }

    std::vector<std::string> splitBySpace(const std::string& line)
    {
        std::vector<std::string> result;
        std::istringstream ss(line);
        std::string tmpStr{};
        while (ss >> tmpStr)
        {
            result.push_back(tmpStr);
        }
        return result;
    }
}

namespace fs
{
    path::path(const std::string& path)
    {
        const size_t lastDot = path.find_last_of('.');
        if(lastDot != std::string::npos)
            m_ext = path.substr(lastDot);
        const size_t lastSlash = path.find_last_of("/\\");
        if(lastSlash != std::string::npos)
            m_filename = path.substr(lastSlash + 1, lastDot - lastSlash - 1);
        m_path = path.substr(0, lastSlash + 1);
    }
    void path::replace_extension(const std::string& newExt)
    {
        m_ext = '.' + newExt;
    }
}