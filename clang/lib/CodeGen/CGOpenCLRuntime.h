//===----- CGOpenCLRuntime.h - Interface to OpenCL Runtimes -----*- C++ -*-===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This provides an abstract class for OpenCL code generation.  Concrete
// subclasses of this implement code generation for specific OpenCL
// runtime libraries.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_CODEGEN_CGOPENCLRUNTIME_H
#define LLVM_CLANG_LIB_CODEGEN_CGOPENCLRUNTIME_H

#include "clang/AST/Expr.h"
#include "clang/AST/Type.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"

namespace clang {

class BlockExpr;
class Expr;
class VarDecl;

namespace CodeGen {

class CodeGenFunction;
class CodeGenModule;

class CGOpenCLRuntime {
protected:
  CodeGenModule &CGM;
  llvm::Type *PipeROTy;
  llvm::Type *PipeWOTy;
#if INTEL_CUSTOMIZATION
  llvm::Type *ChannelTy;
#endif // INTEL_CUSTOMIZATION
  llvm::PointerType *SamplerTy;
  llvm::StringMap<llvm::PointerType *> CachedTys;

  /// Structure for enqueued block information.
  struct EnqueuedBlockInfo {
    llvm::Function *InvokeFunc; /// Block invoke function.
    llvm::Function *Kernel;     /// Enqueued block kernel.
    llvm::Value *BlockArg;      /// The first argument to enqueued block kernel.
  };
  /// Maps block expression to block information.
  llvm::DenseMap<const Expr *, EnqueuedBlockInfo> EnqueuedBlockMap;

  virtual llvm::Type *getPipeType(const PipeType *T, StringRef Name,
                                  llvm::Type *&PipeTy);
  llvm::PointerType *getPointerType(const Type *T, StringRef Name);

public:
#if INTEL_CUSTOMIZATION
  CGOpenCLRuntime(CodeGenModule &CGM)
      : CGM(CGM), PipeROTy(nullptr), PipeWOTy(nullptr),
        ChannelTy(nullptr), SamplerTy(nullptr) {}
#endif // INTEL_CUSTOMIZATION

  virtual ~CGOpenCLRuntime();

  /// Emit the IR required for a work-group-local variable declaration, and add
  /// an entry to CGF's LocalDeclMap for D.  The base class does this using
  /// CodeGenFunction::EmitStaticVarDecl to emit an internal global for D.
  virtual void EmitWorkGroupLocalVarDecl(CodeGenFunction &CGF,
                                         const VarDecl &D);

  virtual llvm::Type *convertOpenCLSpecificType(const Type *T);

  virtual llvm::Type *getPipeType(const PipeType *T);

  llvm::PointerType *getSamplerType(const Type *T);

  // Returns a value which indicates the size in bytes of the pipe
  // element.
  virtual llvm::Value *getPipeElemSize(const Expr *PipeArg);

  // Returns a value which indicates the alignment in bytes of the pipe
  // element.
  virtual llvm::Value *getPipeElemAlign(const Expr *PipeArg);

#if INTEL_CUSTOMIZATION
  virtual llvm::Type *getChannelType();

  // \brief Returns a value which indicates the size in bytes of the channel
  // element.
  virtual llvm::Value *getChannelElemSize(const Expr *ChannelArg);

  // \brief Returns a value which indicates the alignment in bytes of the
  // channel element.
  virtual llvm::Value *getChannelElemAlign(const Expr *ChannelArg);
#endif // INTEL_CUSTOMIZATION

  /// \return __generic void* type.
  llvm::PointerType *getGenericVoidPointerType();

  /// \return enqueued block information for enqueued block.
  EnqueuedBlockInfo emitOpenCLEnqueuedBlock(CodeGenFunction &CGF,
                                            const Expr *E);

  /// Record invoke function and block literal emitted during normal
  /// codegen for a block expression. The information is used by
  /// emitOpenCLEnqueuedBlock to emit wrapper kernel.
  ///
  /// \param InvokeF invoke function emitted for the block expression.
  /// \param Block block literal emitted for the block expression.
  void recordBlockInfo(const BlockExpr *E, llvm::Function *InvokeF,
                       llvm::Value *Block);

  /// \return LLVM block invoke function emitted for an expression derived from
  /// the block expression.
  llvm::Function *getInvokeFunction(const Expr *E);
};

}
}

#endif
