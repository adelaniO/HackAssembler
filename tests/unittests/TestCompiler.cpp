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

class TestWriter : public Compiler::VMWriter
{
public:
    virtual void write(const std::string line) override { m_data.emplace_back(line); }
    std::vector<std::string> m_data;
    virtual void clear() override
    {
        m_data.clear();
        VMWriter::resetLabelIndex();
    }
};

TEST(Compiler, CompileExpressions)
{
    Compiler::Tokenizer tokens{};
    TestWriter writer{};
    Compiler::CompilationEngine compiler{&tokens, &writer};
    compiler.clearData();
    tokens.parseLine("class Test {");
    tokens.parseLine("   static int a;");
    tokens.parseLine("   field int b;");
    tokens.parseLine("   method int test(int dx) {");
    tokens.parseLine("      let a = b + dx;");
    tokens.parseLine("      return a;");
    tokens.parseLine("   }");
    tokens.parseLine("}");
    compiler.startCompilation();
    {
        ASSERT_THAT(writer.m_data.size(), 9);
        EXPECT_THAT(writer.m_data[0], "function Test.test 0");
        EXPECT_THAT(writer.m_data[1], "push argument 0");
        EXPECT_THAT(writer.m_data[2], "pop pointer 0");
        EXPECT_THAT(writer.m_data[3], "push this 0");
        EXPECT_THAT(writer.m_data[4], "push argument 1");
        EXPECT_THAT(writer.m_data[5], "add");
        EXPECT_THAT(writer.m_data[6], "pop static 0");
        EXPECT_THAT(writer.m_data[7], "push static 0");
        EXPECT_THAT(writer.m_data[8], "return");
    }
}

TEST(Compiler, CompileStringConstants)
{
    Compiler::Tokenizer tokens{};
    TestWriter writer{};
    Compiler::CompilationEngine compiler{&tokens, &writer};
    compiler.clearData();
    tokens.parseLine("class Test {");
    tokens.parseLine("    function void test() {");
    tokens.parseLine("      var String str;");
    tokens.parseLine("      let str = \"STR CON\";");
    tokens.parseLine("      return;");
    tokens.parseLine("    }");
    tokens.parseLine("}");
    tokens.parseLine("");
    tokens.parseLine("");
    tokens.parseLine("");
    compiler.startCompilation();
    {
        size_t i{};
        //ASSERT_THAT(writer.m_data.size(), 20);
        EXPECT_THAT(writer.m_data[i++], "function Test.test 1");
        EXPECT_THAT(writer.m_data[i++], "push constant 7");
        EXPECT_THAT(writer.m_data[i++], "call String.new 1");
        EXPECT_THAT(writer.m_data[i++], "push constant 83");
        EXPECT_THAT(writer.m_data[i++], "call String.appendChar 2");
        EXPECT_THAT(writer.m_data[i++], "push constant 84");
        EXPECT_THAT(writer.m_data[i++], "call String.appendChar 2");
        EXPECT_THAT(writer.m_data[i++], "push constant 82");
        EXPECT_THAT(writer.m_data[i++], "call String.appendChar 2");
        EXPECT_THAT(writer.m_data[i++], "push constant 32");
        EXPECT_THAT(writer.m_data[i++], "call String.appendChar 2");
        EXPECT_THAT(writer.m_data[i++], "push constant 67");
        EXPECT_THAT(writer.m_data[i++], "call String.appendChar 2");
        EXPECT_THAT(writer.m_data[i++], "push constant 79");
        EXPECT_THAT(writer.m_data[i++], "call String.appendChar 2");
        EXPECT_THAT(writer.m_data[i++], "push constant 78");
        EXPECT_THAT(writer.m_data[i++], "call String.appendChar 2");
        EXPECT_THAT(writer.m_data[i++], "pop local 0");
        EXPECT_THAT(writer.m_data[i++], "push constant 0");
        EXPECT_THAT(writer.m_data[i++], "return");
    }
}

