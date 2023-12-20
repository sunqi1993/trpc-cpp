// Copyright (c) 2021, Tencent Inc.
// All rights reserved.

#ifdef BUILD_INCLUDE_CODEC_THRIFT
#include "trpc/codec/thrift/thrift_client_codec.h"

#include <deque>
#include <exception>
#include <memory>
#include <utility>

#include "trpc/codec/thrift/rpc_thrift/rpc_message_thrift.h"
#include "trpc/codec/thrift/rpc_thrift/rpc_thrift_idl.h"
#include "trpc/codec/thrift/thrift_proto_checker.h"
#include "trpc/codec/thrift/thrift_protocol.h"
#include "trpc/common/logging/trpc_logging.h"

namespace trpc {

int ThriftClientCodec::ZeroCopyCheck(const ConnectionPtr& conn, NoncontiguousBuffer& in,
                                     std::deque<std::any>& out) {
  return ThriftZeroCopyCheck(conn, in, out);
}

bool ThriftClientCodec::ZeroCopyDecode(const ClientContextPtr& context, std::any&& in,
                                       ProtocolPtr& out) {
  auto buff = std::any_cast<NoncontiguousBuffer&&>(std::move(in));
  auto* response = static_cast<ThriftResponseProtocol*>(out.get());
  bool ret = response->ZeroCopyDecode(buff);

  return ret;
}

bool ThriftClientCodec::ZeroCopyEncode(const ClientContextPtr& context, const ProtocolPtr& in,
                                       NoncontiguousBuffer& out) {
  auto* request = static_cast<ThriftRequestProtocol*>(in.get());

  ThriftMessageHeader request_message_header;
  std::string func_name = context->GetFuncName();
  request_message_header.function_name = func_name;
  request_message_header.message_type = codec::ToUType(ThriftMessageType::kCall);

  uint32_t request_id = context->GetRequestId();
  request_message_header.sequence_id = static_cast<int32_t>(request_id);
  request->SetRequestMessageHeader(std::move(request_message_header));

  return request->ZeroCopyEncode(out);
}

ProtocolPtr ThriftClientCodec::CreateRequestPtr() {
  return std::make_shared<ThriftRequestProtocol>();
}

ProtocolPtr ThriftClientCodec::CreateResponsePtr() {
  return std::make_shared<ThriftResponseProtocol>();
}

bool ThriftClientCodec::Deserialize(const ClientContextPtr& context, NoncontiguousBuffer* buffer,
                                    void* body) {
  serialization::SerializationType serialization_type = context->GetRspEncodeType();
  auto* serialization_factory = serialization::SerializationFactory::GetInstance();
  if (TRPC_UNLIKELY(serialization_type >= trpc::serialization::kMaxType) ||
      serialization_factory->Get(serialization_type) == nullptr) {
    std::string error_msg = "not support deserialization_type:";
    error_msg += std::to_string(serialization_type);

    TRPC_LOG_ERROR(error_msg);

    context->SetStatus(
        Status(GetDefaultClientRetCode(codec::ClientRetCode::ENCODE_ERROR), "unknown encode type"));
    return false;
  }

  serialization::DataType data_type = context->GetRspEncodeDataType();
  auto deserialize_success =
      serialization_factory->Get(serialization_type)->Deserialize(buffer, data_type, body);
  if (TRPC_UNLIKELY(!deserialize_success)) {
    Status status{GetDefaultClientRetCode(codec::ClientRetCode::DECODE_ERROR), 0, "decode failed"};
    TRPC_LOG_ERROR(status.ToString());
    context->SetStatus(std::move(status));
  }
  return deserialize_success;
}

bool ThriftClientCodec::Serialize(const ClientContextPtr& context, void* body,
                                  NoncontiguousBuffer* buffer) {
  serialization::SerializationType serialization_type = context->GetReqEncodeType();
  auto* serialization_factory = serialization::SerializationFactory::GetInstance();
  if (TRPC_UNLIKELY(serialization_type >= trpc::serialization::kMaxType) ||
      serialization_factory->Get(serialization_type) == nullptr) {
    Status status{GetDefaultClientRetCode(codec::ClientRetCode::ENCODE_ERROR), 0,
                  "unsupported serialization type: " + std::to_string(serialization_type)};
    TRPC_LOG_ERROR(status.ToString());
    context->SetStatus(std::move(status));
    return false;
  }

  serialization::DataType data_type = context->GetReqEncodeDataType();
  auto serialize_success =
      serialization_factory->Get(serialization_type)->Serialize(data_type, body, buffer);
  if (TRPC_UNLIKELY(!serialize_success)) {
    Status status{GetDefaultClientRetCode(codec::ClientRetCode::ENCODE_ERROR), 0, "encode failed"};
    TRPC_LOG_ERROR(status.ToString());
    context->SetStatus(std::move(status));
  }
  return serialize_success;
}

bool ThriftClientCodec::FillRequest(const ClientContextPtr& context, const ProtocolPtr& in,
                                    void* body) {
  assert(body);

  auto* thrift_req_protocol = static_cast<ThriftRequestProtocol*>(in.get());
  assert(thrift_req_protocol);
  NoncontiguousBuffer buffer{};
  if (!Serialize(context, body, &buffer)) {
    return false;
  }
  thrift_req_protocol->SetNonContiguousProtocolBody(std::move(buffer));
  return true;
}

bool ThriftClientCodec::FillResponse(const ClientContextPtr& context, const ProtocolPtr& in,
                                     void* body) {
  TRPC_ASSERT(body);

  Status status;

  auto* thrift_rsp_protocol = static_cast<ThriftResponseProtocol*>(in.get());
  TRPC_ASSERT(thrift_rsp_protocol);

  auto rsp_body_data = thrift_rsp_protocol->GetNonContiguousProtocolBody();
  const auto& rsp_header = thrift_rsp_protocol->message_header;

  // thrift协议需要解码后才知道error msg
  if (TRPC_UNLIKELY(rsp_header.message_type == 3)) {
    ThriftException thrift_exception;
    // 此处统一返回false，解出错误码成功则赋值新的status，否则不赋值status
    if (Deserialize(context, &rsp_body_data, &thrift_exception)) {
      Status status{GetDefaultClientRetCode(codec::ClientRetCode::DECODE_ERROR),
                    thrift_exception.type, std::move(std::string{thrift_exception.message})};
      context->SetStatus(std::move(status));
    }
    return false;
  }

  // 正常解码
  if (!Deserialize(context, &rsp_body_data, body)) {
    return false;
  }
  return true;
}

}  // namespace trpc
#endif