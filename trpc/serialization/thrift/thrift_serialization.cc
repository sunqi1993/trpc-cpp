// Copyright (c) 2021, Tencent Inc.
// All rights reserved.
#ifdef BUILD_INCLUDE_CODEC_THRIFT
#include "trpc/serialization/thrift/thrift_serialization.h"

#include <utility>

#include "trpc/codec/trpc/trpc.pb.h"
#include "trpc/common/logging/trpc_logging.h"
#include "trpc/util/buffer/zero_copy_stream.h"
#include "trpc/util/likely.h"

#include "trpc/codec/thrift/rpc_thrift/rpc_thrift_idl.h"

namespace trpc::serialization {

inline SerializationType ThriftSerialization::Type() const { return kThriftType; }

bool ThriftSerialization::Serialize(DataType in_type, void* in, NoncontiguousBuffer* out) {
  TRPC_ASSERT(in_type == kThrift);

  NoncontiguousBufferBuilder builder;
  trpc::ThriftBuffer thirft_buffer(&builder);
  trpc::ThriftIDLMessage* thrift_response = static_cast<trpc::ThriftIDLMessage*>(in);
  if (!thrift_response->descriptor->writer(thrift_response, &thirft_buffer)) {
    TRPC_FMT_ERROR("thrift serialize failed");
    return false;
  }
  *out = builder.DestructiveGet();

  return true;
}

bool ThriftSerialization::Deserialize(NoncontiguousBuffer* in, DataType out_type, void* out) {
  TRPC_ASSERT(out_type == kThrift);

  ThriftBuffer thrift_buffer(in);
  trpc::ThriftIDLMessage* thrift_request = static_cast<trpc::ThriftIDLMessage*>(out);

  return thrift_request->descriptor->reader(&thrift_buffer, thrift_request);
}

}  // namespace trpc::serialization
#endif