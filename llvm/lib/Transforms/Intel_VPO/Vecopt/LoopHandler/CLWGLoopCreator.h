/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __CL_WG_LOOP_CREATOR_H__
#define __CL_WG_LOOP_CREATOR_H__

#include "BuiltinLibInfo.h"
#include "OpenclRuntime.h"
#include "LoopUtils.h"
#include "llvm/Pass.h"
#include "llvm/IR/Instructions.h"
#include <set>

using namespace llvm;

namespace intel {

// class CLWGLoopCreator
//-----------------------------------------------------------------------------
// this pass takes opencl kernel and creates WGLoop around it according to 
// the early exit function created by the EarlyExitKernel.
// Incase the vector kernel exits it inlines it into the scalar kernel and
// create WG loops around the vector kernel and remainder loops around the 
// scalar kernel. WG loops are created with canonical induction variable 
// (satrts in 0 and incremented by 1) as it allows loop optimizations after
// (e.g. stream samplers).
//
// for example on the following kernel
//
//  void foo( .... ) {
//    scalar kernel code..
//
//  void __vectorized.foo( .... ) {
//    vector kernel code..
//
//  [7 x size_t] early.exit.foo( .... )
// 
// it will create the unified kernel:
//
// void foo( .... ) {
//   [7 z size_t] ee = early.exit.foo( .... )
//   size_t uni = ee[0]
//   if (!uni) return
// 
//   size_t dim0_init_gid = ee[1];
//   size_t dim0_size = ee[2];
//   size_t dim1_init_gid = ee[3];
//   size_t dim1_size = ee[4];
//   size_t dim2_init_gid = ee[5];
//   size_t dim2_size = ee[6];
//   size_t dim0_vector_size = dim0_size - dim0_size %packet_width
//   size_t dim0_scalar_size = dim0_size %packet_width
//   if (dim0_vector_size != 0) {
//      size_t dim2_ind_var = 0;
//      do{
//          size_t dim1_ind_var = 0;
//          do{
//              size_t dim0_ind_var = 0;
//              do{
//                  vector code
//              }while (++dim0_ind_var != dim0_vector_size);
//          }while (++dim1_ind_var != dim1_size);
//      }while (++dim2_ind_var != dim2_size);
//   }
//   if (dim0_scalar_size != 0) {
//      size_t dim2_ind_var = 0;
//      do{
//          size_t dim1_ind_var = 0;
//          do{
//              size_t dim0_ind_var = 0;
//              do{
//                  scalar code
//                  tid += packet_width              
//              }while (++dim0_ind_var != dim0_scalar_size);
//          }while (++dim1_ind_var != dim1_size);
//      }while (++dim2_ind_var != dim2_size);
//   }

class CLWGLoopCreator : public ModulePass {
public:
  ///@brief pass identifier.
  static char ID;

  ///@brief C'tor.
  CLWGLoopCreator();

  /// @brief D'tor
  ~CLWGLoopCreator();

  /// @brief Provides name of pass.
  virtual StringRef getPassName() const {
    return "CLWGLoopCreator";
  } 

  ///@brief public interface that allows running on pair of scalar - vector
  ///       kernels not through pass manager.
  bool runOnFunction(Function &F, Function *vectorFunc, unsigned packetWidth);

  ///@brief LLVM interface.
  virtual bool runOnModule(Module &M);

  ///@brief LLVM interface.
  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<BuiltinLibInfo>();
  };

private:
  
  ///@brief struct that contains dimesion 0 loop attributes.
  struct loopBoundaries {
    Value *m_vectorLoopSize;  // num vector loop iterations.
    Value *m_scalarLoopSize;  // num scalar loop iterations.
    Value *m_maxVector;       // max vector global id 

    ///@brief C'tor.
    loopBoundaries (Value *vectorLoopSize, Value *scalarLoopSize,
                    Value *maxVector) :
                m_vectorLoopSize(vectorLoopSize),
                m_scalarLoopSize(scalarLoopSize),
                m_maxVector(maxVector) {}
  };

  ///@brief computes the sizes of scalar and vector loops of the zero
  ///       dimension in case vector kernel exists.
  ///@param initVal - Initial global id of zero dimension.
  ///@param dimSize - Number of iteration of zero dimensions.
  ///@returns struct with the sizes of the vector and scalar loop + the initial
  ///         scalar loop global id.
  loopBoundaries getVectorLoopBoundaries(Value *initVal, Value *dimSize);
  
  ///@brief Generate useful constant values.
  void generateConstants();

  ///@brief calculate prefix for block names.
  void compute_dimStr(unsigned dim, bool isVector);

  ///@brief collect the get_global_id(), get_local_id(), and return of F.
  ///@param F - kernel to collect information for.
  ///@gids - array of get_global_id call to fill.
  ///@lids - array of get_local_id call to fill.
  ///returns kernel single return instruction.
  ReturnInst *getFunctionData(Function *F, IVecVec &gids, IVecVec &lids);


  ///@brief retruns single return instruction of F, if needed merge returns.
  ///@retruns as above.
  ReturnInst *getSingleRet(Function *F);

