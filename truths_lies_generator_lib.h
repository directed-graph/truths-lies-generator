
#ifndef _EVERCHANGING_TRUTHS_LIES_GENERATOR_LIB_H_
#define _EVERCHANGING_TRUTHS_LIES_GENERATOR_LIB_H_

#include <functional>
#include <map>
#include <memory>
#include <string>

#include "truths_lies_config.pb.h"

namespace everchanging {
namespace truths_lies_generator {

template<class... Args>
std::string StringFormat(const std::string& format, const Args&... args);

double GenRandomDouble(double lo, double hi);

class StatementGenerator {
public:
  // constructor will transfer ownership of config to object
  StatementGenerator(TruthsLiesConfig* config)
    : config(config) {};
  ~StatementGenerator() {};
  virtual std::string truth(const ValueMap& valueMap) {
    return applyValues(valueMap);
  };
  virtual std::string truth(int index) {
    return truth(config->arguments()[index]);
  };
  virtual std::string lie(const ValueMap& valueMap) = 0;
  virtual std::string lie(int index) {
    return lie(config->arguments()[index]);
  };
  std::string applyValues(const ValueMap& valueMap);
protected:
  std::unique_ptr<TruthsLiesConfig> config;
};

class CubingStatementGenerator : public StatementGenerator {
public:
  CubingStatementGenerator(TruthsLiesConfig* config)
    : StatementGenerator(config) {};
  std::string truth(const ValueMap& valueMap) override;
  std::string lie(const ValueMap& valueMap) override;
private:
  const std::string format =
      "%.2f seconds rounded to the nearest hundredth of a second";
};

StatementGenerator* CreateStatementGenerator(TruthsLiesConfig* config);

};  // namespace truths_lies_generator
};  // namespace everchanging

#endif  // _EVERCHANGING_TRUTHS_LIES_GENERATOR_LIB_H_

