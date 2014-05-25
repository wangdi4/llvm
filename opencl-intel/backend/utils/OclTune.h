#ifndef __OCL_TUNE_H__
#define __OCL_TUNE_H__

#include <string>
#include <vector>
#include "MetaDataApi.h"
#include "llvm/IR/Function.h"

#ifdef OCLT

namespace intel {


class Statistic {
public:
  typedef std::vector<Statistic*> ActiveStatsT;

public:
  unsigned getValue() const { return Value; }
  const char *getName() const { return Name; }
  const char *getDesc() const { return Desc; }

  /// construct - This should only be called for non-global statistics.
  Statistic(const char *category, const char *name, const char *desc,
      ActiveStatsT &as) :
      Name(name), Desc(desc), ActiveStats(as) {
    Value = 0; Initialized = false;
  }

  void reset() {
    Value = 0; Initialized = false;
  }

  // Allow use of this class as the value itself.
  operator unsigned() const { return Value; }
  const Statistic &operator=(unsigned Val) {
    Value = Val;
    return init();
  }

  const Statistic &operator++() {
    Value++;
    return init();
  }

  unsigned operator++(int) {
    init();
    unsigned OldValue = Value;
    Value++;
    return OldValue;
  }

  const Statistic &operator--() {
    Value--;
    return init();
  }

  unsigned operator--(int) {
    init();
    unsigned OldValue = Value;
    Value--;
    return OldValue;
  }

  const Statistic &operator+=(const unsigned &V) {
    if (!V) return *this;
    Value += V;
    return init();
  }

  const Statistic &operator-=(const unsigned &V) {
    if (!V) return *this;
    Value -= V;
    return init();
  }

  const Statistic &operator*=(const unsigned &V) {
    Value *= V;
    return init();
  }

  const Statistic &operator/=(const unsigned &V) {
    Value /= V;
    return init();
  }

  static void pushFunctionStats (ActiveStatsT &activeStats, llvm::Function &F,
      const char *type);

  static void moveFunctionStats (llvm::Function &FromFunction,
      llvm::Function &ToFunction);

  static void copyFunctionStats (llvm::Function &FromFunction,
      llvm::Function &ToFunction);

  static void removeFunctionStats (llvm::Function &FromFunction);

  static void enableStats (bool status = true) {
    StatFlag = status;
  }

  static bool isEnabled () {
    return StatFlag;
  }

  static void setCurrentStatType(const char *Type) {
    CurrentStatType = Type;
  }

  static bool isCurrentStatType(const char *StatType) {
    return CurrentStatType.empty() || StatType == CurrentStatType;
  }

public:
  static std::string CurrentStatType;
  static bool StatFlag;

protected:
  Statistic &init() {
    if (!Initialized) {
      ActiveStats.push_back(this);
      Initialized = true;
    }
    return *this;
  }

private:
  const char *Name;
  const char *Desc;
  ActiveStatsT &ActiveStats;
  unsigned int Value;
  bool Initialized;
};

// initialize statistic objects that are class memebers
#define OCLSTAT_INIT(NAME,DESC,LIST) \
  NAME(DEBUG_TYPE, #NAME "@" DEBUG_TYPE, DESC, LIST)

// define and initialize a statistic object
#define OCLSTAT_DEFINE(NAME,DESC,LIST) \
		intel::Statistic NAME(DEBUG_TYPE, #NAME "@" DEBUG_TYPE, DESC, LIST)

} // End intel namespace


// execute code only if stats are enabled
#define TUNEOCL(X)  do { if (Statistic::StatFlag) { X; } } while (0)

// execute code only if stats are enabled for this module
#define TUNEOCL_CHECK(X)  \
  do { if (Statistic::StatFlag && Statistic::isCurrentStatType(DEBUG_TYPE)) \
    { X; } }   while (0)

// execute code only if stats are enabled for the requested module
#define TUNEOCL_CHECK_TYPE(X,TYPE)  \
  do { if (Statistic::StatFlag && Statistic::isCurrentStatType(TYPE)) { X; } } \
  while (0)

#else // OCLT

namespace intel {


class Statistic {
public:
  class ActiveStatsT {};

private:

public:
  unsigned getValue() const { return 0; }
  const char *getName() const { return NULL; }
  const char *getDesc() const { return NULL; }

  /// construct - This should only be called for non-global statistics.
  Statistic(const char *category, const char *name, const char *desc,
      ActiveStatsT &as) {}

  void reset() {}

  // Allow use of this class as the value itself.
  operator unsigned() const { return 0; }
  const Statistic &operator=(unsigned Val) {
    return init();
  }

  const Statistic &operator++() {
    return init();
  }

  unsigned operator++(int) {
    return 0;
  }

  const Statistic &operator--() {
    return init();
  }

  unsigned operator--(int) {
    return 0;
  }

  const Statistic &operator+=(const unsigned &V) {
    return init();
  }

  const Statistic &operator-=(const unsigned &V) {
    return init();
  }

  const Statistic &operator*=(const unsigned &V) {
    return init();
  }

  const Statistic &operator/=(const unsigned &V) {
    return init();
  }

protected:
  Statistic &init() {
    return *this;
  }

public:
  static void pushKernelStats (ActiveStatsT &activeStats, llvm::Function &F,
      const char *type) {}

  static void moveKernelStats (llvm::Function &FromFunction,
      llvm::Function &ToFunction) {}

  static void copyKernelStats (llvm::Function &FromFunction,
      llvm::Function &ToFunction) {}

  static void removeKernelStats (llvm::Function &FromFunction) {}

  static void enableStats (bool status = true) {}

  static bool isEnabled () { return false; }

  static void setCurrentStatType(const char *Type){}

  static bool isCurrentStatType(const char *StatType) { return false; }
};

// initialize statistic objects that are class memebers
#define OCLSTAT_INIT(NAME,DESC,LIST) \
  NAME(DEBUG_TYPE, #NAME "@" DEBUG_TYPE, DESC, LIST)

// define and initialize a statistic object
#define OCLSTAT_DEFINE(NAME,DESC,LIST) \
    intel::Statistic NAME(DEBUG_TYPE, #NAME "@" DEBUG_TYPE, DESC, LIST)

} // End intel namespace

#define TUNEOCL(X)

#define TUNEOCL_CHECK(X)

#define TUNEOCL_CHECK_TYPE(X,TYPE)

#endif // OCLT

#endif // __OCL_TUNE_H__
