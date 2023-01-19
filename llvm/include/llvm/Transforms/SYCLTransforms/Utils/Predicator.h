//===- Predicator.h - Volcano predicator utility ----------------*- C++ -*-===//
//
// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//===----------------------------------------------------------------------===//
//
// This file contains predicator utility function from volcano vectorizer. They
// are used in WeightedInstCountAnalysis. We may consider port them to support
// VPlan vectorizer.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_UTILS_PREDICATOR_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_UTILS_PREDICATOR_H

#include "llvm/ADT/StringRef.h"

namespace llvm {
class BasicBlock;
class BranchInst;

namespace Predicator {
/// type of the block inside the allones structure:
// If not a single loop block, then one of:
//
//              entry
//             .     .
//            .       .
//       original    allones
//            .       .
//             .     .
//              exit
//
//
// If a single loop block, then one of:
//             entry
//               .         .
// all-ones -> all-ones      .
//             . .           .
//          test all zeroes  .
//          .    .          .
//         .   entry2      <- entry
//         .     .
//         .  original  <- original
//          .    .   .
//            . exit
enum AllOnesBlockType {
  /// Block not related to an allone bypass.
  NONE = 0,
  ENTRY = 1,
  ORIGINAL = 2,
  ALLONES = 3,
  EXIT = 4,
  SINGLE_BLOCK_LOOP_ENTRY = 5,
  SINGLE_BLOCK_LOOP_ALLONES = 6,
  SINGLE_BLOCK_LOOP_TEST_ALLZEROES = 7,
  SINGLE_BLOCK_LOOP_ENTRY_TO_ORIGINAL = 8,
  SINGLE_BLOCK_LOOP_ORIGINAL = 9,
  SINGLE_BLOCK_LOOP_EXIT = 10
};

// Volcano allOne.
bool isAllOne(StringRef Name);

// Volcano allZero.
bool isAllZero(StringRef Name);

// Volcano vectorizer call.
bool isMangledCall(StringRef Name);

/// @brief If this blocks ends with a conditional branch,
/// and the condition is a call instruction to the allones function,
/// then this method returns this conditional branch.
/// otherwise returns NULL.
/// @param BB
BranchInst *getAllOnesBranch(BasicBlock *BB);

/// @brief If this block is part of an allones bypass,
/// returns the specific AllOnesBlock type. Otherwise
/// returns AllOnesBlock::NONE
/// @param BB
AllOnesBlockType getAllOnesBlockType(BasicBlock *BB);

/// @brief expects a block that is a single loop block and that has an 'allone'
/// single loop block twin. Returns the twin.
/// @param OriginalSingleLoop A block that is both a header and a latch of a
/// loop. This block should be predicated, but should also have an
/// allones uniform twin.
BasicBlock *getAllOnesSingleLoopBlock(BasicBlock *OriginalSingleLoop);

/// @brief expects a block that is of AllOnesBlockType::ORIGINAL type.
/// returns the entry predecessor.
/// @param original A predicated block that was bypassed with an allones
/// bypass.
BasicBlock *getEntryBlockFromOriginal(BasicBlock *Original);

/// @brief expects a block that is of
/// AllOnesBlockType::SINGLE_BLOCK_LOOP_ORIGINAL type.
/// returns the corresponding SINGLE_BLOCK_LOOP_ENTRY.
/// @param LoopOriginal A block that is both a header and a latch of a
/// loop. This block should be predicated, but should also have an
/// allones uniform twin.
BasicBlock *getEntryBlockFromLoopOriginal(BasicBlock *LoopOriginal);
} // namespace Predicator
} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_UTILS_PREDICATOR_H
