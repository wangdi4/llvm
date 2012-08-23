/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#ifndef __VECTORIZER_UTILS_H__
#define __VECTORIZER_UTILS_H__

#include "llvm/Instructions.h"
#include "llvm/Constants.h"
#include <vector>
#include "Logger.h"


using namespace llvm;


namespace intel {

class VectorizerUtils {

public:

  /// @brief set debug location of I according to SetBy.
  /// @param I instruction whose debug location will be set.
  /// @param setBy instruction from which debug location is gathered.
  static void SetDebugLocBy(Instruction *I, Instruction *setBy);
  
  /// @brief set debug location of I according to SetBy.
  /// @param I instruction whose debug location will be set.
  /// @param setBy instruction from which debug location is gathered.
  static void SetDebugLocBy(std::vector<Instruction *> &insts, Instruction *setBy);

  /// @brief Follow thru a function input argument, until finding the root
  //   (where its type matches the "expected" type)
  /// @param arg The actual argument of the CALL inst
  /// @param rootType The desired (or "real") type to find
  /// @param CI The CALL instruction
  /// @return The root value if found, or NULL otherwise
  static Value *RootInputArgument(Value *arg, Type *rootType, CallInst *CI);

  /// @brief Follow thru a function return argument, until its type matches the "expected" type
  /// @param retVal The actual returned value of the CALL inst
  /// @param rootType The desired (or "real") type to find
  /// @param CI The CALL instruction
  /// @return The "proper" retval if found, or NULL otherwise
  static Value *RootReturnValue(Value *retVal, Type *rootType, CallInst *CI);

  /// @brief casts value into desired type if types differ
  /// @param inputVal Value to be casted
  /// @param targetType "actual" desired type according to calling convention
  /// @param insertPoint instruction to insert before
  /// @return casted value
  static Value *getCastedArgIfNeeded(Value *inputVal, Type *targetType, Instruction *insertPoint);

  /// @brief casts call instruction value into desired type if types put casting instruction after the call 
  /// @param CI call instruction to cast
  /// @param targetType "actual" desired type according to calling convention
  /// @return casted value
  static Instruction *getCastedRetIfNeeded(CallInst *CI, Type *targetType);

  /// @brief Creates a call to function "name" with given args. It also creates
  ///       the function declaration if not already exists in the module.
  /// @param pModule module that contains the function declaration
  /// @param name function name
  /// @param retType function return type
  /// @param args list of arguments to call the function with
  /// @param insertBefore instruction to insert new callInst before
  /// @return new call instruction
  static Instruction *createFunctionCall(Module *pModule, std::string name,
    Type *retType, SmallVectorImpl<Value *> &args, Instruction* insertBefore);

  /// @brief Creates a broadcast sequance (InsertElement + Shuffle)
  /// @param pVal value to prodcast
  /// @param packetWidth width of generated vector with broadcast value
  /// @param whereTo instruction to insert new instructions before or after
  /// @param insertAfter if true, insert after whereTo instruction, otherwise insert before it.
  /// @return new broadcast vector
  static Instruction *createBroadcast(Value * pVal, unsigned int packetWidth, Instruction* whereTo, bool insertAfter = false);

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
  static bool isOpaquePtrPair(Type *x, Type *y);

  /// @brief convert given type (of pointer to arrays of scalar/vector)
  ///        to type (of pointer to arrays of vector/scalar).
  /// @param type original type 
  /// @param width if 0 convert original type to scalar type,
  ///        otherwise packetize original type according to width.
  static Type* convertSoaAllocaType(Type *type, unsigned int width);

private:

  /// @brief Generate type-conversion and place in given location
  /// @param orig Source value
  /// @param targetType Target type to convert to
  /// @param insertPoint instruction to insert before
  static Instruction *BitCastValToType(Value *orig, Type *targetType, Instruction *insertPoint);

  /// @brief Generate type-conversion and place in given location 
  ///  but on debug accpets only cases size(orig) >= size(target)
  /// @param orig Source value
  /// @param targetType Target type to convert to
  /// @param insertPoint instruction to insert before
  static Instruction *ExtendValToType(Value *orig, Type *targetType, Instruction *insertPoint);

  /// @brief Generate type-conversion and place in given location 
  ///  but on debug accpets only cases size(orig) <= size(target)
  /// @param orig Source value
  /// @param targetType Target type to convert to
  /// @param insertPoint instruction to insert before
  static Instruction *TruncValToType(Value *orig, Type *targetType, Instruction *insertPoint);

  /// @brief in case target Type is a pointer for orig allocates memory and store orig
  /// @param orig Source value
  /// @param targetType Target type to convert to
  /// @param insertPoint instruction to insert before
  /// @return  allcoated pointer
  static Instruction *convertValToPointer(Value *orig, Type *targetType, Instruction *insertPoint);

  /// @brief check if ShuffleVector instruction is used to artificially extend a vector.
  /// @param SI shuffleVector instruction to check
  /// @param realType the "original" type which we try to root
  /// @return The instruction's input value if True, NULL otherwise
  static Value *isExtendedByShuffle(ShuffleVectorInst *SI, Type *realType);

  /// @brief check if ShuffleVector instruction is used to artificially truncate a vector result
  /// @param SI shuffleVector instruction to check
  /// @return True if truncation was done thru ShuffleVector, False otherwise
  static bool isShuffleVectorTruncate(ShuffleVectorInst *SI);

  /// @brief check if I is obtained by series of insert element instructions to it's start.
  /// @param I - instruction to check.
  /// @param realType - target type to check.
  /// @returns rooted smaller vector of Type realType if possible, NULL otherwise.
  static Value *isInsertEltExtend(Instruction *I, Type *realType);

  /// @brief checks if it is possible to convert v into real type using shuffle
  ///        vector instruction.
  /// @param v - value to convert.
  /// @realType - type to convert into.
  /// @param loc - location of conversion.
  /// @returns converted val if possible, NULL otherwise.
  static Instruction *convertUsingShuffle(Value *v, const Type *realType, Instruction *loc);

  /// @brief checks if any of the values in ValInChain can be coverted into 
  ///        realType using shuffle vector instruction.
  /// @param valInChain - values to check.
  /// @realType - type to convert into.
  /// @param loc - location of conversion.
  /// @returns converted val if possible, NULL otherwise.
  static Value *canRootInputByShuffle(SmallVector<Value *, 4> &valInChain,
                                      const Type * realType, Instruction *loc);

};// VectorizerUtils

} // namespace intel


#endif //__VECTORIZER_UTILS_H__