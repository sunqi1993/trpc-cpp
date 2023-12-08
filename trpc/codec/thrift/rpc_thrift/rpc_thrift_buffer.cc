// Copyright (c) 2021, Tencent Inc.
// All rights reserved.
#ifdef BUILD_INCLUDE_CODEC_THRIFT
#include "rpc_thrift_buffer.h"

#include "trpc/common/logging/trpc_logging.h"

namespace trpc {

uint32_t ThriftBuffer::ReadMessageBegin(std::string& method_name, int8_t& message_type,
                                        int32_t& seqid, bool& is_strict) {
  uint32_t result = 0;
  int32_t header;
  result += ReadI32(header);

  if (header < 0) {
    int32_t version = header & kThriftVersionMask;
    if (version != kThriftVersion_1) {
      TRPC_FMT_ERROR("BAD_VERSION expect: {}, act: {}", kThriftVersion_1, version);
    }
    result += ReadString(method_name);
    result += ReadI32(seqid);
    message_type = header & 0xFF;
    is_strict = true;

  } else {
    result += ReadStringBody(method_name, header);
    result += ReadI08(message_type);
    result += ReadI32(seqid);
    is_strict = false;
  }
  return result;
}

uint32_t ThriftBuffer::WriteMessageBegin(const std::string& method_name, int8_t message_type,
                                         int32_t seqid, uint32_t is_strict) {
  uint32_t wsize = 0;

  if (is_strict) {
    int32_t version = (kThriftVersion_1) | ((int32_t)message_type);
    wsize += WriteI32(version);
    wsize += WriteString(method_name);
    wsize += WriteI32(seqid);
  } else {
    wsize += WriteString(method_name);
    wsize += WriteI08(message_type);
    wsize += WriteI32(seqid);
  }

  return wsize;
}

uint32_t ThriftBuffer::Skip(int8_t field_type) {
  uint32_t size = 0;
  switch (field_type) {
    case codec::ToUType(ThriftDataType::kI08):
    case codec::ToUType(ThriftDataType::kBool):
      buffer->Skip(1);
      return 1;

    case codec::ToUType(ThriftDataType::kI16):
      buffer->Skip(2);
      return 2;

    case codec::ToUType(ThriftDataType::kI32):
      buffer->Skip(4);
      return 4;

    case codec::ToUType(ThriftDataType::kI64):
    case codec::ToUType(ThriftDataType::kU64):
    case codec::ToUType(ThriftDataType::kDouble):
      buffer->Skip(8);
      return 8;

    case codec::ToUType(ThriftDataType::kString):
    case codec::ToUType(ThriftDataType::kUtf8):
    case codec::ToUType(ThriftDataType::kUtf16): {
      int32_t slen;
      size += ReadI32(slen);

      buffer->Skip(slen);
      return size + (uint32_t)slen;
    }
    case codec::ToUType(ThriftDataType::kStruct): {
      int8_t field_type;
      int16_t field_id;

      while (true) {
        size += ReadFieldBegin(field_type, field_id);
        if (field_type == codec::ToUType(ThriftDataType::kStop)) break;
        size += Skip(field_type);
      }

      break;
    }
    case codec::ToUType(ThriftDataType::kMap): {
      int8_t key_type;
      int8_t val_type;
      int32_t count;

      size += ReadI08(key_type);
      size += ReadI08(val_type);
      size += ReadI32(count);

      for (int32_t i = 0; i < count; i++) {
        size += Skip(key_type);
        size += Skip(val_type);
      }

      break;
    }
    case codec::ToUType(ThriftDataType::kList):
    case codec::ToUType(ThriftDataType::kSet): {
      int8_t val_type;
      int32_t count;

      size += ReadI08(val_type);
      size += ReadI32(count);

      for (int32_t i = 0; i < count; i++) {
        size += Skip(val_type);
      }

      break;
    }
    default:
      break;
  }

  return size;
}

}  // namespace trpc
#endif
