#include <gtest/gtest.h>
#include "gmock/gmock.h"
#include "Tokenizer.h"
#include "CompilationEngine.h"


using testing::EndsWith;
using testing::UnorderedElementsAre;
using testing::Pair;
using Compiler::Symbol;
using Compiler::SymbolKind;

TEST(Compiler, TokenSpacing)
{
    Compiler::Tokenizer t1{};
    t1.parseLine("if (x < 0) { let sign = \"negative\"}");
    EXPECT_EQ(t1.numTokens(), 12);
    Compiler::Tokenizer t2{};
    t2.parseLine("if(x<0){let sign=\"negative\"}"); // No Space between symbols
    EXPECT_EQ(t2.numTokens(), 12);
    EXPECT_EQ(t1, t2);
}

TEST(Compiler, TokenComments)
{
    Compiler::Tokenizer t1;
    t1.parseLine("// prints the sign");
    EXPECT_EQ(t1.numTokens(), 0);
    t1.parseLine("let sign = \"negative\";");
    EXPECT_EQ(t1.numTokens(), 5);
}

TEST(Compiler, TokenStrings)
{
    Compiler::Tokenizer t1;
    t1.parseLine("\"negative\";");
    EXPECT_EQ(t1.numTokens(), 2);
    const auto& data = t1.getToken(0);
    const auto expected = "negative";
    EXPECT_EQ(data.first, expected);
}

TEST(Compiler, CompileEmptyClass)
{
    Compiler::Tokenizer t1{};
    t1.parseLine("class Main {}");
    Compiler::CompilationEngine c1{&t1};
    c1.startCompilation();
    const auto m_data = c1.getData();
    ASSERT_EQ(m_data.size(), 4);
    EXPECT_THAT(m_data[0], EndsWith("<keyword> class </keyword>"));
    EXPECT_THAT(m_data[1], EndsWith("<identifier> Main </identifier>"));
    EXPECT_THAT(m_data[2], EndsWith("<symbol> { </symbol>"));
    EXPECT_THAT(m_data[3], EndsWith("<symbol> } </symbol>"));
}

