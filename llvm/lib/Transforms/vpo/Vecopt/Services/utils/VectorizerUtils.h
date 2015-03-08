/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __VECTORIZER_UTILS_H__
#define __VECTORIZER_UTILS_H__

#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include <vector>

namespace intel {

class VectorizerUtils {

public:

  typedef std::vector<std::string> DeclaredVariants;
  typedef std::map<llvm::Function*, DeclaredVariants> FunctionVariants;

  /// @brief Get all functions marked for vectorization in module.
  /// @param M Module to query
  /// @param funcVars Data structure to hold the declared vector variants
  /// (in string form) for each function.
  static void getFunctionsToVectorize(llvm::Module &M,
				      FunctionVariants& funcVars);

  /// @brief set debug location of I according to SetBy.
  /// @param I instruction whose debug location will be set.
  /// @param setBy instruction from which debug location is gathered.
  static void SetDebugLocBy(llvm::Instruction *I, const llvm::Instruction *setBy);

  /// @brief set debug location of I according to SetBy.
  /// @param I instruction whose debug location will be set.
  /// @param setBy instruction from which debug location is gathered.
  static void SetDebugLocBy(std::vector<llvm::Instruction *> &insts, llvm::Instruction *setBy);

  /// @brief Follow thru a function input argument, until finding the root
  //   (where its type matches the "expected" type)
  /// @param arg The actual argument of the CALL inst
  /// @param rootType The desired (or "real") type to find
  /// @param CI The CALL instruction
  /// @return The root value if found, or NULL otherwise
  static llvm::Value *RootInputArgument(llvm::Value *arg, llvm::Type *rootType, llvm::CallInst *CI);

  /// @brief Same as above, but derive the type from the mangled call name
  /// @param arg The actual argument of the call inst
  /// @param paramNum The argument number in the call
  /// @param CI The CALL instruction
  /// @return The root value if found, or NULL otherwise
  static llvm::Value *RootInputArgumentBySignature(llvm::Value *arg, unsigned int paramNum, llvm::CallInst *CI);

  /// @brief Follow thru a function return argument, until its type matches the "expected" type
  /// @param retVal The actual returned value of the CALL inst
  /// @param rootType The desired (or "real") type to find
  /// @param CI The CALL instruction
  /// @return The "proper" retval if found, or NULL otherwise
  static llvm::Value *RootReturnValue(llvm::Value *retVal, llvm::Type *rootType, llvm::CallInst *CI);

  /// @brief casts value into desired type if types differ
  /// @param inputVal Value to be casted
  /// @param targetType "actual" desired type according to calling convention
  /// @param insertPoint instruction to insert before
  /// @return casted value
  static llvm::Value *getCastedArgIfNeeded(llvm::Value *inputVal, llvm::Type *targetType, llvm::Instruction *insertPoint);

  /// @brief casts instruction value into desired type if types are not equal
  /// @param I instruction to cast
  /// @param targetType "actual" desired type
  /// @return casted value
  static llvm::Instruction *getCastedRetIfNeeded(llvm::Instruction *I, llvm::Type *targetType);

  /// @brief Creates a call to function "name" with given args. It also creates
  ///       the function declaration if not already exists in the module.
  /// @param pModule module that contains the function declaration
  /// @param name function name
  /// @param retType function return type
  /// @param args list of arguments to call the function with
  /// @param insertBefore instruction to insert new callInst before
  /// @return new call instruction
  static llvm::CallInst *createFunctionCall(llvm::Module *pModule, const std::string &name,
    llvm::Type *retType, const llvm::SmallVectorImpl<llvm::Value *> &args, const llvm::SmallVectorImpl<llvm::Attribute::AttrKind>& attrs, llvm::Instruction* insertBefore);

  /// @brief Creates a broadcast sequance (InsertElement + Shuffle)
  /// @param pVal value to prodcast
  /// @param packetWidth width of generated vector with broadcast value
  /// @param whereTo instruction to insert new instructions before or after
  /// @param insertAfter if true, insert after whereTo instruction, otherwise insert before it.
  /// @return new broadcast vector
  static llvm::Instruction *createBroadcast(llvm::Value * pVal, unsigned int packetWidth, llvm::Instruction* whereTo, bool insertAfter = false);

  /// @brief Creates a consecutive vector (<pVal, pVal, ...> + <0, 1, ... width>)
  /// @param pVal value to prodcast
  /// @param packetWidth width of generated vector with broadcast value
  /// @param whereTo instruction to insert new instructions before or after
  /// @param insertAfter if true, insert after whereTo instruction, otherwise insert before it.
  /// @return new consecutive vector
  static llvm::Instruction *createConsecutiveVector(llvm::Value *pVal, unsigned int width, llvm::Instruction* whereTo, bool insertAfter = false);

