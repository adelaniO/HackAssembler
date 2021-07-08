#include <algorithm>
#include "CompilationEngine.h"
#include "SymbolTable.h"

namespace Compiler
{
    CompilationEngine::CompilationEngine(Tokenizer* tokens, VMWriter* writer) : m_tokens{ tokens }, m_writer{ writer }
    {
    }

    void CompilationEngine::startCompilation()
    {
        m_symbolTable.clear();
        consume("class");
        m_className = m_tokens->currentString();
        m_symbolTable.setClassName(m_className);
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
        std::string name, type;
        SymbolKind kind;
        int index;
        while(m_tokens->hasMoreTokens() && (m_tokens->currentString() == "static" || m_tokens->currentString() == "field"))
        {
            XMLWriter xmlWriter{ "classVarDec", m_level, &m_data };
            kind = m_tokens->currentString() == "static" ? SymbolKind::STATIC : SymbolKind::FIELD;
            consume();
            type = m_tokens->currentString();
            consume(IntegralTypes, true); // type
            name = m_tokens->currentString();
            consumeIdentifier(); // first varName
            m_symbolTable.define(name, type, kind);
            while(m_tokens->currentString() == ",")
            {
                consume();
                name = m_tokens->currentString();
                m_symbolTable.define(name, type, kind);
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
            //|| m_tokens->currentString() == "void"
            //|| isType()
            ))
        {
            SubroutineType type = m_tokens->currentString() == "constructor" ? SubroutineType::CONSTRUCTOR : SubroutineType::FUNCTION;
            if(m_tokens->currentString() == "method") type = SubroutineType::METHOD;
            XMLWriter xmlWriter{ "subroutineDec", m_level, &m_data };
            consume();
            isType() ? consume() : consume("void");
            const std::string& subroutineName = m_tokens->currentString();
            consumeIdentifier(); // Subroutine name
            m_symbolTable.startSubroutine(type, subroutineName);
            consume("(");
            compileParameterList();
            consume(")");
            compileSubroutineBody();
        }
    }

    void CompilationEngine::compileSubroutineBody()
    {
        XMLWriter xmlWriter{"subroutineBody", m_level, &m_data};
        consume("{");
        while(m_tokens->currentString() == "var")
        {
            compileVarDec();
        }
        if(m_writer)
        {
            const int nLocals = m_symbolTable.varCount(SymbolKind::VAR);
            m_writer->writeFunction(m_className + '.' + m_symbolTable.currentSubroutine(), nLocals);
            if(m_symbolTable.isMethod())
            {
                m_writer->writePush(Segment::ARG, 0);
                m_writer->writePop(Segment::POINTER, 0);
            }
            else if(m_symbolTable.isConstructor())
            {
                int nBytes = m_symbolTable.varCount(SymbolKind::FIELD);
                m_writer->writePush(Segment::CONSTANT, nBytes);
                m_writer->writeCall("Memory.alloc", 1); // Returns base address of newly created object
                m_writer->writePop(Segment::POINTER, 0);
            }
        }
        if(m_tokens->currentString() != "}")
            compileStatements();
        consume("}");
    }

    void CompilationEngine::compileVarDec()
    {
        XMLWriter xmlWriter{"varDec", m_level, &m_data};
        std::string name, type;
        int index;
        consume("var");
        bool firstVar{true};
        while (m_tokens->currentString() != ";")
        {
            if(firstVar)
            {
                type = m_tokens->currentString();
                consumeType();
            }
            name = m_tokens->currentString();
            consumeIdentifier();
            if(m_tokens->currentString() == ",")
                consume();
            firstVar = false;
            m_symbolTable.define(name, type, SymbolKind::VAR);
        }
        consume(";");
    }

    void CompilationEngine::compileParameterList()
    {
        XMLWriter xmlWriter{"parameterList", m_level, &m_data};
        std::string name, type;
        int index;
        while (m_tokens->currentString() != ")")
        {
            type = m_tokens->currentString();
            consumeType();
            name = m_tokens->currentString();
            consumeIdentifier();
            if(m_tokens->currentString() == ",")
                consume();
            m_symbolTable.define(name, type, SymbolKind::ARG);
        }
    }

    void CompilationEngine::compileStatements()
    {
        XMLWriter xmlWriter{"statements", m_level, &m_data};
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
        XMLWriter xmlWriter{"letStatement", m_level, &m_data};
        consume("let");
        const std::string term{ m_tokens->currentString() };
        const auto [segment, index] = symbolInfo(term);

        consumeIdentifier();
        const bool isArray = m_tokens->currentString() == "[";
        if(isArray)
        {
            consume("[");
            compileExpression();
            consume("]");
            if(m_writer)
            {
                m_writer->writePush(segment, index);
                m_writer->writeArithmetic(Command::ADD);
            }
        }
        consume("=");
        compileExpression();
        if (m_writer)
        {
            if(isArray)
            {
                m_writer->writePop(Segment::TEMP, 0);
                m_writer->writePop(Segment::POINTER, 1);
                m_writer->writePush(Segment::TEMP, 0);
                m_writer->writePop(Segment::THAT, 0);
            }
            else
                m_writer->writePop(segment, index);
        }
        consume(";");
    }

