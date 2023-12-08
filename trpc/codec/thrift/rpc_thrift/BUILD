package(default_visibility = ["//visibility:public"])

cc_library(
    name = "rpc_thrift_enum",
    hdrs = ["rpc_thrift_enum.h"],
    defines = [] +
              select({
                  "//trpc:trpc_include_codec_thrift": ["BUILD_INCLUDE_CODEC_THRIFT"],
                  "//conditions:default": [],
              }),
    visibility = [
        "//visibility:public",
    ],
)

cc_library(
    name = "rpc_thrift_buffer",
    srcs = [
        "rpc_thrift_buffer.cc",
    ],
    hdrs = [
        "rpc_thrift_buffer.h",
        "rpc_thrift_idl.h",
    ],
    defines = [] +
              select({
                  "//trpc:trpc_include_codec_thrift": ["BUILD_INCLUDE_CODEC_THRIFT"],
                  "//conditions:default": [],
              }),
    visibility = [
        "//visibility:public",
    ],
    deps = [
        ":rpc_thrift_enum",
        "//trpc/codec:codec_helper",
        "//trpc/common/logging:trpc_logging",
        "//trpc/util/buffer",
    ],
)

cc_library(
    name = "rpc_thrift_idl",
    hdrs = ["rpc_thrift_idl.h"],
    defines = [] +
              select({
                  "//trpc:trpc_include_codec_thrift": ["BUILD_INCLUDE_CODEC_THRIFT"],
                  "//conditions:default": [],
              }),
    visibility = [
        "//visibility:public",
    ],
    deps = [
        ":rpc_thrift_buffer",
        ":rpc_thrift_enum",
    ],
)

cc_library(
    name = "rpc_message_thrift",
    hdrs = ["rpc_message_thrift.h"],
    defines = [] +
              select({
                  "//trpc:trpc_include_codec_thrift": ["BUILD_INCLUDE_CODEC_THRIFT"],
                  "//conditions:default": [],
              }),
    visibility = [
        "//visibility:public",
    ],
    deps = [
        ":rpc_thrift_idl",
        "//trpc/util/buffer",
    ],
)

cc_test(
    name = "rpc_thrift_buffer_test",
    srcs = ["rpc_thrift_buffer_test.cc"],
    defines = [] +
              select({
                  "//trpc:trpc_include_codec_thrift": ["BUILD_INCLUDE_CODEC_THRIFT"],
                  "//conditions:default": [],
              }),
    deps = [
        "//trpc/codec/thrift/rpc_thrift:rpc_message_thrift",
        "//trpc/codec/thrift/rpc_thrift:rpc_thrift_buffer",
        "//trpc/serialization/thrift:thrift_serialization",
        "//trpc/codec:codec_helper",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
    ],
)
