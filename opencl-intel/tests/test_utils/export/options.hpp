//==--- options.hpp - CMD line options utils for unittests -*- C++ -*    ---==//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
#include <algorithm>
#include <cassert>
#include <cstring>
#include <string>

template <typename T> class CommandLineOption {
  std::string m_name;

  T convertToT(const std::string &) const;

public:
  CommandLineOption(const char *cmdName);
  T getValue(const std::string &) const;
  bool isMatch(const std::string &) const;
};

template <typename T>
bool CommandLineOption<T>::isMatch(const std::string &s) const {
  size_t size = m_name.size();
  bool b = (s.size() > size) && (s.substr(0, size) == m_name);
  if (!b) {
    return false;
  }

  return (s.size() > size + ::strlen("=")) && (s.at(size) == '=');
}

template <typename T>
CommandLineOption<T>::CommandLineOption(const char *cmdName)
    : m_name(cmdName) {}

template <>
std::string
CommandLineOption<std::string>::convertToT(const std::string &s) const {
  return s;
}

template <> int CommandLineOption<int>::convertToT(const std::string &s) const {
  return atoi(s.c_str());
}

template <>
bool CommandLineOption<bool>::convertToT(const std::string &s) const {
  std::string t;
  std::transform(s.begin(), s.end(), t.begin(), ::tolower);
  return (t == "true" || t == "1") ? true : false;
}

template <typename T>
T CommandLineOption<T>::getValue(const std::string &cmdString) const {
  assert(isMatch(cmdString));
  size_t valueSeparator = cmdString.find('=');
  assert(std::string::npos != valueSeparator);
  return convertToT(cmdString.substr(valueSeparator + 1, cmdString.size()));
}
