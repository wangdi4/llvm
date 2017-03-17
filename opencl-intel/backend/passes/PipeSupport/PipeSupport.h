/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __PIPE_SUPPORT_H__
#define __PIPE_SUPPORT_H__

// This pass inserts __flush_read/write_pipe calls at the end of every
// functions, that use pipe built-in functions.
//
// This is important to support pipe BIs internal caching mechanism,
// because writes and reads are not immediately committed to the pipe,
// and we must flush them at exist to clear that cache.
//
// Currently it only supports pipes for FPGA emulation, so it works only
// with the Single Call Site restriction applied to the user program:
//   - all pipes used by the function must used only once
//   - pipe call may be in a loop, but this loop cannot be unrolled

#include <llvm/IR/Module.h>

namespace intel {

class PipeSupport : public llvm::ModulePass {
public:
  static char ID;
  PipeSupport();

  virtual const char *getPassName() const {
    return "PipeSupport";
  }

  bool runOnModule(llvm::Module &M);

  virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const;
};

} // namespace intel

#endif // __PIPE_SUPPORT_H__
