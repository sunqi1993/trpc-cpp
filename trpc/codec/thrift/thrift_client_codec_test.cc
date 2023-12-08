// Copyright (c) 2021, Tencent Inc.
// All rights reserved.

#ifdef BUILD_INCLUDE_CODEC_THRIFT

#include "gtest/gtest.h"

#include "trpc/client/client_context.h"
#include "trpc/codec/codec_helper.h"
#include "trpc/codec/thrift/hello.thrift.trpc.h"
#include "trpc/codec/thrift/rpc_thrift/rpc_message_thrift.h"
#include "trpc/codec/thrift/thrift_client_codec.h"
#include "trpc/codec/thrift/thrift_protocol.h"
#include "trpc/serialization/trpc_serialization.h"

namespace trpc::detail {

constexpr int kFramePrefixSize = 12;

}  // namespace trpc::detail

namespace trpc::testing {
class ThriftClientCodecTest : public ::testing::Test {
 public:
  ThriftClientCodecTest()
      : message_header{0, "Test", codec::ToUType(ThriftMessageType::kCall), 930, true},
        struct_body("Hello world!") {}

 protected:
  void SetUp() override {
    message_header.frame_size =
        trpc::detail::kFramePrefixSize + message_header.function_name.size();
    trpc::serialization::Init();
  }

  void TearDown() override { trpc::serialization::Destory(); }

  ThriftMessageHeader message_header;
  std::string struct_body;
  ThriftClientCodec codec;
};

TEST_F(ThriftClientCodecTest, EasyFuncTest) {
  EXPECT_EQ(codec.Name(), "thrift");
}

TEST_F(ThriftClientCodecTest, ThriftClientCodecEncodeSucc) {
  ClientContextPtr context = MakeRefCounted<ClientContext>();
  context->SetEncodeType(EncodeType::THRIFT);
  context->SetEncodeDataType(trpc::serialization::kThrift);

  trpc::test::helloworld::greeter::SayHelloRequestStruct hello_req;
  hello_req.req.name = "helloworld";

  ProtocolPtr req_protocol = codec.CreateRequestPtr();
  NoncontiguousBuffer buff;

  EXPECT_TRUE(codec.FillRequest(context, req_protocol, reinterpret_cast<void*>(&hello_req)));
  EXPECT_TRUE(codec.ZeroCopyEncode(context, req_protocol, buff));

  trpc::serialization::Destory();
  EXPECT_FALSE(codec.FillRequest(context, req_protocol, reinterpret_cast<void*>(&hello_req)));
  trpc::serialization::Init();
}

TEST_F(ThriftClientCodecTest, ThriftClientCodecDecodeSucc) {
  trpc::test::helloworld::greeter::SayHelloResponseStruct hello_rsp;
  hello_rsp.result.message = "hello world";
  NoncontiguousBuffer hello_struct_body;
  serialization::ThriftSerialization thrift_serialization;
  thrift_serialization.Serialize(serialization::kThrift, &hello_rsp, &hello_struct_body);

  ThriftResponseProtocol rsp;
  rsp.SetNonContiguousProtocolBody(std::move(hello_struct_body));
  rsp.message_header = message_header;

  ClientContextPtr context = MakeRefCounted<ClientContext>();

  NoncontiguousBuffer buff;
  EXPECT_EQ(true, rsp.ZeroCopyEncode(buff));
  ProtocolPtr rsp_protocol = codec.CreateResponsePtr();
  context->SetResponse(rsp_protocol);

  context->SetEncodeType(EncodeType::THRIFT);
  context->SetEncodeDataType(trpc::serialization::kThrift);
  EXPECT_TRUE(codec.ZeroCopyDecode(context, buff, rsp_protocol));
  EXPECT_TRUE(codec.FillResponse(context, rsp_protocol, reinterpret_cast<void*>(&hello_rsp)));
}

TEST_F(ThriftClientCodecTest, ThriftClientCodecDecodeFail) {
  trpc::test::helloworld::greeter::SayHelloResponseStruct hello_rsp;

  ThriftResponseProtocol rsp;
  rsp.message_header = message_header;

  std::string body_str = "hello";
  NoncontiguousBufferBuilder builder;
  builder.Append(body_str.c_str(), body_str.size());
  rsp.struct_body = builder.DestructiveGet();

  NoncontiguousBuffer buff;
  EXPECT_EQ(true, rsp.ZeroCopyEncode(buff));
  ProtocolPtr rsp_protocol = codec.CreateResponsePtr();

  ClientContextPtr context = MakeRefCounted<ClientContext>();
  context->SetResponse(rsp_protocol);

  trpc::serialization::Destory();
  EXPECT_TRUE(codec.ZeroCopyDecode(context, buff, rsp_protocol));
  EXPECT_FALSE(codec.FillResponse(context, rsp_protocol, reinterpret_cast<void*>(&hello_rsp)));
  trpc::serialization::Init();

  context->SetEncodeType(EncodeType::THRIFT);
  context->SetEncodeDataType(trpc::serialization::kThrift);
  EXPECT_TRUE(codec.ZeroCopyDecode(context, buff, rsp_protocol));
  EXPECT_FALSE(codec.FillResponse(context, rsp_protocol, reinterpret_cast<void*>(&hello_rsp)));
}

}  // namespace trpc::testing
#endif