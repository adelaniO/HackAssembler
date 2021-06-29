#pragma once

#include "Tokenizer.h"

namespace Compiler
{
    struct XMLWriter
    {
        std::string m_name{};
        int& m_level;
        std::vector<std::string>* m_data;
        XMLWriter() = delete;
        XMLWriter(int &level, std::vector<std::string>* data) : m_name{}, m_level{level}, m_data{ data }
        {
            ++m_level;
        }
        XMLWriter(const std::string& name, int &level, std::vector<std::string>* data) : m_name{ name }, m_level{level}, m_data{ data }
        {
            ++m_level;
            m_data->emplace_back("<"+m_name+">");
        }
        void write(const std::string& tag, const std::string& value)
        {
            m_data->emplace_back("<"+tag+"> " + value + " </"+tag+">");
        }
        ~XMLWriter()
        {
            --m_level;
            if(!m_name.empty()) m_data->emplace_back("</"+m_name+">");
        };
    };
    class CompilationEngine
    {
    public:
        CompilationEngine(Tokenizer* tokens);
        CompilationEngine(const CompilationEngine&) = delete;
        CompilationEngine& operator= (const CompilationEngine&) = delete;
        void startCompilation();
        void compileClassVarDecs();
        void compileStatements();
        void compileIfStatement();
        void compileLetStatement();
        void compileWhileStatement();
        void compileDoStatement();
        void compileReturnStatement();
        void compileExpression();
        void compileTerm();
        const std::vector<std::string>& getData() const { return m_data;}
        const std::string& getDataAt(size_t index) const { return m_data[index];}
    private:
        bool isOperator(const std::string_view symbol);
        void consume();
        void consume(const std::string_view word);
        void consume(std::vector<std::string_view> words, bool includeIdentifiers);
        void consumeIdentifier();
        Tokenizer* m_tokens;
        std::vector<std::string> m_data;
        int m_level{};
        const std::unordered_set<std::string> Operators{ "+","-","*","/","&","|","<",">","=" };
        const std::unordered_set<std::string> KeywordConstants{ "true","false","null","this" };
    };
}