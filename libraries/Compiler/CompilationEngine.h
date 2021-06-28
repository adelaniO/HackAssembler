#pragma once

#include <algorithm>
#include "Tokenizer.h"

namespace Compiler
{
    std::unordered_set<std::string> Operators{"+","-","*","/","&","|","<",">","="};
    std::unordered_set<std::string> KeywordConstants{"true","false","null","this"};
    struct XMLWriter
    {
        std::string m_name{};
        int& m_level;
        std::vector<std::string>* m_data;
        XMLWriter() = delete;
        XMLWriter(const std::string& name, int &level, std::vector<std::string>* data) : m_name{ name }, m_level{level}, m_data{ data }
        {
            ++m_level;
            m_data->emplace_back("<"+m_name+">");
        }
        void write(const std::string& tag, const std::string& value)
        {
            m_data->emplace_back("</"+tag+">\n" + value + "<"+tag+">");
        }
        ~XMLWriter()
        {
            --m_level;
            m_data->emplace_back("</"+m_name+">\n");
        };
    };
    class CompilationEngine
    {
    public:
        CompilationEngine(Tokenizer* tokens);
        void startCompilation();
        void compileStatements();
        void compileIfStatement();
        void compileLetStatement();
        void compileWhileStatement();
        void compileDoStatement();
        void compileReturnStatement();
        void compileExpression();
        void compileTerm();
    private:
        bool isOperator(const std::string& symbol);
        void consume();
        void consume(const std::string& word);
        void consumeIdentifier();
        Tokenizer* m_tokens;
        std::vector<std::string> m_data;
        int m_level;
    };
}