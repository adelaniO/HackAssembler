#include <gtest/gtest.h>
#include "gmock/gmock.h"
#include "Tokenizer.h"
#include "CompilationEngine.h"


using testing::EndsWith;
using testing::UnorderedElementsAre;
using testing::Pair;
using Compiler::Symbol;
using Compiler::SymbolKind;

TEST(SymbolTables , VariableDeclarations)
{
    Compiler::Tokenizer t1{};
    t1.parseLine("var Array a;");
    Compiler::CompilationEngine c1{&t1};
    c1.compileVarDec();
    {
        const std::unordered_map<std::string, Symbol>& classSymbols = c1.getSymbolTable().getClassSymbols();
        const std::unordered_map<std::string, Symbol>& subSymbols = c1.getSymbolTable().getSubroutineSymbols();
        ASSERT_THAT(classSymbols.size(), 0);
        ASSERT_THAT(subSymbols.size(), 1);
        EXPECT_THAT(subSymbols, UnorderedElementsAre(
            Pair("a", Symbol{ "Array", SymbolKind::VAR, 0 })
        ));
    }

    c1.clearSymbolTable();
    c1.clearData();
    t1.parseLine("var int i, sum;");
    c1.compileVarDec();
    {
        const std::unordered_map<std::string, Symbol>& classSymbols = c1.getSymbolTable().getClassSymbols();
        const std::unordered_map<std::string, Symbol>& subSymbols = c1.getSymbolTable().getSubroutineSymbols();
        ASSERT_THAT(classSymbols.size(), 0);
        ASSERT_THAT(subSymbols.size(), 2);
        EXPECT_THAT(subSymbols, UnorderedElementsAre(
            Pair("i", Symbol{ "int", SymbolKind::VAR, 0 }),
            Pair("sum", Symbol{ "int", SymbolKind::VAR, 1 })
        ));
    }
}

TEST(SymbolTables , ClassVariableDeclarations)
{
    Compiler::Tokenizer t1{};
    t1.parseLine("static boolean test;");
    Compiler::CompilationEngine c1{&t1};
    c1.compileClassVarDecs();
    {
        const std::unordered_map<std::string, Symbol>& classSymbols = c1.getSymbolTable().getClassSymbols();
        const std::unordered_map<std::string, Symbol>& subSymbols = c1.getSymbolTable().getSubroutineSymbols();
        ASSERT_THAT(classSymbols.size(), 1);
        ASSERT_THAT(subSymbols.size(), 0);
        EXPECT_THAT(classSymbols, UnorderedElementsAre(
            Pair("test", Symbol{ "boolean", SymbolKind::STATIC, 0 })
        ));
    }

    c1.clearData();
    c1.clearSymbolTable();
    t1.parseLine("static boolean test, more, variables;");
    c1.compileClassVarDecs();
    {
        const std::unordered_map<std::string, Symbol>& classSymbols = c1.getSymbolTable().getClassSymbols();
        const std::unordered_map<std::string, Symbol>& subSymbols = c1.getSymbolTable().getSubroutineSymbols();
        ASSERT_THAT(classSymbols.size(), 3);
        ASSERT_THAT(subSymbols.size(), 0);
        EXPECT_THAT(classSymbols, UnorderedElementsAre(
            Pair("test", Symbol{ "boolean", SymbolKind::STATIC, 0 }),
            Pair("more", Symbol{ "boolean", SymbolKind::STATIC, 1 }),
            Pair("variables", Symbol{ "boolean", SymbolKind::STATIC, 2 })
        ));
    }
}

