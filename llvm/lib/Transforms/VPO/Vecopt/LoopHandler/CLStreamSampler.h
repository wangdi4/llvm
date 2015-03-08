/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

// This pass is unique to apple since in volcano we currently do not have stream built-ins.
// The pass identifies transposed read image call with strided coordinates inside
// of loop with small enough trip count, and transform it into a call to stream sampler read
// before the loop (in the loop pre header) that fills the data into buffers and than read
// sequentially from these buffers inside the loop.
// Similarly the pass identifies transposed write image call with stride x coordinate and
// uniform y coordinate, and transforms it into sequential store to buffers inside the loop
// and storing the buffers into the image after the loop.
// Author: Ran Chachick.

#ifndef __CL_STREAM_SAMPLER_H_
#define __CL_STREAM_SAMPLER_H_

#include "BuiltinLibInfo.h"
#include "OpenclRuntime.h"
#include "LoopWIAnalysis.h"

#include "llvm/Analysis/LoopPass.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/Instructions.h"

using namespace llvm;
namespace intel {

class CLStreamSampler : public LoopPass {
public:
  ///@brief Pass identification.
  static char ID;

  /// @brief C'tor.
  CLStreamSampler();

  /// @brief destructor.
  ~CLStreamSampler() {}

  /// @brief LLVM interface.
  /// @param L - Loop to transform.
  /// @param LPM - Loop Pass manager (unused).
  /// @returns true if the pass made changes.
  virtual bool runOnLoop(Loop *L, LPPassManager &LPM);

  /// @brief LLVM interface.
  /// @param AU - usage of analysis.
  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<LoopWIAnalysis>();
      AU.addRequired<DominatorTree>();
      AU.addRequired<BuiltinLibInfo>();
      AU.setPreservesCFG();
  };

private:

  /// @brief struct that contains attributes of read image call.
  typedef struct TranspReadImgAttr {
    CallInst *m_call;
    Value *m_img;
    Value *m_sampler;
    SmallVector<Value *, 2> m_firstCoords;
    SmallVector<LoadInst *, 4> m_colors;
    unsigned m_width;
  } TranspReadImgAttr;

  /// @brief struct that contains attributes of write image call.
  typedef struct TranspWriteImgAttr {
    CallInst *m_call;
    Value *m_img;
    Value *m_firstXCoord;
    Value *m_yCoord;
    SmallVector<Value *, 4> m_colors;
    unsigned m_width;
  } TranspWriteImgAttr;

  /// @brief current module.
  Module *m_M;

   /// @breif pre header of the current loop.
  BasicBlock *m_preHeader;

  /// @brief latch of the current loop.
  BasicBlock *m_latch;

  /// @brief header of the current loop.
  BasicBlock *m_header;

  /// @brief current loop.
  Loop *m_curLoop;

  /// @brief dominator tree analysis.
  DominatorTree *m_DT;

  /// @brief Loop work item analysis.
  LoopWIAnalysis *m_WIAnalysis;

  /// @brief exit block of the loop.
  BasicBlock *m_exit;

  /// @brief LLVM context.
  LLVMContext *m_context;

  /// @brief loop trip count.
  Value *m_tripCount;

  /// @brief vecotr mapping width to an LLVM stream size(width * tripCount).
  SmallVector<std::pair<unsigned, Value *> , 2>  m_streamSize;

  /// @brief vecotr of blocks that execute exactly once per loop iteration.
  SmallVector<BasicBlock *, 4>  m_alwaysExecuteOnceBlocks;

  /// @brief i32 one constant.
  Constant *m_one;

  /// @brief i32 zero constant.
  Constant *m_zero;

  /// @brief induction variable.
  PHINode *m_indVar;

  /// @brief upper bound on loop trip count.
  unsigned m_tripCountUpperBound;

  /// @brief contains attributes of read image calls.
  SmallVector<TranspReadImgAttr, 2> m_readImageAttributes;

  /// @brief contains attributes of write image calls.
  SmallVector<TranspWriteImgAttr, 2> m_writeImageAttributes;

