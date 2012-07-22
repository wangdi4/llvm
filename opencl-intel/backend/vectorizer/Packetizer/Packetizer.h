/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#ifndef __PACKETIZER_H__
#define __PACKETIZER_H__

#include <string>
#include <sstream>
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "RuntimeServices.h"
#include "WIAnalysis.h"
#include "Logger.h"
#include "VectorizerCommon.h"


static const int __logs_vals[] = {-1, 0, 1, -1, 2, -1, -1, -1, 3, -1, -1, -1, -1, -1, -1, -1, 4};
#define LOG_(x) __logs_vals[x]



namespace intel {


/// A struct to represent a memory operation such as
/// load/store or masked load/store. 
struct MemoryOperation {
  // Memory operation fields
  Value *Mask;
  Value *Ptr;
  Value *Data;
  unsigned Alignment;
  // If these values are set, then Base + Index = Ptr.
  Value *Base;
  Value *Index;
  // Original Instruction
  Instruction *Orig;
};

/// @brief Packetization pass used to for converting code in functions
///  which operate in the scope of a single work item (or similar),
///  To work on a "packet" of work-items. This is done by converting
///  all scalar operations to vectors, where each lane represents
///  data from a different work-item.
///  Code which cannot be packetized this way, is simply duplicated
///  for all packed work items.
///  @Author: Sion Berkowits
class PacketizeFunction : public FunctionPass {

  /// @brief vector conversions map entry
  typedef struct VCMEntry
  {
    Instruction *vectorValue;
    Instruction *multiScalarValues[MAX_PACKET_WIDTH];
    bool isScalarRemoved;
  } VCMEntry;

public:

  static char ID; // Pass identification, replacement for typeid
  PacketizeFunction(bool supportScatterGather = false);
  ~PacketizeFunction();

  /// @brief Provides name of pass
  virtual const char *getPassName() const {
    return "PacketizeFunction";
  }

  virtual bool runOnFunction(Function &F);

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesCFG();
    AU.addRequired<WIAnalysis>();
  }

private:

  /// @brief main Method for packetizing instruction. A sort of dispatch table.
  /// @param I instruction to work on
  void dispatchInstructionToPacketize(Instruction *I);

  /// @brief Instructions which cannot be packetize thru a vector reach this function.
  /// @param I instruction to work on
  void duplicateNonPacketizableInst(Instruction *I);


  /*! \name Instruction Packetization Functions
   *  \{ */
  /// @brief Packetize an instruction
  /// @param I Instruction to packetize
  void packetizeInstruction(BinaryOperator *BI, bool supportsWrap);
  void packetizeInstruction(CastInst *CI);
  void packetizeInstruction(CmpInst *CI);
  void packetizeInstruction(CallInst *CI);
  void packetizeInstruction(InsertElementInst *IEI);
  void packetizeInstruction(ExtractElementInst *EI);
  void packetizeInstruction(SelectInst *SI);
  void packetizeInstruction(AllocaInst *AI);
  void packetizeInstruction(LoadInst *LI);
  void packetizeInstruction(StoreInst *SI);
  void packetizeInstruction(GetElementPtrInst *GI);
  void packetizeInstruction(BranchInst *BI);
  void packetizeInstruction(PHINode *PI);
  void packetizeInstruction(ReturnInst *RI);

  /// @brief obtain the extract users of vector value into extracts vector.
  /// @param vectorValue the vector to obtain extract users
  /// @param extracts - container to fill.
  bool obtainExtracts(Value  *vectorValue,
                      SmallVectorImpl<ExtractElementInst *> &extracts);

  ///@brief creates the transpose shuffle sequence for 4x4 matirx
  ///@param IN input vectors
  ///@param OUT output vectors
  void obtainTranspVals32bitV4(SmallVectorImpl<Value *> &IN, 
                 	           SmallVectorImpl<Instruction *> &OUT,
                 	           Instruction *loc);
  
