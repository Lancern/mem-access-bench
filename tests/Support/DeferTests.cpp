#include "kv/Support/Defer.h"

#include <vector>

#include "gtest/gtest.h"

TEST(Defer, TestBasicDefer) {
  auto flag = 0;

  {
    DEFER(1, flag = 1);
    ASSERT_EQ(flag, 0);
  }

  ASSERT_EQ(flag, 1);
}

TEST(Defer, TestMultipleDeferOrder) {
  std::vector<int> flags;

  {
    DEFER(1, flags.push_back(1));
    DEFER(2, flags.push_back(2));
    ASSERT_TRUE(flags.empty());
  }

  std::vector<int> expected { 2, 1 };
  ASSERT_EQ(flags, expected);
}
