
FROM implementing/grpc:latest

ENV BAZEL_CXXOPTS="-std=c++20"

WORKDIR /github/directed-graph/truths-lies-generator

COPY ./WORKSPACE .
COPY ./BUILD .
COPY ./truths_lies_generator_lib.cc .
COPY ./truths_lies_generator_lib.h .
COPY ./truths_lies_generator_main.cc .
COPY ./truths_lies_generator.proto .

RUN bazel build :generator

ENTRYPOINT ["bazel-bin/generator", "--server"]