  ///@brief creates the transpose shuffle sequence for 8x8 matirx
  ///@param IN input vectors
  ///@param OUT output vectors
  void obtainTranspVals32bitV8(SmallVectorImpl<Value *> &IN,
                               SmallVectorImpl<Instruction *> &OUT,
                               Instruction *loc);

  // Packetize load/store family of functions

  /// @brief Fill the MemoryOperation Index, Base fieds in case the 
  ///        pointer can be represented as ptr = base + pointer.
  /// @param MO - memory operation to check.
  void obtainBaseIndex(MemoryOperation &MO);


  /// @brief handles cases of consecutive (masked, unmasked) load\store.
  /// @param MO - memory operation to handle.
  /// @returns widen memory operation if successful, NULL otherwise.
  Instruction *widenConsecutiveMemOp(MemoryOperation &MO);

  /// @brief handles packetization of memory ops ([masked] load, store).
  /// @brief param MemOp - memory operation to handle.
  void packetizeMemoryOperand(MemoryOperation &MemOp);

  /// @brief Widen a load/store instruction when no mask is used
  /// @param MemOp Memory Operand
  /// @return New Widened instruction
  Instruction* widenConsecutiveUnmaskedMemOp(MemoryOperation &MemOp);

  /// @brief Widen a masked load/store instruction.
  /// @param MemOp Memory Operand
  /// @return New Widened instruction.
  Instruction* widenConsecutiveMaskedMemOp(MemoryOperation &MemOp);

  /// @brief Widen a masked load/store instruction into a scatter/gather call
  /// @param MemOp Memory Operand
  /// @return New Widened instruction (NULL if duplicated inside)
  Instruction* widenScatterGatherOp(MemoryOperation &MemOp);
  /*! \} */

  // Utility functions

  /// @brief checks whether IEI is the start vector assembly sequence to bo scattered
  /// @param IEI canadidate to be the insertElement in a vector assemby sequence
  /// @param InsertEltSequence will hold the entire insert sequence according to index of inserted element
  /// @param AOSVectorWidth will hold the width of the assembled vector
  /// @param lastInChain will hold the last instruction in the def-use chain
  /// @return true iff IEI is the start vector assembly sequence to bo scattered
  bool isScatter(InsertElementInst *IEI, InsertElementInst** InsertEltSequence,
              unsigned &AOSVectorWidth, Instruction *& lastInChain);

  /// @brief generets shuffle sequeces that perform transpose and breakdown to scattered vectors
  /// @param loc location to put shuffle instructions
  /// @param inputVecotrs input vectored to be transposed 
  /// @param transposedVectors will hold the transposed output vectors
  void generateShuffles (unsigned AOSVectorWidth, Instruction *loc,
      Value *inputVectors[MAX_PACKET_WIDTH],  Instruction *transposedVectors[MAX_PACKET_WIDTH]);

  /// @brief Generate a constant vector of values, to be used as ShuffleVector inputs
  /// @param width size of vector
  /// @param values values for the vector
  /// @return ConstantVector with the provided indices
  Constant * createIndicesForShuffles(unsigned width, int * values);

  /// @brief generate instructions for creating vectored indices (x, x+1, x+2, x+3)
  ///  Original instruction remains, and is followed by a sequence generating
  ///  a sequential vector
  /// @param I instruction to work on
  void generateSequentialIndices(Instruction * I);

  /// @brief Given a value, find the instruction after which this value is declared,
  ///  and further instructions can be placed. This is an "issue" when the location
  ///  found is in the "middle" of PHI instructions
  /// @param val value to work on
  /// @return Instruction to be added after
  Instruction * findInsertPoint(Value * val);

  /// @brief Provides vectorized values, to be used as inputs to "currently"
  ///  converted instructions. If value requires preparation (found, but not
  ///  vectorized) - prepare it first. If value is not found - it is in a
  ///  following block. Provide a dummy value and mark for deferred resolution
  /// @param retValue place vector value in there
  /// @param origValue Value which is searched
  /// @param origInst Instruction which is currently being worked on
  void obtainVectorizedValue(Value **retValue, Value * origValue, Instruction * origInst);

