/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __PACKETIZER_H__
#define __PACKETIZER_H__

#include "BuiltinLibInfo.h"
#include "WIAnalysis.h"
#include "Mangler.h"
#include "SoaAllocaAnalysis.h"
#include "Logger.h"
#include "VectorizerCommon.h"
#include "OclTune.h"
#include "TargetArch.h"

#include "llvm/ADT/DenseSet.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/IR/DataLayout.h"

#include <string>
#include <sstream>

namespace intel {

/// Enumeration to represent memory operation type
enum MemoryOperationType {
  LOAD,
  STORE,
  PREFETCH
};
/// A struct to represent a memory operation such as
/// load/store or masked load/store.
struct MemoryOperation {
  // Memory operation fields
  Value *Mask;
  Value *Ptr;
  Value *Data;
  unsigned int Alignment;
  // If these values are set, then Base + Index = Ptr.
  Value *Base;
  Value *Index;
  // Information about above Index
  // Index can be signed/unsigned value
  // Index actual value is limitted to the range:
  // [0, 2^ValidBits] if unsigned
  // [-2^(ValidBits-1), 2^(ValidBits-1)-1] if signed
  bool IndexIsSigned;
  unsigned int IndexValidBits;
  // Original Instruction
  Instruction *Orig;
  MemoryOperationType type;
};

/// @brief Packetization pass used to for converting code in functions
///  which operate in the scope of a single work item (or similar),
///  To work on a "packet" of work-items. This is done by converting
///  all scalar operations to vectors, where each lane represents
///  data from a different work-item.
///  Code which cannot be packetized this way, is simply duplicated
///  for all packed work items.
class PacketizeFunction : public FunctionPass {

  /// @brief vector conversions map entry
  struct VCMEntry
  {
    Instruction *vectorValue;
    Instruction *multiScalarValues[MAX_PACKET_WIDTH];
    bool isScalarRemoved;
  };

public:

  static char ID; // Pass identification, replacement for typeid
  PacketizeFunction(Intel::ECPU Cpu = Intel::DEVICE_INVALID,
                    unsigned int vectorizationDimension=0);
  ~PacketizeFunction();

  /// @brief Provides name of pass
  virtual StringRef getPassName() const {
    return "PacketizeFunction";
  }

  virtual bool runOnFunction(Function &F);

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesCFG();
    AU.addRequired<WIAnalysis>();
    AU.addRequired<SoaAllocaAnalysis>();
    AU.addRequired<BuiltinLibInfo>();
  }

private:

  /// @brief main Method for packetizing instruction. A sort of dispatch table.
  /// @param I instruction to work on
  void dispatchInstructionToPacketize(Instruction *I);

  /// @brief Instructions which cannot be packetize thru a vector reach this function.
  /// @param I instruction to work on
  void duplicateNonPacketizableInst(Instruction *I);

  /// @brief Clone instructions which cannot be packetize.
  /// @param I instruction to clone
  /// @param duplicateInsts output cloned instruction in this buffer
  void cloneNonPacketizableInst(Instruction *I, Instruction **duplicateInsts);

  /// @brief Fix operand of load/store instructions derived from SOA-Alloca instruction.
  /// @param I instruction to fix its operand
  /// @param op index of operand to fix
  /// @param multiOperands input/output operands to fix
  void fixSoaAllocaLoadStoreOperands(Instruction *I, unsigned int op, Value **multiOperands);

  /*! \name Instruction Packetization Functions
   *  \{ */
  /// @brief Packetize an instruction
  /// @param I Instruction to packetize
  void packetizeInstruction(BinaryOperator *BI, bool supportsWrap);
  void packetizeInstruction(CastInst *CI);
  void packetizeInstruction(CmpInst *CI);
  void packetizeInstruction(CallInst *CI);
  void packetizedMemsetSoaAllocaDerivedInst(CallInst *CI);
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
  /// @param allUsersExtract - true if all users are extract instructions.
  /// @param attemptingLoadTranspose - true if called from
  /// ObtainLoadAndTranspose, used for statistical purposes.
  bool obtainExtracts(Value  *vectorValue,
                      SmallVectorImpl<ExtractElementInst *> &extracts,
                      bool &allUsersExtract,
                      bool attemptingLoadTranspose);

