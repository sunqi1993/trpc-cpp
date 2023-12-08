package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "trpc_thrift_tool",
    srcs = glob(["*.cc","*.h"]),
    visibility = ["//visibility:public"],
    copts =["-std=c++17"],
    linkopts = ['-lstdc++fs'],
    defines =["BUILD_INCLUDE_CODEC_THRIFT"],
    deps = [
        "//trpc/codec/thrift/rpc_thrift:rpc_thrift_enum"
    ],
)