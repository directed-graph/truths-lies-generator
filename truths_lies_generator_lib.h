
#include <functional>
#include <map>
#include <string>

#include "truths_lies_config.pb.h"

namespace everchanging {
namespace truths_lies_generator {

template<class... Args>
std::string string_format(const std::string& format, const Args&... args);

double gen_random_double(double lo, double hi);

namespace cubing {
extern const std::string format;
void truth(ValueMap* value_map);
void lie(ValueMap* value_map);
};  // namespace cubing

extern std::map<std::string, std::function<void(ValueMap*)>> functionMap;

};  // namespace truths_lies_generator
};  // namespace everchanging

