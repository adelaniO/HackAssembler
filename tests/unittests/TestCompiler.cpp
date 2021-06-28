#include <gtest/gtest.h>
#include "Tokenizer.h"

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