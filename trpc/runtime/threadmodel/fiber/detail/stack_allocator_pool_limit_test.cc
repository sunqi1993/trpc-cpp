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
#include <vector>

#include "gtest/gtest.h"

#ifndef TRPC_TEST_POOL_SIZE
#error "TRPC_TEST_POOL_SIZE must be defined"
#endif

namespace trpc::fiber::detail {
namespace {

struct Allocation {
  void* stack = nullptr;
  bool is_system = true;
};

TEST(StackAllocatorPoolLimit, HonorsExactLimit) {
  constexpr size_t kPoolSize = TRPC_TEST_POOL_SIZE;
  SetFiberStackSize(64 * 1024);
  SetFiberPoolNumByMmap(kPoolSize);

  std::vector<Allocation> allocations(kPoolSize + 1);
  for (size_t i = 0; i < allocations.size(); ++i) {
    ASSERT_TRUE(Allocate(&allocations[i].stack, &allocations[i].is_system));
    ASSERT_NE(allocations[i].stack, nullptr);
    EXPECT_EQ(allocations[i].is_system, i == kPoolSize);
  }

  for (const auto& allocation : allocations) {
    Deallocate(allocation.stack, allocation.is_system);
  }
}

}  // namespace
}  // namespace trpc::fiber::detail
