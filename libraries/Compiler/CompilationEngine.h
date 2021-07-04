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
            m_data->emplace_back(std::string(m_level*2, ' ') + "<"+m_name+">");
        }
        void write(const std::string& tag, std::string value)
        {
            Utilities::xmlSanitise(value);
            m_data->emplace_back(std::string(m_level*2, ' ') + "<"+tag+"> " + value + " </"+tag+">");
        }
        ~XMLWriter()
        {
            if(!m_name.empty()) m_data->emplace_back(std::string(m_level*2, ' ') + "</"+m_name+">");
            --m_level;
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
        void compileVarDec();
        void compileParameterList();
        void compileSubroutineDecs();
        void compileSubroutineBody();
        void compileStatements();
        void compileIfStatement();
        void compileLetStatement();
        void compileWhileStatement();
        void compileDoStatement();
        void compileReturnStatement();
        void compileExpression();
        void compileExpressionList();
        void compileSubroutineCall();
        void compileTerm();
        const std::vector<std::string>& getData() const { return m_data;}
        const std::string& getDataAt(size_t index) const { return m_data[index];}
        void clearData() { m_data.clear(); m_level = 0; }
        void print(std::ostream& stream) const;
    private:
        bool isOperator(const std::string& symbol);
        bool isKeywordConstant(const std::string& word);
        bool isType();
        bool isStatementStart();
        void consume();
        void consume(const std::string& word);
        void consume(const std::vector<std::string>& words, bool includeIdentifiers);
        void consumeIdentifier();
        void consumeType();
        Tokenizer* m_tokens;
        std::vector<std::string> m_data;
        int m_level{};
        const std::unordered_set<std::string> Operators{ "+","-","*","/","&","|","<",">","=" };
        const std::unordered_set<std::string> KeywordConstants{ "true","false","null","this" };
        const std::vector<std::string> IntegralTypes { "int", "char", "boolean" };
    };
}