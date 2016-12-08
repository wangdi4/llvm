/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __CHANNEL_PIPE_TRANSFORMATION_H__
#define __CHANNEL_PIPE_TRANSFORMATION_H__

#include <llvm/IR/Module.h>

namespace intel {

class ChannelPipeTransformation : public llvm::ModulePass {
public:
  static char ID;
  ChannelPipeTransformation();

  virtual const char *getPassName() const {
    return "ChannelPipeTransformation";
  }

  bool runOnModule(llvm::Module &M);

  virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const;
};

} // namespace intel

#endif // __CHANNEL_PIPE_TRANSFORMATION_H__
