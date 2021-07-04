#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include "Utilities.h"

namespace Compiler
{
    enum class TokenType { KEYWORD, SYMBOL, INT, STRING, IDENTIFIER, INVALID};
    const std::unordered_set<std::string> Keywords{ "class","constructor","function","method","field","static","var","int","char","boolean","void","true","false","null","this","let","do","if","else","while","return" };
    const std::unordered_set<char>Symbols{ '{','}','(',')','[',']','.',',',';','+','-','*','/','&','|','<','>','=','-','~' };
    const size_t maxInt = 32767;
    std::string tokenTypeToString(const Compiler::TokenType& type);
    class Tokenizer
    {
    public:
        void clear() { m_data.clear(); m_currentToken.clear(); }
        bool parse(const fs::path& input);
        bool parseLine(const std::string& line);
        void parseToken();
        void addToken(const std::string& token, TokenType type);
        void printTokens(std::ostream& stream) const;
        bool isKeyWord(const std::string& word) const;
        bool isInt(const std::string& word) const;
        bool isString(const std::string& word) const;
        bool isSymbol(const char symbol) const;
        friend bool operator==(const Tokenizer& lhs, const Tokenizer& rhs);

        size_t numTokens() const { return m_data.size(); }
        const std::vector<std::pair<std::string, TokenType>>& getData() const { return m_data; }
        void resetIndex() { m_tokenIndex = 0;}
        bool hasMoreTokens() const { return m_tokenIndex < m_data.size(); }
        void advance() { ++m_tokenIndex; }
        const std::pair<std::string, TokenType>& getToken(int index) const { return m_data[index]; } 
        const std::pair<std::string, TokenType>& getCurrentToken() const { return m_data[m_tokenIndex]; }
        const std::pair<std::string, TokenType> peekToken(int offset) const;
        const std::string& currentString() const { return m_data[m_tokenIndex].first; } 
        const TokenType currentType() const { return m_data[m_tokenIndex].second; } 

    private:
        char m_currentChar;
        std::string m_currentToken;
        int m_tokenIndex{};
        std::vector<std::pair<std::string, TokenType>> m_data;
    };
}