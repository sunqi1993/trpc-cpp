#! /bin/bash

bazel build //trpc/... \
    --define trpc_include_codec_thrift=true \