TEST(Compiler, CompileUnaryOperators)
{
    Compiler::Tokenizer tokens{};
    TestWriter writer{};
    Compiler::CompilationEngine compiler{&tokens, &writer};
    compiler.clearData();
    tokens.parseLine("class Test {");
    tokens.parseLine("    function void test() {");
    tokens.parseLine("      var int i;");
    tokens.parseLine("      let i = -3;");
    tokens.parseLine("      let i = ~i;");
    tokens.parseLine("      return;");
    tokens.parseLine("    }");
    tokens.parseLine("}");
    compiler.startCompilation();
    {
        size_t i{};
        ASSERT_THAT(writer.m_data.size(), 9);
        EXPECT_THAT(writer.m_data[i++], "function Test.test 1");
        EXPECT_THAT(writer.m_data[i++], "push constant 3");
        EXPECT_THAT(writer.m_data[i++], "neg");
        EXPECT_THAT(writer.m_data[i++], "pop local 0");
        EXPECT_THAT(writer.m_data[i++], "push local 0");
        EXPECT_THAT(writer.m_data[i++], "not");
        EXPECT_THAT(writer.m_data[i++], "pop local 0");
        EXPECT_THAT(writer.m_data[i++], "push constant 0");
        EXPECT_THAT(writer.m_data[i++], "return");
    }
}

TEST(Compiler, CompileIFStatements)
{
    Compiler::Tokenizer tokens{};
    TestWriter writer{};
    Compiler::CompilationEngine compiler{&tokens, &writer};
    compiler.clearData();
    tokens.parseLine("class Test {");
    tokens.parseLine("   function void test(int a) {");
    tokens.parseLine("      if(a){}");
    tokens.parseLine("      return;");
    tokens.parseLine("   }");
    tokens.parseLine("}");
    compiler.startCompilation();
    {
        const std::string L1 = "Test_IF_FALSE0";
        const std::string L2 = "Test_IF_END0";
        ASSERT_THAT(writer.m_data.size(), 9);
        EXPECT_THAT(writer.m_data[0], "function Test.test 0");
        EXPECT_THAT(writer.m_data[1], "push argument 0");
        EXPECT_THAT(writer.m_data[2], "not");
        EXPECT_THAT(writer.m_data[3], "if-goto " + L1);
        // true statements go here
        EXPECT_THAT(writer.m_data[4], "goto " + L2);
        EXPECT_THAT(writer.m_data[5], "label " + L1);
        // false statements go here
        EXPECT_THAT(writer.m_data[6], "label " + L2);
        EXPECT_THAT(writer.m_data[7], "push constant 0");
        EXPECT_THAT(writer.m_data[8], "return");
    }
}

TEST(Compiler, CompileWhileStatements)
{
    Compiler::Tokenizer tokens{};
    TestWriter writer{};
    Compiler::CompilationEngine compiler{&tokens, &writer};
    compiler.clearData();
    tokens.parseLine("class Test {");
    tokens.parseLine("   function void test(int a) {");
    tokens.parseLine("      while(a){}");
    tokens.parseLine("      return;");
    tokens.parseLine("   }");
    tokens.parseLine("}");
    compiler.startCompilation();
    {
        const std::string L1 = "Test_WHILE_EXP0";
        const std::string L2 = "Test_WHILE_END0";
        ASSERT_THAT(writer.m_data.size(), 9);
        EXPECT_THAT(writer.m_data[0], "function Test.test 0");
        EXPECT_THAT(writer.m_data[1], "label " + L1);
        EXPECT_THAT(writer.m_data[2], "push argument 0");
        EXPECT_THAT(writer.m_data[3], "not");
        EXPECT_THAT(writer.m_data[4], "if-goto " + L2);
        // while statements go here
        EXPECT_THAT(writer.m_data[5], "goto " + L1);
        EXPECT_THAT(writer.m_data[6], "label " + L2);
        EXPECT_THAT(writer.m_data[7], "push constant 0");
        EXPECT_THAT(writer.m_data[8], "return");
    }
}

