
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

std::string DecodeValue(const ValueMap_Value& value) {
  switch (value.value_case()) {
    case ValueMap_Value::ValueCase::kAnyValue:
      return "";
    case ValueMap_Value::ValueCase::kIntValue:
      return std::to_string(value.int_value());
    case ValueMap_Value::ValueCase::kDoubleValue:
      return std::to_string(value.double_value());
    case ValueMap_Value::ValueCase::kStringValue:
      return value.string_value();
    default:
      return "";
  }
};

std::string ApplyValues(
    const std::string& templateString,
    const ValueMap& value_map) {
  std::string string(templateString);
  for (auto& [key, value] : value_map.values()) {
    string.replace(
        string.find("{" + key + "}"),
        std::string("{" + key + "}").size(),
        DecodeValue(value));
  }
  return string;
};

double GenRandomDouble(double lo, double hi) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(lo, hi);
  return dis(gen);
};

const std::string cubing::format =
    "%.2f seconds rounded to the nearest hundredth of a second";

void cubing::Truth(ValueMap* value_map) {
  auto values = value_map->values();

  (*value_map->mutable_values())["time"].set_string_value(
      StringFormat<double>(cubing::format, values["time"].double_value()));
};

void cubing::Lie(ValueMap* value_map) {
  auto values = value_map->values();

  (*value_map->mutable_values())["time"].set_string_value(
      StringFormat<double>(cubing::format,
          values["time"].double_value() + GenRandomDouble(-1, 1)));
};

std::map<std::string, std::function<void(ValueMap*)>> functionMap =
{
  { "everchanging::truths_lies_generator::cubing::Truth", cubing::Truth },
  { "everchanging::truths_lies_generator::cubing::Lie", cubing::Lie },
};

};  // namespace truths_lies_generator
};  // namespace everchanging

