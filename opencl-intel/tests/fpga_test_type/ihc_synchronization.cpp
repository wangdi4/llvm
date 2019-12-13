//===--- ihc_synchronization.cpp -                              -*- C++ -*-===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
//
// Internal tests for IHC synchronization feature
//
// ===--------------------------------------------------------------------=== //

#include "ihc_threadsupport.h"
#include <gtest/gtest.h>
#include <sstream>
#include <thread>

using namespace Intel::OpenCL::Utils;

std::stringstream result;

int uniqueEntries(std::stringstream &ss) {
  // Create the set of unique lines.
  // It is used to test that we don't get the output
  // Broken into multiple lines by data races
  std::set<std::string> lines;
  std::string line;
  while (std::getline(ss, line)) {
    lines.insert(line);
  }
  return lines.size();
}

TEST(IHCSyncronization, BasicMutex) {
  std::stringstream result;
  void *_m = _ihc_mutex_create();

  for (int i = 0; i < 10; ++i) {

    _ihc_mutex_lock(_m);
    result << "Iteration ";
    result << "Y" << std::endl;
    _ihc_mutex_unlock(_m);
  }
  _ihc_mutex_delete(_m);
  //Expect 10 lines, one for each iteration
  EXPECT_EQ(uniqueEntries(result), 1);
}

void *mut;
void *cond;
int count = 0;

void *IHCWorker(void *param) {
  for (int i = 0; i < 10; ++i) {
    _ihc_mutex_lock(mut);
    if (count == 0)
      _ihc_cond_notify_one(cond);
    count++;
    result << "Produce ";
    result << "W" << std::endl;
    _ihc_mutex_unlock(mut);
  }
  return nullptr;
}

TEST(IHCSynchronization, ThreadedProgram) {
  cond = _ihc_cond_create();
  mut = _ihc_mutex_create();
  void *threadp = _ihc_pthread_create(&IHCWorker, nullptr);
  for (int i = 0; i < 10; ++i) {
    _ihc_mutex_lock(mut);
    while (count == 0)
      _ihc_cond_wait(mut, cond);
    count--;
    result << "Produce ";
    result << "Q" << std::endl;
    _ihc_mutex_unlock(mut);
  }
  _ihc_pthread_join(threadp);
  _ihc_mutex_delete(mut);
  _ihc_cond_delete(cond);
  // Expect two different patterns, one for each thread
  // Data races could make output interleved, resulting
  // in >2 unique patterns
  EXPECT_EQ(uniqueEntries(result), 2);
}
