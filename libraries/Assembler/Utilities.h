#pragma once

#include <vector>
#include <sstream>

namespace Utilities
{
    std::string trimSpaceAndComment(const std::string& line);
    std::string trimComment(const std::string& line);
    std::vector<std::string> splitBySpace(const std::string& line);
}

namespace fs
{
    class path
    {
    public:
        path(const std::string& path);
        std::string filename() const { return m_filename; }
        std::string extension() const {return m_ext; }
        void replace_extension(const std::string& newExt);
        std::string fullFileName() const { return m_path + m_filename + m_ext; }
    private:
        std::string m_ext{};
        std::string m_filename{};
        std::string m_path{};
    };
}