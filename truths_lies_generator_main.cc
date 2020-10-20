
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

ABSL_FLAG(int, lies, 0, "number of lie statements");
ABSL_FLAG(int, truths, 0, "number of truth statements");
ABSL_FLAG(std::vector<std::string>, input_files, {}, "input data file");

namespace {
using namespace everchanging::truths_lies_generator;
};

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);

  std::vector<int> generatorWeights;
  std::vector<std::unique_ptr<StatementGenerator>> statementGenerators;
  std::vector<std::string> statements;

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
  }

  std::random_device rd;
  std::mt19937 gen(rd());
  std::discrete_distribution<> dd(generatorWeights.begin(),
                                 generatorWeights.end());
  std::uniform_real_distribution<> ud(0.0, 1.0);

  for (int i = 0; i < absl::GetFlag(FLAGS_truths); ++i) {
    int generatorIndex = dd(gen);
    // generatorWeights represent the size of each valueMap vector
    int valueMapIndex = static_cast<int>(
        generatorWeights[generatorIndex] * ud(gen));
    statements.push_back(
        statementGenerators[generatorIndex]->truth(valueMapIndex));
  }

  for (int i = 0; i < absl::GetFlag(FLAGS_lies); ++i) {
    int generatorIndex = dd(gen);
    // generatorWeights represent the size of each valueMap vector
    int valueMapIndex = static_cast<int>(
        generatorWeights[generatorIndex] * ud(gen));
    statements.push_back(
        statementGenerators[generatorIndex]->lie(valueMapIndex));
  }

  for (auto statement : statements) {
    std::cout << statement << std::endl;
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