  ///@brief creates the transpose shuffle sequence for 4x4 matirx
  ///@param IN input vectors
  ///@param OUT output vectors
  ///@param generatedShuffles std vector with all generated shuffles
  void obtainTranspVals32bitV4(SmallVectorImpl<Value *> &IN,
                               SmallVectorImpl<Instruction *> &OUT,
                               std::vector<Instruction *> &generatedShuffles,
                               Instruction *loc);

  ///@brief creates the transpose shuffle sequence for 8x8 matirx
  ///@param IN input vectors
  ///@param OUT output vectors
  ///@param generatedShuffles std vector with all generated shuffles
  void obtainTranspVals32bitV8(SmallVectorImpl<Value *> &IN,
                               SmallVectorImpl<Instruction *> &OUT,
                               std::vector<Instruction *> &generatedShuffles,
                               Instruction *loc);

  ///@brief creates sequence to compute number of vector elements to prefetch in case of consecutive memory pattern access.
  ///@param scalarVal scalarValue to packetize
  ///@param I instruction which uses scalarValue (should be call of prefetch built-in)
  ///@return packetized scalar value.
  Value* obtainNumElemsForConsecutivePrefetch(Value* scalarVal, Instruction* I);

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

  // Transpose functions

  /// @brief Get appropriate transpose function
  /// @param isLoad True if this is load and transpose, false otherwise
  /// @param origVecType Vector type in the original instruction
  /// @param isScatterGather True if this is a scatter/gather, false if normal store/load
  /// @param isMasked True if this is a masked operation, false if a regular op.
  /// @return transpose function to be called
  Function* getTransposeFunc(bool isLoad, VectorType * origVecType, bool isScatterGather, bool isMasked);

  /// @brief Creates load and transpose sequence
  ///        This is the order of the sequence (first load, then transpose), hence the name.
  /// @param I Load instruction
  /// @param loadPtrVal The pointer address which is being loaded
  /// @param loadType The type of the value being loaded
  /// @param Mask Mask value if this is a masked load, null otherwise
  void createLoadAndTranspose(Instruction* I, Value* loadPtrVal, Type* loadType, Value* Mask);

  /// @brief Creates transpose and store sequence
  ///        This is the order of the sequence (first transpose, then store), hence the name.
  /// @param I Store instruction
  /// @param storePtrVal The pointer address which we are going to store
  /// @param storeType The type of the value being stored
  /// @param Mask Mask value if this is a masked load, null otherwise
  void createTransposeAndStore(Instruction* I, Value* storePtrVal, Type* storeType, Value* Mask);

  /// @brief Checks whether transpose is possible
  /// @param addr The address which is being loaded from/stored to as part of the transpose
  /// @param origVal The original value that was loaded/stored and needs to be transposed
  /// @param isLoad True if this is load and transpose, false otherwise
  /// @param isScatterGather True if this is a scatter/gather, false if normal store/load
  /// @param isMasked True if this is a masked operation, false if a regular op.
  /// @return True if this value can be transposed, False otherwise
  bool canTransposeMemory(Value* addr, Value* origVal, bool isLoad, bool isScatterGather, bool isMasked);

  /// @brief When obtaining a load-transpose instruction,
  /// we can only obtain it if all of its users are extractElement instructions.
  /// a PHI-Node user can ruin this optimization opportunity for no reason,
  /// so in this method we 'postpone' such phi-nodes, that is,
  /// move the extractElement instructions upwards, and replace
  /// the phi with (vector_width) scalar phis.
  void postponePHINodesAfterExtracts();

