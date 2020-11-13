
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

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);

  std::vector<int> generatorWeights;
  std::vector<std::unique_ptr<StatementGenerator>> statementGenerators;

  // each element in vector is a set corresponding to a generator; each element
  // in the set corresponds to a truths; the purpose is for fast searching
  std::vector<std::set<std::shared_ptr<Statement>,
                       SharedStatementCmp>> truthsSetPerGenerator;
  // each element in vector is a vector corresponding to a generator; each
  // element in _that_ vector corresponds to a truths corresponding to the
  // index number of the argument; the purpose is indexed access to keep order
  std::vector<std::vector<std::shared_ptr<Statement>>> truthsVectorPerGenerator;

  std::vector<std::shared_ptr<Statement>> statementsVector;
  std::set<std::shared_ptr<Statement>, SharedStatementCmp> statementsSet;

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

    generatorWeights.push_back(config.arguments().size());
    statementGenerators.push_back(
        CreateStatementGenerator(std::move(config)));

    std::set<std::shared_ptr<Statement>, SharedStatementCmp> truthsSet;
    std::vector<std::shared_ptr<Statement>> truthsVector;
    if (absl::GetFlag(FLAGS_ensure_not_true)) {
      for (int i = 0; i < generatorWeights.back(); ++i) {
        std::shared_ptr<Statement> s = std::make_shared<Statement>();
        s->set_statement(statementGenerators.back()->truth(i));
        s->set_truth(true);
        truthsSet.insert(s);
        truthsVector.push_back(s);
      }
    }
    truthsSetPerGenerator.push_back(std::move(truthsSet));
    truthsVectorPerGenerator.push_back(std::move(truthsVector));
  }

  std::random_device rd;
  std::mt19937 gen(rd());
  std::discrete_distribution<> dd(generatorWeights.begin(),
                                 generatorWeights.end());
  std::uniform_real_distribution<> ud(0.0, 1.0);

  for (int i = 0; i < absl::GetFlag(FLAGS_truths); ++i) {
    std::shared_ptr<Statement> s;
    int retries_left = absl::GetFlag(FLAGS_max_retries);
    do {
      int generatorIndex = dd(gen);
      // generatorWeights represent the size of each valueMap vector
      int valueMapIndex = static_cast<int>(
          generatorWeights[generatorIndex] * ud(gen));
      // if ensure_not_true, we already have the statements generatored
      if (absl::GetFlag(FLAGS_ensure_not_true)) {
        s = truthsVectorPerGenerator[generatorIndex][valueMapIndex];
      } else {
        s = std::make_shared<Statement>();
        s->set_statement(
            statementGenerators[generatorIndex]->truth(valueMapIndex));
        s->set_truth(true);
      }
    } while (statementsSet.count(s) > 0 && retries_left-- > 0);
    if (retries_left <= 0) {
      std::cerr
          << "ERROR: generated too many duplicate statements"
          << std::endl;
      return EXIT_FAILURE;
    }
    statementsSet.insert(s);
    statementsVector.push_back(s);
  }

  for (int i = 0; i < absl::GetFlag(FLAGS_lies); ++i) {
    int generatorIndex;
    std::shared_ptr<Statement> s = std::make_shared<Statement>();
    s->set_truth(false);
    int retries_left = absl::GetFlag(FLAGS_max_retries);
    do {
      generatorIndex = dd(gen);
      // generatorWeights represent the size of each valueMap vector
      int valueMapIndex = static_cast<int>(
          generatorWeights[generatorIndex] * ud(gen));
      s->set_statement(statementGenerators[generatorIndex]->lie(valueMapIndex));
    } while (
        truthsSetPerGenerator[generatorIndex].count(s) > 0 &&
        statementsSet.count(s) > 0 && retries_left-- > 0);
    if (retries_left <= 0) {
      std::cerr
          << "ERROR: generated too many duplicate statements"
          << std::endl;
      return EXIT_FAILURE;
    }
    statementsSet.insert(s);
    statementsVector.push_back(s);
  }

  if (absl::GetFlag(FLAGS_random_order)) {
    std::shuffle(statementsVector.begin(), statementsVector.end(), gen);
  } else {
    std::sort(
        statementsVector.begin(), statementsVector.end(),
        [](const std::shared_ptr<Statement>& lhs,
           const std::shared_ptr<Statement>& rhs) -> bool {
          return lhs->statement() < rhs->statement();
        });
  }
  for (auto s : statementsVector) {
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

