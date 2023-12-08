// Copyright (c) 2021, Tencent Inc.
// All rights reserved.
#ifdef BUILD_INCLUDE_CODEC_THRIFT
#include "trpc/codec/thrift/thrift_server_codec.h"

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

int ThriftServerCodec::ZeroCopyCheck(const ConnectionPtr& conn, NoncontiguousBuffer& in,
                                     std::deque<std::any>& out) {
  return ThriftZeroCopyCheck(conn, in, out);
}

bool ThriftServerCodec::ZeroCopyDecode(const ServerContextPtr& context, std::any&& in,
                                       ProtocolPtr& out) {
  bool ret = true;
  try {
    auto buff = std::any_cast<NoncontiguousBuffer&&>(std::move(in));
    auto* request = static_cast<ThriftRequestProtocol*>(out.get());
    ret = request->ZeroCopyDecode(buff);
  } catch (std::exception& ex) {
    TRPC_FMT_ERROR("ex:{}", ex.what());
    ret = false;
  }

  // 如果Decode失败，则不需要框架回包
  if (!ret) {
    context->SetResponseWhenDecodeFail(false);
  }

  return ret;
}

bool ThriftServerCodec::ZeroCopyEncode(const ServerContextPtr& context, ProtocolPtr& in,
                                       NoncontiguousBuffer& out) {
  try {
    ProtocolPtr request_msg = context->GetRequestMsg();
    auto* request = static_cast<ThriftRequestProtocol*>(request_msg.get());
    auto* response = static_cast<ThriftResponseProtocol*>(in.get());

    // 支持 thrift 的多路复用协议 ： service_name + separator + function_name
    // TMultiplexedProtocol allows a Thrift client to communicate with a multiplexing Thrift
    // server, by prepending the service name to the function name during function calls.   Service
    // multiplexing allows services to share a single port, and therefore transport. By allowing
    // many services to operate over a single port,
    ThriftMessageHeader response_message_header;
    std::string func_name = request->GetFuncName();
    std::string separator = ":";
    std::size_t found = func_name.find_last_of(separator);
    if (found == std::string::npos) {
      response_message_header.function_name = func_name;
    } else {
      response_message_header.function_name = func_name.substr(found + 1);
    }

    // 支持thrift 返回异常：从框架错误码和函数返回码中构造异常类型
    Status status = context->GetStatus();
    int framework_ret_code = status.GetFrameworkRetCode();
    int func_ret_code = status.GetFuncRetCode();
    if (framework_ret_code == TrpcRetCode::TRPC_INVOKE_SUCCESS &&
        func_ret_code == TrpcRetCode::TRPC_INVOKE_SUCCESS) {
      response_message_header.message_type = codec::ToUType(ThriftMessageType::kReply);
    } else {
      response_message_header.message_type = codec::ToUType(ThriftMessageType::kException);

      ThriftException thrift_exception;
      thrift_exception.message = status.ErrorMessage();
      if (framework_ret_code != TrpcRetCode::TRPC_INVOKE_SUCCESS) {
        thrift_exception.type =
            codec::ToUType(thrift::GetThriftExpType(static_cast<TrpcRetCode>(framework_ret_code)));
      } else {
        thrift_exception.type = func_ret_code;
      }

      serialization::ThriftSerialization thrift_serialization;
      NoncontiguousBuffer struct_body;
      thrift_serialization.Serialize(serialization::kThrift, &thrift_exception, &struct_body);
      response->SetNonContiguousProtocolBody(std::move(struct_body));
    }

    uint32_t request_id;
    request->GetRequestId(request_id);
    response_message_header.sequence_id = static_cast<int32_t>(request_id);
    response->SetResponseMessageHeader(std::move(response_message_header));

    return response->ZeroCopyEncode(out);
  } catch (std::exception& ex) {
    TRPC_FMT_ERROR("ex: {}", ex.what());
  }

  return false;
}

ProtocolPtr ThriftServerCodec::CreateRequestObject() {
  return std::make_shared<ThriftRequestProtocol>();
}

ProtocolPtr ThriftServerCodec::CreateResponseObject() {
  return std::make_shared<ThriftResponseProtocol>();
}

}  // namespace trpc
#endif