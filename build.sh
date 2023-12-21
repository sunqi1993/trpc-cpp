#! /bin/bash

bazel build //examples/helloworld/... \
    --define trpc_include_codec_thrift=true \
