/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __MANGLER_H_
#define __MANGLER_H_

#include "llvm/IR/DerivedTypes.h"

#include <string>

using namespace llvm;

/// @brief A utility class for mangling names
///  for function calls and load/store operations
class Mangler {
public:

  enum GatherScatterType { Gather,
                           Scatter,
                           GatherPrefetch,
                           ScatterPrefetch };

  /// @brief De-Mangle function call
  /// @param name name to de-mangle
  /// @param masked denotes whether name is a masked version of a function
  /// @return De-mangled name
  static std::string demangle(const std::string& name, bool masked=true);
  /// @brief Mangle original function call
  /// @param name Name to mangle
  /// @return Mangled name
  static std::string mangle(const std::string& name);
  /// @brief Get mangled name for load instruction
  /// @return new name
  static std::string getLoadName(unsigned align);
  /// @brief Get mangled name for store instruction
  /// @return name
  static std::string getStoreName(unsigned align);
  /// @brief Get mangled name for gather or scatter instruction
  /// @param isMasked true for masked instruction, false for non masked instruction
  /// @param isGather true for gather instruction, false for scatter instruction
  /// @param retDataVecTy type of return/data value (should be a vector)
  /// @return name
  static std::string getGatherScatterName(bool isMasked, GatherScatterType gatherType, VectorType *retDataVecTy);
  /// @brief Get internal mangled name for gather or scatter instruction
  /// (this name will be resolved at Resolver pass, thus it is for vectorizer internal use only)
  /// @param isGather true for gather instruction, false for scatter instruction
  /// @param maskType type of mask value (can be scalar of vector)
  /// @param retDataVecTy type of return/data value (should be a vector)
  /// @param indexType type of index element
  /// @return name
  static std::string getGatherScatterInternalName(GatherScatterType gatherType, Type *maskType, VectorType *retDataVecTy, Type *indexType);
  /// @brief Get mangled name for vectorized prefetch built-in.
  /// @param name name of original prefetch built-in name.
  /// @param packetWidth width of vector data type for prefetch.
  /// @return mangled name for prefetch built-in with vector argument
  static std::string getVectorizedPrefetchName(const std::string& name, int packetWidth);
  /// @brief Is this a mangled load instruction
  /// @param name Name of function
  /// @return True if load
  static bool isMangledLoad(const std::string& name);
  /// @brief Is this a mangled store instruction
  /// @param name Name of function
  /// @return True if store
  static bool isMangledStore(const std::string& name);
  /// @brief Is this a mangled function call? More concretely, does the given
  //  string represent the name of a function that starts with the prefix 'mask_',
  //  that was previously synthesized by the Predicator.
  /// @param name Name of function
  /// @return True if mangled
  static bool isMangledCall(const std::string& name);
  /// @brief Is this a mangled gather instruction
  /// @param name Name of function
  /// @return True if mangled
  static bool isMangledGather(const std::string& name);
  /// @brief Is this a mangled scatter instruction
  /// @param name Name of function
  /// @return True if mangled
  static bool isMangledScatter(const std::string& name);
  /// @brief Is this a mangled 'prefetch' function call (masked version is also supported).
  /// @param name Name of function
  /// @return True if mangled 'prefetch' function call
  static bool isMangledPrefetch(const std::string& name);
  /// @brief Is this a mangled 'gather prefetch' function call (masked version is also supported).
  /// @param name Name of function
  /// @return True if mangled 'gather prefetch' function call
  static bool isMangeledGatherPrefetch(const std::string& name);
  /// @brief Is this a special function which checks the mask.
  ///  Is this a call to 'all-one' or 'all-zero' ?
  /// @param name Name of function
  /// @return True if mask-related function name
  static bool isMaskTest(const std::string& name);
  /// @brief Is this the all-zero function ?
  /// @param name Name of function
  /// @return True if this is the all-zero function name.
  static bool isAllZero(const std::string& name);
  /// @brief Is this the all-one function ?
  /// @param name Name of function
  /// @return True if this is the all-one function name.
  static bool isAllOne(const std::string& name);



  /// @brief Get mangled name for transpose function
  /// @param isLoad True if this is load and transpose, false otherwise
  /// @param isScatterGather True if this is a scatter/gather, false if a normal store/load
  /// @param isMasked True if this is a masked operation, false otherwise
  /// @param origVecType Vector type in the original instruction
  /// @param packetWidth Packetization width
  /// @return name
  static std::string getTransposeBuiltinName(bool isLoad, bool isScatterGather, bool isMasked,
          VectorType * origVecType, unsigned int packetWidth);

