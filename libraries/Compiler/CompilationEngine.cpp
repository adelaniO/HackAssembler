#include "CompilationEngine.h"

namespace Compiler
{
    CompilationEngine::CompilationEngine(Tokenizer* tokens) : m_tokens{tokens}
    {
    }

    void CompilationEngine::startCompilation()
    {
        consume("class");
        XMLWriter writer{"class", m_level, &m_data};
        consumeIdentifier();
        consume("{");
        compileStatements();
        consume("}");
    }

    void CompilationEngine::compileStatements()
    {
        while (m_tokens->hasMoreTokens())
        {
            const auto tokenString = m_tokens->currentString();
            if (tokenString == "let")
                compileLetStatement();
            else if (tokenString == "if")
                compileIfStatement();
            else if (tokenString == "while")
                compileWhileStatement();
            else if (tokenString == "do")
                compileDoStatement();
            else if (tokenString == "return")
                compileReturnStatement();
        }
    }

    void CompilationEngine::compileLetStatement()
    {
        consume("let");
        XMLWriter writer{"letStatement", m_level, &m_data};
        consumeIdentifier();
        if(m_tokens->currentString() == "[")
        {
            consume("[");
            compileExpression();
            consume("]");
        }
        consume("=");
        compileExpression();
    }

    void CompilationEngine::compileWhileStatement()
    {
        consume("while");
        XMLWriter writer{"while", m_level, &m_data};
        consume("(");
        compileExpression();
        consume(")");
        consume("{");
        compileStatements();
        consume("}");
    }

    void CompilationEngine::compileExpression()
    {
        XMLWriter writer{"expression", m_level, &m_data};
        compileTerm();
        while(isOperator(m_tokens->currentString()))
        {
            consume();
            compileTerm();
        }
    }

    bool CompilationEngine::isOperator(const std::string& symbol)
    {
        const auto search = Operators.find(symbol);
        return search != Operators.cend();
    }

    void CompilationEngine::consume()
    {
        const auto& [token, type] = m_tokens->getCurrentToken();
        const auto typeString = tokenTypeToString(type);
        m_data.emplace_back("</" + typeString + ">\n" + token + "<" + typeString + ">");
        m_tokens->advance();
    }

    void CompilationEngine::consume(const std::string& word)
    {
        const auto& [token, type] = m_tokens->getCurrentToken();
        if(word == token)
            consume();
        else
            throw std::invalid_argument{"Compiler expected \"" + word + "\" but saw \"" + token + "\""};
    }

    void CompilationEngine::consumeIdentifier()
    {
        const auto& [token, type] = m_tokens->getCurrentToken();
        if(type == TokenType::IDENTIFIER)
            consume();
        else
            throw std::invalid_argument{"Compiler expected and identifier but saw \"" + tokenTypeToString(type) + "\" type: \"" + token + "\""};
    }
}