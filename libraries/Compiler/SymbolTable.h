#pragma once
#include <string>
#include <map>
#include <algorithm>
#include <unordered_map>

namespace Compiler
{
    // subroutine or class name if none
    enum class SymbolKind { STATIC, FIELD, ARG, VAR, NONE };
    struct Symbol
    {
        std::string type;
        SymbolKind kind;
        int index;
        bool operator==(const Symbol& other) const
        {
            return type == other.type && kind == other.kind && index == other.index;
        }
    };

    class SymbolTable
    {
    public:
        SymbolTable(){};
        void setClassName(const std::string& name) { m_className = name;}
        void startSubroutine(bool isMethod)
        {
            m_argIndex = 0;
            m_varIndex = 0;
            m_subroutineSymbolTable.clear();
            if(isMethod)
                m_subroutineSymbolTable["this"] = { m_className, SymbolKind::ARG, m_argIndex++ };
        }
        void define(const std::string& name, const std::string& type, SymbolKind kind)
        {
            if(kind == SymbolKind::STATIC)
                m_classSymbolTable[name] = {type, kind, m_indexStatic++};
            else if(kind == SymbolKind::FIELD)
                m_classSymbolTable[name] = {type, kind, m_indexField++};
            else if(kind == SymbolKind::ARG)
                m_subroutineSymbolTable[name] = {type, kind, m_argIndex++};
            else
                m_subroutineSymbolTable[name] = {type, kind, m_varIndex++};
        }

        int varCount(SymbolKind kind) const
        {
            int count = std::count_if(m_classSymbolTable.begin(), m_classSymbolTable.end(), [&](const std::pair<const std::string,Compiler::Symbol>& s) { return s.second.kind == kind;});
            count += std::count_if(m_subroutineSymbolTable.begin(), m_subroutineSymbolTable.end(), [&](const std::pair<const std::string,Compiler::Symbol>& s) { return s.second.kind == kind;});
            return count;
        }

        SymbolKind kindOf(const std::string& name) const { return find(name).kind; } 
        const std::string& typeOf(const std::string& name) const { return find(name).type; }
        int indexOf(const std::string& name) const { return find(name).index; }

        void clear()
        {
            m_classSymbolTable.clear();
            m_subroutineSymbolTable.clear();
            m_indexStatic = 0;
            m_indexField = 0;
            m_argIndex = 0;
            m_varIndex = 0;
        };
        
        const std::unordered_map<std::string, Symbol>& getClassSymbols() const { return m_classSymbolTable;}
        const std::unordered_map<std::string, Symbol>& getSubroutineSymbols() const { return m_subroutineSymbolTable;}
    private:
        const Symbol& find(const std::string& name) const
        {
            auto& search = m_subroutineSymbolTable.find(name);
            if(search != m_subroutineSymbolTable.end())
                return search->second;
            search = m_classSymbolTable.find(name);
            if(search != m_classSymbolTable.end())
                return search->second;
            return m_invalidSymbol;
        }

    private:
        std::string m_className;
        std::unordered_map<std::string, Symbol> m_classSymbolTable;
        std::unordered_map<std::string, Symbol> m_subroutineSymbolTable;
        Symbol m_invalidSymbol = {"", SymbolKind::NONE, 0};
        int m_indexStatic{}, m_indexField{}, m_varIndex{}, m_argIndex{};
    };
}