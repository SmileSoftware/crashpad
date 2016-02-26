// Copyright 2016 The Crashpad Authors. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "client/simple_address_range_bag.h"

#include "base/logging.h"
#include "gtest/gtest.h"
#include "test/gtest_death_check.h"

namespace crashpad {
namespace test {
namespace {

TEST(SimpleAddressRangeBag, Entry) {
  using TestBag = TSimpleAddressRangeBag<15>;
  TestBag bag;

  const TestBag::Entry* entry = TestBag::Iterator(bag).Next();
  EXPECT_FALSE(entry);

  bag.Insert(reinterpret_cast<void*>(0x1000), 200);
  entry = TestBag::Iterator(bag).Next();
  ASSERT_TRUE(entry);
  EXPECT_EQ(entry->base, 0x1000);
  EXPECT_EQ(entry->size, 200);

  bag.Remove(reinterpret_cast<void*>(0x1000), 200);
  EXPECT_FALSE(entry->is_active());
  EXPECT_EQ(entry->base, 0);
  EXPECT_EQ(entry->size, 0);
}

TEST(SimpleAddressRangeBag, SimpleAddressRangeBag) {
  SimpleAddressRangeBag bag;

  EXPECT_TRUE(bag.Insert(reinterpret_cast<void*>(0x1000), 10));
  EXPECT_TRUE(bag.Insert(reinterpret_cast<void*>(0x2000), 20));
  EXPECT_TRUE(bag.Insert(CheckedRange<uint64_t>(0x3000, 30)));

  EXPECT_EQ(bag.GetCount(), 3u);

  // Duplicates added too.
  EXPECT_TRUE(bag.Insert(CheckedRange<uint64_t>(0x3000, 30)));
  EXPECT_TRUE(bag.Insert(CheckedRange<uint64_t>(0x3000, 30)));
  EXPECT_EQ(bag.GetCount(), 5u);

  // Can be removed 3 times, but not the 4th time.
  EXPECT_TRUE(bag.Remove(CheckedRange<uint64_t>(0x3000, 30)));
  EXPECT_TRUE(bag.Remove(CheckedRange<uint64_t>(0x3000, 30)));
  EXPECT_TRUE(bag.Remove(CheckedRange<uint64_t>(0x3000, 30)));
  EXPECT_EQ(bag.GetCount(), 2);
  EXPECT_FALSE(bag.Remove(CheckedRange<uint64_t>(0x3000, 30)));
  EXPECT_EQ(bag.GetCount(), 2);

  EXPECT_TRUE(bag.Remove(reinterpret_cast<void*>(0x1000), 10));
  EXPECT_TRUE(bag.Remove(reinterpret_cast<void*>(0x2000), 20));
  EXPECT_EQ(bag.GetCount(), 0);
}

TEST(SimpleAddressRangeBag, CopyAndAssign) {
  TSimpleAddressRangeBag<10> bag;
  EXPECT_TRUE(bag.Insert(CheckedRange<uint64_t>(1, 2)));
  EXPECT_TRUE(bag.Insert(CheckedRange<uint64_t>(3, 4)));
  EXPECT_TRUE(bag.Insert(CheckedRange<uint64_t>(5, 6)));
  EXPECT_TRUE(bag.Remove(CheckedRange<uint64_t>(3, 4)));
  EXPECT_EQ(2u, bag.GetCount());

  // Test copy.
  TSimpleAddressRangeBag<10> bag_copy(bag);
  EXPECT_EQ(2u, bag_copy.GetCount());
  EXPECT_TRUE(bag_copy.Remove(CheckedRange<uint64_t>(1, 2)));
  EXPECT_TRUE(bag_copy.Remove(CheckedRange<uint64_t>(5, 6)));
  EXPECT_EQ(0u, bag_copy.GetCount());
  EXPECT_EQ(2u, bag.GetCount());

  // Test assign.
  TSimpleAddressRangeBag<10> bag_assign;
  bag_assign = bag;
  EXPECT_EQ(2u, bag_assign.GetCount());
  EXPECT_TRUE(bag_assign.Remove(CheckedRange<uint64_t>(1, 2)));
  EXPECT_TRUE(bag_assign.Remove(CheckedRange<uint64_t>(5, 6)));
  EXPECT_EQ(0u, bag_assign.GetCount());
  EXPECT_EQ(2u, bag.GetCount());
}

// Running out of space shouldn't crash.
TEST(SimpleAddressRangeBag, OutOfSpace) {
  TSimpleAddressRangeBag<2> bag;
  EXPECT_TRUE(bag.Insert(CheckedRange<uint64_t>(1, 2)));
  EXPECT_TRUE(bag.Insert(CheckedRange<uint64_t>(3, 4)));
  EXPECT_FALSE(bag.Insert(CheckedRange<uint64_t>(5, 6)));
  EXPECT_EQ(2u, bag.GetCount());
  EXPECT_FALSE(bag.Remove(CheckedRange<uint64_t>(5, 6)));
}

}  // namespace
}  // namespace test
}  // namespace crashpad