TEST(Compiler, CompileObjectConstruction)
{
    Compiler::Tokenizer tokens{};
    TestWriter writer{};
    Compiler::CompilationEngine compiler{&tokens, &writer};
    compiler.clearData();
    tokens.parseLine("class Test {");
    tokens.parseLine("    constructor Test new()");
    tokens.parseLine("    {");
    tokens.parseLine("      return this;");
    tokens.parseLine("    }");
    tokens.parseLine("    function void test() {");
    tokens.parseLine("      var Test y;");
    tokens.parseLine("      let y = Test.new();");
    tokens.parseLine("      return;");
    tokens.parseLine("   }");
    tokens.parseLine("}");
    compiler.startCompilation();
    {
        ASSERT_THAT(writer.m_data.size(), 11);
        EXPECT_THAT(writer.m_data[0], "function Test.new 0");
        EXPECT_THAT(writer.m_data[1], "push constant 0");
        EXPECT_THAT(writer.m_data[2], "call Memory.alloc 1");
        EXPECT_THAT(writer.m_data[3], "pop pointer 0");
        EXPECT_THAT(writer.m_data[4], "push pointer 0");
        EXPECT_THAT(writer.m_data[5], "return");
        EXPECT_THAT(writer.m_data[6], "function Test.test 1");
        EXPECT_THAT(writer.m_data[7], "call Test.new 0");
        EXPECT_THAT(writer.m_data[8], "pop local 0");
        EXPECT_THAT(writer.m_data[9], "push constant 0");
        EXPECT_THAT(writer.m_data[10],"return");
    }
}

TEST(Compiler, CompileArrays)
{
    Compiler::Tokenizer tokens{};
    TestWriter writer{};
    Compiler::CompilationEngine compiler{&tokens, &writer};
    compiler.clearData();
    tokens.parseLine("class Test {");
    tokens.parseLine("    function void test() {");
    tokens.parseLine("      var Array arr;");
    tokens.parseLine("      var int i;");
    tokens.parseLine("      let i = 0;");
    tokens.parseLine("      let arr = Array.new(10);");
    tokens.parseLine("      let arr[i] = 4;");
    tokens.parseLine("      let arr[1] = 8;");
    tokens.parseLine("      let arr[3] = arr[6];");
    tokens.parseLine("      return;");
    tokens.parseLine("    }");
    tokens.parseLine("}");
    compiler.startCompilation();
    {
        size_t i{};
        ASSERT_THAT(writer.m_data.size(), 36);
        EXPECT_THAT(writer.m_data[i++], "function Test.test 2");
        EXPECT_THAT(writer.m_data[i++], "push constant 0");
        EXPECT_THAT(writer.m_data[i++], "pop local 1");
        EXPECT_THAT(writer.m_data[i++], "push constant 10");
        EXPECT_THAT(writer.m_data[i++], "call Array.new 1");
        EXPECT_THAT(writer.m_data[i++], "pop local 0");
        EXPECT_THAT(writer.m_data[i++], "push local 1");
        EXPECT_THAT(writer.m_data[i++], "push local 0");
        EXPECT_THAT(writer.m_data[i++], "add");
        EXPECT_THAT(writer.m_data[i++], "push constant 4");
        EXPECT_THAT(writer.m_data[i++], "pop temp 0");
        EXPECT_THAT(writer.m_data[i++], "pop pointer 1");
        EXPECT_THAT(writer.m_data[i++], "push temp 0");
        EXPECT_THAT(writer.m_data[i++], "pop that 0");
        EXPECT_THAT(writer.m_data[i++], "push constant 1");
        EXPECT_THAT(writer.m_data[i++], "push local 0");
        EXPECT_THAT(writer.m_data[i++], "add");
        EXPECT_THAT(writer.m_data[i++], "push constant 8");
        EXPECT_THAT(writer.m_data[i++], "pop temp 0");
        EXPECT_THAT(writer.m_data[i++], "pop pointer 1");
        EXPECT_THAT(writer.m_data[i++], "push temp 0");
        EXPECT_THAT(writer.m_data[i++], "pop that 0");
        EXPECT_THAT(writer.m_data[i++], "push constant 3");
        EXPECT_THAT(writer.m_data[i++], "push local 0");
        EXPECT_THAT(writer.m_data[i++], "add");
        EXPECT_THAT(writer.m_data[i++], "push constant 6");
        EXPECT_THAT(writer.m_data[i++], "push local 0");
        EXPECT_THAT(writer.m_data[i++], "add");
        EXPECT_THAT(writer.m_data[i++], "pop pointer 1");
        EXPECT_THAT(writer.m_data[i++], "push that 0");
        EXPECT_THAT(writer.m_data[i++], "pop temp 0");
        EXPECT_THAT(writer.m_data[i++], "pop pointer 1");
        EXPECT_THAT(writer.m_data[i++], "push temp 0");
        EXPECT_THAT(writer.m_data[i++], "pop that 0");
        EXPECT_THAT(writer.m_data[i++], "push constant 0");
        EXPECT_THAT(writer.m_data[i++], "return");
    }
}

