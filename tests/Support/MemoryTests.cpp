#include "kv/Support/Memory.h"

#include <limits>

#include "gtest/gtest.h"

TEST(RawAllocator, TestBasicAlloc) {
  kv::RawAllocator allocator;

  auto ptrAlign8 = allocator.Allocate(8, 8);
  ASSERT_EQ(reinterpret_cast<uintptr_t>(ptrAlign8) & 7, 0);

  auto ptrAlign32 = allocator.Allocate(8, 32);
  ASSERT_EQ(reinterpret_cast<uintptr_t>(ptrAlign32) & 31, 0);
}

TEST(RawAllocator, TestAllocTooLarge) {
  kv::RawAllocator allocator;

  ASSERT_THROW(allocator.Allocate(std::numeric_limits<size_t>::max() - 8, 8), std::bad_alloc);
}

TEST(RawAllocator, TestBasicRelease) {
  kv::RawAllocator allocator;

  auto ptr1 = allocator.Allocate(8, 8);
  allocator.Release(ptr1);

  auto ptr2 = allocator.Allocate(8, 8);
  ASSERT_EQ(ptr1, ptr2);
}

TEST(RawAllocator, TestChunkMergeForward) {
  kv::RawAllocator allocator;

  auto ptr1 = allocator.Allocate(8, 8);
  auto ptr2 = allocator.Allocate(8, 8);
  allocator.Release(ptr2);
  allocator.Release(ptr1);

  auto ptr3 = allocator.Allocate(16, 8);
  ASSERT_EQ(ptr1, ptr3);
}
