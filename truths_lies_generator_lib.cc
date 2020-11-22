
#include <functional>
#include <random>
#include <string>

#include <google/protobuf/any.h>
#include <google/protobuf/wrappers.pb.h>

#include "absl/strings/str_cat.h"
#include "truths_lies_config.pb.h"
#include "truths_lies_generator_lib.h"

namespace everchanging {
namespace truths_lies_generator {

// based on https://stackoverflow.com/a/26221725
template<class... Args>
std::string StringFormat(const std::string& format, const Args&... args) {
  std::string string;
  size_t size = snprintf(nullptr, 0, format.c_str(), args ... ) + 1;
  std::unique_ptr<char[]> buffer(new char[size]);
  std::snprintf(buffer.get(), size, format.c_str(), args ... );
  return std::move(std::string(buffer.get(), buffer.get() + size - 1));
};

double GenRandomDouble(double lo, double hi) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(lo, hi);
  return dis(gen);
};

std::string StatementGenerator::applyValues(const ValueMap& valueMap) const {
  std::string string(config.template_string());
  std::string oneof_value;
  for (auto& [key, value] : valueMap.values()) {
    switch (value.value_case()) {
      case ValueMap_Value::ValueCase::kAnyValue:
        oneof_value = "";
        break;
      case ValueMap_Value::ValueCase::kIntValue:
        oneof_value = std::to_string(value.int_value());
        break;
      case ValueMap_Value::ValueCase::kDoubleValue:
        oneof_value = std::to_string(value.double_value());
        break;
      case ValueMap_Value::ValueCase::kStringValue:
        oneof_value = value.string_value();
        break;
      default:
        oneof_value = "";
    }
    string.replace(
        string.find("{" + key + "}"),
        std::string("{" + key + "}").size(),
        oneof_value);
  }
  return string;
};

size_t StatementGenerator::size() const {
  return config.arguments().size();
};

std::string CubingStatementGenerator::truth(const ValueMap& valueMap_) const {
  auto values = valueMap_.values();
  ValueMap valueMap(valueMap_);

  (*valueMap.mutable_values())["time"].set_string_value(
      StringFormat<double>(format, values["time"].double_value()));

  return applyValues(valueMap);
};

std::string CubingStatementGenerator::lie(const ValueMap& valueMap_) const {
  auto values = valueMap_.values();
  ValueMap valueMap(valueMap_);

  (*valueMap.mutable_values())["time"].set_string_value(
      StringFormat<double>(format,
          values["time"].double_value() + GenRandomDouble(-1, 1)));

  return applyValues(valueMap);
};

std::shared_ptr<StatementGenerator> CreateStatementGenerator(
    TruthsLiesConfig config) {
  if (config.class_name() == "CubingStatementGenerator")
      return std::make_shared<CubingStatementGenerator>(std::move(config));
  return nullptr;
};

std::shared_ptr<Statement> StatementCollection::operator[](size_t index) const {
  if (index < statementVector.size())
    return statementVector[index];
  return nullptr;
};

absl::Status StatementCollection::insert(std::shared_ptr<Statement> s) {
  if (count(s) > 0)
    return absl::AlreadyExistsError(
        absl::StrCat("statement already in the collection: ", s->statement()));
  statementSet.insert(s);
  statementVector.push_back(s);
  if (s->truth()) {
    truthsSet.insert(s);
  } else {
    liesSet.insert(s);
  }
  return absl::OkStatus();
};

absl::Status StatementCollection::sort() {
  std::sort(begin(), end(), SharedStatementCmp());
  return absl::OkStatus();
};

int StatementCollection::count(std::shared_ptr<Statement> statement) const {
  return statementSet.count(statement);
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
    int truths, int lies, int max_retries, bool ensure_not_true) {
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

};  // namespace truths_lies_generator
};  // namespace everchanging

