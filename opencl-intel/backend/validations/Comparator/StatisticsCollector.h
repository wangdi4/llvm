// INTEL CONFIDENTIAL
//
// Copyright 2011 Intel Corporation.
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

#ifndef __STATISTICSCOLLECTOR_H__
#define __STATISTICSCOLLECTOR_H__

#include "Exception.h"
#include "llvm/Support/DataTypes.h"
#include <map>
#include <sstream>
#include <string>

namespace Validation {

class StatisticsCollector {
public:
  /// Type of statistics to gather
  /// COUNT - Count specified string
  /// SUM - compute sum of values
  /// AVG - compute average value
  enum STAT_TYPE { COUNT, SUM, AVG };

  struct StatValue {
    double value;
    double res;
    uint64_t count;
    STAT_TYPE type;
  };

  StatisticsCollector() {}
  void UpdateStatistics(STAT_TYPE statType, std::string stat_name,
                        double in_val);
  void CountStatistics(std::string stat_name);

  double GetResult(const std::string &stat_name);

  std::string ToString();

private:
  void Finalize();
  typedef std::map<std::string, StatValue> StatMap;
  StatMap m_values;
};

} // namespace Validation
#endif // __STATISTICSCOLLECTOR_H__
