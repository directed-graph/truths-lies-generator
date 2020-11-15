
#ifndef _EVERCHANGING_TRUTHS_LIES_GENERATOR_LIB_H_
#define _EVERCHANGING_TRUTHS_LIES_GENERATOR_LIB_H_

#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <string>

#include "absl/status/status.h"
#include "truths_lies_config.pb.h"

namespace everchanging {
namespace truths_lies_generator {

template<class... Args>
std::string StringFormat(const std::string& format, const Args&... args);

double GenRandomDouble(double lo, double hi);

class StatementGenerator {
public:
  StatementGenerator(TruthsLiesConfig config)
    : config(std::move(config)) {};
  ~StatementGenerator() {};
  // truths generator should be deterministic
  virtual std::string truth(const ValueMap& valueMap) const {
    return applyValues(valueMap);
  };
  virtual std::string truth(int index) const {
    return truth(config.arguments()[index]);
  };
  // lies generator can be non-deterministic
  virtual std::string lie(const ValueMap& valueMap) const = 0;
  virtual std::string lie(int index) const {
    return lie(config.arguments()[index]);
  };
  std::string applyValues(const ValueMap& valueMap) const;
  size_t size() const;
protected:
  TruthsLiesConfig config;
};

class CubingStatementGenerator : public StatementGenerator {
public:
  CubingStatementGenerator(TruthsLiesConfig config)
    : StatementGenerator(std::move(config)) {};
  std::string truth(const ValueMap& valueMap) const override;
  std::string lie(const ValueMap& valueMap) const override;
private:
  const std::string format =
      "%.2f seconds rounded to the nearest hundredth of a second";
};

std::shared_ptr<StatementGenerator> CreateStatementGenerator(
    TruthsLiesConfig config);

class StatementCollection {
public:
  std::shared_ptr<Statement> operator[](size_t index) const;
  absl::Status insert(std::shared_ptr<Statement> s);
  absl::Status sort();
  int count(std::shared_ptr<Statement> s) const;
  auto begin() { return statementVector.begin(); };
  auto begin() const { return statementVector.begin(); };
  auto end() { return statementVector.end(); };
  auto end() const { return statementVector.end(); };
private:
  struct SharedStatementCmp {
    bool operator()(const std::shared_ptr<Statement>& lhs,
                    const std::shared_ptr<Statement>& rhs) const {
      return lhs->statement() < rhs->statement();
    }
  };
  std::set<std::shared_ptr<Statement>, SharedStatementCmp> statementSet;
  std::vector<std::shared_ptr<Statement>> statementVector;
};

};  // namespace truths_lies_generator
};  // namespace everchanging

#endif  // _EVERCHANGING_TRUTHS_LIES_GENERATOR_LIB_H_