    void CompilationEngine::compileIfStatement()
    {
        const std::string labelId = m_writer ? m_writer->newLabelId() : "";
        const std::string L1 = generateLabelName("IF_FALSE", labelId);
        const std::string L2 = generateLabelName("IF_END", labelId);

        XMLWriter xmlWriter{"ifStatement", m_level, &m_data};
        consume("if");
        consume("(");
        compileExpression();
        consume(")");
        if(m_writer) m_writer->writeArithmetic(Command::NOT);
        if(m_writer) m_writer->writeIf(L1);
        consume("{");
        compileStatements();
        consume("}");
        if(m_writer) m_writer->writeGoto(L2);
        if(m_writer) m_writer->writeLabel(L1);
        if(m_tokens->hasMoreTokens() && m_tokens->currentString() == "else")
        {
            consume("else");
            consume("{");
            compileStatements();
            consume("}");
        }
        if(m_writer) m_writer->writeLabel(L2);
    }

    void CompilationEngine::compileWhileStatement()
    {
        const std::string labelId = m_writer ? m_writer->newLabelId() : "";
        const std::string L1 = generateLabelName("WHILE_EXP", labelId);
        const std::string L2 = generateLabelName("WHILE_END", labelId);
        XMLWriter xmlWriter{"whileStatement", m_level, &m_data};
        if(m_writer) m_writer->writeLabel(L1);
        consume("while");
        consume("(");
        compileExpression();
        consume(")");
        if(m_writer) m_writer->writeArithmetic(Command::NOT);
        if(m_writer) m_writer->writeIf(L2);
        consume("{");
        compileStatements();
        consume("}");
        if(m_writer) m_writer->writeGoto(L1);
        if(m_writer) m_writer->writeLabel(L2);
    }

    void CompilationEngine::compileDoStatement()
    {
        XMLWriter xmlWriter{"doStatement", m_level, &m_data};
        consume("do");
        compileSubroutineCall();
        consume(";");
        if (m_writer) m_writer->writePop(Segment::TEMP, 0);
    }

    void CompilationEngine::compileReturnStatement()
    {
        XMLWriter xmlWriter{"returnStatement", m_level, &m_data};
        consume("return");
        const std::string& expression = m_tokens->currentString();
        if(expression != ";")
            compileExpression();
        else if(m_writer)
            m_writer->writePush(Segment::CONSTANT, 0);
        consume(";");
        if(m_writer) m_writer->writeReturn();
    }

    void CompilationEngine::compileExpression()
    {
        XMLWriter xmlWriter{"expression", m_level, &m_data};
        compileTerm();

        while(isOperator(m_tokens->currentString()))
        {
            const std::string op{ m_tokens->currentString() };
            consume();
            compileTerm();
            if(m_writer)
            {
                if (op == "+")
                    m_writer->writeArithmetic(Command::ADD);
                else if (op == "-")
                    m_writer->writeArithmetic(Command::SUB);
                else if (op == "*")
                    m_writer->writeCall("Math.multiply", 2);
                else if (op == "/")
                    m_writer->writeCall("Math.divide", 2);
                else if (op == "&")
                    m_writer->writeArithmetic(Command::AND);
                else if (op == "|")
                    m_writer->writeArithmetic(Command::OR);
                else if (op == "<")
                    m_writer->writeArithmetic(Command::LT);
                else if (op == ">")
                    m_writer->writeArithmetic(Command::GT);
                else if (op == "=")
                    m_writer->writeArithmetic(Command::EQ);
                else if (op == "~")
                    m_writer->writeArithmetic(Command::NOT);
            }
        }
    }

    void CompilationEngine::compileExpressionList(int &nArgs)
    {
        XMLWriter xmlWriter{"expressionList", m_level, &m_data};
        while(m_tokens->currentString() != ")")
        {
            compileExpression();
            ++nArgs;
            if(m_tokens->currentString() == ",")
                consume();
        }
    }

