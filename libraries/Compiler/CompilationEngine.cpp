#include <algorithm>
#include "CompilationEngine.h"

namespace Compiler
{
    CompilationEngine::CompilationEngine(Tokenizer* tokens) : m_tokens{tokens}
    {
    }

    void CompilationEngine::startCompilation()
    {
        //XMLWriter writer{"class", m_level, &m_data};
        consume("class");
        consumeIdentifier();
        consume("{");
        const auto& token{ m_tokens->currentString() };
        if(token == "static" || token == "field")
            compileClassVarDecs();
        if(isStatementStart())
            compileStatements();
        if(m_tokens->currentString() != "}" && m_tokens->hasMoreTokens())
            compileSubroutineDecs();
        consume("}");
    }

    void CompilationEngine::compileClassVarDecs()
    {
        while(m_tokens->hasMoreTokens() && (m_tokens->currentString() == "static" || m_tokens->currentString() == "field"))
        {
            XMLWriter writer{ "classVarDec", m_level, &m_data };
            consume();
            consume(IntegralTypes, true); // type
            consumeIdentifier(); // first varName
            while(m_tokens->currentString() == ",")
            {
                consume();
                consumeIdentifier(); // comma seperated varNames
            }
            consume(";");
        }
    }

    void CompilationEngine::compileSubroutineDecs()
    {
        while (m_tokens->hasMoreTokens() && (m_tokens->currentString() == "constructor"
            || m_tokens->currentString() == "function"
            || m_tokens->currentString() == "method"
            || m_tokens->currentString() == "void"
            || isType()
            ))
        {
            XMLWriter writer{ "subroutineDec", m_level, &m_data };
            consume();
            isType() ? consume() : consume("void");
            consumeIdentifier(); // Subroutine name
            consume("(");
            compileParameterList();
            consume(")");
            compileSubroutineBody();
        }
    }

    void CompilationEngine::compileSubroutineBody()
    {
        XMLWriter writer{"subroutineBody", m_level, &m_data};
        consume("{");
        while(m_tokens->currentString() == "var")
        {
            compileVarDec();
        }
        if(m_tokens->currentString() != "}")
            compileStatements();
        consume("}");
    }

    void CompilationEngine::compileVarDec()
    {
        XMLWriter writer{"varDec", m_level, &m_data};
        consume("var");
        bool firstVar{true};
        while (m_tokens->currentString() != ";")
        {
            if(firstVar)
                consumeType();
            consumeIdentifier();
            if(m_tokens->currentString() == ",")
                consume();
            firstVar = false;
        }
        consume(";");
    }

    void CompilationEngine::compileParameterList()
    {
        XMLWriter writer{"parameterList", m_level, &m_data};
        while (m_tokens->currentString() != ")")
        {
            consumeType();
            consumeIdentifier();
            if(m_tokens->currentString() == ",")
                consume();
        }
    }

