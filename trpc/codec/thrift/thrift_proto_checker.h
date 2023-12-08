// Copyright (c) 2021, Tencent Inc.
// All rights reserved.

#ifdef BUILD_INCLUDE_CODEC_THRIFT
#pragma once

#include <any>
#include <deque>

#include "trpc/util/buffer/buffer.h"
#include "trpc/runtime/iomodel/reactor/common/connection.h"

namespace trpc {

int ThriftZeroCopyCheck(const ConnectionPtr& conn, NoncontiguousBuffer& in,
                        std::deque<std::any>& out);

}  // namespace trpc
#endif