  /// @brief Obtains information about extracts and inserts that will need to be transposed
  /// This function has side effects through the m_storeTranspMap, m_loadTranspMap and
  /// m_removedInst fields.
  void obtainTranspose();

  /// @brief Obtains the extracts that may need to be transposed
  /// @param LI The load/masked load instruction which may need to perfrom load and transpose
  /// @param loadAdd The address being loaded
  /// @param masked True if this is a masked load, false if regular load
  void obtainLoadAndTranspose(Instruction* LI, Value* loadAdd, bool masked);

  /// @brief Obtains the inserts that may need to be transposed
  /// @param SI The store instruction which may need to perfrom transpose and store
  /// @param storeAdd The address stored to
  /// @param storeVal The value being stored
  /// @param masked True if this is a masked store, false if regular store
  void obtainTransposeAndStore(Instruction* SI, Value* storeAdd, Value* storeVal, bool masked);


  // Utility functions

  /// @brief checks whether IEI is the start vector assembly sequence to be transposed
  /// @param IEI canadidate to be the insertElement in a vector assemby sequence
  /// @param InsertEltSequence will hold the entire insert sequence according to index of inserted element
  /// @param AOSVectorWidth will hold the width of the assembled vector
  /// @param lastInChain will hold the last instruction in the def-use chain
  /// @param prePacketization true if this runs on unpacketized code, false otherwise
  /// @return true iff IEI is the start vector assembly sequence to be transposed
  bool obtainInsertElts(InsertElementInst *IEI, InsertElementInst** InsertEltSequence,
              unsigned &AOSVectorWidth, Instruction *& lastInChain, bool prePacketization = false);

  /// @brief generets shuffle sequeces that perform transpose and breakdown to scattered vectors
  /// @param loc location to put shuffle instructions
  /// @param inputVecotrs input vectored to be transposed
  /// @param transposedVectors will hold the transposed output vectors
  /// @param generatedShuffles std vector to hold all generated shuffles.
  void generateShuffles (unsigned AOSVectorWidth, Instruction *loc,
      Value *inputVectors[MAX_PACKET_WIDTH],  Instruction *transposedVectors[MAX_PACKET_WIDTH],
      std::vector<Instruction *> &generatedShuffles);

  /// @brief Generate a constant vector of values, to be used as ShuffleVector inputs
  /// @param width size of vector
  /// @param values values for the vector
  /// @return ConstantVector/ConstantDataVector with the provided indices
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

  /// @brief Check what needs to be done in order to obtain the vectorized value.
  /// If this requires creating multiple (vector_width) InsertElement instructions,
  /// returns true. Otherwise returns false.
  /// @param origValue the value that we might want to vectorize.
  bool isInsertNeededToObtainVectorizedValue(Value * origValue);

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
  Value *handleParamSOA(CallInst* CI, Value *scalarParam);

  /// @brief obtain the scalar elements of the vector param and add their
  ///        vectors to the arguments list.
  /// @param CI - scalar call.
  /// @scalarParam - current scalar param (of vector type)
  /// @LibFuncTy - type of packetized builtin.
  /// @param args - vector of arguments to packetized built-in.
  /// @returns true if successfully added vector arguments to args.
  bool spreadVectorParam(CallInst* CI, Value *scalarParam,
                         FunctionType *LibFuncTy, std::vector<Value *> &args);

  /// @brief handles case when scalar return value is vector and packetized
  ///        return value is SOA
  /// @param CI call instruction to be packetized
  /// @param soaRet return value (instruction Value) of packetized function
  /// @returns the parameter for packetized function
  bool handleReturnValueSOA (CallInst* CI, CallInst *soaRet);

  /// @brief handles case when scalar built-in returns a vector and the
  ///        packetized built-in has return values by pointers.
  /// @param CI - scalar call.
  /// @param newCall - packetized call.
  /// @returns true iff successfully handled return values.
  bool handleReturnByPointers (CallInst* CI, CallInst *newCall);

