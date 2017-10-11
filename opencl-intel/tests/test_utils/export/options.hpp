//==--- options.hpp - CMD line options utils for unittests -*- C++ -*    ---==//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
#include <cassert>
#include <cstdio>

template <typename T>
class CommandLineOption{
  std::string m_name;

  T convertToT(const std::string&)const;
public:
  CommandLineOption(const char* cmdName);
  T getValue(const std::string&)const;
  bool isMatch(const std::string&)const;
};

template <typename T>
bool CommandLineOption<T>::isMatch(const std::string& s)const{
  size_t size = m_name.size();
  bool b = s.substr(0, size) == m_name;
  if (!b){
    printf("substr=%s\n", s.substr(0, size).c_str());
    printf("m_name=%s\n", m_name.c_str());
    printf("%s:%d\n", __FILE__, __LINE__);
    return false;
  }
  b = s.at(size) == '=';
  if (!b){
    printf("%s:%d\n", __FILE__, __LINE__);
    return false;
  }
  return true;
}

template <typename T>
CommandLineOption<T>::CommandLineOption(const char* cmdName):
  m_name(cmdName){
}

template <>
std::string CommandLineOption<std::string>::convertToT(const std::string& s)const{
  return s;
}

template<>
int CommandLineOption<int>::convertToT(const std::string& s)const{
  return atoi(s.c_str());
}

template<>
bool CommandLineOption<bool>::convertToT(const std::string& s)const{
  return s == "true" ? true : false;
}

template <typename T>
T CommandLineOption<T>::getValue(const std::string& cmdString)const{
  assert(isMatch(cmdString));
  size_t valueSeparator = cmdString.find('=');
  assert(std::string::npos != valueSeparator);
  return convertToT(cmdString.substr(valueSeparator+1, cmdString.size()));
}
