#pragma once

#include <vector>
#include <sstream>

namespace Utilities
{
    std::string trimSpaceAndComment(const std::string& line);
    std::string trimComment(const std::string& line);
    std::vector<std::string> splitBySpace(const std::string& line);
}