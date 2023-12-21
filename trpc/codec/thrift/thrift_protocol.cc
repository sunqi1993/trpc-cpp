// Copyright (c) 2021, Tencent Inc.
// All rights reserved.

#ifdef BUILD_INCLUDE_CODEC_THRIFT

#include "trpc/codec/thrift/thrift_protocol.h"

#include <utility>

#include "trpc/codec/thrift/rpc_thrift/rpc_thrift_buffer.h"
#include "trpc/util/buffer/buffer.h"
#include "trpc/common/logging/trpc_logging.h"

namespace trpc {

namespace thrift {

TrpcRetCode GetTrpcRetCode(ThriftExceptionType exception_type) {
  if (exception_type == ThriftExceptionType::kUnknownMethod) {
    return TrpcRetCode::TRPC_SERVER_NOSERVICE_ERR;
  }
  if (exception_type == ThriftExceptionType::kInvalidMessageType ||
      exception_type == ThriftExceptionType::kBadSequenceId ||
      exception_type == ThriftExceptionType::kProtocolError ||
      exception_type == ThriftExceptionType::kInvalidTransform ||
      exception_type == ThriftExceptionType::kInvalidProtocol) {
    return TrpcRetCode::TRPC_SERVER_DECODE_ERR;
  }
  if (exception_type == ThriftExceptionType::kWrongMethodName) {
    return TrpcRetCode::TRPC_SERVER_NOFUNC_ERR;
  }
  if (exception_type == ThriftExceptionType::kMissingResult) {
    return TrpcRetCode::TRPC_SERVER_ENCODE_ERR;
  }
  if (exception_type == ThriftExceptionType::kUnsupportedClientType) {
    return TrpcRetCode::TRPC_CLIENT_CONNECT_ERR;
  }

  return TrpcRetCode::TRPC_INVOKE_UNKNOWN_ERR;
}

bool IsInternalError(TrpcRetCode ret_code) {
  if (ret_code == TrpcRetCode::TRPC_SERVER_TIMEOUT_ERR ||
      ret_code == TrpcRetCode::TRPC_SERVER_FULL_LINK_TIMEOUT_ERR ||
      ret_code == TrpcRetCode::TRPC_SERVER_OVERLOAD_ERR ||
      ret_code == TrpcRetCode::TRPC_SERVER_LIMITED_ERR ||
      ret_code == TrpcRetCode::TRPC_CLIENT_INVOKE_TIMEOUT_ERR ||
      ret_code == TrpcRetCode::TRPC_CLIENT_FULL_LINK_TIMEOUT_ERR ||
      ret_code == TrpcRetCode::TRPC_CLIENT_LIMITED_ERR ||
      ret_code == TrpcRetCode::TRPC_CLIENT_OVERLOAD_ERR ||
      ret_code == TrpcRetCode::TRPC_CLIENT_CONNECT_ERR ||
      ret_code == TrpcRetCode::TRPC_CLIENT_ROUTER_ERR) {
    return true;
  }
  return false;
}

ThriftExceptionType GetThriftExpType(TrpcRetCode ret_code) {
  if (ret_code == TrpcRetCode::TRPC_SERVER_DECODE_ERR || ret_code == TrpcRetCode::TRPC_SERVER_ENCODE_ERR ||
      ret_code == TrpcRetCode::TRPC_CLIENT_ENCODE_ERR || ret_code == TrpcRetCode::TRPC_CLIENT_DECODE_ERR) {
    return ThriftExceptionType::kProtocolError;
  }
  if (ret_code == TrpcRetCode::TRPC_SERVER_NOSERVICE_ERR) {
    return ThriftExceptionType::kUnknownMethod;
  }
  if (ret_code == TrpcRetCode::TRPC_SERVER_NOFUNC_ERR) {
    return ThriftExceptionType::kWrongMethodName;
  }
  if (IsInternalError(ret_code)) {
    return ThriftExceptionType::kInternalError;
  }
  return ThriftExceptionType::kUnknown;
}

}  // namespace thrift

// frame_size | message_begin | common_header | struct_body | attachment | crc_code
// message_begin = (function_name + message_type + sequence_id + is_strict)
bool ThriftRequestProtocol::ZeroCopyDecode(NoncontiguousBuffer& buff) {
  ThriftBuffer thrift_buffer(&buff);

  // 1. read frame size
  if (thrift_buffer.ReadI32(message_header.frame_size) != ThriftMessageHeader::kPrefixLength) {
    return false;
  }

  // 2. read message head
  thrift_buffer.ReadMessageBegin(message_header.function_name, message_header.message_type,
                                 message_header.sequence_id, message_header.is_strict);

  // 3. request struct body
  struct_body = std::move(buff);

  return true;
}

bool ThriftRequestProtocol::ZeroCopyEncode(NoncontiguousBuffer& buff) {
  NoncontiguousBufferBuilder message_builder;
  ThriftBuffer thrift_buffer(&message_builder);
  // 1. calculate frame size
  constexpr int kFramePrefixSize = 12;
  message_header.frame_size =
      static_cast<int32_t>(kFramePrefixSize + message_header.function_name.size() + struct_body.ByteSize() + 4);
  if (thrift_buffer.WriteI32(message_header.frame_size) != ThriftMessageHeader::kPrefixLength) {
    return false;
  }

  // 2. write message begin
  thrift_buffer.WriteMessageBegin(message_header.function_name, message_header.message_type,
                                  message_header.sequence_id, message_header.is_strict);

  // 3. write response body
  thrift_buffer.WriteFieldBegin(12, 0);
  message_builder.Append(std::move(struct_body));
  thrift_buffer.WriteI08(0);
  buff = message_builder.DestructiveGet();

  return true;
}

bool ThriftRequestProtocol::GetRequestId(uint32_t& req_id) const {
  req_id = static_cast<uint32_t>(message_header.sequence_id);
  return true;
}

bool ThriftRequestProtocol::SetRequestId(uint32_t req_id) {
  message_header.sequence_id = static_cast<int32_t>(req_id);
  return true;
}

// void ThriftRequestProtocol::SetCallType(RpcCallType call_type) {
//   if (call_type == RpcCallType::UNARY_CALL) {
//     message_header.message_type = codec::ToUType(ThriftMessageType::kCall);
//   } else if (call_type == RpcCallType::ONEWAY_CALL) {
//     message_header.message_type = codec::ToUType(ThriftMessageType::kOneway);
//   }
// }

// RpcCallType ThriftRequestProtocol::GetCallType() {
//   return message_header.message_type == codec::ToUType(ThriftMessageType::kCall) ? RpcCallType::UNARY_CALL
//                                                                                  : RpcCallType::ONEWAY_CALL;
// }

void ThriftRequestProtocol::SetFuncName(std::string func_name) { message_header.function_name = func_name; }

const std::string& ThriftRequestProtocol::GetFuncName() const { return message_header.function_name; }

NoncontiguousBuffer ThriftRequestProtocol::GetNonContiguousProtocolBody() { return std::move(struct_body); }

void ThriftRequestProtocol::SetNonContiguousProtocolBody(NoncontiguousBuffer&& buff) { struct_body = std::move(buff); }

// uint8_t ThriftRequestProtocol::GetEncodeType() { return EncodeType::THRIFT; }

uint32_t ThriftRequestProtocol::GetMessageSize()const  { return static_cast<uint32_t>(message_header.frame_size); }

void ThriftRequestProtocol::SetRequestMessageHeader(ThriftMessageHeader&& request_message_header) {
  message_header = std::move(request_message_header);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

bool ThriftResponseProtocol::ZeroCopyEncode(NoncontiguousBuffer& buff) {
  NoncontiguousBufferBuilder message_builder;
  ThriftBuffer thrift_buffer(&message_builder);
  // 1. calculate frame size
  // TODO 内存计量
  constexpr int kFramePrefixSize = 12;
  message_header.frame_size =
      static_cast<int32_t>(kFramePrefixSize + message_header.function_name.size() + struct_body.ByteSize()+3+1);
  if (thrift_buffer.WriteI32(message_header.frame_size) != ThriftMessageHeader::kPrefixLength) {
    return false;
  }

  // 2. write message begin
  thrift_buffer.WriteMessageBegin(message_header.function_name, message_header.message_type,
                                  message_header.sequence_id, message_header.is_strict);

  // 3. write response body
  
  thrift_buffer.WriteFieldBegin(12, 0);
  message_builder.Append(std::move(struct_body));
  thrift_buffer.WriteI08(0);
  buff = message_builder.DestructiveGet();

  return true;
}

// frame_size | message_begin | common_header | struct_body | attachment | crc_code
// message_begin = (function_name + message_type + sequence_id + is_strict)
bool ThriftResponseProtocol::ZeroCopyDecode(NoncontiguousBuffer& buff) {
  ThriftBuffer thrift_buffer(&buff);

  // 1. read frame size
  if (thrift_buffer.ReadI32(message_header.frame_size) != ThriftMessageHeader::kPrefixLength) {
    return false;
  }

  // 2. read message head
  thrift_buffer.ReadMessageBegin(message_header.function_name, message_header.message_type,
                                 message_header.sequence_id, message_header.is_strict);

  // 3. request struct body
  struct_body = std::move(buff);

  return true;
}

bool ThriftResponseProtocol::GetRequestId(uint32_t& req_id) const {
  req_id = static_cast<uint32_t>(message_header.sequence_id);
  return true;
}

bool ThriftResponseProtocol::SetRequestId(uint32_t req_id) {
  message_header.sequence_id = static_cast<int32_t>(req_id);
  return true;
}

// void ThriftResponseProtocol::SetCallType(RpcCallType call_type) {
//   if (call_type == RpcCallType::UNARY_CALL || call_type == RpcCallType::ONEWAY_CALL) {
//     message_header.message_type = codec::ToUType(ThriftMessageType::kReply);
//   }
// }

// RpcCallType ThriftResponseProtocol::GetCallType() {
//   if (message_header.message_type == codec::ToUType(ThriftMessageType::kReply) ||
//       message_header.message_type == codec::ToUType(ThriftMessageType::kException)) {
//     return RpcCallType::UNARY_CALL;
//   }
//   return RpcCallType::ONEWAY_CALL;
// }

void ThriftResponseProtocol::SetFuncName(std::string func_name) { message_header.function_name = func_name; }

const std::string& ThriftResponseProtocol::GetFuncName() const { return message_header.function_name; }

NoncontiguousBuffer ThriftResponseProtocol::GetNonContiguousProtocolBody() { return std::move(struct_body); }

void ThriftResponseProtocol::SetNonContiguousProtocolBody(NoncontiguousBuffer&& buff) { struct_body = std::move(buff); }

// uint8_t ThriftResponseProtocol::GetEncodeType() { return EncodeType::THRIFT; }

uint32_t ThriftResponseProtocol::GetMessageSize() const { return static_cast<uint32_t>(message_header.frame_size); }

void ThriftResponseProtocol::SetResponseMessageHeader(ThriftMessageHeader&& response_message_header) {
  message_header = std::move(response_message_header);
}

}  // namespace trpc
#endif