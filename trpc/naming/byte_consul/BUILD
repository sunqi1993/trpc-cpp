# Description: trpc-cpp.

licenses(["notice"])

package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "selector_byteconsul",
    srcs = ["selector_byteconsul.cc"],
    hdrs = ["selector_byteconsul.h"],
    deps = [
        #"//trpc/common:plugin_class_registry",
        "//trpc/client:make_client_context",
        "//trpc/client:trpc_client",
        "//trpc/common/config:domain_naming_conf",
        "//trpc/common/config:domain_naming_conf_parser",
        "//trpc/common/config:trpc_config",
        "//trpc/util/log:logging",
        "//trpc/naming:load_balance_factory",
        "//trpc/naming:selector_factory",
        "//trpc/naming/common/util:utils_help",
        "//trpc/naming/common/util/loadbalance/polling:polling_load_balance",
        "//trpc/runtime/common:periphery_task_scheduler",
        "//trpc/util/string:string_util",
        "//trpc/client/http:http_service_proxy",
        "//trpc/util:time",
    ],
)

cc_library(
    name = "selector_byteconsul_filter",
    hdrs = ["byteconsul_selector_filter.h"],
    visibility = [
        "//visibility:public",
    ],
    deps = [
        "//trpc/filter",
        "//trpc/naming:selector_workflow",
    ],
)


