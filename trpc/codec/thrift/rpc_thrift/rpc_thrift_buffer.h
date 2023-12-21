// Copyright (c) 2021, Tencent Inc.
// All rights reserved.
#ifdef BUILD_INCLUDE_CODEC_THRIFT
#pragma once

#include <arpa/inet.h>
#ifndef TRPC_HTONLL
#define TRPC_HTONLL(x) ((((uint64_t)htonl((x)&0xFFFFFFFF)) << 32) + htonl((x) >> 32))
#define TRPC_NTOHLL(x) ((((uint64_t)ntohl((x)&0xFFFFFFFF)) << 32) + ntohl((x) >> 32))
#endif

#include <cstdint>

#include <stdint.h>
#include <string.h>

#include <string>

#include "rpc_thrift_enum.h"
#include "trpc/codec/codec_helper.h"
#include "trpc/codec/thrift/rpc_thrift/rpc_thrift_enum.h"
#include "trpc/util/buffer/buffer.h"

namespace trpc {

constexpr int32_t kThriftVersionMask = ((int32_t)0xffff0000);
constexpr int32_t kThriftVersion_1 = ((int32_t)0x80010000);
constexpr int kThriftGetFrameSize = 1;
constexpr int kThriftGetData = 2;
constexpr int kThriftParseEnd = 3;

class ThriftBuffer {
 public:
  ThriftBuffer() {}
  explicit ThriftBuffer(NoncontiguousBuffer* buffer) : buffer(buffer), builder(nullptr) {}
  explicit ThriftBuffer(NoncontiguousBufferBuilder* builder) : buffer(nullptr), builder(builder) {}
  ThriftBuffer(NoncontiguousBuffer* buffer, NoncontiguousBufferBuilder* builder)
      : buffer(buffer), builder(builder) {}

  ThriftBuffer(const ThriftBuffer&) = delete;
  ThriftBuffer& operator=(const ThriftBuffer&) = delete;
  ThriftBuffer(ThriftBuffer&& move) = delete;
  ThriftBuffer& operator=(ThriftBuffer&& move) = delete;

  virtual uint32_t ReadMessageBegin(std::string& method_name, int8_t& message_type, int32_t& seqid,
                            bool& is_strict);
  virtual uint32_t ReadFieldBegin(int8_t& field_type, int16_t& field_id);
  virtual uint32_t ReadI08(int8_t& val, bool need_calculate_crc = true);
  virtual uint32_t ReadI16(int16_t& val, bool need_calculate_crc = true);
  virtual uint32_t ReadI32(int32_t& val, bool need_calculate_crc = true);
  virtual uint32_t ReadI64(int64_t& val, bool need_calculate_crc = true);
  virtual uint32_t ReadU64(uint64_t& val, bool need_calculate_crc = true);
  virtual uint32_t ReadString(std::string& str, bool need_calculate_crc = true);
  virtual uint32_t ReadStringBody(std::string& str, int32_t slen, bool need_calculate_crc = true);

  uint32_t Skip(int8_t field_type);

  virtual uint32_t WriteMessageBegin(const std::string& method_name, int8_t message_type, int32_t seqid,
                             uint32_t is_strict);
  uint32_t WriteFieldBegin(int8_t field_type, int16_t field_id);
  uint32_t WriteFieldStop();
  virtual uint32_t WriteI08(int8_t val, bool need_calculate_crc = true);
  virtual uint32_t WriteI16(int16_t val, bool need_calculate_crc = true);
  virtual uint32_t WriteI32(int32_t val, bool need_calculate_crc = true);
  virtual uint32_t WriteI64(int64_t val, bool need_calculate_crc = true);
  virtual uint32_t WriteU64(uint64_t val, bool need_calculate_crc = true);
  virtual uint32_t WriteString(const std::string& str, bool need_calculate_crc = true);

  void SetBuilder(NoncontiguousBufferBuilder* b) { this->builder = b; }
  void SetBuffer(NoncontiguousBuffer* buf) { this->buffer = buf; }

 protected:
  uint32_t Read(void* buf, size_t buflen, bool is_skip = true) const;
  uint32_t Write(const void* buf, size_t buflen) const;

