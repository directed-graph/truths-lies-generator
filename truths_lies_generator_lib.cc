
#include <functional>
#include <random>
#include <string>

#include <google/protobuf/any.h>
#include <google/protobuf/wrappers.pb.h>

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

std::string StatementGenerator::applyValues(const ValueMap& valueMap) {
  std::string string(config->template_string());
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

std::string CubingStatementGenerator::truth(const ValueMap& valueMap_) {
  auto values = valueMap_.values();
  ValueMap valueMap(valueMap_);

  (*valueMap.mutable_values())["time"].set_string_value(
      StringFormat<double>(format, values["time"].double_value()));

  return applyValues(valueMap);
};

std::string CubingStatementGenerator::lie(const ValueMap& valueMap_) {
  auto values = valueMap_.values();
  ValueMap valueMap(valueMap_);

  (*valueMap.mutable_values())["time"].set_string_value(
      StringFormat<double>(format,
          values["time"].double_value() + GenRandomDouble(-1, 1)));

  return applyValues(valueMap);
};

StatementGenerator* CreateStatementGenerator(TruthsLiesConfig* config) {
  if (config->class_name() == "CubingStatementGenerator")
      return new CubingStatementGenerator(config);
  return nullptr;
};

};  // namespace truths_lies_generator
};  // namespace everchanging