    void CompilationEngine::compileStatements()
    {
        XMLWriter writer{"statements", m_level, &m_data};
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
            else break;
        }
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
        consume(";");
    }

    void CompilationEngine::compileIfStatement()
    {
        XMLWriter writer{"ifStatement", m_level, &m_data};
        consume("if");
        consume("(");
        compileExpression();
        consume(")");
        consume("{");
        //if(m_tokens->currentString() != "}")
            compileStatements();
        consume("}");
        if(m_tokens->hasMoreTokens() && m_tokens->currentString() == "else")
        {
            consume("else");
            consume("{");
            compileStatements();
            consume("}");
        }
    }

    void CompilationEngine::compileWhileStatement()
    {
        XMLWriter writer{"whileStatement", m_level, &m_data};
        consume("while");
        consume("(");
        compileExpression();
        consume(")");
        consume("{");
        compileStatements();
        consume("}");
    }

    void CompilationEngine::compileDoStatement()
    {
        XMLWriter writer{"doStatement", m_level, &m_data};
        consume("do");
        compileSubroutineCall();
        consume(";");
    }

    void CompilationEngine::compileReturnStatement()
    {
        XMLWriter writer{"returnStatement", m_level, &m_data};
        consume("return");
        if(m_tokens->currentString() != ";")
            compileExpression();
        consume(";");
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

    void CompilationEngine::compileExpressionList()
    {
        XMLWriter writer{"expressionList", m_level, &m_data};
        while(m_tokens->currentString() != ")")
        {
            compileExpression();
            if(m_tokens->currentString() == ",")
                consume();
        }
    }

    void CompilationEngine::compileSubroutineCall()
    {
        consumeIdentifier();
        if(m_tokens->currentString() == ".") // class subroutine call
        {
            consume(".");
            consumeIdentifier();
        }
        consume("(");
        compileExpressionList();
        consume(")");
    }

    void CompilationEngine::compileTerm()
    {
        XMLWriter writer{"term", m_level, &m_data};
        const auto [token, type] = m_tokens->getCurrentToken(); 
        if(type == TokenType::INT || type == TokenType::STRING || isKeywordConstant(token))
        {
            consume();
        }
        else if(token == "-" || token == "~") // UnaryOp
        {
            consume();
            compileTerm();
        }
        else if(token == "(") // bracketed term
        {
            consume("(");
            compileExpression();
            consume(")");
        }
        else if(type == TokenType::IDENTIFIER)
        {
            const auto [nextToken, nextType] = m_tokens->peekToken(1);
            if (nextToken == "[") // array
            {
                consumeIdentifier(); // array name
                consume("[");
                compileExpression();
                consume("]");
            }
            else if (nextToken == "(" || nextToken == ".") // subroutine call
            {
                compileSubroutineCall();
            }
            else // varName
            {
                consumeIdentifier();
            }
        }
    }

    void CompilationEngine::print(std::ostream& stream) const
    {
        stream << "<class>\n";
        for (const auto& line : m_data)
        {
            stream << line << "\n";
        }
        stream << "</class>\n";
    }

    bool CompilationEngine::isKeywordConstant(const std::string& word)
    {
        return KeywordConstants.find(word.data()) != KeywordConstants.cend();
    }

    bool CompilationEngine::isOperator(const std::string& symbol)
    {
        return Operators.find(symbol.data()) != Operators.cend();
    }

    bool CompilationEngine::isStatementStart()
    {
        const auto tokenString = m_tokens->currentString();
        return tokenString == "let"
            || tokenString == "if"
            || tokenString == "while"
            || tokenString == "do"
            || tokenString == "return";
    }

    bool CompilationEngine::isType()
    {
        const auto& [token, type] = m_tokens->getCurrentToken();
        bool isIntegralType = std::find(IntegralTypes.begin(), IntegralTypes.end(), token) != IntegralTypes.end();
        return isIntegralType || type == TokenType::IDENTIFIER;
    }

    void CompilationEngine::consume()
    {
        const auto& [token, type] = m_tokens->getCurrentToken();
        const auto tag = tokenTypeToString(type);
        XMLWriter writer{m_level, &m_data};
        writer.write(tag, token);
        m_tokens->advance();
    }

    void CompilationEngine::consume(const std::string& word)
    {
        const auto& [token, type] = m_tokens->getCurrentToken();
        if(word == token)
            consume();
        else
            throw std::invalid_argument{"Compiler expected \"" + std::string{word} + "\" but saw \"" + token + "\""};
    }

    void CompilationEngine::consume(const std::vector<std::string>& words, bool includeIdentifiers)
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

    void CompilationEngine::consumeType()
    {
        const auto& [token, type] = m_tokens->getCurrentToken();
        if(isType())
            consume();
        else
            throw std::invalid_argument{"Compiler expected a type but saw \"" + tokenTypeToString(type) + "\" type: \"" + token + "\""};
    }
}