#include "CompilationEngine.h"

namespace Compiler
{
    CompilationEngine::CompilationEngine(Tokenizer* tokens) : m_tokens{tokens}
    {
    }

    void CompilationEngine::startCompilation()
    {
        XMLWriter writer{"class", m_level, &m_data};
        consume("class");
        consumeIdentifier();
        consume("{");
        compileClassVarDecs();
        compileStatements();
        consume("}");
    }

    void CompilationEngine::compileClassVarDecs()
    {
        if(m_tokens->currentString() != "static" && m_tokens->currentString() != "field")
            return;
        XMLWriter writer{"classVarDec", m_level, &m_data};
        while(m_tokens->hasMoreTokens() && (m_tokens->currentString() == "static" || m_tokens->currentString() == "field"))
        {
            consume();
            consume({ "int", "char", "boolean" }, true); // type
            consumeIdentifier(); // first varName
            while(m_tokens->currentString() == ",")
            {
                consume();
                consumeIdentifier(); // comma seperated varNames
            }
            consume(";");
        }
    }

    void CompilationEngine::compileStatements()
    {
        while (m_tokens->currentString() != "}")
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

    void CompilationEngine::compileIfStatement()
    {
    }

    void CompilationEngine::compileLetStatement()
    {
        XMLWriter writer{"letStatement", m_level, &m_data};
        consume("let");
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

    void CompilationEngine::compileDoStatement()
    {
    }

    void CompilationEngine::compileReturnStatement()
    {
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

    void CompilationEngine::compileTerm()
    {
        XMLWriter writer{"term", m_level, &m_data};
    }

    bool CompilationEngine::isOperator(const std::string_view symbol)
    {
        const auto search = Operators.find(symbol.data());
        return search != Operators.cend();
    }

    void CompilationEngine::consume()
    {
        const auto& [token, type] = m_tokens->getCurrentToken();
        const auto tag = tokenTypeToString(type);
        XMLWriter writer{m_level, &m_data};
        writer.write(tag, token);
        m_tokens->advance();
    }

    void CompilationEngine::consume(const std::string_view word)
    {
        const auto& [token, type] = m_tokens->getCurrentToken();
        if(word == token)
            consume();
        else
            throw std::invalid_argument{"Compiler expected \"" + std::string{word} + "\" but saw \"" + token + "\""};
    }

    void CompilationEngine::consume(std::vector<std::string_view> words, bool includeIdentifiers)
    {
        const auto& [token, type] = m_tokens->getCurrentToken();
        const bool found = std::find(words.begin(), words.end(), token) != words.end();
        if(found || (includeIdentifiers && type == TokenType::IDENTIFIER))
            consume();
        else
        {
            std::string expectedWords{' '};
            for(const auto word : words)
                expectedWords += std::string{word} + ' ';
            throw std::invalid_argument{"Compiler expected the following words [" + expectedWords + "] but saw \"" + token + "\""};
        }
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