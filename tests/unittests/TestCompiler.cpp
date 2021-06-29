#include <gtest/gtest.h>
#include "Tokenizer.h"
#include "CompilationEngine.h"

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
    EXPECT_EQ(m_data.size(), 6);
    EXPECT_EQ(m_data[0], "<class>");
    EXPECT_EQ(m_data[1], "<keyword> class </keyword>");
    EXPECT_EQ(m_data[2], "<identifier> Main </identifier>");
    EXPECT_EQ(m_data[3], "<symbol> { </symbol>");
    EXPECT_EQ(m_data[4], "<symbol> } </symbol>");
    EXPECT_EQ(m_data[5], "</class>");
}

TEST(Compiler, CompileClassVarDecs)
{
    Compiler::Tokenizer t1{};
    t1.parseLine("static boolean test;");
    Compiler::CompilationEngine c1{&t1};
    c1.compileClassVarDecs();
    const auto& m_data = c1.getData();
    EXPECT_EQ(m_data.size(), 6);
    EXPECT_EQ(m_data[0], "<classVarDec>");
    EXPECT_EQ(m_data[1], "<keyword> static </keyword>");
    EXPECT_EQ(m_data[2], "<keyword> boolean </keyword>");
    EXPECT_EQ(m_data[3], "<identifier> test </identifier>");
    EXPECT_EQ(m_data[4], "<symbol> ; </symbol>");
    EXPECT_EQ(m_data[5], "</classVarDec>");

    Compiler::Tokenizer t2{};
    t2.parseLine("static boolean test, more, variables;");
    Compiler::CompilationEngine c2{&t2};
    c2.compileClassVarDecs();
    const auto& m_data2 = c2.getData();
    EXPECT_EQ(m_data2.size(), 10);
    EXPECT_EQ(m_data2[4], "<symbol> , </symbol>");
    EXPECT_EQ(m_data2[5], "<identifier> more </identifier>");
    EXPECT_EQ(m_data2[6], "<symbol> , </symbol>");
    EXPECT_EQ(m_data2[7], "<identifier> variables </identifier>");
    EXPECT_EQ(m_data2[8], "<symbol> ; </symbol>");
    EXPECT_EQ(m_data2[9], "</classVarDec>");
}