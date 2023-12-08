// Copyright (c) 2021, Tencent Inc.
// All rights reserved.
#ifdef BUILD_INCLUDE_CODEC_THRIFT
#pragma once

#include <string>

#include "trpc/codec/thrift/rpc_thrift/rpc_thrift_idl.h"
#include "trpc/util/buffer/buffer.h"

namespace trpc {

class ThriftException : public ThriftIDLMessage {
 public:
  struct ISSET {
    bool message = true;
    bool type = true;
  } __isset;  // NOLINT

  ThriftException() {
    this->elements = ThriftElementsImpl<ThriftException>::GetInstance();
    this->descriptor =
        ThriftDescriptorImpl<ThriftException, codec::ToUType(ThriftDataType::kStruct), void,
                             void>::GetInstance();
  }

  static void StaticElementsImpl(std::list<struct_element>* elements) {
    const auto* st = reinterpret_cast<const ThriftException*>(0);
    const auto* base = reinterpret_cast<const char*>(st);
    using subtype_1 =
        ThriftDescriptorImpl<std::string, codec::ToUType(ThriftDataType::kString), void, void>;
    using subtype_2 =
        ThriftDescriptorImpl<int32_t, codec::ToUType(ThriftDataType::kI32), void, void>;

    elements->push_back({subtype_1::GetInstance(), "message",
                         (const char*)(&st->__isset.message) - base,
                         (const char*)(&st->message) - base, 1});
    elements->push_back({subtype_2::GetInstance(), "type", (const char*)(&st->__isset.type) - base,
                         (const char*)(&st->type) - base, 2});
  }

 public:
  std::string message;
  int32_t type{};
};

}  // namespace trpc
#endif
