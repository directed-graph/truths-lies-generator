load("@rules_cc//cc:defs.bzl", "cc_proto_library")
load("@rules_proto//proto:defs.bzl", "proto_library")
load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")

proto_library(
  name = "truths_lies_generator_proto",
  srcs = ["truths_lies_generator.proto"],
  deps = [
    "@com_google_protobuf//:any_proto",
  ],
)

cc_proto_library(
  name = "truths_lies_generator_cc_proto",
  deps = [":truths_lies_generator_proto"],
)

cc_grpc_library(
  name = "truths_lies_generator_cc_grpc_proto",
  srcs = [":truths_lies_generator_proto"],
  grpc_only = True,
  deps = [":truths_lies_generator_cc_proto"],
)

cc_library(
  name = "truths_lies_generator_lib",
  srcs = ["truths_lies_generator_lib.cc"],
  hdrs = ["truths_lies_generator_lib.h"],
  deps = [
    ":truths_lies_generator_cc_proto",
    "@com_google_absl//absl/status:status",
    "@com_google_absl//absl/status:statusor",
    "@com_google_absl//absl/strings:strings",
  ],
)

cc_binary(
  name = "generator",
  srcs = ["truths_lies_generator_main.cc"],
  deps = [
    ":truths_lies_generator_lib",
    ":truths_lies_generator_cc_grpc_proto",
    "@com_google_absl//absl/flags:flag",
    "@com_google_absl//absl/flags:parse",
    "@com_google_absl//absl/status:status",
    "@com_google_absl//absl/status:statusor",
  ],
)
