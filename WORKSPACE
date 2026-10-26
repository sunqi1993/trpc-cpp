workspace(name = "trpc_cpp")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# Hedron's Compile Commands Extractor for Bazel.
# https://github.com/hedronvision/bazel-compile-commands-extractor
http_archive(
    name = "hedron_compile_commands",
    sha256 = "1b08abffbfbe89f6dbee6a5b33753792e8004f6a36f37c0f72115bec86e68724",
    strip_prefix = "bazel-compile-commands-extractor-abb61a688167623088f8768cc9264798df6a9d10",
    url = "https://github.com/hedronvision/bazel-compile-commands-extractor/archive/abb61a688167623088f8768cc9264798df6a9d10.tar.gz",
)

load("@hedron_compile_commands//:workspace_setup.bzl", "hedron_compile_commands_setup")

hedron_compile_commands_setup()

load("@hedron_compile_commands//:workspace_setup_transitive.bzl", "hedron_compile_commands_setup_transitive")

hedron_compile_commands_setup_transitive()

load(
    "@hedron_compile_commands//:workspace_setup_transitive_transitive.bzl",
    "hedron_compile_commands_setup_transitive_transitive",
)

hedron_compile_commands_setup_transitive_transitive()

load(
    "@hedron_compile_commands//:workspace_setup_transitive_transitive_transitive.bzl",
    "hedron_compile_commands_setup_transitive_transitive_transitive",
)

hedron_compile_commands_setup_transitive_transitive_transitive()

load("//trpc:workspace.bzl", "trpc_workspace")

trpc_workspace()