  ///@brief obtains the base global id for dimesion dim.
  ///@param dim - dimesion to get base global id for.
  ///@retruns - base global id value.
  Instruction *obtainBaseGID(unsigned dim);

  ///@brief add work group loops on the kernel. converts get_***_id according 
  ///       to the generated loops. Moves Alloca instruction in kernel entry 
  ///       block to the new entry block on the way.
  ///@param kernelEntry - entry block of the kernel.
  ///@param isVector - true iff working on vector kernel
  ///@param ret - singel return instruction of the kernel.
  ///@param GIDs - array with get_global_id calls.
  ///@param LIDs - array with get_local_id calls.
  ///@param initGIDs - initial global id per dimension.
  ///@param loopSizes - number of loop iteration per dimension.
  ///@retruns struct with preHeader and exit block of the outmost loop.
  loopRegion AddWGLoops(BasicBlock *kernelEntry, bool isVector, ReturnInst *ret,
      IVecVec &GIDs, IVecVec &LIDs, VVec &initGIDs, VVec &loopSizes);

  ///@brief inline the vector kernel into the scalar kernel.
  ///@param BB - location to move the vector blocks.
  ///@returns the vector kernel entry block.
  BasicBlock *inlineVectorFunction(BasicBlock *BB);

  ///@brief create a conditional jump over the entire wG loops according
  ///       to uniform early exit value.
  ///@param ret block - retrun block to jump to incase of uniform early exit.
  void handleUniformEE(BasicBlock *retBlock);
  
  ///@brief returns the early exit call.
  ///@returns as above.
  CallInst *createEECall();

  ///@brief moves alloca to new entry block.
  void moveAllocaToEntry(BasicBlock *BB);
  
  ///@brief obtain initial global id, and loop size per dimension.
  void getLoopsBoundaries();

  ///@brief create WG loops over vector kernel and remainder loop over scalar
  ///       kernel.
  ///@retruns a struct with entry and exit block of the WG loop region. 
  loopRegion createVectorAndRemainderLoops();

  ///@brief replace the get***tid calls with incremented phi in loop head.
  ///@param TIDs - array of get***id to replace.
  ///@param initVal - inital value (for the first iteration).
  ///@param incBy - amount by which to increase the tid in each iteration.
  ///@param head - head block of the loop.
  ///@param preHead - pre header of the loop.
  ///@param latch - latch block of the loop.
  void replaceTIDsWithPHI(IVec &TIDs, Value *initVal, Value *incBy,
                BasicBlock *head, BasicBlock *preHead, BasicBlock *latch);

  /// @brief calculate the number of dimensions for which we need to create
  ///         work group loops.
  /// @returns number of dimensions.
  unsigned computeNumDim();

  /// @brief returns the "true" dimension taking into account that
  /// the vectorized dimension might not be 0. If m_vectorizedDim
  /// is 0, then the returned value is always the same as dim.
  /// but suppose m_vectorizedDim is 1, then resolveDimension(1) = 0,
  /// and resolveDimension(0) = 1. Since now dim 1 is the innermost loop.
  unsigned int resolveDimension(unsigned int dim);

  ///@brief size_t type.
  Type *m_indTy;

  ///@brief size_t one constant.
  Constant *m_constZero;

  ///@brief size_t one constant.
  Constant *m_constOne;

  ///@brief size_t one constant.
  Constant *m_constPacket;

  ///@brief Function being processed.
  Function *m_F;

  ///@brief Module of function being processed.
  Module *m_M;

  ///@brief LLVM context of the current function.
  LLVMContext *m_context;

  ///@brief prefix for name of instructions used for loop of the dimension.
  std::string m_dimStr;

  ///@brief new entry block.
  BasicBlock *m_newEntry;

  ///@brief scalar kernel entry.
  BasicBlock *m_scalarEntry;

  ///@brief vectorized inner loop func.
  Function *m_vectorFunc;

  ///@brief vectorized function width.
  unsigned m_packetWidth;

  ///@brief number of WG dimensions.
  unsigned m_numDim;

  ///@brief runtime services.
  const OpenclRuntime* m_rtServices;

  ///@brief index i contains vector with scalar kernel get_global_id(i) calls.
  IVecVec m_gidCallsSc;

  ///@brief index i contains vector with scalar kernel get_local_id(i) calls.
  IVecVec m_lidCallsSc;
  
  ///@brief index i contains vector with vector kernel get_global_id(i) calls.
  IVecVec m_gidCallsVec;

  ///@brief index i contains vector with vector kernel get_local_id(i) calls.
  IVecVec m_lidCallsVec;

  ///@brief index i contains the base global id for dimension i.
  IVec m_baseGids;

  ///@brief scalar kernel return.
  ReturnInst *m_scalarRet;

  ///@brief vector kernel return.
  ReturnInst *m_vectorRet;
  
  ///@brief global_id lower bounds per dimension.
  VVec m_initGIDs;
  
  ///@brief m_loopSize per dimension.
  VVec m_loopSizes;

  ///@brief early exit call.
  CallInst *m_EECall;

  ///@brief the dimension by which we vectorize (usually 0).
  unsigned int m_vectorizedDim;
};// CLWGLoopCreator
} //namespace

#endif //__CL_WG_LOOP_CREATOR_H__
