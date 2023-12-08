// Copyright (c) 2021, Tencent Inc.
// All rights reserved.
#ifdef BUILD_INCLUDE_CODEC_THRIFT
#pragma once

#include <cstdint>
#include <memory>

#include "trpc/serialization/serialization.h"

namespace trpc::serialization {

/**
 * Thrift数据序列化与反序列化实现
 */
class ThriftSerialization : public Serialization {
 public:
  // 业务层数据序列化与反序列化的类型
  // 不同序列化与反序列化的实现，需要有不同的类型定义
  SerializationType Type() const override;

  // 业务层数据序列化
  // 输入参数in是thrift struct类型的输入对象
  // 输出参数out是序列化后的二进制数据对象
  // 返回参数，表示成功或失败
  bool Serialize(DataType in_type, void* in, NoncontiguousBuffer* out) override;

  // 业务层数据反序列化
  // 输入参数in是需要反序列化的二进制数据
  // 输出参数out是thrift struct类型的输出对象
  // 返回参数，表示成功或失败
  bool Deserialize(NoncontiguousBuffer* in, DataType out_type, void* out) override;
};

}  // namespace trpc::serialization
#endif