  /// @brief Calculate BSR - (bit set reverse order) if argument is not zero
  ///  it is equivalent to count-leading-zeroes + 1. Mathematically, it computes floor(log(x)).
  /// @param number given number
  /// @return index [1-64] of highest bit set to 1 in given number
  ///         returns zero for input zero
  static unsigned int getBSR(uint64_t number);

  /// @brief Calculate LOG - Assumes number is a power of 2.
  /// @param number given number
  /// @return index [1-64] of highest bit set to 1 in given number.
  static unsigned int getLOG(uint64_t number);

  /// @brief checks if both types are pointer to opaque types
  /// @param x type 1
  /// @param y type 2
  static bool isOpaquePtrPair(llvm::Type *x, llvm::Type *y);

  /// @brief convert given type (of pointer to arrays of scalar/vector)
  ///        to type (of pointer to arrays of vector/scalar).
  /// @param type original type
  /// @param width if 0 convert original type to scalar type,
  ///        otherwise packetize original type according to width.
  static llvm::Type* convertSoaAllocaType(llvm::Type *type, unsigned int width);

  /// @brief Generate type-conversion and place in given location
  ///  but on debug accpets only cases size(orig) >= size(target)
  /// @param orig Source value
  /// @param targetType Target type to convert to
  /// @param insertPoint instruction to insert before
  static llvm::Instruction *ExtendValToType(llvm::Value *orig, llvm::Type *targetType, llvm::Instruction *insertPoint);

  /// @brief Generate type-conversion and place in given location
  ///  but on debug accpets only cases size(orig) <= size(target)
  /// @param orig Source value
  /// @param targetType Target type to convert to
  /// @param insertPoint instruction to insert before
  static llvm::Instruction *TruncValToType(llvm::Value *orig, llvm::Type *targetType, llvm::Instruction *insertPoint);

private:

  /// @brief Generate type-conversion and place in given location
  /// @param orig Source value
  /// @param targetType Target type to convert to
  /// @param insertPoint instruction to insert before
  static llvm::Instruction *BitCastValToType(llvm::Value *orig, llvm::Type *targetType, llvm::Instruction *insertPoint);

  /// @brief in case target Type is a pointer for orig allocates memory and store orig
  /// @param orig Source value
  /// @param targetType Target type to convert to
  /// @param insertPoint instruction to insert before
  /// @return  allcoated pointer
  static llvm::Instruction *convertValToPointer(llvm::Value *orig, llvm::Type *targetType, llvm::Instruction *insertPoint);

  /// @brief check if ShuffleVector instruction is used to artificially extend a vector.
  /// @param SI shuffleVector instruction to check
  /// @param realType the "original" type which we try to root
  /// @return The instruction's input value if True, NULL otherwise
  static llvm::Value *isExtendedByShuffle(llvm::ShuffleVectorInst *SI, llvm::Type *realType);

  /// @brief check if ShuffleVector instruction is used to artificially truncate a vector result
  /// @param SI shuffleVector instruction to check
  /// @return True if truncation was done thru ShuffleVector, False otherwise
  static bool isShuffleVectorTruncate(llvm::ShuffleVectorInst *SI);

  /// @brief check if I is obtained by series of insert element instructions to it's start.
  /// @param I - instruction to check.
  /// @param realType - target type to check.
  /// @returns rooted smaller vector of Type realType if possible, NULL otherwise.
  static llvm::Value *isInsertEltExtend(llvm::Instruction *I, llvm::Type *realType);

  /// @brief checks if it is possible to convert v into real type using shuffle
  ///        vector instruction.
  /// @param v - value to convert.
  /// @realType - type to convert into.
  /// @param loc - location of conversion.
  /// @returns converted val if possible, NULL otherwise.
  static llvm::Instruction *convertUsingShuffle(llvm::Value *v, const llvm::Type *realType, llvm::Instruction *loc);

  /// @brief checks if any of the values in ValInChain can be coverted into
  ///        realType using shuffle vector instruction.
  /// @param valInChain - values to check.
  /// @realType - type to convert into.
  /// @param loc - location of conversion.
  /// @returns converted val if possible, NULL otherwise.
  static llvm::Value *canRootInputByShuffle(llvm::SmallVector<llvm::Value *, 4> &valInChain,
                                      const llvm::Type * realType, llvm::Instruction *loc);

};// VectorizerUtils

} // namespace intel


#endif //__VECTORIZER_UTILS_H__
