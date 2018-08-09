// INTEL CONFIDENTIAL
//
// Copyright 2010-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#ifndef __OCL_TUNE_H__
#define __OCL_TUNE_H__

#include "llvm/IR/Function.h"

#include <string>
#include <vector>

#ifndef INTEL_PRODUCT_RELEASE

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

  static void setModuleStatInfo (llvm::Module *M, const char * workloadName,
      const char * moduleName);

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
  NAME(DEBUG_TYPE, DEBUG_TYPE "@" #NAME, DESC, LIST)

// define and initialize a statistic object
#define OCLSTAT_DEFINE(NAME,DESC,LIST) \
		intel::Statistic NAME(DEBUG_TYPE, DEBUG_TYPE "@" #NAME, DESC, LIST)

} // End intel namespace


// execute code only if stats are enabled
#define OCLSTAT_GATHER(X)  do { if (Statistic::StatFlag) { X; } } while (0)

// execute code only if stats are enabled for this module
#define OCLSTAT_GATHER_CHECK(X)  \
  do { if (Statistic::StatFlag && Statistic::isCurrentStatType(DEBUG_TYPE)) \
    { X; } }   while (0)

// execute code only if stats are enabled for the requested module
#define OCLSTAT_GATHER_CHECK_TYPE(X,TYPE)  \
  do { if (Statistic::StatFlag && Statistic::isCurrentStatType(TYPE)) { X; } } \
  while (0)

#else // INTEL_PRODUCT_RELEASE

namespace intel {


class Statistic {
public:
  class ActiveStatsT {};

private:

public:
  unsigned getValue() const { return 0; }
  const char *getName() const { return nullptr; }
  const char *getDesc() const { return nullptr; }

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
  static void setModuleStatInfo (llvm::Module *M, const char * workloadName,
      const char * moduleName) {}

  static void pushFunctionStats (ActiveStatsT &activeStats, llvm::Function &F,
      const char *type) {}

  static void moveFunctionStats (llvm::Function &FromFunction,
      llvm::Function &ToFunction) {}

  static void copyFunctionStats (llvm::Function &FromFunction,
      llvm::Function &ToFunction) {}

  static void removeFunctionStats (llvm::Function &FromFunction) {}

  static void enableStats (bool status = false) {}

  static bool isEnabled () { return false; }

  static void setCurrentStatType(const char *Type){}

  static bool isCurrentStatType(const char *StatType) { return false; }
};

// initialize statistic objects that are class memebers
#define OCLSTAT_INIT(NAME,DESC,LIST) \
  NAME(DEBUG_TYPE, DEBUG_TYPE "@" #NAME, DESC, LIST)

// define and initialize a statistic object
#define OCLSTAT_DEFINE(NAME,DESC,LIST) \
    intel::Statistic NAME(DEBUG_TYPE, DEBUG_TYPE "@" #NAME, DESC, LIST)

} // End intel namespace

#define OCLSTAT_GATHER(X)

#define OCLSTAT_GATHER_CHECK(X)

#define OCLSTAT_GATHER_CHECK_TYPE(X,TYPE)

#endif // INTEL_PRODUCT_RELEASE

#endif // __OCL_TUNE_H__