TEST(Compiler, CompileFunctionCall)
{
    Compiler::Tokenizer tokens{};
    TestWriter writer{};
    Compiler::CompilationEngine compiler{&tokens, &writer};
    compiler.clearData();
    tokens.parseLine("class Test {");
    tokens.parseLine("    method void test() {");
    tokens.parseLine("      var int y;");
    tokens.parseLine("      let y = Test.test2();");
    tokens.parseLine("      do Test.test2();");
    tokens.parseLine("      return;");
    tokens.parseLine("    }");
    tokens.parseLine("    function int test2() { return 1; }");
    tokens.parseLine("}");
    compiler.startCompilation();
    {
        size_t i{};
        ASSERT_THAT(writer.m_data.size(), 12);
        EXPECT_THAT(writer.m_data[i++], "function Test.test 1");
        EXPECT_THAT(writer.m_data[i++], "push argument 0");
        EXPECT_THAT(writer.m_data[i++], "pop pointer 0");
        EXPECT_THAT(writer.m_data[i++], "call Test.test2 0");
        EXPECT_THAT(writer.m_data[i++], "pop local 0");
        EXPECT_THAT(writer.m_data[i++], "call Test.test2 0");
        EXPECT_THAT(writer.m_data[i++], "pop temp 0");
        EXPECT_THAT(writer.m_data[i++], "push constant 0");
        EXPECT_THAT(writer.m_data[i++], "return");
        EXPECT_THAT(writer.m_data[i++], "function Test.test2 0");
        EXPECT_THAT(writer.m_data[i++], "push constant 1");
        EXPECT_THAT(writer.m_data[i++], "return");
    }
}

TEST(Compiler, CompileMethodCall)
{
    Compiler::Tokenizer tokens{};
    TestWriter writer{};
    Compiler::CompilationEngine compiler{&tokens, &writer};
    compiler.clearData();
    tokens.parseLine("class Test {");
    tokens.parseLine("    constructor Test new() { return this; }");
    tokens.parseLine("    method void test() {");
    tokens.parseLine("      var Test y;");
    tokens.parseLine("      let y = Test.new();");
    tokens.parseLine("      do y.test2();");
    tokens.parseLine("      do test2();");
    tokens.parseLine("      return;");
    tokens.parseLine("    }");
    tokens.parseLine("    method void test2() { return; }");
    tokens.parseLine("}");
    compiler.startCompilation();
    {
        size_t i{};
        ASSERT_THAT(writer.m_data.size(), 24);
        EXPECT_THAT(writer.m_data[i++], "function Test.new 0");
        EXPECT_THAT(writer.m_data[i++], "push constant 0");
        EXPECT_THAT(writer.m_data[i++], "call Memory.alloc 1");
        EXPECT_THAT(writer.m_data[i++], "pop pointer 0");
        EXPECT_THAT(writer.m_data[i++], "push pointer 0");
        EXPECT_THAT(writer.m_data[i++], "return");
        EXPECT_THAT(writer.m_data[i++], "function Test.test 1");
        EXPECT_THAT(writer.m_data[i++], "push argument 0");
        EXPECT_THAT(writer.m_data[i++], "pop pointer 0");
        EXPECT_THAT(writer.m_data[i++], "call Test.new 0");
        EXPECT_THAT(writer.m_data[i++],"pop local 0");
        EXPECT_THAT(writer.m_data[i++],"push local 0");
        EXPECT_THAT(writer.m_data[i++],"call Test.test2 1");
        EXPECT_THAT(writer.m_data[i++],"pop temp 0");
        EXPECT_THAT(writer.m_data[i++],"push pointer 0");
        EXPECT_THAT(writer.m_data[i++],"call Test.test2 1");
        EXPECT_THAT(writer.m_data[i++],"pop temp 0");
        EXPECT_THAT(writer.m_data[i++],"push constant 0");
        EXPECT_THAT(writer.m_data[i++],"return");
        EXPECT_THAT(writer.m_data[i++],"function Test.test2 0");
        EXPECT_THAT(writer.m_data[i++],"push argument 0");
        EXPECT_THAT(writer.m_data[i++],"pop pointer 0");
        EXPECT_THAT(writer.m_data[i++],"push constant 0");
        EXPECT_THAT(writer.m_data[i++],"return");
    }
}