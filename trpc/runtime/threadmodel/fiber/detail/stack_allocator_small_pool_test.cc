//
// Tencent is pleased to support the open source community by making tRPC
// available.
//
// Copyright (C) 2023 Tencent.
// All rights reserved.
//
// tRPC is licensed under the Apache 2.0 License.
//

#include "trpc/runtime/threadmodel/fiber/detail/stack_allocator_impl.h"

#include <cstddef>
#include <thread>
#include <vector>

#include "gtest/gtest.h"

namespace trpc::fiber::detail {
namespace {

struct Allocation {
  void* stack = nullptr;
  bool is_system = true;
};

TEST(StackAllocatorSmallPool, HonorsExactNonBlockMultipleMmapLimit) {
  constexpr uint32_t kPoolSize = 200;
  EXPECT_EQ(PrewarmFiberPool(0), 0);
  SetFiberStackSize(64 * 1024);
  SetFiberPoolNumByMmap(kPoolSize);

  std::vector<Allocation> allocations(kPoolSize + 1);
  for (size_t i = 0; i < allocations.size(); ++i) {
    ASSERT_TRUE(Allocate(&allocations[i].stack, &allocations[i].is_system));
    ASSERT_NE(allocations[i].stack, nullptr);
    EXPECT_EQ(allocations[i].is_system, i == kPoolSize);
  }

  for (size_t i = 32; i < allocations.size(); ++i) {
    Deallocate(allocations[i].stack, allocations[i].is_system);
  }

  for (size_t i = 0; i < 32; ++i) {
    std::thread([allocation = allocations[i]] {
      Deallocate(allocation.stack, allocation.is_system);
    }).join();
  }

  std::vector<Allocation> reused(kPoolSize + 1);
  for (size_t i = 0; i < reused.size(); ++i) {
    ASSERT_TRUE(Allocate(&reused[i].stack, &reused[i].is_system));
    ASSERT_NE(reused[i].stack, nullptr);
    EXPECT_EQ(reused[i].is_system, i == kPoolSize);
  }

  for (const auto& allocation : reused) {
    Deallocate(allocation.stack, allocation.is_system);
  }

  EXPECT_DEATH(SetFiberPoolNumByMmap(kPoolSize + 1),
               "before the first fiber stack allocation");
}

}  // namespace
}  // namespace trpc::fiber::detail
