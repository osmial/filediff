#include <gtest/gtest.h>
#include <iostream>

#include "../adler32.h"

namespace testing {

class adler32TestSuite : public ::testing::Test {
};

TEST(adler32TestSuite, OneWordTest)
{
    std::string testString { "Wikipedia" };
    ASSERT_EQ(0x11E60398, adler32(testString));
}

TEST(adler32TestSuite, LoremIpsumTest)
{
    std::string loremIpsum { "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum." };
    ASSERT_EQ(0xa05ca509, adler32(loremIpsum));
}

}
