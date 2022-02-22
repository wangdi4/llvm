//===- DPCPPStatistic.h - Save statistic as metadata ------------*- C++ -*-===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_DPCPPKERNELTRANSFORMS_UTILS_DPCPPSTATISTIC_H
#define LLVM_TRANSFORMS_INTEL_DPCPPKERNELTRANSFORMS_UTILS_DPCPPSTATISTIC_H

#include "llvm/IR/Module.h"

namespace llvm {

#ifndef INTEL_PRODUCT_RELEASE

class DPCPPStatistic {
public:
  using ActiveStatsT = SmallVector<DPCPPStatistic *, 16>;

  unsigned getValue() const { return Value; }
  StringRef getName() const { return Name; }
  StringRef getDesc() const { return Desc; }

  // Ctor - this should only be called for non-global statistics.
  DPCPPStatistic(StringRef, StringRef Name, StringRef Desc, ActiveStatsT &AS)
      : Name(Name), Desc(Desc), ActiveStats(AS), Value(0), Initialized(false) {}

  void reset() {
    Value = 0;
    Initialized = false;
  }

  /// Allow use of this class as the value itself.
  operator unsigned() const { return Value; }
  DPCPPStatistic &operator=(unsigned Val) {
    Value = Val;
    init();
    return *this;
  }

  const DPCPPStatistic &operator++() {
    Value++;
    return init();
  }

  unsigned operator++(int) {
    init();
    unsigned OldValue = Value;
    Value++;
    return OldValue;
  }

  const DPCPPStatistic &operator--() {
    Value--;
    return init();
  }

  unsigned operator--(int) {
    init();
    unsigned OldValue = Value;
    Value--;
    return OldValue;
  }

  const DPCPPStatistic &operator+=(const unsigned &V) {
    if (!V)
      return *this;
    Value += V;
    return init();
  }

  const DPCPPStatistic &operator-=(const unsigned &V) {
    if (!V)
      return *this;
    Value -= V;
    return init();
  }

  const DPCPPStatistic &operator*=(const unsigned &V) {
    Value *= V;
    return init();
  }

  const DPCPPStatistic &operator/=(const unsigned &V) {
    Value /= V;
    return init();
  }

  static void setModuleStatInfo(Module *M, StringRef RunTimeVersion,
                                StringRef WorkloadName, StringRef ModuleName);

  static void pushFunctionStats(ActiveStatsT &ActiveStats, Function &F,
                                StringRef Ty);

  static void moveFunctionStats(Function &FromFunction, Function &ToFunction);

  static void copyFunctionStats(Function &FromFunction, Function &ToFunction);

  static void removeFunctionStats(Function &FromFunction);

  static void enableStats(bool Status = true) { StatFlag = Status; }

  static bool isEnabled() { return StatFlag; }

  static void setCurrentStatType(StringRef Type) {
    CurrentStatType = Type.str();
  }

  static bool isCurrentStatType(StringRef StatType) {
    return CurrentStatType.empty() || StatType == CurrentStatType;
  }

public:
  static std::string CurrentStatType;
  static bool StatFlag;

protected:
  DPCPPStatistic &init() {
    if (!Initialized) {
      ActiveStats.push_back(this);
      Initialized = true;
    }
    return *this;
  }

private:
  StringRef Name;
  StringRef Desc;
  ActiveStatsT &ActiveStats;
  unsigned Value;
  bool Initialized;
};

// initialize statistic objects that are class memebers
#define DPCPP_STAT_INIT(NAME, DESC, LIST)                                      \
  NAME(DEBUG_TYPE, DEBUG_TYPE "." #NAME, DESC, LIST)

// define and initialize a statistic object
#define DPCPP_STAT_DEFINE(NAME, DESC, LIST)                                    \
  DPCPPStatistic NAME(DEBUG_TYPE, DEBUG_TYPE "." #NAME, DESC, LIST)

// execute code only if stats are enabled
#define DPCPP_STAT_GATHER(X)                                                   \
  do {                                                                         \
    if (DPCPPStatistic::StatFlag) {                                            \
      X;                                                                       \
    }                                                                          \
  } while (0)

// execute code only if stats are enabled for this module
#define DPCPP_STAT_GATHER_CHECK(X)                                             \
  do {                                                                         \
    if (DPCPPStatistic::StatFlag &&                                            \
        DPCPPStatistic::isCurrentStatType(DEBUG_TYPE)) {                       \
      X;                                                                       \
    }                                                                          \
  } while (0)

// execute code only if stats are enabled for the requested module
#define DPCPP_STAT_GATHER_CHECK_TYPE(X, TYPE)                                  \
  do {                                                                         \
    if (DPCPPStatistic::StatFlag && DPCPPStatistic::isCurrentStatType(TYPE)) { \
      X;                                                                       \
    }                                                                          \
  } while (0)

#else // INTEL_PRODUCT_RELEASE

class DPCPPStatistic {
public:
  class ActiveStatsT {};

private:
public:
  unsigned getValue() const { return 0; }
  StringRef getName() const { return ""; }
  StringRef getDesc() const { return ""; }

  /// construct - This should only be called for non-global statistics.
  DPCPPStatistic(StringRef Category, StringRef Name, StringRef Desc,
                 ActiveStatsT &AS) {}

  void reset() {}

  // Allow use of this class as the value itself.
  operator unsigned() const { return 0; }
  const DPCPPStatistic &operator=(unsigned Val) { return init(); }

  const DPCPPStatistic &operator++() { return init(); }

  unsigned operator++(int) { return 0; }

  const DPCPPStatistic &operator--() { return init(); }

  unsigned operator--(int) { return 0; }

  const DPCPPStatistic &operator+=(const unsigned &V) { return init(); }

  const DPCPPStatistic &operator-=(const unsigned &V) { return init(); }

  const DPCPPStatistic &operator*=(const unsigned &V) { return init(); }

  const DPCPPStatistic &operator/=(const unsigned &V) { return init(); }

protected:
  DPCPPStatistic &init() { return *this; }

public:
  static void setModuleStatInfo(Module *M, StringRef RunTimeVersion,
                                StringRef WorkloadName, StringRef ModuleName) {}

  static void pushFunctionStats(ActiveStatsT &activeStats, Function &F,
                                StringRef type) {}

  static void moveFunctionStats(Function &FromFunction, Function &ToFunction) {}

  static void copyFunctionStats(Function &FromFunction, Function &ToFunction) {}

  static void removeFunctionStats(Function &FromFunction) {}

  static void enableStats(bool Status = false) {}

  static bool isEnabled() { return false; }

  static void setCurrentStatType(StringRef Type) {}

  static bool isCurrentStatType(StringRef StatType) { return false; }
};

// initialize statistic objects that are class memebers
#define DPCPP_STAT_INIT(NAME, DESC, LIST)                                      \
  NAME(DEBUG_TYPE, DEBUG_TYPE "." #NAME, DESC, LIST)

// define and initialize a statistic object
#define DPCPP_STAT_DEFINE(NAME, DESC, LIST)                                    \
  DPCPPStatistic NAME(DEBUG_TYPE, DEBUG_TYPE "." #NAME, DESC, LIST)

#define DPCPP_STAT_GATHER(X)

#define DPCPP_STAT_GATHER_CHECK(X)

#define DPCPP_STAT_GATHER_CHECK_TYPE(X, TYPE)

#endif // INTEL_PRODUCT_RELEASE

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPPKERNELTRANSFORMS_UTILS_DPCPPSTATISTIC_H