TEST(SymbolTables , ClassAndMethod)
{
    Compiler::Tokenizer t1{};
    t1.parseLine("class Point {");
    t1.parseLine("  field int x, y;");
    t1.parseLine("  static int pointCount;");
    t1.parseLine("  method int distance(Point other) {");
    t1.parseLine("    var int dx, dy;");
    t1.parseLine("    return 0;");
    t1.parseLine("  }");
    t1.parseLine("}");
    Compiler::CompilationEngine c1{&t1};
    c1.startCompilation();
    {
        const std::unordered_map<std::string, Symbol>& classSymbols = c1.getSymbolTable().getClassSymbols();
        const std::unordered_map<std::string, Symbol>& subSymbols = c1.getSymbolTable().getSubroutineSymbols();
        ASSERT_THAT(classSymbols.size(), 3);
        EXPECT_THAT(classSymbols, UnorderedElementsAre(
            Pair("x", Symbol{ "int", SymbolKind::FIELD, 0 }),
            Pair("y", Symbol{ "int", SymbolKind::FIELD, 1 }),
            Pair("pointCount", Symbol{ "int", SymbolKind::STATIC, 0 })
        ));
        ASSERT_THAT(subSymbols.size(), 4);
        EXPECT_THAT(subSymbols, UnorderedElementsAre(
            Pair("this", Symbol{ "Point", SymbolKind::ARG, 0 }),
            Pair("other", Symbol{ "Point", SymbolKind::ARG, 1 }),
            Pair("dx", Symbol{ "int", SymbolKind::VAR, 0 }),
            Pair("dy", Symbol{ "int", SymbolKind::VAR, 1 })
        ));
    }
}

TEST(Compiler, CompileIfStatements)
{
    Compiler::Tokenizer t1{};
    Compiler::CompilationEngine c1{&t1};
     // Expressionless condition if
    t1.parseLine("if (key) { let exit = exit; }");
    c1.compileIfStatement();
    {
    }

    // Expression condition if
    c1.clearData();
    t1.parseLine("if (key = 81)  { let exit = true; }     // q key");
    c1.compileIfStatement();
    {
    }

    // Complete Expressionless if/else Statement
    c1.clearData();
    t1.parseLine("if (b) {");
    t1.parseLine("}");
    t1.parseLine("else {");
    t1.parseLine("}");
    c1.compileIfStatement();
    {
    }

    // Complete if/else Statement
    c1.clearData();
    t1.parseLine("if (false) {");
    t1.parseLine("    let s = \"string constant\";");
    t1.parseLine("    let s = null;");
    t1.parseLine("    let a[1] = a[2];");
    t1.parseLine("}");
    t1.parseLine("else {              // There is no else keyword in the Square files.");
    t1.parseLine("    let i = i * (-j);");
    t1.parseLine("    let j = j / (-2);   // note: unary negate constant 2");
    t1.parseLine("    let i = i | j;");
    t1.parseLine("}");
    c1.compileIfStatement();
    {
    }
}

TEST(Compiler, CompileWhileStatements)
{
    Compiler::Tokenizer t1{};
    Compiler::CompilationEngine c1{&t1};
     // Expressionless condition if
    t1.parseLine("while (key) {");
    t1.parseLine("   let key = key;");
    t1.parseLine("   do moveSquare();");
    t1.parseLine("}");
    c1.compileWhileStatement();
    {
    }
}

TEST(Compiler, CompileDoStatements)
{
    Compiler::Tokenizer t1{};
    Compiler::CompilationEngine c1{&t1};
     // Expressionless condition if
    t1.parseLine("do moveSquare();"); // Subroutine call
    c1.compileDoStatement();
    {
    }

    c1.clearData();
    t1.parseLine("do Sys.wait(direction);"); // Class function call
    c1.compileDoStatement();
    {
    }

    c1.clearData();
    t1.parseLine("do Screen.drawRectangle((x + size) - 1, y, x + size, y + size);"); // Class function call with ful expression
    c1.compileDoStatement();
    {
    }
}

TEST(Compiler, CompileReturnStatements)
{
    Compiler::Tokenizer t1{};
    Compiler::CompilationEngine c1{&t1};
     // Expressionless condition if
    t1.parseLine("return;"); // Void return
    c1.compileReturnStatement();
    {
    }

    c1.clearData();
    t1.parseLine("return this;"); // Variable return
    c1.compileReturnStatement();
    {
    }
}

TEST(Compiler, CompileSubroutineDec)
{
    Compiler::Tokenizer t1{};
    Compiler::CompilationEngine c1{&t1};
     // Empty
    t1.parseLine("function void Main() {}");
    c1.compileSubroutineDecs();
    {
    }

    // Non void return type
    c1.clearData();
    t1.parseLine("constructor SquareGame new() {}");
    c1.compileSubroutineDecs();
    {
    }
}