  /// @brief Get mangled name for masked load/store function
  /// @param isLoad True if this is load, false otherwise
  /// @param vecType Vector type of the data to load/store
  /// @return name
  static std::string getMaskedLoadStoreBuiltinName(bool isLoad, VectorType * vecType);

  /// @brief returns fake builtin name for a given builtin name
  /// @param name - original builtin name
  /// @return the builtin name with a fake builtin name
  static std::string getFakeBuiltinName(const std::string& name);

  /// @brief returns ret-by-array or ret-by-vector builtin name for a given builtin name
  /// @param name - original builtin name
  /// @return the builtin name with a ret-by-array or ret-by-vector builtin name
  static std::string getRetByArrayBuiltinName(const std::string& name);

  /// @brief returns the alignment of mangled store function by it's name
  /// @param name Name of function
  /// @return the alignment of the store
  static unsigned getMangledStoreAlignment(const std::string& name);

  /// @brief returns the alignment of mangled load function by it's name
  /// @param name Name of function
  /// @return the alignment of the load
  static unsigned getMangledLoadAlignment(const std::string& name);

  /// @brief recovers origianl builtin name from the mangled fake builtin name
  /// @param name - fake builtin name
  /// @return the original builtin name
  static std::string demangle_fake_builtin(const std::string& name);

  /// @brief recovers origianl builtin name from the mangled ret-by-vector builtin name
  /// @param name - ret-by-vector builtin name
  /// @return the original builtin name
  static std::string get_original_scalar_name_from_retbyvector_builtin(const std::string& name);

  /// @brief checks whether builtin name is mangled fake builtin name
  /// @param name - name of the function
  /// @return true if has fake builtin prefix
  static bool isFakeBuiltin(const std::string& name);

  /// @brief checks whether builtin name is mangled ret-by-vector builtin name
  /// @param name - name of the function
  /// @return true if has ret-by-vector builtin prefix
  static bool isRetByVectorBuiltin(const std::string& name);

  /// @brief Get mangled name for load instruction
  /// @return new name
  static std::string getFakeExtractName();
  /// @brief Get mangled name for store instruction
  /// @return name
  static std::string getFakeInsertName();
  /// @brief Is this a mangled load instruction
  /// @param name Name of function
  /// @return True if load
  static bool isFakeExtract(const std::string& name);
  /// @brief Is this a mangled store instruction
  /// @param name Name of function
  /// @return True if store
  static bool isFakeInsert(const std::string& name);

private:
  /// @brief mangling delimiter
  static const std::string mask_delim;
  /// @brief mangling function call prefix
  static const std::string mask_prefix_func;
  /// @brief mangling of load operations
  static const std::string mask_prefix_load;
  /// @brief mangling of store operations
  static const std::string mask_prefix_store;
  /// @brief mangling of internal gather operations
  static const std::string prefix_gather;
  /// @brief mangling of internal scatter operations
  static const std::string prefix_scatter;
  /// @brief mangling of internal gather prefetch operations
  static const std::string prefix_gather_prefetch;
  /// @brief mangling of internal scatter prefetch operations
  static const std::string prefix_scatter_prefetch;
  /// @brief mangling of 'prefetch' function call
  static const std::string prefetch;
  /// @brief mangling of fake built-ins used for vectorization
  static const std::string fake_builtin_prefix;
  /// @brief mangling of fake built-ins used for vectorization
  ///        returns two values by array
  static const std::string retbyarray_builtin_prefix;
  /// @brief mangling of fake built-ins used for vectorization
  ///        returns two values by vector
  static const std::string retbyvector_builtin_prefix;
  /// @brief mangling fake extract calls used for vectorization of
  ///        scalar built-ins that return a vector
  static const std::string fake_prefix_extract;
  /// @brief mangling fake insert calls used for vectorization of
  ///        scalar built-ins that have vector arguments.
  static const std::string fake_prefix_insert;

public:
  /// @brief mangled name of 'allone function'
  static const std::string name_allOne;
  /// @brief mangled name of 'allzero function'
  static const std::string name_allZero;
};


#endif // __MANGLER_H_
