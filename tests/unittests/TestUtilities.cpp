#include <gtest/gtest.h>
#include "Utilities.h"

TEST(Utilities, SplitBySpaceKeepQuoted)
{
    auto result = Utilities::splitBySpaceKeepQuoted("\"Between Quotes\"");
    std::string expected{"Between Quotes"};
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], expected);
    result = Utilities::splitBySpaceKeepQuoted("BeforeOnly \"Between Quotes\"");
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[1], expected);
    result = Utilities::splitBySpaceKeepQuoted("\"Between Quotes\" AfterOnly");
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], expected);
    result = Utilities::splitBySpaceKeepQuoted("Before \"Between Quotes\" After");
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[1], expected);
    result = Utilities::splitBySpaceKeepQuoted("No\"Between Quotes\"Space");
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[1], expected);
    result = Utilities::splitBySpaceKeepQuoted("Include\"Between Quotes\"Quotes", true);
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[1], "\"Between Quotes\"");

}

TEST(Utilities, SplitBySpace)
{
    auto result = Utilities::splitBySpace("");
    EXPECT_EQ(result.size(), 0);
    result = Utilities::splitBySpace("  ");
    EXPECT_EQ(result.size(), 0);
    result = Utilities::splitBySpace("New Line   \n");
    EXPECT_EQ(result.size(), 2);
    result = Utilities::splitBySpace("push constant   10");
    EXPECT_EQ(result.size(), 3);
    result = Utilities::splitBySpace("   A B C D");
    EXPECT_EQ(result.size(), 4);
    result = Utilities::splitBySpace("Add Add ");
    EXPECT_EQ(result.size(), 2);
}