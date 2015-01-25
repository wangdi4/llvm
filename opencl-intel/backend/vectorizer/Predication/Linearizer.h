/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef LINEARIZER_H
#define LINEARIZER_H
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Transforms/Scalar.h"

using namespace llvm;

namespace intel {

/// @brief Represents linearization scheduling constraints
///  All blocks in scope must be consecutive
class SchedulingScope {
private:
  typedef std::vector<BasicBlock*> BBVector;
  typedef std::vector<BasicBlock*>::iterator BBVectorIter;
  typedef std::vector<SchedulingScope*> SchedulingScopeSet;
  typedef std::vector<SchedulingScope*>::iterator SchedulingScopeSetIter;
public:
  /// @brief C'tor
  /// @param leader Leader of scope
  /// @param ucfScope true if this a scope with original control flow
  SchedulingScope(BasicBlock* leader, bool ucfScope = false);
  /// @brief D'tor
  /// @note the destructor deletes and frees the memory of all
  ///  sub-scopes
  ~SchedulingScope();
  /// @brief Checkes if this scope contains a basic block
  /// @param bb BB to search
  /// @return True if contains
  bool has(BasicBlock* bb);
  /// @brief Adds a basic block to this scope
  /// @param bb BasicBlock to add
  void addBasicBlock(BasicBlock* bb);
  /// @brief Checks if this scope is a subset of another scope
  /// @param scp Possibly containing scope
  /// @return True if this is a subset
  bool isSubsetOf(SchedulingScope* scp);
  /// @brief Add a subscope (smaller scope) as a child of this scope
  /// @param scp SchedulingScope to add
  void addSubSchedulingScope(SchedulingScope* scp);
  /// @brief Debug: verify the correctness of this scope
  void verify();
  /// @brief Debug: print structure of scope
  /// @param OS stream to print
  /// @param indent Indetation level for print
  void print(raw_ostream &OS, unsigned indent=0);
  /// @brief Check if any of the contained basic blocks
  ///  have users which are unscueduled and outside this scope
  /// @param schedule Already scheduled basic blocks
  /// @return True if has unscueduled preds
  bool hasUnscheduledPreds(const BBVector& schedule);
  /// @brief Utility function to check if a basic block has unsceduled preds
  /// @param schedule Scheduled basic blocks
  /// @param schedule Unscheduled basic blocks whcih are not in any of nested scopes
  /// @param bb Basic Block to check
  /// @return True if not ready for scheduling
  bool hasUnscheduledPreds(const BBVector& schedule,
                                  const BBVector& thisScopeOnly, BasicBlock* bb);
  /// @brief Schedule the content of this scope
  /// @param schedule Saves the calculated schedule
  void schedule(BBVector& schedule);
  /// @brief return the first block scheduled which is not part of this
  ///  scope
  /// @param schedule Schedule
  /// @return The first after or Null
  BasicBlock* getFirstBlockAfter(const BBVector& schedule);
private:
  /// @brief After adding a sub-scope, compress multiple scopes
  ///  to contain one-another
  /// TODO: this is inefficient, change this to only
  /// compress newly added blocks
  void compress();
  /// @brief Collect a list of all basic blocks which do not belong to
  ///  sub scopes.
  /// @param thisScopeOnly Set of un-scoped blocks
  void getNonSchedulingScopedInstructions(SchedulingScope::BBVector &thisScopeOnly);
private:
  /// the contained blocks
  BBVector m_blocks;
  /// the contained sub-scopes
  SchedulingScopeSet m_subscope;
  /// @brief Block Leader is the block
  /// which is scheduled first in case there are no dependencies
  /// outside the scope. This is used when we have loops or UCF scope
  BasicBlock* m_leader;
  /// @brief Identify UCF scheduling scope
  bool const m_isUCFScope;
};


}//namespace
#endif /* LINEARIZER_H */
