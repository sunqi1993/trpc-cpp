// Copyright (c) 2021, Tencent Inc.
// All rights reserved.
#ifdef BUILD_INCLUDE_CODEC_THRIFT
#pragma once

namespace trpc {

constexpr int kThriftStructFieldRequired = 0;
constexpr int kThriftStructFieldOptional = 1;
constexpr int kThriftStructFieldDefault = 2;

enum ThriftMessageType : int8_t { kCall = 1, kReply = 2, kException = 3, kOneway = 4 };

enum ThriftDataType : int8_t {
  kStop = 0,
  kVoid = 1,
  kBool = 2,
  kByte = 3,
  kI08 = 3,
  kI16 = 6,
  kI32 = 8,
  kU64 = 9,
  kI64 = 10,
  kDouble = 4,
  kString = 11,
  kUtf7 = 11,
  kStruct = 12,
  kMap = 13,
  kSet = 14,
  kList = 15,
  kUtf8 = 16,
  kUtf16 = 17
};

enum ThriftExceptionType : int8_t{
  kUnknown = 0,
  kUnknownMethod = 1,
  kInvalidMessageType = 2,
  kWrongMethodName = 3,
  kBadSequenceId = 4,
  kMissingResult = 5,
  kInternalError = 6,
  kProtocolError = 7,
  kInvalidTransform = 8,
  kInvalidProtocol = 9,
  kUnsupportedClientType = 10
};

}  // namespace trpc
#endif