    void CompilationEngine::compileSubroutineCall()
    {
        std::string className = m_className;
        std::string callName = m_tokens->currentString();
        int nArgs{};
        consumeIdentifier();
        if(m_tokens->currentString() == ".") // class subroutine call
        {
            consume(".");
            className = callName;
            callName = m_tokens->currentString();
            consumeIdentifier();
            const auto [segment, index] = symbolInfo(className);
            if(index >= 0)
            {
                nArgs = 1;
                className = m_symbolTable.typeOf(className);
                m_writer->writePush(segment, index);
            }
        }
        else if(m_writer)
        {
            nArgs = 1;
            m_writer->writePush(Segment::POINTER, 0); // Member call from owning class
        }
        consume("(");
        compileExpressionList(nArgs);
        consume(")");
        if(m_writer) m_writer->writeCall(className + "." + callName, nArgs);
    }

    void CompilationEngine::compileTerm()
    {
        XMLWriter xmlWriter{"term", m_level, &m_data};
        const auto& [token, type] = m_tokens->getCurrentToken();
        if (type == TokenType::INT)
        {
            try
            {
                if (m_writer) m_writer->writePush(Segment::CONSTANT, std::atoi(token.c_str()));
            }
            catch (...)
            {
                throw std::invalid_argument{ "Unable to conver token \'" + token + "\' to Integer" };
            }
            consume();
        }
        else if(type == TokenType::STRING )
        {
            if (m_writer)
            {
                m_writer->writePush(Segment::CONSTANT, static_cast<int>(token.size()));
                m_writer->writeCall("String.new", 1);
                for(const auto& character : token)
                {
                    m_writer->writePush(Segment::CONSTANT, static_cast<int>(character));
                    m_writer->writeCall("String.appendChar", 2);
                }
            }

            consume();
        }
        else if(isKeywordConstant(token))
        {
            std::string val = "0";
            if(token == "true" && m_writer)
            {
                m_writer->writePush(Segment::CONSTANT, 1);
                m_writer->writeArithmetic(Command::NEG);
            }
            else if(token == "this" && m_writer) m_writer->writePush(Segment::POINTER, 0);
            else if(m_writer) m_writer->writePush(Segment::CONSTANT, 0);
            consume();
        }
        else if(token == "-" || token == "~") // UnaryOp
        {
            consume();
            compileTerm();
            if(m_writer)
            {
                if (token == "-")
                    m_writer->writeArithmetic(Command::NEG);
                else 
                    m_writer->writeArithmetic(Command::NOT);
            }
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
            const auto [segment, index] = symbolInfo(token);
            if (nextToken == "[") // array
            {
                consumeIdentifier(); // array name
                consume("[");
                compileExpression();
                consume("]");
                if (m_writer)
                {
                    m_writer->writePush(segment, index);
                    m_writer->writeArithmetic(Command::ADD);
                    m_writer->writePop(Segment::POINTER, 1);
                    m_writer->writePush(Segment::THAT, 0);
                }
            }
            else if (nextToken == "(" || nextToken == ".") // subroutine call
            {
                compileSubroutineCall();
            }
            else // varName
            {
                if(m_writer) m_writer->writePush(segment, m_symbolTable.indexOf(token));
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

    bool CompilationEngine::isKeywordConstant(const std::string& word) const
    {
        return KeywordConstants.find(word.data()) != KeywordConstants.cend();
    }

    bool CompilationEngine::isOperator(const std::string& symbol) const
    {
        return Operators.find(symbol.data()) != Operators.cend();
    }

    bool CompilationEngine::isStatementStart() const
    {
        const auto tokenString = m_tokens->currentString();
        return tokenString == "let"
            || tokenString == "if"
            || tokenString == "while"
            || tokenString == "do"
            || tokenString == "return";
    }

    bool CompilationEngine::isType() const
    {
        const auto& [token, type] = m_tokens->getCurrentToken();
        bool isIntegralType = std::find(IntegralTypes.begin(), IntegralTypes.end(), token) != IntegralTypes.end();
        return isIntegralType || type == TokenType::IDENTIFIER;
    }

    void CompilationEngine::consume()
    {
        const auto& [token, type] = m_tokens->getCurrentToken();
        const auto tag = tokenTypeToString(type);
        XMLWriter xmlWriter{m_level, &m_data};
        xmlWriter.write(tag, token);
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

    std::pair<Segment, int> CompilationEngine::symbolInfo(const std::string& identifier) const
    {
        const int index = m_symbolTable.indexOf(identifier);
        Segment segment = Segment::CONSTANT;
        switch (m_symbolTable.kindOf(identifier))
        {
        case(SymbolKind::STATIC):
            segment = Segment::STATIC;
            break;
        case(SymbolKind::FIELD):
            segment = Segment::THIS;
            break;
        case(SymbolKind::ARG):
            segment = Segment::ARG;
            break;
        case(SymbolKind::VAR):
            segment = Segment::LOCAL;
            break;
        default:
            return { segment, -1 };
            break;
        }
        return {segment, index};
    }
}