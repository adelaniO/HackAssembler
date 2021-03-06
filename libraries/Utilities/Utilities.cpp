#include "Utilities.h"
#include <iomanip>

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
        auto timmedBlockComment = line.substr(0, line.find("/**"));
        return timmedBlockComment.substr(0, timmedBlockComment.find("//")); // Ignore everything after a comment
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

    std::vector<std::string> splitBySpaceKeepQuoted(const std::string& line, bool includeQuotes)
    {
        std::vector<std::string> result;
        std::istringstream ss(line);
        char tmpChar{};
        std::string tmpStr{};
        const auto addWord = [&]()
        {
            result.push_back(tmpStr);
            tmpStr.clear();
        };
        bool betweenQuotes{};
        ss >> std::noskipws;
        while (ss >> tmpChar)
        {
            if(tmpChar == '\"')
            {
                if(includeQuotes && betweenQuotes) tmpStr+=tmpChar;
                if(betweenQuotes || !tmpStr.empty())
                    addWord();
                if(includeQuotes && !betweenQuotes) tmpStr+=tmpChar;
                betweenQuotes = !betweenQuotes;
            }
            else if(std::isblank(tmpChar) && !betweenQuotes && !tmpStr.empty())
            {
                addWord();
            }
            else
            {
               tmpStr+=tmpChar;
            }
        }
        if(!betweenQuotes && !tmpStr.empty())
                addWord();
        return result;
    }

    void xmlSanitise(std::string& data) {
        std::string buffer;
        buffer.reserve(data.size());
        for (size_t pos = 0; pos != data.size(); ++pos) {
            switch (data[pos]) {
            case '&':  buffer.append("&amp;");       break;
            case '\"': buffer.append("&quot;");      break;
            case '\'': buffer.append("&apos;");      break;
            case '<':  buffer.append("&lt;");        break;
            case '>':  buffer.append("&gt;");        break;
            default:   buffer.append(&data[pos], 1); break;
            }
        }
        data.swap(buffer);
    }
}

namespace fs
{
    path::path(const std::string& path)
    {
        const size_t lastDot = path.find_last_of('.');
        const size_t lastSlash = path.find_last_of("/\\");
        if(lastDot != std::string::npos && lastDot > lastSlash)
            m_ext = path.substr(lastDot);
        if(lastSlash != std::string::npos)
            m_filename = path.substr(lastSlash + 1, lastDot - lastSlash - 1);
        m_path = path.substr(0, lastSlash + 1);
    }
    void path::replace_extension(const std::string& newExt)
    {
        m_ext = '.' + newExt;
    }
}