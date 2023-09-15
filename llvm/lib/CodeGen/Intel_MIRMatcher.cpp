//===- Intel_MIRMatcher.cpp - MIR pattern matcher -------------------------===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements a pattern matcher for MIR instruction graphs.
//
//===----------------------------------------------------------------------===//

#include "llvm/CodeGen/Intel_MIRMatcher.h"

namespace llvm {

bool mirmatch::MatchResult::setRegMapping(unsigned localId, unsigned reg)
{
  if (0 == localId)
    return true;  // localId of zero matches any register. Do not map.

  unsigned index = localId - 1;
  if (m_regMap.size() <= index)
    m_regMap.resize(index + 1, 0);

  if (0 == m_regMap[index]) {
    m_regMap[index] = reg;
    return true;
  } else
    return m_regMap[index] == reg;
}

unsigned mirmatch::MatchResult::reg(unsigned localId) const {
  if (0 == localId)
    return 0;

  unsigned index = localId - 1;
  if (index < m_regMap.size())
    return m_regMap[index];
  else
    return 0;
}

llvm::MachineInstr* mirmatch::MatchResult::instr(const void* instrId) const {
  for (auto& entry : m_instrMap)
    if (entry.m_key == instrId)
      return entry.m_value;
  return nullptr;
}

} // Close namespace llvm

// End Intel_MIRMatcher.cpp
