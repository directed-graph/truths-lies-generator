workspace(name = "com_github_directed-graph")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

# absl library
http_archive(
    name = "com_google_absl",
    sha256 = "9929f3662141bbb9c6c28accf68dcab34218c5ee2d83e6365d9cb2594b3f3171",
    urls = ["https://github.com/abseil/abseil-cpp/archive/0f3bb466b868b523cf1dc9b2aaaed65c77b28862.zip"],
    strip_prefix = "abseil-cpp-0f3bb466b868b523cf1dc9b2aaaed65c77b28862",
)

# grpc deps
http_archive(
    name = "com_github_grpc_grpc",
    urls = [
        "https://github.com/grpc/grpc/archive/ee5b762f33a42170144834f5ab7efda9d76c480b.tar.gz",
    ],
    strip_prefix = "grpc-ee5b762f33a42170144834f5ab7efda9d76c480b",
)
load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")
grpc_deps()
load("@com_github_grpc_grpc//bazel:grpc_extra_deps.bzl", "grpc_extra_deps")
grpc_extra_deps()
