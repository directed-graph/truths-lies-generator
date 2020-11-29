
#include <algorithm>
#include <fstream>
#include <random>
#include <set>
#include <string>

#include <google/protobuf/any.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/wrappers.pb.h>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "truths_lies_generator.grpc.pb.h"
#include "truths_lies_generator_lib.h"
#include "truths_lies_generator.pb.h"

ABSL_FLAG(bool, ensure_not_true, true,
          "ensure that generated lies statements are not true; "
          "implementation-wise, this involves brute-force computing "
          "all truths statements once");
ABSL_FLAG(bool, random_order, false,
          "if set, randomize output order; "
          "if not set, output is sorted lexically");
ABSL_FLAG(int, lies, 0, "number of lie statements");
ABSL_FLAG(int, max_retries, 10,
          "number of times to retry if we generate identical statements");
ABSL_FLAG(int, truths, 0, "number of truth statements");
ABSL_FLAG(std::vector<std::string>, input_files, {}, "input data file");

namespace {
using namespace everchanging::truths_lies_generator;
};

class TruthsLiesGeneratorServiceImpl
    : public TruthsLiesGeneratorService::Service {
public:
  ::grpc::Status Generate(
      ::grpc::ServerContext* context,
      const GenerateRequest* request,
      GenerateResponse* response) {
    ::grpc::Status s = checkGenerateRequest(request);
    if (!s.ok()) {
      return s;
    }
    auto statementGenerators = makeStatementGenerators(request);
    absl::StatusOr<StatementCollection> statusOrStatements =
        GenerateTruthsLies(
            statementGenerators, request->truths_count(),
            request->lies_count(), absl::GetFlag(FLAGS_max_retries),
            absl::GetFlag(FLAGS_ensure_not_true));
    // TODO: check status
    for (std::shared_ptr<Statement> statement : statusOrStatements.value()) {
      response->add_statements()->CopyFrom(*statement);
    }
    return ::grpc::Status::OK;
  };

  ::grpc::Status GeneratePartitioned(
      ::grpc::ServerContext* context,
      const GenerateRequest* request,
      GeneratePartitionedResponse* response) {
    ::grpc::Status s = checkGenerateRequest(request);
    if (!s.ok()) {
      return s;
    }
    auto statementGenerators = makeStatementGenerators(request);
    absl::StatusOr<StatementCollection> statusOrStatements =
        GenerateTruthsLies(
            statementGenerators, request->truths_count(),
            request->lies_count(), absl::GetFlag(FLAGS_max_retries),
            absl::GetFlag(FLAGS_ensure_not_true));
    // TODO: check status
    for (std::shared_ptr<Statement> statement : statusOrStatements.value()) {
      if (statement->truth()) {
        response->add_truths()->CopyFrom(*statement);
      } else {
        response->add_lies()->CopyFrom(*statement);
      }
    }
    return ::grpc::Status::OK;
  };

private:
  ::grpc::Status checkGenerateRequest(const GenerateRequest* request) {
    if (request->configs_size() == 0) {
      return ::grpc::Status(::grpc::INVALID_ARGUMENT,
                            "must provide at least one TruthsLiesConfig");
    }
    if (request->truths_count() < 0 || request->lies_count() < 0) {
      return ::grpc::Status(::grpc::INVALID_ARGUMENT,
                            "statement count cannot be negative");
    }
    if (request->truths_count() == 0 && request->lies_count() == 0) {
      return ::grpc::Status(::grpc::INVALID_ARGUMENT,
                            "truths and lies statements cannot both be zero");
    }
    return ::grpc::Status::OK;
  };

  std::vector<std::shared_ptr<StatementGenerator>> makeStatementGenerators(
      const GenerateRequest* request) {
    std::vector<std::shared_ptr<StatementGenerator>> statementGenerators;
    for (auto const& config : request->configs()) {
      statementGenerators.push_back(CreateStatementGenerator(config));
    }
    return statementGenerators;
  };
};

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);

  std::vector<std::shared_ptr<StatementGenerator>> statementGenerators;
  for (std::string& input_file : absl::GetFlag(FLAGS_input_files)) {
    TruthsLiesConfig config;

    std::ifstream stream(input_file);
    if (!stream.is_open()) {
      std::cerr << "ERROR: unable to open file: " << input_file << std::endl;
      return EXIT_FAILURE;
    }
    google::protobuf::io::IstreamInputStream data(&stream);
    google::protobuf::TextFormat::Parser parser;
    parser.Parse(&data, &config);
    stream.close();

    statementGenerators.push_back(
        CreateStatementGenerator(std::move(config)));
  }

  absl::StatusOr<StatementCollection> statusOrStatements = GenerateTruthsLies(
      statementGenerators, absl::GetFlag(FLAGS_truths),
      absl::GetFlag(FLAGS_lies), absl::GetFlag(FLAGS_max_retries),
      absl::GetFlag(FLAGS_ensure_not_true));
  StatementCollection statements = std::move(statusOrStatements.value());

  if (absl::GetFlag(FLAGS_random_order)) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(statements.begin(), statements.end(), gen);
  } else {
    absl::Status status = statements.sort();
  }
  for (auto const& s : statements) {
    std::cout << s->truth() << ": " << s->statement() << std::endl;
  }

  return EXIT_SUCCESS;
}