  /// @brief Provides multiple scalar values to be used as inputs to "currently"
  ///  converted instruction (as that inst is not vectorizable). If value is not
  ///  an instruction - use "as is" for all instances . If value has the original
  ///  instruction not removed - use it for all instances. If value only exists in
  ///  vector form - break it down to scalar elements for usage.
  /// @param retValues Array to place multi-scalar values in
  /// @param origValue Value which is searched
  /// @param origInst Instruction which is currently being worked on
  void obtainMultiScalarValues(Value * retValues[], Value * origValue, Instruction * origInst);

  /// @brief Use the existing instruction without modifying it
  /// @param origInst instruction to work on
  void useOriginalConstantInstruction(Instruction * origInst);


  // DRL-related functions

  /// @brief for Deferred resolution: Create dummy vector values (to be
  ///  replaced by "real" values after function vectorizing is complete)
  void createDummyVectorVal(Value *origValue, Value **vectorVal);

  /// @brief for Deferred resolution: Create dummy scalar values (to be
  ///  replaced by "real" values after function vectorizing is complete)
  void createDummyMultiScalarVals(Value *origValue, Value *multiScalarVals[]);

  /// @brief DRL (Deferred resolution handling). Run after function vectorization
  ///  completes, and replace all dummy values with real (now vectorized) values
  bool resolveDeferredInstructions();


  // VCM-related functions

  /// @brief Create VCM entry and fill it with vectored values (original is removed)
  void createVCMEntryWithVectorValue(Instruction *origInst, Instruction *vectoredValue);

  /// @brief Create VCM entry and fill it with multi-scalar values (original is removed)
  void createVCMEntryWithMultiScalarValues(Instruction * origInst,
                                           Instruction * multiScalarValues[]);

  /// @brief Allocate a clean VCM entry
  VCMEntry * allocateNewVCMEntry();

  /// @brief Release all allocations of VCM entries
  void releaseAllVCMEntries();

  
  /// @brief updating the VCM with return value packetized call instruction
  /// @param CI - original call
  /// @param newCall - new packetized call
  /// @return true if packteization succeded and types match
  bool handleCallReturn(CallInst *CI, CallInst * newCall);

  /// @brief obtaining argumetns for packetization of call instruction from VCM
  /// @param CI - original call
  /// @param LibFunc - new function to be called as packetized call
  /// @param isMangled - true if origianl call is masked
  /// @param isMaskedFunctionCall - true if LibFunc is masked call
  /// @param newArgs - vector of arguments to fill
  /// @return true if all arguments were succesfully obtained
  bool obtainNewCallArgs(CallInst *CI, const Function *LibFunc, bool isMangled,
               bool isMaskedFunctionCall, std::vector<Value *>& newArgs);
 
  /// @brief Find the insertElement roots of packetized vector type.
  //  Fill an array of the newly packetized values.
  //  We use the obtainVectorizedValue to convert each of the values.
  /// @param val Initial insertElement to search.
  /// @param roots Small vector used for returning the roots
  /// @param nElts Number of items to search (N)
  /// @param place Location obtainVectorizedValue uses to place new vects
  /// @return True if was able to find N roots.
  bool obtainInsertElement(Value* val, SmallVectorImpl<Value *> &roots,
                           unsigned nElts, Instruction* place);

  /// @brief handles case when scalar param is vector, packetized param is SOA
  /// @param CI call instruction to be packetized
  /// @param scalarParam parameter of scalar function
  /// @returns the parameter for packetized function
  Value *HandleParamSOA(CallInst* CI, Value *scalarParam);

  /// @brief obtain the scalar elements of the vector param and add their 
  ///        vectors to the arguments list.
  /// @param CI - scalar call.
  /// @scalarParam - current scalar param (of vector type)
  /// @LibFuncTy - type of packetized builtin.
  /// @param args - vector of arguments to packetized built-in.
  /// @returns true if successfully added vector arguments to args.
  bool SpreadVectorParam(CallInst* CI, Value *scalarParam,
                         FunctionType *LibFuncTy, std::vector<Value *> &args);