 public:
  NoncontiguousBuffer* buffer;
  NoncontiguousBufferBuilder* builder;
  size_t readbuf_size = 0;
  size_t framesize_read_byte = 0;
  int32_t framesize = 0;
  int status = kThriftGetFrameSize;
};

inline uint32_t ThriftBuffer::Read(void* buf, size_t buflen, bool is_skip) const {
  FlattenToSlow(*buffer, buf, buflen);
  if (is_skip) {
    buffer->Skip(buflen);
  }
  return buflen;
}

inline uint32_t ThriftBuffer::Write(const void* buf, size_t buflen) const {
  builder->Append(buf, buflen);
  return buflen;
}

inline uint32_t ThriftBuffer::ReadI08(int8_t& val, bool need_calculate_crc) {
  Read(reinterpret_cast<char*>(&val), 1);
  return 1;
}

inline uint32_t ThriftBuffer::ReadI16(int16_t& val, bool need_calculate_crc) {
  Read(reinterpret_cast<char*>(&val), 2);
  val = ntohs(val);
  return 2;
}

inline uint32_t ThriftBuffer::ReadI32(int32_t& val, bool need_calculate_crc) {
  Read(reinterpret_cast<char*>(&val), 4);
  val = (int32_t)ntohl(val);
  return 4;
}

inline uint32_t ThriftBuffer::ReadI64(int64_t& val, bool need_calculate_crc) {
  Read(reinterpret_cast<char*>(&val), 8);
  val = TRPC_NTOHLL(val);
  return 8;
}

inline uint32_t ThriftBuffer::ReadU64(uint64_t& val, bool need_calculate_crc) {
  Read(reinterpret_cast<char*>(&val), 8);
  val = TRPC_NTOHLL(val);
  return 8;
}

inline uint32_t ThriftBuffer::ReadFieldBegin(int8_t& field_type, int16_t& field_id) {
  uint32_t result = ReadI08(field_type);
  if (field_type == codec::ToUType(ThriftDataType::kStop)) {
    field_id = 0;
  } else {
    result += ReadI16(field_id);
  }
  return result;
}

inline uint32_t ThriftBuffer::ReadString(std::string& str, bool need_calculate_crc) {
  int32_t slen;
  uint32_t result = ReadI32(slen);
  result += ReadStringBody(str, slen);
  return result;
}

inline uint32_t ThriftBuffer::ReadStringBody(std::string& str, int32_t slen, bool need_calculate_crc) {
  if (slen < 0) {
    TRPC_FMT_ERROR("slen : {} < 0", slen);
    return 0;
  }

  if (slen == 0) {
    str.clear();
    return 0;
  }
  str.resize(slen);

  Read(const_cast<char*>(str.c_str()), slen);
  return (uint32_t)slen;
}

inline uint32_t ThriftBuffer::WriteFieldStop() {
  return WriteI08(codec::ToUType(ThriftDataType::kStop));
}

inline uint32_t ThriftBuffer::WriteI08(int8_t val, bool need_calculate_crc) {
  Write(reinterpret_cast<char*>(&val), 1);
  return 1;
}

inline uint32_t ThriftBuffer::WriteI16(int16_t val, bool need_calculate_crc) {
  int16_t x = htons(val);
  Write(reinterpret_cast<char*>(&x), 2);
  return 2;
}

inline uint32_t ThriftBuffer::WriteI32(int32_t val, bool need_calculate_crc) {
  int32_t x = htonl(val);
  Write(reinterpret_cast<char*>(&x), 4);
  return 4;
}

inline uint32_t ThriftBuffer::WriteI64(int64_t val, bool need_calculate_crc) {
  int64_t x = TRPC_HTONLL(val);
  Write(reinterpret_cast<char*>(&x), 8);
  return 8;
}

inline uint32_t ThriftBuffer::WriteU64(uint64_t val, bool need_calculate_crc) {
  uint64_t x = TRPC_HTONLL(val);
  Write(reinterpret_cast<char*>(&x), 8);

  return 8;
}

inline uint32_t ThriftBuffer::WriteFieldBegin(int8_t field_type, int16_t field_id) {
  uint32_t result = WriteI08(field_type);
  result += WriteI16(field_id);
  return result;
}

inline uint32_t ThriftBuffer::WriteString(const std::string& str, bool need_calculate_crc) {
  uint32_t slen = str.size();
  uint32_t result = WriteI32((int32_t)slen);
  Write(str.c_str(), str.size());

  return result + slen;
}

}  // namespace trpc
#endif