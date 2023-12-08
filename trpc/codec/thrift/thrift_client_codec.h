// Copyright (c) 2021, Tencent Inc.
// All rights reserved.

#ifdef BUILD_INCLUDE_CODEC_THRIFT
#pragma once

#include <any>
#include <deque>
#include <string>

#include "trpc/client/client_context.h"
#include "trpc/codec/client_codec.h"
#include "trpc/codec/thrift/rpc_thrift/rpc_thrift_enum.h"
#include "trpc/serialization/serialization_factory.h"
#include "trpc/serialization/thrift/thrift_serialization.h"

namespace trpc {

/**
 * 客户端thrift协议编解码的实现类
 */
class ThriftClientCodec : public trpc::ClientCodec {
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
  bool ZeroCopyDecode(const ClientContextPtr& context, std::any&& in, ProtocolPtr& out) override;

  /// @brief 协议编码，零拷贝
  /// @param context 请求编码的上下文
  /// @param in 协议编码的响应消息对象(继承Protocol)
  /// @param out 协议编码的可以进行网络发送的二进制数据
  /// @return bool true:false 协议是否编码成功
  bool ZeroCopyEncode(const ClientContextPtr& context, const ProtocolPtr& in,
                      NoncontiguousBuffer& out) override;

  /// @brief 构造请求和回复的智能指针
  ProtocolPtr CreateRequestPtr() override;
  ProtocolPtr CreateResponsePtr() override;

  /// @brief 填充请求
  /// @param context 请求编码的上下文
  /// @param in 协议编码的请求消息对象(继承Protocol)
  /// @param body 用户传入的请求IDL对象
  /// @return bool true:false 请求是否填充成功
  bool FillRequest(const ClientContextPtr& context, const ProtocolPtr& in, void* body) override;

  /// @brief 填充回复
  /// @param context 请求编码的上下文
  /// @param in 协议编码的响应消息对象(继承Protocol)
  /// @param body 用户传入的回复IDL对象
  /// @return bool true:false 回复是否填充成功
  bool FillResponse(const ClientContextPtr& context, const ProtocolPtr& in, void* body) override;

 private:
  /// @brief 反序列化
  /// @param context 请求编码的上下文
  /// @param buffer 网络中读取的二进制数据
  /// @param body 二进制数据反序列化得到的IDL对象
  /// @return bool true:false 是否反序列化成功
  bool Deserialize(const ClientContextPtr& context, NoncontiguousBuffer* buffer, void* body);

  /// @brief 序列化
  /// @param context 请求编码的上下文
  /// @param body IDL对象
  /// @param buffer IDL对象序列化得到的二进制数据
  /// @return bool true:false 是否序列化成功
  bool Serialize(const ClientContextPtr& context, void* body, NoncontiguousBuffer* buffer);
};

}  // namespace trpc
#endif
