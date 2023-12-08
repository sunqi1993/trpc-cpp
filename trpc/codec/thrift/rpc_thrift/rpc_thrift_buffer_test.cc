// Copyright (c) 2021, Tencent Inc.
// All rights reserved.
#ifdef BUILD_INCLUDE_CODEC_THRIFT

#include "gtest/gtest.h"

#include "rpc_thrift_buffer.h"

#include "trpc/codec/codec_helper.h"
#include "trpc/codec/thrift/rpc_thrift/rpc_message_thrift.h"
#include "trpc/codec/thrift/rpc_thrift/rpc_thrift_buffer.h"
#include "trpc/serialization/thrift/thrift_serialization.h"

namespace trpc::testing {

TEST(ThriftBufferTest, Skip) {
  ThriftException thrift_exception;
  thrift_exception.message = "SkipTest";
  NoncontiguousBuffer struct_body;
  serialization::ThriftSerialization thrift_serialization;
  thrift_serialization.Serialize(serialization::kThrift, &thrift_exception, &struct_body);
  uint32_t expect_size = struct_body.ByteSize();
  ThriftBuffer thrift_buffer(&struct_body);
  EXPECT_EQ(expect_size, thrift_buffer.Skip(trpc::codec::ToUType(ThriftDataType::kStruct)));
}

TEST(ThriftBufferTest, StrictTrue) {
  NoncontiguousBufferBuilder builder;
  ThriftBuffer thrift_buffer(&builder);

  std::string expect_method_name = "Test";
  int8_t expect_message_type = codec::ToUType(ThriftMessageType::kCall);
  int32_t expect_seqid = 930;
  uint32_t expect_is_strict = true;
  uint32_t expect_size = thrift_buffer.WriteMessageBegin(expect_method_name, expect_message_type,
                                                         expect_seqid, expect_is_strict);

  NoncontiguousBuffer buffer = builder.DestructiveGet();
  thrift_buffer.SetBuffer(&buffer);
  std::string method_name;
  int8_t message_type = codec::ToUType(ThriftMessageType::kReply);
  int32_t seqid = 0;
  bool is_strict = false;

  EXPECT_EQ(expect_size,
            thrift_buffer.ReadMessageBegin(method_name, message_type, seqid, is_strict));
  EXPECT_EQ(expect_method_name, method_name);
  EXPECT_EQ(expect_message_type, message_type);
  EXPECT_EQ(expect_seqid, seqid);
  EXPECT_EQ(expect_is_strict, is_strict);
}

TEST(ThriftBufferTest, StrictFalse) {
  NoncontiguousBufferBuilder builder;
  ThriftBuffer thrift_buffer(&builder);

  std::string expect_method_name = "Test";
  int8_t expect_message_type = codec::ToUType(ThriftMessageType::kCall);
  int32_t expect_seqid = 930;
  uint32_t expect_is_strict = false;
  uint32_t expect_size = thrift_buffer.WriteMessageBegin(expect_method_name, expect_message_type,
                                                         expect_seqid, expect_is_strict);

  NoncontiguousBuffer buffer = builder.DestructiveGet();
  thrift_buffer.SetBuffer(&buffer);
  std::string method_name;
  int8_t message_type = codec::ToUType(ThriftMessageType::kReply);
  int32_t seqid = 0;
  bool is_strict = true;

  EXPECT_EQ(expect_size,
            thrift_buffer.ReadMessageBegin(method_name, message_type, seqid, is_strict));
  EXPECT_EQ(expect_method_name, method_name);
  EXPECT_EQ(expect_message_type, message_type);
  EXPECT_EQ(expect_seqid, seqid);
  EXPECT_EQ(expect_is_strict, is_strict);
}

TEST(ThriftBufferTest, NotBeCalledFunc) {
  ThriftException thrift_exception;
  thrift_exception.message = "88888888888";
  NoncontiguousBuffer struct_body;
  serialization::ThriftSerialization thrift_serialization;
  thrift_serialization.Serialize(serialization::kThrift, &thrift_exception, &struct_body);
  ThriftBuffer thrift_buffer(&struct_body);
  uint64_t count_u64;
  int64_t count_i64;
  int expect_buf_len = 8;
  int ret_u64 = thrift_buffer.ReadU64(count_u64);
  int ret_i64 = thrift_buffer.ReadI64(count_i64);

  EXPECT_EQ(expect_buf_len, ret_u64);
  EXPECT_EQ(expect_buf_len, ret_i64);

  NoncontiguousBufferBuilder builder;
  int64_t count_i64_write = 99999;
  uint64_t count_u64_write = 99999;
  ThriftBuffer thrift_write_buffer(&builder);
  int ret_i64_write = thrift_write_buffer.WriteI64(count_i64_write);
  int ret_u64_write = thrift_write_buffer.WriteI64(count_u64_write);
  EXPECT_EQ(expect_buf_len, ret_i64_write);
  EXPECT_EQ(expect_buf_len, ret_u64_write);
}

}  // namespace trpc::testing

#endif
