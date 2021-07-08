#pragma once
#include "Tokenizer.h"
#include "SymbolTable.h"
#include "VMWriter.h"

namespace Compiler
{
    class SymbolTable;

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
        CompilationEngine(Tokenizer* tokens, VMWriter* writer = nullptr);
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
        void compileExpressionList(int &nArgs);
        void compileSubroutineCall();
        void compileTerm();
        const std::vector<std::string>& getData() const { return m_data;}
        const std::string& getDataAt(size_t index) const { return m_data[index];}
        const SymbolTable& getSymbolTable() const { return m_symbolTable; }
        void print(std::ostream& stream) const;
        void clearSymbolTable() { m_symbolTable.clear(); }
        const std::string& className() const { return m_className; }
        std::string generateLabelName(const std::string& type, const std::string& id) const { return m_className + '_' + type + id; }
        void clearData()
        {
            m_data.clear();
            m_level = 0;
            if(m_writer) m_writer->clear();
        }
    private:
        bool isOperator(const std::string& symbol) const;
        bool isKeywordConstant(const std::string& word) const;
        bool isType() const;
        bool isStatementStart() const;

        void consume();
        void consume(const std::string& word);
        void consume(const std::vector<std::string>& words, bool includeIdentifiers);
        void consumeIdentifier();
        void consumeType();

        std::string m_className;
        std::pair<Segment, int> symbolInfo(const std::string& identifier) const;

        Tokenizer* m_tokens;
        VMWriter* m_writer;
        SymbolTable m_symbolTable{};
        std::vector<std::string> m_data;
        int m_level{};

        const std::unordered_set<std::string> Operators{ "+","-","*","/","&","|","<",">","=" };
        const std::unordered_set<std::string> KeywordConstants{ "true","false","null","this" };
        const std::vector<std::string> IntegralTypes { "int", "char", "boolean" };
    };
}