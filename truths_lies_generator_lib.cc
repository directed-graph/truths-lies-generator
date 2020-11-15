
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
  return absl::OkStatus();
};

int StatementCollection::count(std::shared_ptr<Statement> statement) const {
  return statementSet.count(statement);
};

};  // namespace truths_lies_generator
};  // namespace everchanging

