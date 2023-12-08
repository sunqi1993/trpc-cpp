load("@trpc_cpp//trpc:trpc.bzl", "trpc_thrift_library")
# package(
#     default_visibility = ["//trpc:internal"],
# )
package(default_visibility = ["//visibility:public"])
trpc_thrift_library(
    name = "hello_thrift",
    srcs = ["hello.thrift"],
    visibility = [
        "//visibility:public",
    ],

)

cc_library(
    name = "thrift_protocol",
    srcs = ["thrift_protocol.cc"],
    hdrs = ["thrift_protocol.h"],
    defines = [] +
              select({
                  "//trpc:trpc_include_codec_thrift": ["BUILD_INCLUDE_CODEC_THRIFT"],
                  "//conditions:default": [],
              }),
    visibility = [
        "//visibility:public",
    ],
    deps = [
        "//trpc/codec:protocol",
        "//trpc/codec/thrift/rpc_thrift:rpc_thrift_buffer",
        "//trpc/codec/thrift/rpc_thrift:rpc_thrift_enum",
        "//trpc/common:buffer",
        "//trpc/common/logging:trpc_logging",
    ],
)

cc_library(
    name = "thrift_server_codec",
    srcs = ["thrift_server_codec.cc"],
    hdrs = ["thrift_server_codec.h"],
    defines = [] +
              select({
                  "//trpc:trpc_include_codec_thrift": ["BUILD_INCLUDE_CODEC_THRIFT"],
                  "//conditions:default": [],
              }),
    visibility = [
        "//visibility:public",
    ],
    deps = [
        ":thrift_proto_checker",
        ":thrift_protocol",
        "//trpc/codec:server_codec",
        "//trpc/codec/thrift/rpc_thrift:rpc_message_thrift",
        "//trpc/codec/thrift/rpc_thrift:rpc_thrift_enum",
        "//trpc/codec/thrift/rpc_thrift:rpc_thrift_idl",
        "//trpc/common/logging:trpc_logging",
        "//trpc/serialization/thrift:thrift_serialization",
        "//trpc/server:server_context",
    ],
)

cc_library(
    name = "thrift_client_codec",
    srcs = ["thrift_client_codec.cc"],
    hdrs = ["thrift_client_codec.h"],
    defines = [] +
              select({
                  "//trpc:trpc_include_codec_thrift": ["BUILD_INCLUDE_CODEC_THRIFT"],
                  "//conditions:default": [],
              }),
    visibility = [
        "//visibility:public",
    ],
    deps = [
        ":thrift_proto_checker",
        ":thrift_protocol",
        "//trpc/codec:client_codec",
        "//trpc/codec/thrift/rpc_thrift:rpc_message_thrift",
        "//trpc/codec/thrift/rpc_thrift:rpc_thrift_enum",
        "//trpc/codec/thrift/rpc_thrift:rpc_thrift_idl",
        "//trpc/common/logging:trpc_logging",
        "//trpc/serialization/thrift:thrift_serialization",
        "//trpc/serialization:serialization_factory",
        "//trpc/client:client_context",
    ],
)

cc_library(
    name = "thrift_proto_checker",
    srcs = ["thrift_proto_checker.cc"],
    hdrs = ["thrift_proto_checker.h"],
    defines = [] +
              select({
                  "//trpc:trpc_include_codec_thrift": ["BUILD_INCLUDE_CODEC_THRIFT"],
                  "//conditions:default": [],
              }),
    visibility = [
        "//visibility:public",
    ],
    deps = [
        ":thrift_protocol",
        "//trpc/codec/thrift/rpc_thrift:rpc_thrift_buffer",
        "//trpc/common:buffer",
        "//trpc/common/logging:trpc_logging",
        "//trpc/runtime/iomodel/reactor/default:tcp_connection",
    ],
)

cc_test(
    name = "thrift_proto_checker_test",
    srcs = ["thrift_proto_checker_test.cc"],
    defines = [] +
              select({
                  "//trpc:trpc_include_codec_thrift": ["BUILD_INCLUDE_CODEC_THRIFT"],
                  "//conditions:default": [],
              }),
    deps = [
        ":thrift_proto_checker",
        "//trpc/codec/thrift/rpc_thrift:rpc_thrift_buffer",
        "//trpc/runtime/iomodel/reactor/default:tcp_connection",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "thrift_protocol_test",
    srcs = ["thrift_protocol_test.cc"],
    defines = [] +
              select({
                  "//trpc:trpc_include_codec_thrift": ["BUILD_INCLUDE_CODEC_THRIFT"],
                  "//conditions:default": [],
              }),
    deps = [
        "//trpc/codec:codec_helper",
        "//trpc/codec/thrift:thrift_protocol",
        "//trpc/codec/thrift/rpc_thrift:rpc_thrift_buffer",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
    ],
)


cc_test(
    name = "thrift_server_codec_test",
    srcs = ["thrift_server_codec_test.cc"],
    defines = [] +
              select({
                  "//trpc:trpc_include_codec_thrift": ["BUILD_INCLUDE_CODEC_THRIFT"],
                  "//conditions:default": [],
              }),
    deps = [
        "//trpc/codec:codec_helper",
        "//trpc/codec/thrift:thrift_server_codec",
        "//trpc/codec/thrift:thrift_protocol",
        "//trpc/codec/thrift/rpc_thrift:rpc_message_thrift",
        "//trpc/server:server_context",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "thrift_client_codec_test",
    srcs = ["thrift_client_codec_test.cc"],
    defines = [] +
              select({
                  "//trpc:trpc_include_codec_thrift": ["BUILD_INCLUDE_CODEC_THRIFT"],
                  "//conditions:default": [],
              }),
    deps = [
        ":hello_thrift",
        "//trpc/codec:codec_helper",
        "//trpc/codec/thrift:thrift_client_codec",
        "//trpc/codec/thrift:thrift_protocol",
        "//trpc/codec/thrift/rpc_thrift:rpc_message_thrift",
        "//trpc/serialization:trpc_serialization",
        "//trpc/client:client_context",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
    ],
)