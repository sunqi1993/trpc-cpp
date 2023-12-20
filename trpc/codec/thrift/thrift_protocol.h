// Copyright (c) 2021, Tencent Inc.
// All rights reserved.

#ifdef BUILD_INCLUDE_CODEC_THRIFT
#pragma once

#include <stdint.h>

#include <string>

#include "trpc/codec/protocol.h"
#include "trpc/codec/thrift/rpc_thrift/rpc_thrift_enum.h"

namespace trpc {

namespace thrift {

TrpcRetCode GetTrpcRetCode(ThriftExceptionType exception_type);

ThriftExceptionType GetThriftExpType(TrpcRetCode ret_code);

}  // namespace thrift

struct ThriftMessageHeader {
  static constexpr uint32_t kPrefixLength = 4;
  static constexpr int kDefaultMaxFrameSize = 256 * 1024 * 1024;

  int32_t frame_size = 0;

  // 先写后读： function_name, message_type and sequence_id
  std::string function_name;
  int8_t message_type;
  int32_t sequence_id;

  bool is_strict = true;
};

class ThriftRequestProtocol : public trpc::Protocol {
 public:
  bool ZeroCopyDecode(NoncontiguousBuffer& buff) override;
  bool ZeroCopyEncode(NoncontiguousBuffer& buff) override;

  bool GetRequestId(uint32_t& req_id) const override;
  bool SetRequestId(uint32_t req_id) override;

  // void SetCallType(RpcCallType call_type) override;
  // RpcCallType GetCallType() override;

  void SetFuncName(std::string func_name) override;
  const std::string& GetFuncName() const override;

  NoncontiguousBuffer GetNonContiguousProtocolBody() override;
  void SetNonContiguousProtocolBody(NoncontiguousBuffer&& buff) override;

  // uint8_t GetEncodeType() override;

  uint32_t GetMessageSize() const override;

  void SetRequestMessageHeader(ThriftMessageHeader&& request_message_header);

 public:
  ThriftMessageHeader message_header;
  NoncontiguousBuffer struct_body;
};

class ThriftResponseProtocol : public trpc::Protocol {
 public:
  bool ZeroCopyEncode(NoncontiguousBuffer& buff) override;
  bool ZeroCopyDecode(NoncontiguousBuffer& buff) override;

  bool GetRequestId(uint32_t& req_id) const override;
  bool SetRequestId(uint32_t req_id) override;

  // void SetCallType(RpcCallType call_type) ;
  // RpcCallType GetCallType() ;

  void SetFuncName(std::string func_name) override;
  const std::string& GetFuncName() const override;

  NoncontiguousBuffer GetNonContiguousProtocolBody() override;
  void SetNonContiguousProtocolBody(NoncontiguousBuffer&& buff) override;

  // uint8_t GetEncodeType() override;

  uint32_t GetMessageSize() const override;

  void SetResponseMessageHeader(ThriftMessageHeader&& response_message_header);

 public:
  ThriftMessageHeader message_header;
  NoncontiguousBuffer struct_body;
};

using ThriftRequestProtocolPtr = std::shared_ptr<ThriftRequestProtocol>;
using ThriftResponseProtocolPtr = std::shared_ptr<ThriftResponseProtocol>;

}  // namespace trpc
#endif