// Copyright (c) 2021, Tencent Inc.
// All rights reserved.
#ifdef BUILD_INCLUDE_CODEC_THRIFT

#include "gtest/gtest.h"

#include "trpc/codec/codec_helper.h"
#include "trpc/codec/thrift/rpc_thrift/rpc_thrift_buffer.h"
#include "trpc/codec/thrift/thrift_protocol.h"

namespace {

constexpr int kFramePrefixSize = 12;

}  // namespace

namespace trpc::testing {
class ThriftProtocolTest : public ::testing::Test {
 public:
  ThriftProtocolTest()
      : message_header_{0, "Test", codec::ToUType(ThriftMessageType::kCall), 930, true},
        struct_body_("Hello world!") {}

 protected:
  void SetUp() override {
    message_header_.frame_size = kFramePrefixSize + message_header_.function_name.size() +
                                 sizeof(int32_t) + struct_body_.size();
  }

  ThriftMessageHeader message_header_;
  std::string struct_body_;
  ThriftRequestProtocol request_protocol_;
  ThriftResponseProtocol response_protocol_;
};

TEST_F(ThriftProtocolTest, RequestProtocolZeroCopyEncode) {
  request_protocol_.SetRequestId(message_header_.sequence_id);
  request_protocol_.SetCallType(RpcCallType::UNARY_CALL);
  request_protocol_.SetFuncName(message_header_.function_name);
  NoncontiguousBufferBuilder builder;
  ThriftBuffer write_buffer(&builder);
  write_buffer.WriteString(struct_body_);
  request_protocol_.SetNonContiguousProtocolBody(builder.DestructiveGet());
  NoncontiguousBuffer binary_data;
  EXPECT_TRUE(request_protocol_.ZeroCopyEncode(binary_data));
  EXPECT_EQ(0, request_protocol_.GetNonContiguousProtocolBody().ByteSize());
  EXPECT_TRUE(response_protocol_.ZeroCopyDecode(binary_data));
  uint32_t request_id = 0;
  response_protocol_.GetRequestId(request_id);
  EXPECT_EQ(message_header_.sequence_id, request_id);
  EXPECT_EQ(message_header_.message_type, response_protocol_.GetCallType());
  EXPECT_EQ(message_header_.function_name, response_protocol_.GetFuncName());
  EXPECT_EQ(static_cast<uint32_t>(message_header_.frame_size), response_protocol_.GetMessageSize());
  EXPECT_EQ(EncodeType::THRIFT, response_protocol_.GetEncodeType());
}

TEST_F(ThriftProtocolTest, ResponseProtocolZeroCopyEncodeDecode) {
  response_protocol_.SetRequestId(message_header_.sequence_id);
  response_protocol_.SetCallType(RpcCallType::UNARY_CALL);
  response_protocol_.SetFuncName(message_header_.function_name);
  NoncontiguousBufferBuilder builder;
  ThriftBuffer write_buffer(&builder);
  write_buffer.WriteString(struct_body_);

  response_protocol_.SetNonContiguousProtocolBody(builder.DestructiveGet());
  NoncontiguousBuffer binary_data;
  EXPECT_TRUE(response_protocol_.ZeroCopyEncode(binary_data));
  EXPECT_TRUE(request_protocol_.ZeroCopyDecode(binary_data));
  uint32_t request_id = 0;
  request_protocol_.GetRequestId(request_id);
  EXPECT_EQ(message_header_.sequence_id, request_id);
  EXPECT_EQ(message_header_.message_type, request_protocol_.GetCallType());
  EXPECT_EQ(message_header_.function_name, request_protocol_.GetFuncName());
  EXPECT_EQ(static_cast<uint32_t>(message_header_.frame_size), request_protocol_.GetMessageSize());
  EXPECT_EQ(EncodeType::THRIFT, request_protocol_.GetEncodeType());
}

}  // namespace trpc::testing

#endif
