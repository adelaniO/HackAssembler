#include <fstream>
#include <algorithm>
#include "Tokenizer.h"

namespace Compiler
{
    bool Tokenizer::parse(const fs::path& input)
    {
        std::ifstream inputStream{ input.fullFileName() };
        if (!inputStream)
        {
            std::cerr << "Unable to open Input File\n";
            return false;
        }
        std::cout << "Tokenizing " << input.fullFileName() << '\n';
        std::string lineString;
        while(!inputStream.eof())
        {
            std::getline(inputStream, lineString);
            const auto parsed = parseLine(lineString);
        }
        inputStream.close();
        printTokens(std::cout);
        return true;
    }

    bool Tokenizer::parseLine(const std::string& line)
    {
        const auto codeLine = Utilities::trimComment(line);
        if(codeLine.empty()) return true;
        const auto splitCodeLine = Utilities::splitBySpace(codeLine);
        for(const auto& word : splitCodeLine)
        {
            std::istringstream strStream{ word };
            while (strStream >> m_currentChar)
            {
                if(isSymbol(m_currentChar))
                {
                    parseToken(); // parse existing tokens when a new symbol is hit
                    m_currentToken.clear();
                    addToken({m_currentChar}, TokenType::SYMBOL);
                }
                else
                {
                    m_currentToken += m_currentChar;
                }
            }
            parseToken();
            m_currentToken.clear();
        }
        return true;
    }

    void Tokenizer::parseToken()
    {
        if (!m_currentToken.empty())
        {
            if (isInt(m_currentToken))
            {
                addToken(m_currentToken, TokenType::INT);
            }
            else if (isString(m_currentToken))
            {
                const auto removedQuotes = m_currentToken.substr(1, m_currentToken.size() - 2);
                addToken(removedQuotes, TokenType::STRING);
            }
            else if (isKeyWord(m_currentToken))
            {
                addToken(m_currentToken, TokenType::KEYWORD);
            }
            else // user defined Identifier
            {
                addToken(m_currentToken, TokenType::IDENTIFIER);
            }
        }
    }

    void Tokenizer::addToken(const std::string& token, TokenType type)
    {
        m_data.emplace_back(token, type);
    }

    void Tokenizer::printTokens(std::ostream& stream) const
    {
        for(const auto& [token , type] : m_data)
        {
            std::string typeStr = tokenTypeToString(type);
            stream << "<" << typeStr<< "> " << token << " </" << typeStr << ">\n";
        }
    }

    std::string tokenTypeToString(const Compiler::TokenType& type)
    {
        std::string result;
        if (type == TokenType::IDENTIFIER)
            result = "identifier";
        else if (type == TokenType::INT)
            result = "integerConstant";
        else if (type == TokenType::KEYWORD)
            result = "keyword";
        else if (type == TokenType::STRING)
            result = "stringConstant";
        else if (type == TokenType::SYMBOL)
            result = "symbol";
        return result;
    }

    bool Tokenizer::isKeyWord(const std::string& word) const
    {
        if(word.empty()) return false;
        const auto search = Keywords.find(word);
        return search != Keywords.cend();
    }

    bool Tokenizer::isInt(const std::string& word) const
    {
        if(word.empty()) return false;
        return std::all_of(word.begin(), word.end(), std::isdigit);
    }

    bool Tokenizer::isString(const std::string& word) const
    {
        if(word.empty()) return false;
        return word[0] == '"';
    }

    bool Tokenizer::isSymbol(const char symbol) const
    {
        const auto search = Symbols.find(symbol);
        return search != Symbols.cend();
    }

    bool operator==(const Tokenizer& lhs, const Tokenizer& rhs)
    {
        if(lhs.m_data.size() == rhs.m_data.size())
        {
            for (size_t i{}; i < lhs.m_data.size(); i++)
            {
                if(lhs.m_data[i].first != rhs.m_data[i].first || lhs.m_data[i].second != rhs.m_data[i].second)
                    return false;
            }
            return true;
        }
        return false;
    }
}