  /// @brief maps the obtained returned values of the packetized built-in
  ///        to their corresponding scalar in packetizer data structures.
  ///
  /// @param CI - scalar call which has fake extract usages.
  /// @returnedVals - returned values.
  void mapFakeExtractUsagesTo (CallInst* CI, SmallVectorImpl<Instruction *>& returnedVals);

  /// @brief Checks whether packetized loads from the address are scatter-gather
  /// @param Address Address to check.
  /// @returns true if a scatter/gather is needed, false otherwise.
  bool isScatterGatherAddr(Value* Address);

  /// @brief Check if we have gather/scatter implementation for given parameters.
  /// @param masked - true if gather/scatter is masked
  /// @param type   - operation type: gather or scatter
  /// @param VecTy  - vector type to gather or scatter
  /// @returns true if operation is available for VecTy.
  bool isGatherScatterType(bool masked,
                           Mangler::GatherScatterType type,
                           VectorType *VecTy);
  /// @brief Returns the mask type a transpose function expects
  /// @param TransFunc Transpose function to which the mask is passed
  /// @returns the type of the mask
  Type* getMaskTypeForTranpose(Function* TransFunc);

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

  // @brief pointer to Soa alloca analysis performed for this function
  SoaAllocaAnalysis *m_soaAllocaAnalysis;

  // Contains all the removed instructions. After packetizing completes, they are removed.
  DenseSet<Instruction *> m_removedInsts;

  /// @brief Per each value - map to vectorized and multi-scalar counterparts
  DenseMap<Value *, VCMEntry *> m_VCM;

  DenseMap<Instruction *, SmallVector<ExtractElementInst *, 16> > m_loadTranspMap;
  DenseMap<Instruction *, SmallVector<InsertElementInst *, 16> > m_storeTranspMap;

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

  /// @brief flag to enable prefetch gather/sctatters.
  bool UseScatterGatherPrefetch;

  /// @brief DataLayout of processed module
  const DataLayout *m_pDL;

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
  /// @brief the dimension by which we vectorize (usually 0).
  unsigned int m_vectorizedDim;
  // @brief target CPU
  Intel::ECPU m_Cpu;

  // Statistics:
  Statistic::ActiveStatsT m_kernelStats;
  Statistic GEP_With_2_Indices;
  Statistic GEP_With_More_Than_2_Indices;
  Statistic Array_Of_Structs_Store_Or_Loads;
  Statistic Cant_Load_Transpose_Because_Of_Non_Extract_Users;
  Statistic Cant_Load_Transpose_Because_Multiple_Extract_Users_With_The_Same_Index;
  Statistic Load_Transpose_Created_For_A_Single_Scalar_Value;
  Statistic Store_Transpose_Given_Up_Because_Of_Multiple_Users_Of_The_Stored_Vector;
  Statistic Store_Transpose_Given_Up_Due_To_Not_Supported_Vector_Size;
  Statistic Store_Transpose_Created_For_A_Single_Scalar;
  Statistic Insert_Element_Transpose_Given_Up_Due_To_Not_supported_Vector_Size;
  Statistic Transposing_ExtractElement_For_A_Single_Extract;
  Statistic Scalarize_An_Instruction_That_Does_Not_Have_Vector_Support;
  Statistic Scalarize_Memory_Operand_That_Does_Not_Have_Vector_Support;
  Statistic Wide_Unmasked_Memory_Operation_Created;
  Statistic Wide_Masked_Memory_Operation_Created;
  Statistic Gather_Scatter_Created;
  Statistic Scalarize_Memory_Operand_Because_Cant_Create_Gather_Scatter;
  Statistic Scalarize_Function_Call;
  Statistic Scalarize_ExtractElement_Because_Cant_Transpose;
  Statistic Created_Transpose_For_Insert_Element;
  Statistic Created_Transpose_For_Extract_Element;
  Statistic Created_Load_And_Transpose;
  Statistic Created_Transpose_And_Store;
};

} //namespace

#endif // __PACKETIZER_H__

