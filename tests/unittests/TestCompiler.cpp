#include <gtest/gtest.h>
#include "gmock/gmock.h"
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

//TEST(Compiler, CompileEmptyClass)
//{
//    Compiler::Tokenizer t1{};
//    t1.parseLine("class Main {}");
//    Compiler::CompilationEngine c1{&t1};
//    c1.startCompilation();
//    const auto m_data = c1.getData();
//    EXPECT_EQ(m_data.size(), 6);
//    EXPECT_THAT(m_data[0], ::testing::EndsWith("<class>"));
//    EXPECT_THAT(m_data[1], ::testing::EndsWith("<keyword> class </keyword>"));
//    EXPECT_THAT(m_data[2], ::testing::EndsWith("<identifier> Main </identifier>"));
//    EXPECT_THAT(m_data[3], ::testing::EndsWith("<symbol> { </symbol>"));
//    EXPECT_THAT(m_data[4], ::testing::EndsWith("<symbol> } </symbol>"));
//    EXPECT_THAT(m_data[5], ::testing::EndsWith("</class>"));
//}

TEST(Compiler, CompileVarDecs)
{
    Compiler::Tokenizer t1{};
    t1.parseLine("var Array a;");
    Compiler::CompilationEngine c1{&t1};
    c1.compileVarDec();
    {
        const auto& m_data = c1.getData();
        EXPECT_EQ(m_data.size(), 6);
        EXPECT_THAT(m_data[0], ::testing::EndsWith("<varDec>"));
        EXPECT_THAT(m_data[1], ::testing::EndsWith("  <keyword> var </keyword>"));
        EXPECT_THAT(m_data[2], ::testing::EndsWith("  <identifier> Array </identifier>"));
        EXPECT_THAT(m_data[3], ::testing::EndsWith("  <identifier> a </identifier>"));
        EXPECT_THAT(m_data[4], ::testing::EndsWith("  <symbol> ; </symbol>"));
        EXPECT_THAT(m_data[5], ::testing::EndsWith("</varDec>"));
    }

    c1.clearData();
    t1.parseLine("var int i, sum;");
    c1.compileVarDec();
    {
        const auto& m_data = c1.getData();
        EXPECT_EQ(m_data.size(), 8);
        EXPECT_THAT(m_data[0], ::testing::EndsWith("<varDec>"));
        EXPECT_THAT(m_data[1], ::testing::EndsWith("  <keyword> var </keyword>"));
        EXPECT_THAT(m_data[2], ::testing::EndsWith("  <keyword> int </keyword>"));
        EXPECT_THAT(m_data[3], ::testing::EndsWith("  <identifier> i </identifier>"));
        EXPECT_THAT(m_data[4], ::testing::EndsWith("  <symbol> , </symbol>"));
        EXPECT_THAT(m_data[5], ::testing::EndsWith("  <identifier> sum </identifier>"));
        EXPECT_THAT(m_data[6], ::testing::EndsWith("  <symbol> ; </symbol>"));
        EXPECT_THAT(m_data[7], ::testing::EndsWith("</varDec>"));
    }
}
TEST(Compiler, CompileClassVarDecs)
{
    Compiler::Tokenizer t1{};
    t1.parseLine("static boolean test;");
    Compiler::CompilationEngine c1{&t1};
    c1.compileClassVarDecs();
    const auto& m_data = c1.getData();
    EXPECT_EQ(m_data.size(), 6);
    EXPECT_THAT(m_data[0], ::testing::EndsWith("<classVarDec>"));
    EXPECT_THAT(m_data[1], ::testing::EndsWith("<keyword> static </keyword>"));
    EXPECT_THAT(m_data[2], ::testing::EndsWith("<keyword> boolean </keyword>"));
    EXPECT_THAT(m_data[3], ::testing::EndsWith("<identifier> test </identifier>"));
    EXPECT_THAT(m_data[4], ::testing::EndsWith("<symbol> ; </symbol>"));
    EXPECT_THAT(m_data[5], ::testing::EndsWith("</classVarDec>"));

    Compiler::Tokenizer t2{};
    t2.parseLine("static boolean test, more, variables;");
    Compiler::CompilationEngine c2{&t2};
    c2.compileClassVarDecs();
    const auto& m_data2 = c2.getData();
    EXPECT_EQ(m_data2.size(), 10);
    EXPECT_THAT(m_data2[4], ::testing::EndsWith("<symbol> , </symbol>"));
    EXPECT_THAT(m_data2[5], ::testing::EndsWith("<identifier> more </identifier>"));
    EXPECT_THAT(m_data2[6], ::testing::EndsWith("<symbol> , </symbol>"));
    EXPECT_THAT(m_data2[7], ::testing::EndsWith("<identifier> variables </identifier>"));
    EXPECT_THAT(m_data2[8], ::testing::EndsWith("<symbol> ; </symbol>"));
    EXPECT_THAT(m_data2[9], ::testing::EndsWith("</classVarDec>"));
}