TEST(Compiler, CompileVarDecs)
{
    Compiler::Tokenizer t1{};
    t1.parseLine("var Array a;");
    Compiler::CompilationEngine c1{&t1};
    c1.compileVarDec();
    {
        const auto& m_data = c1.getData();
        ASSERT_EQ(m_data.size(), 6);
        EXPECT_THAT(m_data[0], EndsWith("<varDec>"));
        EXPECT_THAT(m_data[1], EndsWith("  <keyword> var </keyword>"));
        EXPECT_THAT(m_data[2], EndsWith("  <identifier> Array </identifier>"));
        EXPECT_THAT(m_data[3], EndsWith("  <identifier> a </identifier>"));
        EXPECT_THAT(m_data[4], EndsWith("  <symbol> ; </symbol>"));
        EXPECT_THAT(m_data[5], EndsWith("</varDec>"));

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
        const auto& m_data = c1.getData();
        ASSERT_EQ(m_data.size(), 8);
        EXPECT_THAT(m_data[0], EndsWith("<varDec>"));
        EXPECT_THAT(m_data[1], EndsWith("  <keyword> var </keyword>"));
        EXPECT_THAT(m_data[2], EndsWith("  <keyword> int </keyword>"));
        EXPECT_THAT(m_data[3], EndsWith("  <identifier> i </identifier>"));
        EXPECT_THAT(m_data[4], EndsWith("  <symbol> , </symbol>"));
        EXPECT_THAT(m_data[5], EndsWith("  <identifier> sum </identifier>"));
        EXPECT_THAT(m_data[6], EndsWith("  <symbol> ; </symbol>"));
        EXPECT_THAT(m_data[7], EndsWith("</varDec>"));

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

TEST(Compiler, SymbolTables)
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

TEST(Compiler, CompileClassVarDecs)
{
    Compiler::Tokenizer t1{};
    t1.parseLine("static boolean test;");
    Compiler::CompilationEngine c1{&t1};
    c1.compileClassVarDecs();
    {
        const auto& m_data = c1.getData();
        ASSERT_EQ(m_data.size(), 6);
        EXPECT_THAT(m_data[0], EndsWith("<classVarDec>"));
        EXPECT_THAT(m_data[1], EndsWith("<keyword> static </keyword>"));
        EXPECT_THAT(m_data[2], EndsWith("<keyword> boolean </keyword>"));
        EXPECT_THAT(m_data[3], EndsWith("<identifier> test </identifier>"));
        EXPECT_THAT(m_data[4], EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[5], EndsWith("</classVarDec>"));

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
        const auto& m_data = c1.getData();
        ASSERT_EQ(m_data.size(), 10);
        EXPECT_THAT(m_data[0], EndsWith("<classVarDec>"));
        EXPECT_THAT(m_data[1], EndsWith("<keyword> static </keyword>"));
        EXPECT_THAT(m_data[2], EndsWith("<keyword> boolean </keyword>"));
        EXPECT_THAT(m_data[3], EndsWith("<identifier> test </identifier>"));
        EXPECT_THAT(m_data[4], EndsWith("<symbol> , </symbol>"));
        EXPECT_THAT(m_data[5], EndsWith("<identifier> more </identifier>"));
        EXPECT_THAT(m_data[6], EndsWith("<symbol> , </symbol>"));
        EXPECT_THAT(m_data[7], EndsWith("<identifier> variables </identifier>"));
        EXPECT_THAT(m_data[8], EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[9], EndsWith("</classVarDec>"));

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

TEST(Compiler, CompileLetStatements)
{
    Compiler::Tokenizer t1{};
    Compiler::CompilationEngine c1{&t1};
    t1.parseLine("let game = game;");
    c1.compileLetStatement();
    {
        const auto m_data = c1.getData();
        ASSERT_EQ(m_data.size(), 11);
        EXPECT_THAT(m_data[0], EndsWith("<letStatement>"));
        EXPECT_THAT(m_data[1], EndsWith("<keyword> let </keyword>"));
        EXPECT_THAT(m_data[2], EndsWith("<identifier> game </identifier>"));
        EXPECT_THAT(m_data[3], EndsWith("<symbol> = </symbol>"));
        EXPECT_THAT(m_data[4], EndsWith("<expression>"));
        EXPECT_THAT(m_data[5], EndsWith("<term>"));
        EXPECT_THAT(m_data[6], EndsWith("<identifier> game </identifier>"));
        EXPECT_THAT(m_data[7], EndsWith("</term>"));
        EXPECT_THAT(m_data[8], EndsWith("</expression>"));
        EXPECT_THAT(m_data[9], EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[10], EndsWith("</letStatement>"));
    }

    c1.clearData();
    t1.parseLine("let a[i] = Keyboard.readInt(\"ENTER THE NEXT NUMBER: \");");
    c1.compileLetStatement();
    {
        const auto m_data = c1.getData();
        ASSERT_EQ(m_data.size(), 29);
        EXPECT_THAT(m_data[0], EndsWith("<letStatement>"));
        EXPECT_THAT(m_data[1], EndsWith("<keyword> let </keyword>"));
        EXPECT_THAT(m_data[2], EndsWith("<identifier> a </identifier>"));
        EXPECT_THAT(m_data[3], EndsWith("<symbol> [ </symbol>"));
        EXPECT_THAT(m_data[4], EndsWith("<expression>"));
        EXPECT_THAT(m_data[5], EndsWith("<term>"));
        EXPECT_THAT(m_data[6], EndsWith("<identifier> i </identifier>"));
        EXPECT_THAT(m_data[7], EndsWith("</term>"));
        EXPECT_THAT(m_data[8], EndsWith("</expression>"));
        EXPECT_THAT(m_data[9], EndsWith("<symbol> ] </symbol>"));
        EXPECT_THAT(m_data[10], EndsWith("<symbol> = </symbol>"));
        EXPECT_THAT(m_data[11], EndsWith("<expression>"));
        EXPECT_THAT(m_data[12], EndsWith("<term>"));
        EXPECT_THAT(m_data[13], EndsWith("<identifier> Keyboard </identifier>"));
        EXPECT_THAT(m_data[14], EndsWith("<symbol> . </symbol>"));
        EXPECT_THAT(m_data[15], EndsWith("<identifier> readInt </identifier>"));
        EXPECT_THAT(m_data[16], EndsWith("<symbol> ( </symbol>"));
        EXPECT_THAT(m_data[17], EndsWith("<expressionList>"));
        EXPECT_THAT(m_data[18], EndsWith("<expression>"));
        EXPECT_THAT(m_data[19], EndsWith("<term>"));
        EXPECT_THAT(m_data[20], EndsWith("<stringConstant> ENTER THE NEXT NUMBER:  </stringConstant>"));
        EXPECT_THAT(m_data[21], EndsWith("</term>"));
        EXPECT_THAT(m_data[22], EndsWith("</expression>"));
        EXPECT_THAT(m_data[23], EndsWith("</expressionList>"));
        EXPECT_THAT(m_data[24], EndsWith("<symbol> ) </symbol>"));
        EXPECT_THAT(m_data[25], EndsWith("</term>"));
        EXPECT_THAT(m_data[26], EndsWith("</expression>"));
        EXPECT_THAT(m_data[27], EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[28], EndsWith("</letStatement>"));
    }

    c1.clearData();
    t1.parseLine("let a = Array.new(length);");
    c1.compileLetStatement();
    {
        const auto m_data = c1.getData();
        ASSERT_EQ(m_data.size(), 22);
        EXPECT_THAT(m_data[0], EndsWith("<letStatement>"));
        EXPECT_THAT(m_data[1], EndsWith("<keyword> let </keyword>"));
        EXPECT_THAT(m_data[2], EndsWith("<identifier> a </identifier>"));
        EXPECT_THAT(m_data[3], EndsWith("<symbol> = </symbol>"));
        EXPECT_THAT(m_data[4], EndsWith("<expression>"));
        EXPECT_THAT(m_data[5], EndsWith("<term>"));
        EXPECT_THAT(m_data[6], EndsWith("<identifier> Array </identifier>"));
        EXPECT_THAT(m_data[7], EndsWith("<symbol> . </symbol>"));
        EXPECT_THAT(m_data[8], EndsWith("<identifier> new </identifier>"));
        EXPECT_THAT(m_data[9], EndsWith("<symbol> ( </symbol>"));
        EXPECT_THAT(m_data[10], EndsWith("<expressionList>"));
        EXPECT_THAT(m_data[11], EndsWith("<expression>"));
        EXPECT_THAT(m_data[12], EndsWith("<term>"));
        EXPECT_THAT(m_data[13], EndsWith("<identifier> length </identifier>"));
        EXPECT_THAT(m_data[14], EndsWith("</term>"));
        EXPECT_THAT(m_data[15], EndsWith("</expression>"));
        EXPECT_THAT(m_data[16], EndsWith("</expressionList>"));
        EXPECT_THAT(m_data[17], EndsWith("<symbol> ) </symbol>"));
        EXPECT_THAT(m_data[18], EndsWith("</term>"));
        EXPECT_THAT(m_data[19], EndsWith("</expression>"));
        EXPECT_THAT(m_data[20], EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[21], EndsWith("</letStatement>"));
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
        const auto m_data = c1.getData();
        ASSERT_EQ(m_data.size(), 25);
        EXPECT_THAT(m_data[0], EndsWith("<ifStatement>"));
        EXPECT_THAT(m_data[1], EndsWith("<keyword> if </keyword>"));
        EXPECT_THAT(m_data[2], EndsWith("<symbol> ( </symbol>"));
        EXPECT_THAT(m_data[3], EndsWith("<expression>"));
        EXPECT_THAT(m_data[4], EndsWith("<term>"));
        EXPECT_THAT(m_data[5], EndsWith("<identifier> key </identifier>"));
        EXPECT_THAT(m_data[6], EndsWith("</term>"));
        EXPECT_THAT(m_data[7], EndsWith("</expression>"));
        EXPECT_THAT(m_data[8], EndsWith("<symbol> ) </symbol>"));
        EXPECT_THAT(m_data[9], EndsWith("<symbol> { </symbol>"));
        EXPECT_THAT(m_data[10], EndsWith("<statements>"));
        EXPECT_THAT(m_data[11], EndsWith("<letStatement>"));
        EXPECT_THAT(m_data[12], EndsWith("<keyword> let </keyword>"));
        EXPECT_THAT(m_data[13], EndsWith("<identifier> exit </identifier>"));
        EXPECT_THAT(m_data[14], EndsWith("<symbol> = </symbol>"));
        EXPECT_THAT(m_data[15], EndsWith("<expression>"));
        EXPECT_THAT(m_data[16], EndsWith("<term>"));
        EXPECT_THAT(m_data[17], EndsWith("<identifier> exit </identifier>"));
        EXPECT_THAT(m_data[18], EndsWith("</term>"));
        EXPECT_THAT(m_data[19], EndsWith("</expression>"));
        EXPECT_THAT(m_data[20], EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[21], EndsWith("</letStatement>"));
        EXPECT_THAT(m_data[22], EndsWith("</statements>"));
        EXPECT_THAT(m_data[23], EndsWith("<symbol> } </symbol>"));
        EXPECT_THAT(m_data[24], EndsWith("</ifStatement>"));
    }

    // Expression condition if
    c1.clearData();
    t1.parseLine("if (key = 81)  { let exit = true; }     // q key");
    c1.compileIfStatement();
    {
        const auto m_data = c1.getData();
        ASSERT_EQ(m_data.size(), 29);
        EXPECT_THAT(m_data[0], EndsWith("<ifStatement>"));
        EXPECT_THAT(m_data[1], EndsWith("<keyword> if </keyword>"));
        EXPECT_THAT(m_data[2], EndsWith("<symbol> ( </symbol>"));
        EXPECT_THAT(m_data[3], EndsWith("<expression>"));
        EXPECT_THAT(m_data[4], EndsWith("<term>"));
        EXPECT_THAT(m_data[5], EndsWith("<identifier> key </identifier>"));
        EXPECT_THAT(m_data[6], EndsWith("</term>"));
        EXPECT_THAT(m_data[7], EndsWith("<symbol> = </symbol>"));
        EXPECT_THAT(m_data[8], EndsWith("<term>"));
        EXPECT_THAT(m_data[9], EndsWith("<integerConstant> 81 </integerConstant>"));
        EXPECT_THAT(m_data[10], EndsWith("</term>"));
        EXPECT_THAT(m_data[11], EndsWith("</expression>"));
        EXPECT_THAT(m_data[12], EndsWith("<symbol> ) </symbol>"));
        EXPECT_THAT(m_data[13], EndsWith("<symbol> { </symbol>"));
        EXPECT_THAT(m_data[14], EndsWith("<statements>"));
        EXPECT_THAT(m_data[15], EndsWith("<letStatement>"));
        EXPECT_THAT(m_data[16], EndsWith("<keyword> let </keyword>"));
        EXPECT_THAT(m_data[17], EndsWith("<identifier> exit </identifier>"));
        EXPECT_THAT(m_data[18], EndsWith("<symbol> = </symbol>"));
        EXPECT_THAT(m_data[19], EndsWith("<expression>"));
        EXPECT_THAT(m_data[20], EndsWith("<term>"));
        EXPECT_THAT(m_data[21], EndsWith("<keyword> true </keyword>"));
        EXPECT_THAT(m_data[22], EndsWith("</term>"));
        EXPECT_THAT(m_data[23], EndsWith("</expression>"));
        EXPECT_THAT(m_data[24], EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[25], EndsWith("</letStatement>"));
        EXPECT_THAT(m_data[26], EndsWith("</statements>"));
        EXPECT_THAT(m_data[27], EndsWith("<symbol> } </symbol>"));
        EXPECT_THAT(m_data[28], EndsWith("</ifStatement>"));
    }

    // Complete Expressionless if/else Statement
    c1.clearData();
    t1.parseLine("if (b) {");
    t1.parseLine("}");
    t1.parseLine("else {");
    t1.parseLine("}");
    c1.compileIfStatement();
    {
        const auto m_data = c1.getData();
        ASSERT_EQ(m_data.size(), 19);
        EXPECT_THAT(m_data[0], EndsWith("<ifStatement>"));
        EXPECT_THAT(m_data[1], EndsWith("<keyword> if </keyword>"));
        EXPECT_THAT(m_data[2], EndsWith("<symbol> ( </symbol>"));
        EXPECT_THAT(m_data[3], EndsWith("<expression>"));
        EXPECT_THAT(m_data[4], EndsWith("<term>"));
        EXPECT_THAT(m_data[5], EndsWith("<identifier> b </identifier>"));
        EXPECT_THAT(m_data[6], EndsWith("</term>"));
        EXPECT_THAT(m_data[7], EndsWith("</expression>"));
        EXPECT_THAT(m_data[8], EndsWith("<symbol> ) </symbol>"));
        EXPECT_THAT(m_data[9], EndsWith("<symbol> { </symbol>"));
        EXPECT_THAT(m_data[10], EndsWith("<statements>"));
        EXPECT_THAT(m_data[11], EndsWith("</statements>"));
        EXPECT_THAT(m_data[12], EndsWith("<symbol> } </symbol>"));
        EXPECT_THAT(m_data[13], EndsWith("<keyword> else </keyword>"));
        EXPECT_THAT(m_data[14], EndsWith("<symbol> { </symbol>"));
        EXPECT_THAT(m_data[15], EndsWith("<statements>"));
        EXPECT_THAT(m_data[16], EndsWith("</statements>"));
        EXPECT_THAT(m_data[17], EndsWith("<symbol> } </symbol>"));
        EXPECT_THAT(m_data[18], EndsWith("</ifStatement>"));
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
        const auto m_data = c1.getData();
        ASSERT_EQ(m_data.size(), 129);
        EXPECT_THAT(m_data[0], EndsWith("<ifStatement>"));
        EXPECT_THAT(m_data[1], EndsWith("<keyword> if </keyword>"));
        EXPECT_THAT(m_data[2], EndsWith("<symbol> ( </symbol>"));
        EXPECT_THAT(m_data[3], EndsWith("<expression>"));
        EXPECT_THAT(m_data[4], EndsWith("<term>"));
        EXPECT_THAT(m_data[5], EndsWith("<keyword> false </keyword>"));
        EXPECT_THAT(m_data[6], EndsWith("</term>"));
        EXPECT_THAT(m_data[7], EndsWith("</expression>"));
        EXPECT_THAT(m_data[8], EndsWith("<symbol> ) </symbol>"));
        EXPECT_THAT(m_data[9], EndsWith("<symbol> { </symbol>"));
        EXPECT_THAT(m_data[10], EndsWith("<statements>"));
        EXPECT_THAT(m_data[11], EndsWith("<letStatement>"));
        EXPECT_THAT(m_data[12], EndsWith("<keyword> let </keyword>"));
        EXPECT_THAT(m_data[13], EndsWith("<identifier> s </identifier>"));
        EXPECT_THAT(m_data[14], EndsWith("<symbol> = </symbol>"));
        EXPECT_THAT(m_data[15], EndsWith("<expression>"));
        EXPECT_THAT(m_data[16], EndsWith("<term>"));
        EXPECT_THAT(m_data[17], EndsWith("<stringConstant> string constant </stringConstant>"));
        EXPECT_THAT(m_data[18], EndsWith("</term>"));
        EXPECT_THAT(m_data[19], EndsWith("</expression>"));
        EXPECT_THAT(m_data[20], EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[21], EndsWith("</letStatement>"));
        EXPECT_THAT(m_data[22], EndsWith("<letStatement>"));
        EXPECT_THAT(m_data[23], EndsWith("<keyword> let </keyword>"));
        EXPECT_THAT(m_data[24], EndsWith("<identifier> s </identifier>"));
        EXPECT_THAT(m_data[25], EndsWith("<symbol> = </symbol>"));
        EXPECT_THAT(m_data[26], EndsWith("<expression>"));
        EXPECT_THAT(m_data[27], EndsWith("<term>"));
        EXPECT_THAT(m_data[28], EndsWith("<keyword> null </keyword>"));
        EXPECT_THAT(m_data[29], EndsWith("</term>"));
        EXPECT_THAT(m_data[30], EndsWith("</expression>"));
        EXPECT_THAT(m_data[31], EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[32], EndsWith("</letStatement>"));
        EXPECT_THAT(m_data[33], EndsWith("<letStatement>"));
        EXPECT_THAT(m_data[34], EndsWith("<keyword> let </keyword>"));
        EXPECT_THAT(m_data[35], EndsWith("<identifier> a </identifier>"));
        EXPECT_THAT(m_data[36], EndsWith("<symbol> [ </symbol>"));
        EXPECT_THAT(m_data[37], EndsWith("<expression>"));
        EXPECT_THAT(m_data[38], EndsWith("<term>"));
        EXPECT_THAT(m_data[39], EndsWith("<integerConstant> 1 </integerConstant>"));
        EXPECT_THAT(m_data[40], EndsWith("</term>"));
        EXPECT_THAT(m_data[41], EndsWith("</expression>"));
        EXPECT_THAT(m_data[42], EndsWith("<symbol> ] </symbol>"));
        EXPECT_THAT(m_data[43], EndsWith("<symbol> = </symbol>"));
        EXPECT_THAT(m_data[44], EndsWith("<expression>"));
        EXPECT_THAT(m_data[45], EndsWith("<term>"));
        EXPECT_THAT(m_data[46], EndsWith("<identifier> a </identifier>"));
        EXPECT_THAT(m_data[47], EndsWith("<symbol> [ </symbol>"));
        EXPECT_THAT(m_data[48], EndsWith("<expression>"));
        EXPECT_THAT(m_data[49], EndsWith("<term>"));
        EXPECT_THAT(m_data[50], EndsWith("<integerConstant> 2 </integerConstant>"));
        EXPECT_THAT(m_data[51], EndsWith("</term>"));
        EXPECT_THAT(m_data[52], EndsWith("</expression>"));
        EXPECT_THAT(m_data[53], EndsWith("<symbol> ] </symbol>"));
        EXPECT_THAT(m_data[54], EndsWith("</term>"));
        EXPECT_THAT(m_data[55], EndsWith("</expression>"));
        EXPECT_THAT(m_data[56], EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[57], EndsWith("</letStatement>"));
        EXPECT_THAT(m_data[58], EndsWith("</statements>"));
        EXPECT_THAT(m_data[59], EndsWith("<symbol> } </symbol>"));
        EXPECT_THAT(m_data[60], EndsWith("<keyword> else </keyword>"));
        EXPECT_THAT(m_data[61], EndsWith("<symbol> { </symbol>"));
        EXPECT_THAT(m_data[62], EndsWith("<statements>"));
        EXPECT_THAT(m_data[63], EndsWith("<letStatement>"));
        EXPECT_THAT(m_data[64], EndsWith("<keyword> let </keyword>"));
        EXPECT_THAT(m_data[65], EndsWith("<identifier> i </identifier>"));
        EXPECT_THAT(m_data[66], EndsWith("<symbol> = </symbol>"));
        EXPECT_THAT(m_data[67], EndsWith("<expression>"));
        EXPECT_THAT(m_data[68], EndsWith("<term>"));
        EXPECT_THAT(m_data[69], EndsWith("<identifier> i </identifier>"));
        EXPECT_THAT(m_data[70], EndsWith("</term>"));
        EXPECT_THAT(m_data[71], EndsWith("<symbol> * </symbol>"));
        EXPECT_THAT(m_data[72], EndsWith("<term>"));
        EXPECT_THAT(m_data[73], EndsWith("<symbol> ( </symbol>"));
        EXPECT_THAT(m_data[74], EndsWith("<expression>"));
        EXPECT_THAT(m_data[75], EndsWith("<term>"));
        EXPECT_THAT(m_data[76], EndsWith("<symbol> - </symbol>"));
        EXPECT_THAT(m_data[77], EndsWith("<term>"));
        EXPECT_THAT(m_data[78], EndsWith("<identifier> j </identifier>"));
        EXPECT_THAT(m_data[79], EndsWith("</term>"));
        EXPECT_THAT(m_data[80], EndsWith("</term>"));
        EXPECT_THAT(m_data[81], EndsWith("</expression>"));
        EXPECT_THAT(m_data[82], EndsWith("<symbol> ) </symbol>"));
        EXPECT_THAT(m_data[83], EndsWith("</term>"));
        EXPECT_THAT(m_data[84], EndsWith("</expression>"));
        EXPECT_THAT(m_data[85], EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[86], EndsWith("</letStatement>"));
        EXPECT_THAT(m_data[87], EndsWith("<letStatement>"));
        EXPECT_THAT(m_data[88], EndsWith("<keyword> let </keyword>"));
        EXPECT_THAT(m_data[89], EndsWith("<identifier> j </identifier>"));
        EXPECT_THAT(m_data[90], EndsWith("<symbol> = </symbol>"));
        EXPECT_THAT(m_data[91], EndsWith("<expression>"));
        EXPECT_THAT(m_data[92], EndsWith("<term>"));
        EXPECT_THAT(m_data[93], EndsWith("<identifier> j </identifier>"));
        EXPECT_THAT(m_data[94], EndsWith("</term>"));
        EXPECT_THAT(m_data[95], EndsWith("<symbol> / </symbol>"));
        EXPECT_THAT(m_data[96], EndsWith("<term>"));
        EXPECT_THAT(m_data[97], EndsWith("<symbol> ( </symbol>"));
        EXPECT_THAT(m_data[98], EndsWith("<expression>"));
        EXPECT_THAT(m_data[99], EndsWith("<term>"));
        EXPECT_THAT(m_data[100], EndsWith("<symbol> - </symbol>"));
        EXPECT_THAT(m_data[101], EndsWith("<term>"));
        EXPECT_THAT(m_data[102], EndsWith("<integerConstant> 2 </integerConstant>"));
        EXPECT_THAT(m_data[103], EndsWith("</term>"));
        EXPECT_THAT(m_data[104], EndsWith("</term>"));
        EXPECT_THAT(m_data[105], EndsWith("</expression>"));
        EXPECT_THAT(m_data[106], EndsWith("<symbol> ) </symbol>"));
        EXPECT_THAT(m_data[107], EndsWith("</term>"));
        EXPECT_THAT(m_data[108], EndsWith("</expression>"));
        EXPECT_THAT(m_data[109], EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[110], EndsWith("</letStatement>"));
        EXPECT_THAT(m_data[111], EndsWith("<letStatement>"));
        EXPECT_THAT(m_data[112], EndsWith("<keyword> let </keyword>"));
        EXPECT_THAT(m_data[113], EndsWith("<identifier> i </identifier>"));
        EXPECT_THAT(m_data[114], EndsWith("<symbol> = </symbol>"));
        EXPECT_THAT(m_data[115], EndsWith("<expression>"));
        EXPECT_THAT(m_data[116], EndsWith("<term>"));
        EXPECT_THAT(m_data[117], EndsWith("<identifier> i </identifier>"));
        EXPECT_THAT(m_data[118], EndsWith("</term>"));
        EXPECT_THAT(m_data[119], EndsWith("<symbol> | </symbol>"));
        EXPECT_THAT(m_data[120], EndsWith("<term>"));
        EXPECT_THAT(m_data[121], EndsWith("<identifier> j </identifier>"));
        EXPECT_THAT(m_data[122], EndsWith("</term>"));
        EXPECT_THAT(m_data[123], EndsWith("</expression>"));
        EXPECT_THAT(m_data[124], EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[125], EndsWith("</letStatement>"));
        EXPECT_THAT(m_data[126], EndsWith("</statements>"));
        EXPECT_THAT(m_data[127], EndsWith("<symbol> } </symbol>"));
        EXPECT_THAT(m_data[128], EndsWith("</ifStatement>"));
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
        const auto m_data = c1.getData();
        ASSERT_EQ(m_data.size(), 34);
        EXPECT_THAT(m_data[0], EndsWith("<whileStatement>"));
        EXPECT_THAT(m_data[1], EndsWith("<keyword> while </keyword>"));
        EXPECT_THAT(m_data[2], EndsWith("<symbol> ( </symbol>"));
        EXPECT_THAT(m_data[3], EndsWith("<expression>"));
        EXPECT_THAT(m_data[4], EndsWith("<term>"));
        EXPECT_THAT(m_data[5], EndsWith("<identifier> key </identifier>"));
        EXPECT_THAT(m_data[6], EndsWith("</term>"));
        EXPECT_THAT(m_data[7], EndsWith("</expression>"));
        EXPECT_THAT(m_data[8], EndsWith("<symbol> ) </symbol>"));
        EXPECT_THAT(m_data[9], EndsWith("<symbol> { </symbol>"));
        EXPECT_THAT(m_data[10], EndsWith("<statements>"));
        EXPECT_THAT(m_data[11], EndsWith("<letStatement>"));
        EXPECT_THAT(m_data[12], EndsWith("<keyword> let </keyword>"));
        EXPECT_THAT(m_data[13], EndsWith("<identifier> key </identifier>"));
        EXPECT_THAT(m_data[14], EndsWith("<symbol> = </symbol>"));
        EXPECT_THAT(m_data[15], EndsWith("<expression>"));
        EXPECT_THAT(m_data[16], EndsWith("<term>"));
        EXPECT_THAT(m_data[17], EndsWith("<identifier> key </identifier>"));
        EXPECT_THAT(m_data[18], EndsWith("</term>"));
        EXPECT_THAT(m_data[19], EndsWith("</expression>"));
        EXPECT_THAT(m_data[20], EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[21], EndsWith("</letStatement>"));
        EXPECT_THAT(m_data[22], EndsWith("<doStatement>"));
        EXPECT_THAT(m_data[23], EndsWith("<keyword> do </keyword>"));
        EXPECT_THAT(m_data[24], EndsWith("<identifier> moveSquare </identifier>"));
        EXPECT_THAT(m_data[25], EndsWith("<symbol> ( </symbol>"));
        EXPECT_THAT(m_data[26], EndsWith("<expressionList>"));
        EXPECT_THAT(m_data[27], EndsWith("</expressionList>"));
        EXPECT_THAT(m_data[28], EndsWith("<symbol> ) </symbol>"));
        EXPECT_THAT(m_data[29], EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[30], EndsWith("</doStatement>"));
        EXPECT_THAT(m_data[31], EndsWith("</statements>"));
        EXPECT_THAT(m_data[32], EndsWith("<symbol> } </symbol>"));
        EXPECT_THAT(m_data[33], EndsWith("</whileStatement>"));
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
        const auto m_data = c1.getData();
        ASSERT_EQ(m_data.size(), 9);
        EXPECT_THAT(m_data[0], EndsWith("<doStatement>"));
        EXPECT_THAT(m_data[1], EndsWith("<keyword> do </keyword>"));
        EXPECT_THAT(m_data[2], EndsWith("<identifier> moveSquare </identifier>"));
        EXPECT_THAT(m_data[3], EndsWith("<symbol> ( </symbol>"));
        EXPECT_THAT(m_data[4], EndsWith("<expressionList>"));
        EXPECT_THAT(m_data[5], EndsWith("</expressionList>"));
        EXPECT_THAT(m_data[6], EndsWith("<symbol> ) </symbol>"));
        EXPECT_THAT(m_data[7], EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[8], EndsWith("</doStatement>"));
    }

    c1.clearData();
    t1.parseLine("do Sys.wait(direction);"); // Class function call
    c1.compileDoStatement();
    {
        const auto m_data = c1.getData();
        ASSERT_EQ(m_data.size(), 16);
        EXPECT_THAT(m_data[0], EndsWith("<doStatement>"));
        EXPECT_THAT(m_data[1], EndsWith("<keyword> do </keyword>"));
        EXPECT_THAT(m_data[2], EndsWith("<identifier> Sys </identifier>"));
        EXPECT_THAT(m_data[3], EndsWith("<symbol> . </symbol>"));
        EXPECT_THAT(m_data[4], EndsWith("<identifier> wait </identifier>"));
        EXPECT_THAT(m_data[5], EndsWith("<symbol> ( </symbol>"));
        EXPECT_THAT(m_data[6], EndsWith("<expressionList>"));
        EXPECT_THAT(m_data[7], EndsWith("<expression>"));
        EXPECT_THAT(m_data[8], EndsWith("<term>"));
        EXPECT_THAT(m_data[9], EndsWith("<identifier> direction </identifier>"));
        EXPECT_THAT(m_data[10], EndsWith("</term>"));
        EXPECT_THAT(m_data[11], EndsWith("</expression>"));
        EXPECT_THAT(m_data[12], EndsWith("</expressionList>"));
        EXPECT_THAT(m_data[13], EndsWith("<symbol> ) </symbol>"));
        EXPECT_THAT(m_data[14], EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[15], EndsWith("</doStatement>"));
    }

    c1.clearData();
    t1.parseLine("do Screen.drawRectangle((x + size) - 1, y, x + size, y + size);"); // Class function call with ful expression
    c1.compileDoStatement();
    {
        const auto m_data = c1.getData();
        ASSERT_EQ(m_data.size(), 56);
        EXPECT_THAT(m_data[0], EndsWith("<doStatement>"));
        EXPECT_THAT(m_data[1], EndsWith("<keyword> do </keyword>"));
        EXPECT_THAT(m_data[2], EndsWith("<identifier> Screen </identifier>"));
        EXPECT_THAT(m_data[3], EndsWith("<symbol> . </symbol>"));
        EXPECT_THAT(m_data[4], EndsWith("<identifier> drawRectangle </identifier>"));
        EXPECT_THAT(m_data[5], EndsWith("<symbol> ( </symbol>"));
        EXPECT_THAT(m_data[6], EndsWith("<expressionList>"));
        EXPECT_THAT(m_data[7], EndsWith("<expression>"));
        EXPECT_THAT(m_data[8], EndsWith("<term>"));
        EXPECT_THAT(m_data[9], EndsWith("<symbol> ( </symbol>"));
        EXPECT_THAT(m_data[10], EndsWith("<expression>"));
        EXPECT_THAT(m_data[11], EndsWith("<term>"));
        EXPECT_THAT(m_data[12], EndsWith("<identifier> x </identifier>"));
        EXPECT_THAT(m_data[13], EndsWith("</term>"));
        EXPECT_THAT(m_data[14], EndsWith("<symbol> + </symbol>"));
        EXPECT_THAT(m_data[15], EndsWith("<term>"));
        EXPECT_THAT(m_data[16], EndsWith("<identifier> size </identifier>"));
        EXPECT_THAT(m_data[17], EndsWith("</term>"));
        EXPECT_THAT(m_data[18], EndsWith("</expression>"));
        EXPECT_THAT(m_data[19], EndsWith("<symbol> ) </symbol>"));
        EXPECT_THAT(m_data[20], EndsWith("</term>"));
        EXPECT_THAT(m_data[21], EndsWith("<symbol> - </symbol>"));
        EXPECT_THAT(m_data[22], EndsWith("<term>"));
        EXPECT_THAT(m_data[23], EndsWith("<integerConstant> 1 </integerConstant>"));
        EXPECT_THAT(m_data[24], EndsWith("</term>"));
        EXPECT_THAT(m_data[25], EndsWith("</expression>"));
        EXPECT_THAT(m_data[26], EndsWith("<symbol> , </symbol>"));
        EXPECT_THAT(m_data[27], EndsWith("<expression>"));
        EXPECT_THAT(m_data[28], EndsWith("<term>"));
        EXPECT_THAT(m_data[29], EndsWith("<identifier> y </identifier>"));
        EXPECT_THAT(m_data[30], EndsWith("</term>"));
        EXPECT_THAT(m_data[31], EndsWith("</expression>"));
        EXPECT_THAT(m_data[32], EndsWith("<symbol> , </symbol>"));
        EXPECT_THAT(m_data[33], EndsWith("<expression>"));
        EXPECT_THAT(m_data[34], EndsWith("<term>"));
        EXPECT_THAT(m_data[35], EndsWith("<identifier> x </identifier>"));
        EXPECT_THAT(m_data[36], EndsWith("</term>"));
        EXPECT_THAT(m_data[37], EndsWith("<symbol> + </symbol>"));
        EXPECT_THAT(m_data[38], EndsWith("<term>"));
        EXPECT_THAT(m_data[39], EndsWith("<identifier> size </identifier>"));
        EXPECT_THAT(m_data[40], EndsWith("</term>"));
        EXPECT_THAT(m_data[41], EndsWith("</expression>"));
        EXPECT_THAT(m_data[42], EndsWith("<symbol> , </symbol>"));
        EXPECT_THAT(m_data[43], EndsWith("<expression>"));
        EXPECT_THAT(m_data[44], EndsWith("<term>"));
        EXPECT_THAT(m_data[45], EndsWith("<identifier> y </identifier>"));
        EXPECT_THAT(m_data[46], EndsWith("</term>"));
        EXPECT_THAT(m_data[47], EndsWith("<symbol> + </symbol>"));
        EXPECT_THAT(m_data[48], EndsWith("<term>"));
        EXPECT_THAT(m_data[49], EndsWith("<identifier> size </identifier>"));
        EXPECT_THAT(m_data[50], EndsWith("</term>"));
        EXPECT_THAT(m_data[51], EndsWith("</expression>"));
        EXPECT_THAT(m_data[52], EndsWith("</expressionList>"));
        EXPECT_THAT(m_data[53], EndsWith("<symbol> ) </symbol>"));
        EXPECT_THAT(m_data[54], EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[55], EndsWith("</doStatement>"));
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
        const auto m_data = c1.getData();
        ASSERT_EQ(m_data.size(), 4);
        EXPECT_THAT(m_data[0], EndsWith("<returnStatement>"));
        EXPECT_THAT(m_data[1], EndsWith("<keyword> return </keyword>"));
        EXPECT_THAT(m_data[2], EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[3], EndsWith("</returnStatement>"));
    }

    c1.clearData();
    t1.parseLine("return this;"); // Variable return
    c1.compileReturnStatement();
    {
        const auto m_data = c1.getData();
        ASSERT_EQ(m_data.size(), 9);
        EXPECT_THAT(m_data[0], EndsWith("<returnStatement>"));
        EXPECT_THAT(m_data[1], EndsWith("<keyword> return </keyword>"));
        EXPECT_THAT(m_data[2], EndsWith("<expression>"));
        EXPECT_THAT(m_data[3], EndsWith("<term>"));
        EXPECT_THAT(m_data[4], EndsWith("<keyword> this </keyword>"));
        EXPECT_THAT(m_data[5], EndsWith("</term>"));
        EXPECT_THAT(m_data[6], EndsWith("</expression>"));
        EXPECT_THAT(m_data[7], EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[8], EndsWith("</returnStatement>"));
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
        const auto m_data = c1.getData();
        ASSERT_EQ(m_data.size(), 13);
        EXPECT_THAT(m_data[0], EndsWith("<subroutineDec>"));
        EXPECT_THAT(m_data[1], EndsWith("<keyword> function </keyword>"));
        EXPECT_THAT(m_data[2], EndsWith("<keyword> void </keyword>"));
        EXPECT_THAT(m_data[3], EndsWith("<identifier> Main </identifier>"));
        EXPECT_THAT(m_data[4], EndsWith("<symbol> ( </symbol>"));
        EXPECT_THAT(m_data[5], EndsWith("<parameterList>"));
        EXPECT_THAT(m_data[6], EndsWith("</parameterList>"));
        EXPECT_THAT(m_data[7], EndsWith("<symbol> ) </symbol>"));
        EXPECT_THAT(m_data[8], EndsWith("<subroutineBody>"));
        EXPECT_THAT(m_data[9], EndsWith("<symbol> { </symbol>"));
        EXPECT_THAT(m_data[10], EndsWith("<symbol> } </symbol>"));
        EXPECT_THAT(m_data[11], EndsWith("</subroutineBody>"));
        EXPECT_THAT(m_data[12], EndsWith("</subroutineDec>"));
    }

    // Non void return type
    c1.clearData();
    t1.parseLine("constructor SquareGame new() {}");
    c1.compileSubroutineDecs();
    {
        const auto m_data = c1.getData();
        ASSERT_EQ(m_data.size(), 13);
        EXPECT_THAT(m_data[0], EndsWith("<subroutineDec>"));
        EXPECT_THAT(m_data[1], EndsWith("<keyword> constructor </keyword>"));
        EXPECT_THAT(m_data[2], EndsWith("<identifier> SquareGame </identifier>"));
        EXPECT_THAT(m_data[3], EndsWith("<identifier> new </identifier>"));
        EXPECT_THAT(m_data[4], EndsWith("<symbol> ( </symbol>"));
        EXPECT_THAT(m_data[5], EndsWith("<parameterList>"));
        EXPECT_THAT(m_data[6], EndsWith("</parameterList>"));
        EXPECT_THAT(m_data[7], EndsWith("<symbol> ) </symbol>"));
        EXPECT_THAT(m_data[8], EndsWith("<subroutineBody>"));
        EXPECT_THAT(m_data[9], EndsWith("<symbol> { </symbol>"));
        EXPECT_THAT(m_data[10], EndsWith("<symbol> } </symbol>"));
        EXPECT_THAT(m_data[11], EndsWith("</subroutineBody>"));
        EXPECT_THAT(m_data[12], EndsWith("</subroutineDec>"));
    }
}