  /// @brief handles case when scalar return value is vector and packetized
  ///        return value is SOA
  /// @param CI call instruction to be packetized
  /// @param soaRet return value (instruction Value) of packetized function
  /// @returns the parameter for packetized function
  bool HandleReturnValueSOA (CallInst* CI, CallInst *soaRet);

  /// @brief handles case when scalar built-in returns a vector and the 
  ///        packetized built-in has return values by pointers.
  /// @param CI - scalar call.
  /// @param newCall - packetized call.
  /// @returns true iff successfully handled return values.
  bool HandleReturnByPointers (CallInst* CI, CallInst *newCall);

  /// @brief maps the obtained returned values of the packetized built-in
  ///        to their corresponding scalar in packetizer data structures.
  /// @param CI - scalar call.
  /// @returnedVals - returned values.
  void MapVectorMultiReturn (CallInst* CI, SmallVectorImpl<Instruction *>& returnedVals);

  // Pointer to runtime service object
  const RuntimeServices * m_rtServices;

  /// @brief Pointer to current function
  Function * m_currFunc;

  /// @brief Width of Function Packetization
  unsigned m_packetWidth;

  // Pointer to current context
  LLVMContext * m_moduleContext;
  LLVMContext& context() {return *m_moduleContext;}

  // @brief pointer to work-item analysis performed for this function
  WIAnalysis *m_depAnalysis;

  // Contains all the removed instructions. After packetizing completes, they are removed.
  SmallPtrSet<Instruction *, ESTIMATED_INST_NUM> m_removedInsts;

  /// @brief Per each value - map to vectorized and multi-scalar counterparts
  DenseMap<Value *, VCMEntry *> m_VCM;

  /// @brief An array of available VCMEntry's
  VCMEntry * m_VCMAllocationArray;

  /// @brief Index, in "VCMAllocationArray", of next free VCMEntry
  unsigned m_VCMArrayLocation;

  /// @brief Vector containing all the "VCMAllocationArray" arrays which were allocated
  SmallVector<VCMEntry *, 4> m_VCMArrays;

  /// @brief map of all dummy values used (to be resolved during DRL)
  DenseMap<Value *, VCMEntry *> m_deferredResMap;
  std::vector<Value *> m_deferredResOrder;

  /// @brief BAG (Broadcast Arguments and Globals) map. Map to broadcasted vals.
  DenseMap<Value *, Value *> m_BAG;

  /// @brief maximum log buffere size.
  static const unsigned int MaxLogBufferSize;

  /// @brief flag to enable scatter/gather to/from memory.
  bool UseScatterGather;

  /// @brief counter of cases when packetizing is cancelled because of shufflevector instruction
  int m_shuffleCtr;
  /// @brief counter of cases when packetizing is cancelled because of extractvalue instruction
  int m_extractCtr;
  /// @brief counter of cases when packetizing is cancelled because of insertvalue instruction
  int m_insertCtr;
  /// @brief counter of cases when packetizing is cancelled because of getelementpointer instruction
  int m_getElemPtrCtr;
  /// @brief counter of cases when packetizing is cancelled because of alloca instruction
  int m_allocaCtr;
  /// @brief counter of cases when packetizing is cancelled because of non-primitive type in an instruction
  int m_nonPrimitiveCtr[Instruction::OtherOpsEnd];
  /// @brief counter of cases when packetizing is cancelled because of non-vectorizable cast instruction
  int m_castCtr[Instruction::OtherOpsEnd];
  /// @brief counter of cases when packetizing is cancelled because of non-consecutive indices
  int m_nonConsecCtr;
  /// @brief counter of cases when packetizing is cancelled because of unavailability of vectorized version of a function
  int m_noVectorFuncCtr;
  /// @brief counter of unsupported corner cases when packetizing is cancelled 
  int m_cannotHandleCtr;
  
};

} //namespace

#endif // __PACKETIZER_H__

