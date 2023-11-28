//===- CSAAsmWrapOstream.h - Assembly-wrapping ostream ----------*- C++ -*-===//
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
// This file defines a raw_ostream adapter which supports transparently
// string-wrapping assembly to make it survive offload.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_CSAASMWRAPOSTREAM_H
#define LLVM_LIB_TARGET_CSA_CSAASMWRAPOSTREAM_H

#include <memory>

namespace llvm {

class MCStreamer;
class raw_ostream;
class formatted_raw_ostream;

/// Starts wrapping assembly for CSA offloading, writing a '"' and entering
/// string escaping mode.
void startCSAAsmString(MCStreamer &OutStreamer);

/// Stops wrapping assembly for CSA offloading, exiting string escaping mode and
/// writing a '"'.
void endCSAAsmString(MCStreamer &OutStreamer);

/// Wraps an output stream to enable asm wrapping.
std::unique_ptr<formatted_raw_ostream>
wrapStreamForCSAAsmWrapping(raw_ostream &);

} // end namespace llvm

#endif // LLVM_LIB_TARGET_CSA_CSAASMWRAPOSTREAM_H
