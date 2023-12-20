// Copyright (c) 2021, Tencent Inc.
// All rights reserved.
#ifdef BUILD_INCLUDE_CODEC_THRIFT
#pragma once

#include <any>
#include <deque>
#include <string>

#include "trpc/codec/server_codec.h"
#include "trpc/codec/thrift/rpc_thrift/rpc_thrift_enum.h"
#include "trpc/serialization/thrift/thrift_serialization.h"
#include "trpc/server/server_context.h"

namespace trpc {

/**
 * 服务端trpc协议编解码的实现类
 */
class ThriftServerCodec : public trpc::ServerCodec {
 public:
  [[nodiscard]] std::string Name() const override { return "thrift"; }

  /// @brief 协议完整性检查
  /// @param conn 对当前二进制数据进行协议完整性检查时所在的连接对象
  /// @param in 从socket上读取的非连续的二进制数据
  /// @param out 协议完整性检查后的输出对象，可以是已经解码的请求或响应消息对象
  /// @return int -1:0:1 包错误:包缺失:完整包
  int ZeroCopyCheck(const ConnectionPtr& conn, NoncontiguousBuffer& in,
                    std::deque<std::any>& out) override;

  /// @brief 协议解码，零拷贝
  /// @param context 请求编码的上下文
  /// @param in 从ZeroCheck得到的非连续的二进制数据
  /// @param out 协议解码后的消息对象(继承Protocol)
  /// @return bool true:false 协议是否解码成功
  bool ZeroCopyDecode(const ServerContextPtr& context, std::any&& in, ProtocolPtr& out) override;

  /// @brief 协议编码，零拷贝
  /// @param context 请求编码的上下文
  /// @param in 协议编码的响应消息对象(继承Protocol)
  /// @param out 协议编码的可以进行网络发送的二进制数据
  /// @return bool true:false 协议是否编码成功
  bool ZeroCopyEncode(const ServerContextPtr& context, ProtocolPtr& in,
                      NoncontiguousBuffer& out) override;

  /// @brief 构造请求和回复的智能指针
  ProtocolPtr CreateRequestObject() override;
  ProtocolPtr CreateResponseObject() override;
};

}  // namespace trpc
#endif
