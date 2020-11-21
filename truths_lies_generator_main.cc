
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
#include "truths_lies_config.pb.h"
#include "truths_lies_generator_lib.h"

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

struct SharedStatementCmp {
  bool operator()(const std::shared_ptr<Statement>& lhs,
                  const std::shared_ptr<Statement>& rhs) const {
    return lhs->statement() < rhs->statement();
  }
};

std::shared_ptr<StatementCollection> GenerateAllTruths(
    std::shared_ptr<const StatementGenerator> g) {
  std::shared_ptr<StatementCollection> collection =
      std::make_shared<StatementCollection>();
  for (size_t i = 0; i < g->size(); ++i) {
    std::shared_ptr<Statement> s = std::make_shared<Statement>();
    s->set_statement(g->truth(i));
    s->set_truth(true);
    absl::Status status = collection->insert(s);
  }
  return collection;
}

absl::StatusOr<StatementCollection> GenerateTruthsLies(
    const std::vector<std::shared_ptr<StatementGenerator>>&
        statementGenerators,
    int truths, int lies, int max_retries = 10, bool ensure_not_true = true) {
  std::vector<int> generatorWeights;

  for (auto const& generator : statementGenerators) {
    generatorWeights.push_back(generator->size());
  }
  std::random_device rd;
  std::mt19937 gen(rd());
  std::discrete_distribution<> dd(generatorWeights.begin(),
                                 generatorWeights.end());
  std::uniform_real_distribution<> ud(0.0, 1.0);

  std::vector<std::shared_ptr<StatementCollection>> truthsPerGenerator;

  if (ensure_not_true) {
    for (auto const& generator : statementGenerators) {
      truthsPerGenerator.push_back(GenerateAllTruths(generator));
    }
  }

  StatementCollection statements;

  for (int i = 0; i < truths; ++i) {
    std::shared_ptr<Statement> s;
    int retries_left = max_retries;
    do {
      int generatorIndex = dd(gen);
      // generatorWeights represent the size of each valueMap vector
      int valueMapIndex = static_cast<int>(
          generatorWeights[generatorIndex] * ud(gen));
      // if ensure_not_true, we already have the statements generatored
      if (ensure_not_true) {
        s = (*truthsPerGenerator[generatorIndex])[valueMapIndex];
      } else {
        s = std::make_shared<Statement>();
        s->set_statement(
            statementGenerators[generatorIndex]->truth(valueMapIndex));
        s->set_truth(true);
      }
    } while (statements.count(s) > 0 && retries_left-- > 0);
    if (retries_left <= 0) {
      return absl::AlreadyExistsError(
          "generated too many duplicate statements");
    }
    absl::Status status = statements.insert(s);
    if (!status.ok()) {
      return status;
    }
  }

  for (int i = 0; i < lies; ++i) {
    int generatorIndex;
    std::shared_ptr<Statement> s = std::make_shared<Statement>();
    s->set_truth(false);
    int retries_left = max_retries;
    do {
      generatorIndex = dd(gen);
      // generatorWeights represent the size of each valueMap vector
      int valueMapIndex = static_cast<int>(
          generatorWeights[generatorIndex] * ud(gen));
      s->set_statement(statementGenerators[generatorIndex]->lie(valueMapIndex));
    } while (
        truthsPerGenerator[generatorIndex]->count(s) > 0 &&
        statements.count(s) > 0 && retries_left-- > 0);
    if (retries_left <= 0) {
      return absl::AlreadyExistsError(
          "generated too many duplicate statements");
    }
    absl::Status status = statements.insert(s);
    if (!status.ok()) {
      return status;
    }
  }
  return statements;
}

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
  for (auto s : statements) {
    std::cout << s->truth() << ": " << s->statement() << std::endl;
  }

  //std::string output;
  //functionMap[config.lies_generator()](config.mutable_arguments(0));
  //google::protobuf::TextFormat::PrintToString(config, &output);
  //std::cout
  //    << output
  //    << ApplyValues(config.template_string(), config.arguments(0))
  //    << std::endl;
  return EXIT_SUCCESS;
}

