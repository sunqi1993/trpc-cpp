# Description: trpc-cpp.

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "thrift_serialization",
    srcs = ["thrift_serialization.cc"],
    hdrs = ["thrift_serialization.h"],
    defines = [] +
              select({
                  "//trpc:trpc_include_codec_thrift": ["BUILD_INCLUDE_CODEC_THRIFT"],
                  "//conditions:default": [],
              }),
    visibility = [
        "//visibility:public",
    ],
    deps = [
        "//trpc/codec/thrift/rpc_thrift:rpc_thrift_idl",
        "//trpc/codec/trpc",
        "//trpc/common/logging:trpc_logging",
        "//trpc/serialization",
        "//trpc/util:likely",
        "//trpc/util/buffer:zero_copy_stream",
    ],
)

cc_test(
    name = "thrift_serialization_test",
    srcs = ["thrift_serialization_test.cc"],
    defines = [] +
              select({
                  "//trpc:trpc_include_codec_thrift": ["BUILD_INCLUDE_CODEC_THRIFT"],
                  "//conditions:default": [],
              }),
    deps = [
        ":thrift_serialization",
        "//trpc/codec:codec_helper",
        "//trpc/codec/thrift/rpc_thrift:rpc_message_thrift",
        "//trpc/codec/thrift/rpc_thrift:rpc_thrift_enum",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
    ],
)
