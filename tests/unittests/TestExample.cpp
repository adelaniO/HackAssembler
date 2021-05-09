#include <gtest/gtest.h>
#include "Utilities.h"

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