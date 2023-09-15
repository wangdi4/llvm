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

#include "StatisticsCollector.h"

using namespace Validation;

void StatisticsCollector::CountStatistics(std::string stat_name) {
  UpdateStatistics(COUNT, stat_name, 0.0);
}

void StatisticsCollector::UpdateStatistics(STAT_TYPE in_statType,
                                           std::string stat_name,
                                           double in_val) {
  StatMap::iterator mit = m_values.find(stat_name);
  // if not exist add with zero initialized
  if (mit == m_values.end()) {
    StatValue val;
    val.type = in_statType;
    val.value = 0;
    val.count = 0;
    val.res = 0;
    mit = m_values.insert(std::pair<std::string, StatValue>(stat_name, val))
              .first;
  }

  if ((mit->second).type != in_statType)
    throw Exception::InvalidArgument(
        "Statistics type is different from previously set type");

  (mit->second).value += in_val;
  (mit->second).count++;
}

void StatisticsCollector::Finalize() {
  for (StatMap::iterator e = m_values.end(), it = m_values.begin(); it != e;
       ++it) {
    switch ((it->second).type) {
    case AVG:
      (it->second).res = (double)(it->second).value / (it->second).count;
      break;
    case SUM:
      (it->second).res = (double)(it->second).value;
      break;
    case COUNT:
      (it->second).res = (double)(it->second).count;
      break;
    }
  }
}

double StatisticsCollector::GetResult(const std::string &in_name) {
  StatMap::iterator mit = m_values.find(in_name);
  // if not exist add with zero initialized
  if (mit == m_values.end())
    return 0.0;
  Finalize();
  return mit->second.res;
}

std::string StatisticsCollector::ToString() {
  Finalize();
  std::stringstream ss;
  for (StatMap::iterator e = m_values.end(), it = m_values.begin(); it != e;
       ++it) {
    ss << (it->first) << " : ";
    switch (it->second.type) {
    case AVG:
      ss << (it->second).res;
      break;
    case SUM:
      ss << (it->second).value;
      break;
    case COUNT:
      ss << (it->second).count;
      break;
    }
    ss << "\n";
  }
  return ss.str();
}