  /// @brief apple runtime services object.
  const OpenclRuntime *m_rtServices;

  DenseMap<Value *, Value *> m_firstIterVal;

  /// @brief collect attributes from read image call, if successful store
  ///        the attributes in m_readImageAttributes
  /// @param readImgCall - call to obtain attributes for.
  void CollectReadImgAttributes(CallInst *readImgCall);

  /// @brief collect attributes from read image call in the current loop.
  void getReadImgAttributes();

  /// @brief Return a Value* that represents the trip count.
  ///        Imported from LLVM 3.0 LoopInfo
  /// @param L - Loop to return the count for
  /// @return The Value representing the trip count (compared to the induction var)
  Value* getTripCountValue(Loop* L, PHINode* IV) const;
  
  /// @brief returns constant upper bound for the loop.
  /// @param tripCount - trip count of the loop.
  /// @retruns upper bound if poosible, 0 otherwise.
  unsigned getTripCountUpperBound(Value *tripCount);

  /// @brief Replace read image calls for which attributes were collected with
  ///        stream calls in the pre header.
  void hoistReadImgCalls();

  /// @brief hoist read image call for the given attibutes.
  /// @param attr - attribute of the current call.
  /// @param readStreamFunc - read stream function.
  void hoistReadImgCall(TranspReadImgAttr &attr, Function *readStreamFunc);

  /// @brief Replace write image calls for which attributes were collected with
  ///        stream calls in the exit block.
  void sinkWriteImgCalls();

  /// @brief collect attributes from write image call in the current loop.
  void getWriteImgAttributes();

  /// @brief get Function declaration according to runtime module function.
  /// @param LibFunc - runtime module function.
  /// @returns function declararion inside the current module.
  Function *getLibraryFunc(Function *LibFunc);

  /// @brief collect attributes from write image call, if successful store
  ///        the attributes in m_writeImageAttributes
  /// @param writeImgCall - call to obtain attributes for.
  void CollectWriteImgAttributes(CallInst *writeImgCall);

  /// @brief get stream size value for width.
  /// @param width - width to obtain stream size for.
  /// @returns the stream size.
  Value *getStreamSize(unsigned width);

  /// @brief retruns the invariant Y coord for stream write.
  /// @param v - write_image y coord.
  /// @retunrs stream write y  coord.
  Value *getStreamWriteYcoord(Value *v);

  /// @brief sink write image call for the given attibutes.
  /// @param attr - attribute of the current call.
  /// @param writeStreamFunc - write stream function.
  void sinkWriteImgCall(TranspWriteImgAttr &attr, Function *writeStreamFunc);

  /// @brief generate allocas and pointers for stream function, according to
  ///        trip count upper bound.
  /// @param width - of the stream call.
  /// @param colorAllocas - vector of alloca instructions of buffers to fill.
  /// @param colorStorage - vector of pointers to the buffers to fill.
  void generateAllocasForStream(unsigned width,
                               SmallVector<Instruction *, 4>& colorAllocas,
                               SmallVector<Instruction *, 4>& colorStorage);

  /// @brief utility function that removes phi nodes that became redundant 
  ///        because of replacing read\write image with stream calls.
  /// @param PN - phi node to remove if becam redundant.
  void removeRedundantPHI(PHINode *PN);

  /// @brief checks if v is a phi node strided value in the header block 
  ///        of the current loop.
  /// @param v - value to check
  /// @returns true iff v is strided in the header block.
  bool isHeaderPhiStrided(Value *v);

  /// @brief calculates the start value and the stride of the input coordinate.
  /// @param coord -coord to obtain start value and stride.
  /// @param loc - place to put intruction to calculate start \ stride.
  /// @returns pair with first element the start value, and second the stride.
  std::pair<Value *, Value *> createStartStride(Value *coord, Instruction *loc);

  Value *calcFirstIterValIfPossible(Instruction *I);

  Value *obtainInitialStridedVal(Instruction *I);
};
}

#endif //define __CL_STREAM_SAMPLER_H_
