
#include <functional>
#include <map>
#include <string>

#include "truths_lies_config.pb.h"

namespace everchanging {
namespace truths_lies_generator {

template<class... Args>
std::string StringFormat(const std::string& format, const Args&... args);

double GenRandomDouble(double lo, double hi);

namespace cubing {
extern const std::string format;
void Truth(ValueMap* value_map);
void Lie(ValueMap* value_map);
};  // namespace cubing

extern std::map<std::string, std::function<void(ValueMap*)>> functionMap;

};  // namespace truths_lies_generator
};  // namespace everchanging

