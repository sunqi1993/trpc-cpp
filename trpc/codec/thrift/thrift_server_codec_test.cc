// Copyright (c) 2021, Tencent Inc.
// All rights reserved.
#ifdef BUILD_INCLUDE_CODEC_THRIFT

#include "gtest/gtest.h"

#include "trpc/codec/codec_helper.h"
#include "trpc/codec/thrift/rpc_thrift/rpc_message_thrift.h"
#include "trpc/codec/thrift/thrift_protocol.h"
#include "trpc/codec/thrift/thrift_server_codec.h"
#include "trpc/server/server_context.h"

namespace {

constexpr int kFramePrefixSize = 12;

trpc::STransportReqMsg PackTransportReqMsg() {
  trpc::STransportReqMsg req_msg;

  req_msg.basic_info = trpc::object_pool::MakeLwShared<trpc::BasicInfo>();

  req_msg.basic_info->connection_id = 1;
  req_msg.basic_info->addr.ip = "127.0.0.1";
  req_msg.basic_info->addr.port = 10000;
  req_msg.basic_info->fd = 1001;
  req_msg.basic_info->stream_id = 1;
  req_msg.basic_info->begin_timestamp_us = 0;

  return req_msg;
}

}  // namespace

namespace trpc::testing {
class ThriftServerCodecTest : public ::testing::Test {
 public:
  ThriftServerCodecTest()
      : message_header_{0, "Test", codec::ToUType(ThriftMessageType::kCall), 930, true},
        struct_body_("Hello world!") {}

 protected:
  void SetUp() override {
    message_header_.frame_size = kFramePrefixSize + message_header_.function_name.size();
  }

  ThriftMessageHeader message_header_;
  std::string struct_body_;
  ThriftServerCodec codec_;
};

TEST_F(ThriftServerCodecTest, ThriftServerCodecName) { EXPECT_EQ("thrift", codec_.Name()); }

TEST_F(ThriftServerCodecTest, SupportZeroCopyWithTrue) { EXPECT_TRUE(codec_.SupportZeroCopy()); }

TEST_F(ThriftServerCodecTest, GetTrpcRetCode) {
  EXPECT_EQ(TrpcRetCode::TRPC_SERVER_NOSERVICE_ERR,
            trpc::thrift::GetTrpcRetCode(ThriftExceptionType::kUnknownMethod));
  EXPECT_EQ(TrpcRetCode::TRPC_SERVER_DECODE_ERR,
            trpc::thrift::GetTrpcRetCode(ThriftExceptionType::kInvalidMessageType));
  EXPECT_EQ(TrpcRetCode::TRPC_SERVER_NOFUNC_ERR,
            trpc::thrift::GetTrpcRetCode(ThriftExceptionType::kWrongMethodName));
  EXPECT_EQ(TrpcRetCode::TRPC_SERVER_ENCODE_ERR,
            trpc::thrift::GetTrpcRetCode(ThriftExceptionType::kMissingResult));
  EXPECT_EQ(TrpcRetCode::TRPC_INVOKE_UNKNOWN_ERR,
            trpc::thrift::GetTrpcRetCode(ThriftExceptionType::kUnknown));
}

TEST_F(ThriftServerCodecTest, ThriftServerRetException) {
  // 构造超时的返回码
  STransportReqMsg req_msg;
  req_msg.basic_info = object_pool::MakeLwShared<trpc::BasicInfo>();

  ServerContextPtr context = MakeRefCounted<ServerContext>(req_msg);

  context->SetRequestMsg(codec_.CreateRequestObject());
  context->SetResponseMsg(codec_.CreateResponseObject());
  ProtocolPtr& req = context->GetRequestMsg();
  req->SetRequestId(1);
  ProtocolPtr& rsp = context->GetResponseMsg();
  std::string expect_error_message = "TimeOut";

  context->GetStatus().SetFrameworkRetCode(TrpcRetCode::TRPC_SERVER_TIMEOUT_ERR);
  context->GetStatus().SetErrorMessage(expect_error_message);

  NoncontiguousBuffer send_data;
  EXPECT_TRUE(codec_.ZeroCopyEncode(context, rsp, send_data));

  auto* thrift_req = dynamic_cast<ThriftRequestProtocol*>(req.get());
  thrift_req->ZeroCopyDecode(send_data);
  ThriftException thrift_exception;
  serialization::ThriftSerialization thrift_serialization;
  NoncontiguousBuffer struct_body = thrift_req->GetNonContiguousProtocolBody();
  thrift_serialization.Deserialize(&struct_body, serialization::kThrift,
                                   reinterpret_cast<void*>(&thrift_exception));

  EXPECT_EQ(
      trpc::codec::ToUType(trpc::thrift::GetThriftExpType(TrpcRetCode::TRPC_SERVER_TIMEOUT_ERR)),
      thrift_exception.type);
  EXPECT_EQ(expect_error_message, thrift_exception.message);
}

TEST_F(ThriftServerCodecTest, ThriftServerCodecEncode) {
  ServerContextPtr context = MakeRefCounted<ServerContext>(PackTransportReqMsg());

  ProtocolPtr req = codec_.CreateRequestObject();
  ProtocolPtr rsp = codec_.CreateResponseObject();

  auto* thrift_req = dynamic_cast<ThriftRequestProtocol*>(req.get());

  thrift_req->SetRequestMessageHeader(std::move(message_header_));

  context->SetRequestMsg(std::move(req));

  // 判断thrift_req是否设置到context中
  EXPECT_EQ(context->GetRequestMsg().get(), thrift_req);

  NoncontiguousBuffer buff;
  EXPECT_TRUE(codec_.ZeroCopyEncode(context, rsp, buff));
}

TEST_F(ThriftServerCodecTest, ThriftServerCodecDecode) {
  ThriftRequestProtocol req;

  req.SetRequestMessageHeader(std::move(message_header_));

  NoncontiguousBuffer buff;

  EXPECT_EQ(true, req.ZeroCopyEncode(buff));

  ServerContextPtr context = MakeRefCounted<ServerContext>(PackTransportReqMsg());

  ProtocolPtr thrift_req = codec_.CreateRequestObject();

  context->SetRequestMsg(std::move(thrift_req));

  context->SetResponseMsg(codec_.CreateResponseObject());

  EXPECT_TRUE(codec_.ZeroCopyDecode(context, std::move(buff), context->GetRequestMsg()));
}

}  // namespace trpc::testing

#endif
