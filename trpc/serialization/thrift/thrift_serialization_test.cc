// Copyright (c) 2021, Tencent Inc.
// All rights reserved.
#ifdef BUILD_INCLUDE_CODEC_THRIFT

#include "trpc/serialization/thrift/thrift_serialization.h"

#include <memory>

#include "gtest/gtest.h"

#include "trpc/codec/codec_helper.h"
#include "trpc/codec/thrift/rpc_thrift/rpc_message_thrift.h"
#include "trpc/codec/thrift/rpc_thrift/rpc_thrift_enum.h"
namespace trpc::testing {

using namespace trpc::serialization;

TEST(ThriftSerializationTest, SerializationTest) {
  std::unique_ptr<ThriftSerialization> thrift_serialization =
      std::make_unique<ThriftSerialization>();
  EXPECT_TRUE(thrift_serialization->Type() == kThriftType);

  trpc::ThriftException request;
  request.message = "thrift serialization test";
  request.type = trpc::codec::ToUType(trpc::ThriftExceptionType::kProtocolError);
  NoncontiguousBuffer binary_data;
  EXPECT_TRUE(thrift_serialization->Serialize(kThrift, static_cast<void*>(&request), &binary_data));

  trpc::ThriftException expect_request;
  EXPECT_TRUE(thrift_serialization->Deserialize(&binary_data, kThrift,
                                                static_cast<void*>(&expect_request)));

  EXPECT_EQ(expect_request.message, request.message);
  EXPECT_EQ(expect_request.type, request.type);
}

}  // namespace trpc::testing

#endif