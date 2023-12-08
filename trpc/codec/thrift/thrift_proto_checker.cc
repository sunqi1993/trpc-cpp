// Copyright (c) 2021, Tencent Inc.
// All rights reserved.

#ifdef BUILD_INCLUDE_CODEC_THRIFT
#include "trpc/codec/thrift/thrift_proto_checker.h"

#include "trpc/codec/thrift/rpc_thrift/rpc_thrift_buffer.h"
#include "trpc/codec/thrift/thrift_protocol.h"
#include "trpc/common/logging/trpc_logging.h"

namespace {

union bytes {
  uint8_t b[4];
  int32_t all;
} theBytes;

uint32_t ReadI32NoSkip(trpc::NoncontiguousBuffer& buff, int32_t& i32) {
  FlattenToSlow(buff, theBytes.b, 4);
  i32 = (int32_t)ntohl(theBytes.all);
  return 4;
}

}  // namespace

namespace trpc {

// Framed transport:
// With framed transport the full request and response (the TMessage and the following struct) are
// first written to a buffer. Then when the struct is complete (transport method flush is hijacked
// for this), the length of the buffer is written to the socket first, followed by the buffered
// bytes. The combination is called a frame.This allows the receiver on the other end to always do
// fixed-length reads.
// A Frame: length prefix(4 byte signed int) +  buffered bytes
// +--------+--------+--------+--------+--------+...+--------+
// | prefix length                     | buffered bytes      |
// +--------+--------+--------+--------+--------+...+--------+
int ThriftZeroCopyCheck(const ConnectionPtr& conn, NoncontiguousBuffer& in,
                        std::deque<std::any>& out) {
  while (true) {
    // 判断收到包的大小是否小于4字节
    uint32_t total_buff_size = in.ByteSize();
    TRPC_FMT_DEBUG("total_buff_size: {}", total_buff_size);

    if (TRPC_UNLIKELY(total_buff_size < ThriftMessageHeader::kPrefixLength)) {
      TRPC_FMT_TRACE("Check less thrift frame transport prefix length");
      break;
    }

    int32_t frame_size = 0;
    ReadI32NoSkip(in, frame_size);

    // 判断包的大小是否正常
    if (TRPC_UNLIKELY(ThriftMessageHeader::kDefaultMaxFrameSize < frame_size)) {
      TRPC_FMT_ERROR("thrift protocol frame size more than 10M error");
      return PacketChecker::PACKET_ERR;
    }

    // 判断收到包的大小是否有完整的请求
    if (TRPC_UNLIKELY(total_buff_size < frame_size + sizeof(frame_size))) {
      TRPC_FMT_TRACE("Check less total_buff_size:{}, frame_size: {}", total_buff_size, frame_size);
      break;
    }
    out.emplace_back(in.Cut(sizeof(frame_size) + frame_size));
  }

  return out.empty() ? PacketChecker::PACKET_LESS : PacketChecker::PACKET_FULL;
}

}  // namespace trpc
#endif