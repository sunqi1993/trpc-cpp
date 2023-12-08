// Copyright (c) 2021, Tencent Inc.
// All rights reserved.
#ifdef BUILD_INCLUDE_CODEC_THRIFT

#include <any>
#include <list>
#include <memory>

#include "gtest/gtest.h"

#include "trpc/codec/thrift/rpc_thrift/rpc_thrift_buffer.h"
#include "trpc/codec/thrift/thrift_proto_checker.h"
#include "trpc/runtime/iomodel/reactor/default/tcp_connection.h"

namespace trpc::testing {
class ThriftZeroCopyCheckTest : public ::testing::Test {
 public:
  ThriftZeroCopyCheckTest() : thrift_buffer_(&builder_) {}

 protected:
  void SetUp() override {
    in_.Clear();
    out_.clear();
    Connection::Options options;
    options.socket = Socket::CreateTcpSocket(false);
    options.type = ConnectionType::TCP_LONG;
    DefaultConnection::Options default_options;
    default_options.options = options;
    conn_ = new TcpConnection(default_options);
  }
  void TearDown() override { delete conn_; }

  NoncontiguousBuffer in_;
  std::deque<std::any> out_;
  ConnectionPtr conn_;
  NoncontiguousBufferBuilder builder_;
  ThriftBuffer thrift_buffer_;
};

TEST_F(ThriftZeroCopyCheckTest, EmptyPacket) {
  in_ = builder_.DestructiveGet();
  EXPECT_EQ(PacketChecker::PACKET_LESS, ThriftZeroCopyCheck(conn_, in_, out_));
  EXPECT_EQ(0, out_.size());
  EXPECT_EQ(0, in_.ByteSize());
}

TEST_F(ThriftZeroCopyCheckTest, FramePrefixSizeNotFull) {
  constexpr int8_t kPrefixSize = 1;
  thrift_buffer_.WriteI08(kPrefixSize);
  in_ = builder_.DestructiveGet();
  EXPECT_EQ(PacketChecker::PACKET_LESS, ThriftZeroCopyCheck(conn_, in_, out_));
  EXPECT_EQ(0, out_.size());
  EXPECT_EQ(sizeof(kPrefixSize), in_.ByteSize());
}

TEST_F(ThriftZeroCopyCheckTest, OverflowPacket) {
  constexpr int32_t kPrefixSize = 256 * 1024 * 1024 + 1;

  thrift_buffer_.WriteI32(kPrefixSize);
  in_ = builder_.DestructiveGet();

  EXPECT_EQ(PacketChecker::PACKET_ERR, ThriftZeroCopyCheck(conn_, in_, out_));
  EXPECT_EQ(sizeof(kPrefixSize), in_.ByteSize());
  EXPECT_EQ(0, out_.size());
}

TEST_F(ThriftZeroCopyCheckTest, LessPacket) {
  constexpr int32_t kPrefixSize = 4;
  int8_t request_data = 1;
  thrift_buffer_.WriteI32(kPrefixSize);
  thrift_buffer_.WriteI08(kPrefixSize);
  in_ = builder_.DestructiveGet();
  EXPECT_EQ(PacketChecker::PACKET_LESS, ThriftZeroCopyCheck(conn_, in_, out_));
  EXPECT_EQ(sizeof(kPrefixSize) + sizeof(request_data), in_.ByteSize());
  EXPECT_EQ(0, out_.size());
}

TEST_F(ThriftZeroCopyCheckTest, FullPacket) {
  constexpr int32_t kPrefixSize = 4;
  int32_t request_data = 1;
  thrift_buffer_.WriteI32(kPrefixSize);
  thrift_buffer_.WriteI32(request_data);

  in_ = builder_.DestructiveGet();
  EXPECT_EQ(PacketChecker::PACKET_FULL, ThriftZeroCopyCheck(conn_, in_, out_));
  EXPECT_EQ(0, in_.ByteSize());
  EXPECT_EQ(1, out_.size());
}

}  // namespace trpc::testing

#endif
