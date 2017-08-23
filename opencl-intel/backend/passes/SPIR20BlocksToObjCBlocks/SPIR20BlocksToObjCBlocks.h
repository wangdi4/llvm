// Copyright (c) 2015 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#ifndef __SPIR20_BLOCKS_TO_OBJC_BLOCKS_H__
#define __SPIR20_BLOCKS_TO_OBJC_BLOCKS_H__

#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/Pass.h>

#include <set>

namespace intel {

// This pass converts SPIR 2.0 blocks to Objective-C blocks CPU BE is tuned to
// handle.
// For more information about SPIR 2.0 blocks see the provisional
// specification:
// https://cvs.khronos.org/svn/repos/OpenCL/trunk/Khronos/specs/spir20_spec-provisional.pdf
// For more information about Objective-C blocks see the following:
// http://clang.llvm.org/docs/Block-ABI-Apple.html
//
// Current implementation is limited by device execution scope. I.e. it handles
// "spir_block_bind" and "enqueue_kernel" calls only. If SPIR 2.0 blocks are
// used in another context it will fail.
class SPIR20BlocksToObjCBlocks : public llvm::ModulePass {
public:
  static char ID;

  SPIR20BlocksToObjCBlocks() : llvm::ModulePass(ID) {}

  bool runOnModule(llvm::Module &) final;

  virtual llvm::StringRef getPassName() const final {
    return "intel::SPIR20BlocksToObjCBlocks";
  }

private:
  /// Create Objective-C block descriptor type and initialize the pass.
  void initPass(llvm::Module &);

  /// Replace all GEPs to SPIR 2.0 block context with GEPs to ObjectiveC block
  /// context.
  /*!
    \param base pointer to SPIR 2.0 block context
    \param base pointer to ObjectiveC block context
   */
  void replaceGEPs(llvm::Value *, llvm::Value *);

  /// Create and initialize a specific Objective-C block using call to
  /// "spir_block_bind" to gather necessary data.
  /*!
    \param module
    \param call instruciton to "spir_block_bind"
   */
  llvm::Value *getOrCreateObjCBlock(llvm::Module &, llvm::CallInst *);

  /// SPIR 2.0 block invoke functions expects only captured data while
  /// Objecitve-C blocks expects pointer to the block. So, the invoke functions
  /// should be fixed accordingly.
  /*!
    \param module
    \param pointer to invoke function
    \param type of ObjectiveC block
   */
  void fixBlockInvoke(llvm::Module &, llvm::Value *, llvm::StructType *);

  enum { ObjCBlockElementsNumWithoutContext = 5 };

  llvm::StructType *m_objcBlockDescrTy;
  llvm::SmallVector<llvm::Type *, ObjCBlockElementsNumWithoutContext>
      m_objcBlockContextElements;
  llvm::Type *m_int64Ty;
  llvm::Type *m_int32Ty;

  std::map<llvm::Value *, llvm::Value *> m_objcBlocks;
};

} // namespace intel

#endif // __SPIR20_BLOCKS_TO_OBJC_BLOCKS_H__