TEST(Compiler, CompileLetStatements)
{
    Compiler::Tokenizer t1{};
    Compiler::CompilationEngine c1{&t1};
    t1.parseLine("let game = game;");
    c1.compileLetStatement();
    {
        const auto m_data = c1.getData();
        EXPECT_EQ(m_data.size(), 11);
        EXPECT_THAT(m_data[0], ::testing::EndsWith("<letStatement>"));
        EXPECT_THAT(m_data[1], ::testing::EndsWith("<keyword> let </keyword>"));
        EXPECT_THAT(m_data[2], ::testing::EndsWith("<identifier> game </identifier>"));
        EXPECT_THAT(m_data[3], ::testing::EndsWith("<symbol> = </symbol>"));
        EXPECT_THAT(m_data[4], ::testing::EndsWith("<expression>"));
        EXPECT_THAT(m_data[5], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[6], ::testing::EndsWith("<identifier> game </identifier>"));
        EXPECT_THAT(m_data[7], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[8], ::testing::EndsWith("</expression>"));
        EXPECT_THAT(m_data[9], ::testing::EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[10], ::testing::EndsWith("</letStatement>"));
    }

    c1.clearData();
    t1.parseLine("let a[i] = Keyboard.readInt(\"ENTER THE NEXT NUMBER: \");");
    c1.compileLetStatement();
    {
        const auto m_data = c1.getData();
        EXPECT_EQ(m_data.size(), 29);
        EXPECT_THAT(m_data[0], ::testing::EndsWith("<letStatement>"));
        EXPECT_THAT(m_data[1], ::testing::EndsWith("<keyword> let </keyword>"));
        EXPECT_THAT(m_data[2], ::testing::EndsWith("<identifier> a </identifier>"));
        EXPECT_THAT(m_data[3], ::testing::EndsWith("<symbol> [ </symbol>"));
        EXPECT_THAT(m_data[4], ::testing::EndsWith("<expression>"));
        EXPECT_THAT(m_data[5], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[6], ::testing::EndsWith("<identifier> i </identifier>"));
        EXPECT_THAT(m_data[7], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[8], ::testing::EndsWith("</expression>"));
        EXPECT_THAT(m_data[9], ::testing::EndsWith("<symbol> ] </symbol>"));
        EXPECT_THAT(m_data[10], ::testing::EndsWith("<symbol> = </symbol>"));
        EXPECT_THAT(m_data[11], ::testing::EndsWith("<expression>"));
        EXPECT_THAT(m_data[12], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[13], ::testing::EndsWith("<identifier> Keyboard </identifier>"));
        EXPECT_THAT(m_data[14], ::testing::EndsWith("<symbol> . </symbol>"));
        EXPECT_THAT(m_data[15], ::testing::EndsWith("<identifier> readInt </identifier>"));
        EXPECT_THAT(m_data[16], ::testing::EndsWith("<symbol> ( </symbol>"));
        EXPECT_THAT(m_data[17], ::testing::EndsWith("<expressionList>"));
        EXPECT_THAT(m_data[18], ::testing::EndsWith("<expression>"));
        EXPECT_THAT(m_data[19], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[20], ::testing::EndsWith("<stringConstant> ENTER THE NEXT NUMBER:  </stringConstant>"));
        EXPECT_THAT(m_data[21], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[22], ::testing::EndsWith("</expression>"));
        EXPECT_THAT(m_data[23], ::testing::EndsWith("</expressionList>"));
        EXPECT_THAT(m_data[24], ::testing::EndsWith("<symbol> ) </symbol>"));
        EXPECT_THAT(m_data[25], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[26], ::testing::EndsWith("</expression>"));
        EXPECT_THAT(m_data[27], ::testing::EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[28], ::testing::EndsWith("</letStatement>"));
    }

    c1.clearData();
    t1.parseLine("let a = Array.new(length);");
    c1.compileLetStatement();
    {
        const auto m_data = c1.getData();
        EXPECT_THAT(m_data[0], ::testing::EndsWith("<letStatement>"));
        EXPECT_THAT(m_data[1], ::testing::EndsWith("<keyword> let </keyword>"));
        EXPECT_THAT(m_data[2], ::testing::EndsWith("<identifier> a </identifier>"));
        EXPECT_THAT(m_data[3], ::testing::EndsWith("<symbol> = </symbol>"));
        EXPECT_THAT(m_data[4], ::testing::EndsWith("<expression>"));
        EXPECT_THAT(m_data[5], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[6], ::testing::EndsWith("<identifier> Array </identifier>"));
        EXPECT_THAT(m_data[7], ::testing::EndsWith("<symbol> . </symbol>"));
        EXPECT_THAT(m_data[8], ::testing::EndsWith("<identifier> new </identifier>"));
        EXPECT_THAT(m_data[9], ::testing::EndsWith("<symbol> ( </symbol>"));
        EXPECT_THAT(m_data[10], ::testing::EndsWith("<expressionList>"));
        EXPECT_THAT(m_data[11], ::testing::EndsWith("<expression>"));
        EXPECT_THAT(m_data[12], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[13], ::testing::EndsWith("<identifier> length </identifier>"));
        EXPECT_THAT(m_data[14], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[15], ::testing::EndsWith("</expression>"));
        EXPECT_THAT(m_data[16], ::testing::EndsWith("</expressionList>"));
        EXPECT_THAT(m_data[17], ::testing::EndsWith("<symbol> ) </symbol>"));
        EXPECT_THAT(m_data[18], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[19], ::testing::EndsWith("</expression>"));
        EXPECT_THAT(m_data[20], ::testing::EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[21], ::testing::EndsWith("</letStatement>"));
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
        EXPECT_THAT(m_data[0], ::testing::EndsWith("<ifStatement>"));
        EXPECT_THAT(m_data[1], ::testing::EndsWith("<keyword> if </keyword>"));
        EXPECT_THAT(m_data[2], ::testing::EndsWith("<symbol> ( </symbol>"));
        EXPECT_THAT(m_data[3], ::testing::EndsWith("<expression>"));
        EXPECT_THAT(m_data[4], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[5], ::testing::EndsWith("<identifier> key </identifier>"));
        EXPECT_THAT(m_data[6], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[7], ::testing::EndsWith("</expression>"));
        EXPECT_THAT(m_data[8], ::testing::EndsWith("<symbol> ) </symbol>"));
        EXPECT_THAT(m_data[9], ::testing::EndsWith("<symbol> { </symbol>"));
        EXPECT_THAT(m_data[10], ::testing::EndsWith("<statements>"));
        EXPECT_THAT(m_data[11], ::testing::EndsWith("<letStatement>"));
        EXPECT_THAT(m_data[12], ::testing::EndsWith("<keyword> let </keyword>"));
        EXPECT_THAT(m_data[13], ::testing::EndsWith("<identifier> exit </identifier>"));
        EXPECT_THAT(m_data[14], ::testing::EndsWith("<symbol> = </symbol>"));
        EXPECT_THAT(m_data[15], ::testing::EndsWith("<expression>"));
        EXPECT_THAT(m_data[16], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[17], ::testing::EndsWith("<identifier> exit </identifier>"));
        EXPECT_THAT(m_data[18], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[19], ::testing::EndsWith("</expression>"));
        EXPECT_THAT(m_data[20], ::testing::EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[21], ::testing::EndsWith("</letStatement>"));
        EXPECT_THAT(m_data[22], ::testing::EndsWith("</statements>"));
        EXPECT_THAT(m_data[23], ::testing::EndsWith("<symbol> } </symbol>"));
        EXPECT_THAT(m_data[24], ::testing::EndsWith("</ifStatement>"));
    }

    // Expression condition if
    c1.clearData();
    t1.parseLine("if (key = 81)  { let exit = true; }     // q key");
    c1.compileIfStatement();
    {
        const auto m_data = c1.getData();
        EXPECT_THAT(m_data[0], ::testing::EndsWith("<ifStatement>"));
        EXPECT_THAT(m_data[1], ::testing::EndsWith("<keyword> if </keyword>"));
        EXPECT_THAT(m_data[2], ::testing::EndsWith("<symbol> ( </symbol>"));
        EXPECT_THAT(m_data[3], ::testing::EndsWith("<expression>"));
        EXPECT_THAT(m_data[4], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[5], ::testing::EndsWith("<identifier> key </identifier>"));
        EXPECT_THAT(m_data[6], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[7], ::testing::EndsWith("<symbol> = </symbol>"));
        EXPECT_THAT(m_data[8], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[9], ::testing::EndsWith("<integerConstant> 81 </integerConstant>"));
        EXPECT_THAT(m_data[10], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[11], ::testing::EndsWith("</expression>"));
        EXPECT_THAT(m_data[12], ::testing::EndsWith("<symbol> ) </symbol>"));
        EXPECT_THAT(m_data[13], ::testing::EndsWith("<symbol> { </symbol>"));
        EXPECT_THAT(m_data[14], ::testing::EndsWith("<statements>"));
        EXPECT_THAT(m_data[15], ::testing::EndsWith("<letStatement>"));
        EXPECT_THAT(m_data[16], ::testing::EndsWith("<keyword> let </keyword>"));
        EXPECT_THAT(m_data[17], ::testing::EndsWith("<identifier> exit </identifier>"));
        EXPECT_THAT(m_data[18], ::testing::EndsWith("<symbol> = </symbol>"));
        EXPECT_THAT(m_data[19], ::testing::EndsWith("<expression>"));
        EXPECT_THAT(m_data[20], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[21], ::testing::EndsWith("<keyword> true </keyword>"));
        EXPECT_THAT(m_data[22], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[23], ::testing::EndsWith("</expression>"));
        EXPECT_THAT(m_data[24], ::testing::EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[25], ::testing::EndsWith("</letStatement>"));
        EXPECT_THAT(m_data[26], ::testing::EndsWith("</statements>"));
        EXPECT_THAT(m_data[27], ::testing::EndsWith("<symbol> } </symbol>"));
        EXPECT_THAT(m_data[28], ::testing::EndsWith("</ifStatement>"));
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
        EXPECT_THAT(m_data[0], ::testing::EndsWith("<ifStatement>"));
        EXPECT_THAT(m_data[1], ::testing::EndsWith("<keyword> if </keyword>"));
        EXPECT_THAT(m_data[2], ::testing::EndsWith("<symbol> ( </symbol>"));
        EXPECT_THAT(m_data[3], ::testing::EndsWith("<expression>"));
        EXPECT_THAT(m_data[4], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[5], ::testing::EndsWith("<identifier> b </identifier>"));
        EXPECT_THAT(m_data[6], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[7], ::testing::EndsWith("</expression>"));
        EXPECT_THAT(m_data[8], ::testing::EndsWith("<symbol> ) </symbol>"));
        EXPECT_THAT(m_data[9], ::testing::EndsWith("<symbol> { </symbol>"));
        EXPECT_THAT(m_data[10], ::testing::EndsWith("<statements>"));
        EXPECT_THAT(m_data[11], ::testing::EndsWith("</statements>"));
        EXPECT_THAT(m_data[12], ::testing::EndsWith("<symbol> } </symbol>"));
        EXPECT_THAT(m_data[13], ::testing::EndsWith("<keyword> else </keyword>"));
        EXPECT_THAT(m_data[14], ::testing::EndsWith("<symbol> { </symbol>"));
        EXPECT_THAT(m_data[15], ::testing::EndsWith("<statements>"));
        EXPECT_THAT(m_data[16], ::testing::EndsWith("</statements>"));
        EXPECT_THAT(m_data[17], ::testing::EndsWith("<symbol> } </symbol>"));
        EXPECT_THAT(m_data[18], ::testing::EndsWith("</ifStatement>"));
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
        EXPECT_THAT(m_data[0], ::testing::EndsWith("<ifStatement>"));
        EXPECT_THAT(m_data[1], ::testing::EndsWith("<keyword> if </keyword>"));
        EXPECT_THAT(m_data[2], ::testing::EndsWith("<symbol> ( </symbol>"));
        EXPECT_THAT(m_data[3], ::testing::EndsWith("<expression>"));
        EXPECT_THAT(m_data[4], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[5], ::testing::EndsWith("<keyword> false </keyword>"));
        EXPECT_THAT(m_data[6], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[7], ::testing::EndsWith("</expression>"));
        EXPECT_THAT(m_data[8], ::testing::EndsWith("<symbol> ) </symbol>"));
        EXPECT_THAT(m_data[9], ::testing::EndsWith("<symbol> { </symbol>"));
        EXPECT_THAT(m_data[10], ::testing::EndsWith("<statements>"));
        EXPECT_THAT(m_data[11], ::testing::EndsWith("<letStatement>"));
        EXPECT_THAT(m_data[12], ::testing::EndsWith("<keyword> let </keyword>"));
        EXPECT_THAT(m_data[13], ::testing::EndsWith("<identifier> s </identifier>"));
        EXPECT_THAT(m_data[14], ::testing::EndsWith("<symbol> = </symbol>"));
        EXPECT_THAT(m_data[15], ::testing::EndsWith("<expression>"));
        EXPECT_THAT(m_data[16], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[17], ::testing::EndsWith("<stringConstant> string constant </stringConstant>"));
        EXPECT_THAT(m_data[18], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[19], ::testing::EndsWith("</expression>"));
        EXPECT_THAT(m_data[20], ::testing::EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[21], ::testing::EndsWith("</letStatement>"));
        EXPECT_THAT(m_data[22], ::testing::EndsWith("<letStatement>"));
        EXPECT_THAT(m_data[23], ::testing::EndsWith("<keyword> let </keyword>"));
        EXPECT_THAT(m_data[24], ::testing::EndsWith("<identifier> s </identifier>"));
        EXPECT_THAT(m_data[25], ::testing::EndsWith("<symbol> = </symbol>"));
        EXPECT_THAT(m_data[26], ::testing::EndsWith("<expression>"));
        EXPECT_THAT(m_data[27], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[28], ::testing::EndsWith("<keyword> null </keyword>"));
        EXPECT_THAT(m_data[29], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[30], ::testing::EndsWith("</expression>"));
        EXPECT_THAT(m_data[31], ::testing::EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[32], ::testing::EndsWith("</letStatement>"));
        EXPECT_THAT(m_data[33], ::testing::EndsWith("<letStatement>"));
        EXPECT_THAT(m_data[34], ::testing::EndsWith("<keyword> let </keyword>"));
        EXPECT_THAT(m_data[35], ::testing::EndsWith("<identifier> a </identifier>"));
        EXPECT_THAT(m_data[36], ::testing::EndsWith("<symbol> [ </symbol>"));
        EXPECT_THAT(m_data[37], ::testing::EndsWith("<expression>"));
        EXPECT_THAT(m_data[38], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[39], ::testing::EndsWith("<integerConstant> 1 </integerConstant>"));
        EXPECT_THAT(m_data[40], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[41], ::testing::EndsWith("</expression>"));
        EXPECT_THAT(m_data[42], ::testing::EndsWith("<symbol> ] </symbol>"));
        EXPECT_THAT(m_data[43], ::testing::EndsWith("<symbol> = </symbol>"));
        EXPECT_THAT(m_data[44], ::testing::EndsWith("<expression>"));
        EXPECT_THAT(m_data[45], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[46], ::testing::EndsWith("<identifier> a </identifier>"));
        EXPECT_THAT(m_data[47], ::testing::EndsWith("<symbol> [ </symbol>"));
        EXPECT_THAT(m_data[48], ::testing::EndsWith("<expression>"));
        EXPECT_THAT(m_data[49], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[50], ::testing::EndsWith("<integerConstant> 2 </integerConstant>"));
        EXPECT_THAT(m_data[51], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[52], ::testing::EndsWith("</expression>"));
        EXPECT_THAT(m_data[53], ::testing::EndsWith("<symbol> ] </symbol>"));
        EXPECT_THAT(m_data[54], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[55], ::testing::EndsWith("</expression>"));
        EXPECT_THAT(m_data[56], ::testing::EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[57], ::testing::EndsWith("</letStatement>"));
        EXPECT_THAT(m_data[58], ::testing::EndsWith("</statements>"));
        EXPECT_THAT(m_data[59], ::testing::EndsWith("<symbol> } </symbol>"));
        EXPECT_THAT(m_data[60], ::testing::EndsWith("<keyword> else </keyword>"));
        EXPECT_THAT(m_data[61], ::testing::EndsWith("<symbol> { </symbol>"));
        EXPECT_THAT(m_data[62], ::testing::EndsWith("<statements>"));
        EXPECT_THAT(m_data[63], ::testing::EndsWith("<letStatement>"));
        EXPECT_THAT(m_data[64], ::testing::EndsWith("<keyword> let </keyword>"));
        EXPECT_THAT(m_data[65], ::testing::EndsWith("<identifier> i </identifier>"));
        EXPECT_THAT(m_data[66], ::testing::EndsWith("<symbol> = </symbol>"));
        EXPECT_THAT(m_data[67], ::testing::EndsWith("<expression>"));
        EXPECT_THAT(m_data[68], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[69], ::testing::EndsWith("<identifier> i </identifier>"));
        EXPECT_THAT(m_data[70], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[71], ::testing::EndsWith("<symbol> * </symbol>"));
        EXPECT_THAT(m_data[72], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[73], ::testing::EndsWith("<symbol> ( </symbol>"));
        EXPECT_THAT(m_data[74], ::testing::EndsWith("<expression>"));
        EXPECT_THAT(m_data[75], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[76], ::testing::EndsWith("<symbol> - </symbol>"));
        EXPECT_THAT(m_data[77], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[78], ::testing::EndsWith("<identifier> j </identifier>"));
        EXPECT_THAT(m_data[79], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[80], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[81], ::testing::EndsWith("</expression>"));
        EXPECT_THAT(m_data[82], ::testing::EndsWith("<symbol> ) </symbol>"));
        EXPECT_THAT(m_data[83], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[84], ::testing::EndsWith("</expression>"));
        EXPECT_THAT(m_data[85], ::testing::EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[86], ::testing::EndsWith("</letStatement>"));
        EXPECT_THAT(m_data[87], ::testing::EndsWith("<letStatement>"));
        EXPECT_THAT(m_data[88], ::testing::EndsWith("<keyword> let </keyword>"));
        EXPECT_THAT(m_data[89], ::testing::EndsWith("<identifier> j </identifier>"));
        EXPECT_THAT(m_data[90], ::testing::EndsWith("<symbol> = </symbol>"));
        EXPECT_THAT(m_data[91], ::testing::EndsWith("<expression>"));
        EXPECT_THAT(m_data[92], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[93], ::testing::EndsWith("<identifier> j </identifier>"));
        EXPECT_THAT(m_data[94], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[95], ::testing::EndsWith("<symbol> / </symbol>"));
        EXPECT_THAT(m_data[96], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[97], ::testing::EndsWith("<symbol> ( </symbol>"));
        EXPECT_THAT(m_data[98], ::testing::EndsWith("<expression>"));
        EXPECT_THAT(m_data[99], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[100], ::testing::EndsWith("<symbol> - </symbol>"));
        EXPECT_THAT(m_data[101], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[102], ::testing::EndsWith("<integerConstant> 2 </integerConstant>"));
        EXPECT_THAT(m_data[103], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[104], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[105], ::testing::EndsWith("</expression>"));
        EXPECT_THAT(m_data[106], ::testing::EndsWith("<symbol> ) </symbol>"));
        EXPECT_THAT(m_data[107], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[108], ::testing::EndsWith("</expression>"));
        EXPECT_THAT(m_data[109], ::testing::EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[110], ::testing::EndsWith("</letStatement>"));
        EXPECT_THAT(m_data[111], ::testing::EndsWith("<letStatement>"));
        EXPECT_THAT(m_data[112], ::testing::EndsWith("<keyword> let </keyword>"));
        EXPECT_THAT(m_data[113], ::testing::EndsWith("<identifier> i </identifier>"));
        EXPECT_THAT(m_data[114], ::testing::EndsWith("<symbol> = </symbol>"));
        EXPECT_THAT(m_data[115], ::testing::EndsWith("<expression>"));
        EXPECT_THAT(m_data[116], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[117], ::testing::EndsWith("<identifier> i </identifier>"));
        EXPECT_THAT(m_data[118], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[119], ::testing::EndsWith("<symbol> | </symbol>"));
        EXPECT_THAT(m_data[120], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[121], ::testing::EndsWith("<identifier> j </identifier>"));
        EXPECT_THAT(m_data[122], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[123], ::testing::EndsWith("</expression>"));
        EXPECT_THAT(m_data[124], ::testing::EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[125], ::testing::EndsWith("</letStatement>"));
        EXPECT_THAT(m_data[126], ::testing::EndsWith("</statements>"));
        EXPECT_THAT(m_data[127], ::testing::EndsWith("<symbol> } </symbol>"));
        EXPECT_THAT(m_data[128], ::testing::EndsWith("</ifStatement>"));
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
        EXPECT_THAT(m_data[0], ::testing::EndsWith("<whileStatement>"));
        EXPECT_THAT(m_data[1], ::testing::EndsWith("<keyword> while </keyword>"));
        EXPECT_THAT(m_data[2], ::testing::EndsWith("<symbol> ( </symbol>"));
        EXPECT_THAT(m_data[3], ::testing::EndsWith("<expression>"));
        EXPECT_THAT(m_data[4], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[5], ::testing::EndsWith("<identifier> key </identifier>"));
        EXPECT_THAT(m_data[6], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[7], ::testing::EndsWith("</expression>"));
        EXPECT_THAT(m_data[8], ::testing::EndsWith("<symbol> ) </symbol>"));
        EXPECT_THAT(m_data[9], ::testing::EndsWith("<symbol> { </symbol>"));
        EXPECT_THAT(m_data[10], ::testing::EndsWith("<statements>"));
        EXPECT_THAT(m_data[11], ::testing::EndsWith("<letStatement>"));
        EXPECT_THAT(m_data[12], ::testing::EndsWith("<keyword> let </keyword>"));
        EXPECT_THAT(m_data[13], ::testing::EndsWith("<identifier> key </identifier>"));
        EXPECT_THAT(m_data[14], ::testing::EndsWith("<symbol> = </symbol>"));
        EXPECT_THAT(m_data[15], ::testing::EndsWith("<expression>"));
        EXPECT_THAT(m_data[16], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[17], ::testing::EndsWith("<identifier> key </identifier>"));
        EXPECT_THAT(m_data[18], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[19], ::testing::EndsWith("</expression>"));
        EXPECT_THAT(m_data[20], ::testing::EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[21], ::testing::EndsWith("</letStatement>"));
        EXPECT_THAT(m_data[22], ::testing::EndsWith("<doStatement>"));
        EXPECT_THAT(m_data[23], ::testing::EndsWith("<keyword> do </keyword>"));
        EXPECT_THAT(m_data[24], ::testing::EndsWith("<identifier> moveSquare </identifier>"));
        EXPECT_THAT(m_data[25], ::testing::EndsWith("<symbol> ( </symbol>"));
        EXPECT_THAT(m_data[26], ::testing::EndsWith("<expressionList>"));
        EXPECT_THAT(m_data[27], ::testing::EndsWith("</expressionList>"));
        EXPECT_THAT(m_data[28], ::testing::EndsWith("<symbol> ) </symbol>"));
        EXPECT_THAT(m_data[29], ::testing::EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[30], ::testing::EndsWith("</doStatement>"));
        EXPECT_THAT(m_data[31], ::testing::EndsWith("</statements>"));
        EXPECT_THAT(m_data[32], ::testing::EndsWith("<symbol> } </symbol>"));
        EXPECT_THAT(m_data[33], ::testing::EndsWith("</whileStatement>"));
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
        EXPECT_THAT(m_data[0], ::testing::EndsWith("<doStatement>"));
        EXPECT_THAT(m_data[1], ::testing::EndsWith("<keyword> do </keyword>"));
        EXPECT_THAT(m_data[2], ::testing::EndsWith("<identifier> moveSquare </identifier>"));
        EXPECT_THAT(m_data[3], ::testing::EndsWith("<symbol> ( </symbol>"));
        EXPECT_THAT(m_data[4], ::testing::EndsWith("<expressionList>"));
        EXPECT_THAT(m_data[5], ::testing::EndsWith("</expressionList>"));
        EXPECT_THAT(m_data[6], ::testing::EndsWith("<symbol> ) </symbol>"));
        EXPECT_THAT(m_data[7], ::testing::EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[8], ::testing::EndsWith("</doStatement>"));
    }

    c1.clearData();
    t1.parseLine("do Sys.wait(direction);"); // Class function call
    c1.compileDoStatement();
    {
        const auto m_data = c1.getData();
        EXPECT_THAT(m_data[0], ::testing::EndsWith("<doStatement>"));
        EXPECT_THAT(m_data[1], ::testing::EndsWith("<keyword> do </keyword>"));
        EXPECT_THAT(m_data[2], ::testing::EndsWith("<identifier> Sys </identifier>"));
        EXPECT_THAT(m_data[3], ::testing::EndsWith("<symbol> . </symbol>"));
        EXPECT_THAT(m_data[4], ::testing::EndsWith("<identifier> wait </identifier>"));
        EXPECT_THAT(m_data[5], ::testing::EndsWith("<symbol> ( </symbol>"));
        EXPECT_THAT(m_data[6], ::testing::EndsWith("<expressionList>"));
        EXPECT_THAT(m_data[7], ::testing::EndsWith("<expression>"));
        EXPECT_THAT(m_data[8], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[9], ::testing::EndsWith("<identifier> direction </identifier>"));
        EXPECT_THAT(m_data[10], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[11], ::testing::EndsWith("</expression>"));
        EXPECT_THAT(m_data[12], ::testing::EndsWith("</expressionList>"));
        EXPECT_THAT(m_data[13], ::testing::EndsWith("<symbol> ) </symbol>"));
        EXPECT_THAT(m_data[14], ::testing::EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[15], ::testing::EndsWith("</doStatement>"));
    }

    c1.clearData();
    t1.parseLine("do Screen.drawRectangle((x + size) - 1, y, x + size, y + size);"); // Class function call with ful expression
    c1.compileDoStatement();
    {
        const auto m_data = c1.getData();
        EXPECT_THAT(m_data[0], ::testing::EndsWith("<doStatement>"));
        EXPECT_THAT(m_data[1], ::testing::EndsWith("<keyword> do </keyword>"));
        EXPECT_THAT(m_data[2], ::testing::EndsWith("<identifier> Screen </identifier>"));
        EXPECT_THAT(m_data[3], ::testing::EndsWith("<symbol> . </symbol>"));
        EXPECT_THAT(m_data[4], ::testing::EndsWith("<identifier> drawRectangle </identifier>"));
        EXPECT_THAT(m_data[5], ::testing::EndsWith("<symbol> ( </symbol>"));
        EXPECT_THAT(m_data[6], ::testing::EndsWith("<expressionList>"));
        EXPECT_THAT(m_data[7], ::testing::EndsWith("<expression>"));
        EXPECT_THAT(m_data[8], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[9], ::testing::EndsWith("<symbol> ( </symbol>"));
        EXPECT_THAT(m_data[10], ::testing::EndsWith("<expression>"));
        EXPECT_THAT(m_data[11], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[12], ::testing::EndsWith("<identifier> x </identifier>"));
        EXPECT_THAT(m_data[13], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[14], ::testing::EndsWith("<symbol> + </symbol>"));
        EXPECT_THAT(m_data[15], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[16], ::testing::EndsWith("<identifier> size </identifier>"));
        EXPECT_THAT(m_data[17], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[18], ::testing::EndsWith("</expression>"));
        EXPECT_THAT(m_data[19], ::testing::EndsWith("<symbol> ) </symbol>"));
        EXPECT_THAT(m_data[20], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[21], ::testing::EndsWith("<symbol> - </symbol>"));
        EXPECT_THAT(m_data[22], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[23], ::testing::EndsWith("<integerConstant> 1 </integerConstant>"));
        EXPECT_THAT(m_data[24], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[25], ::testing::EndsWith("</expression>"));
        EXPECT_THAT(m_data[26], ::testing::EndsWith("<symbol> , </symbol>"));
        EXPECT_THAT(m_data[27], ::testing::EndsWith("<expression>"));
        EXPECT_THAT(m_data[28], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[29], ::testing::EndsWith("<identifier> y </identifier>"));
        EXPECT_THAT(m_data[30], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[31], ::testing::EndsWith("</expression>"));
        EXPECT_THAT(m_data[32], ::testing::EndsWith("<symbol> , </symbol>"));
        EXPECT_THAT(m_data[33], ::testing::EndsWith("<expression>"));
        EXPECT_THAT(m_data[34], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[35], ::testing::EndsWith("<identifier> x </identifier>"));
        EXPECT_THAT(m_data[36], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[37], ::testing::EndsWith("<symbol> + </symbol>"));
        EXPECT_THAT(m_data[38], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[39], ::testing::EndsWith("<identifier> size </identifier>"));
        EXPECT_THAT(m_data[40], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[41], ::testing::EndsWith("</expression>"));
        EXPECT_THAT(m_data[42], ::testing::EndsWith("<symbol> , </symbol>"));
        EXPECT_THAT(m_data[43], ::testing::EndsWith("<expression>"));
        EXPECT_THAT(m_data[44], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[45], ::testing::EndsWith("<identifier> y </identifier>"));
        EXPECT_THAT(m_data[46], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[47], ::testing::EndsWith("<symbol> + </symbol>"));
        EXPECT_THAT(m_data[48], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[49], ::testing::EndsWith("<identifier> size </identifier>"));
        EXPECT_THAT(m_data[50], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[51], ::testing::EndsWith("</expression>"));
        EXPECT_THAT(m_data[52], ::testing::EndsWith("</expressionList>"));
        EXPECT_THAT(m_data[53], ::testing::EndsWith("<symbol> ) </symbol>"));
        EXPECT_THAT(m_data[54], ::testing::EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[55], ::testing::EndsWith("</doStatement>"));
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
        EXPECT_THAT(m_data[0], ::testing::EndsWith("<returnStatement>"));
        EXPECT_THAT(m_data[1], ::testing::EndsWith("<keyword> return </keyword>"));
        EXPECT_THAT(m_data[2], ::testing::EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[3], ::testing::EndsWith("</returnStatement>"));
    }

    c1.clearData();
    t1.parseLine("return this;"); // Variable return
    c1.compileReturnStatement();
    {
        const auto m_data = c1.getData();
        EXPECT_THAT(m_data[0], ::testing::EndsWith("<returnStatement>"));
        EXPECT_THAT(m_data[1], ::testing::EndsWith("<keyword> return </keyword>"));
        EXPECT_THAT(m_data[2], ::testing::EndsWith("<expression>"));
        EXPECT_THAT(m_data[3], ::testing::EndsWith("<term>"));
        EXPECT_THAT(m_data[4], ::testing::EndsWith("<keyword> this </keyword>"));
        EXPECT_THAT(m_data[5], ::testing::EndsWith("</term>"));
        EXPECT_THAT(m_data[6], ::testing::EndsWith("</expression>"));
        EXPECT_THAT(m_data[7], ::testing::EndsWith("<symbol> ; </symbol>"));
        EXPECT_THAT(m_data[8], ::testing::EndsWith("</returnStatement>"));
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
        EXPECT_EQ(m_data.size(), 13);
        EXPECT_THAT(m_data[0], ::testing::EndsWith("<subroutineDec>"));
        EXPECT_THAT(m_data[1], ::testing::EndsWith("<keyword> function </keyword>"));
        EXPECT_THAT(m_data[2], ::testing::EndsWith("<keyword> void </keyword>"));
        EXPECT_THAT(m_data[3], ::testing::EndsWith("<identifier> Main </identifier>"));
        EXPECT_THAT(m_data[4], ::testing::EndsWith("<symbol> ( </symbol>"));
        EXPECT_THAT(m_data[5], ::testing::EndsWith("<parameterList>"));
        EXPECT_THAT(m_data[6], ::testing::EndsWith("</parameterList>"));
        EXPECT_THAT(m_data[7], ::testing::EndsWith("<symbol> ) </symbol>"));
        EXPECT_THAT(m_data[8], ::testing::EndsWith("<subroutineBody>"));
        EXPECT_THAT(m_data[9], ::testing::EndsWith("<symbol> { </symbol>"));
        EXPECT_THAT(m_data[10], ::testing::EndsWith("<symbol> } </symbol>"));
        EXPECT_THAT(m_data[11], ::testing::EndsWith("</subroutineBody>"));
        EXPECT_THAT(m_data[12], ::testing::EndsWith("</subroutineDec>"));
    }

    // Non void return type
    c1.clearData();
    t1.parseLine("constructor SquareGame new() {}");
    c1.compileSubroutineDecs();
    {
        const auto m_data = c1.getData();
        EXPECT_EQ(m_data.size(), 13);
        EXPECT_THAT(m_data[0], ::testing::EndsWith("<subroutineDec>"));
        EXPECT_THAT(m_data[1], ::testing::EndsWith("<keyword> constructor </keyword>"));
        EXPECT_THAT(m_data[2], ::testing::EndsWith("<identifier> SquareGame </identifier>"));
        EXPECT_THAT(m_data[3], ::testing::EndsWith("<identifier> new </identifier>"));
        EXPECT_THAT(m_data[4], ::testing::EndsWith("<symbol> ( </symbol>"));
        EXPECT_THAT(m_data[5], ::testing::EndsWith("<parameterList>"));
        EXPECT_THAT(m_data[6], ::testing::EndsWith("</parameterList>"));
        EXPECT_THAT(m_data[7], ::testing::EndsWith("<symbol> ) </symbol>"));
        EXPECT_THAT(m_data[8], ::testing::EndsWith("<subroutineBody>"));
        EXPECT_THAT(m_data[9], ::testing::EndsWith("<symbol> { </symbol>"));
        EXPECT_THAT(m_data[10], ::testing::EndsWith("<symbol> } </symbol>"));
        EXPECT_THAT(m_data[11], ::testing::EndsWith("</subroutineBody>"));
        EXPECT_THAT(m_data[12], ::testing::EndsWith("</subroutineDec>"));
    }
}