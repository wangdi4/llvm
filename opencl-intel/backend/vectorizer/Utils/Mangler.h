#ifndef __MANGLER_H_
#define __MANGLER_H_

#include "llvm/DerivedTypes.h"

#include <string>

using namespace llvm;

/// @brief A utility class for mangling names
///  for function calls and load/store operations
/// @author Nadav Rotem
class Mangler {
public:

  /// @brief De-Mangle function call
  /// @param name name to de-mangle
  /// @return De-mangled name
  static std::string demangle(const std::string& name);
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
  static std::string getGatherScatterName(bool isMasked, bool isGather, VectorType *retDataVecTy);
  /// @brief Get internal mangled name for gather or scatter instruction
  /// (this name will be resolved at Resolver pass, thus it is for vectorizer internal use only)
  /// @param isGather true for gather instruction, false for scatter instruction
  /// @param maskType type of mask value (can be scalar of vector)
  /// @param retDataVecTy type of return/data value (should be a vector)
  /// @param indexType type of index element
  /// @return name
  static std::string getGatherScatterInternalName(bool isGather, Type *maskType, VectorType *retDataVecTy, Type *indexType);
  /// @brief Is this a mangled load instruction
  /// @param name Name of function
  /// @return True if load
  static bool isMangledLoad(const std::string& name);
  /// @brief Is this a mangled store instruction
  /// @param name Name of function
  /// @return True if store
  static bool isMangledStore(const std::string& name);
  /// @brief Is this a mangled function call ?
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
  /// @brief Is this a special function which checks the mask.
  ///  Is this a call to 'all-one' or 'all-zero' ?
  /// @param name Name of function
  /// @return True if mask-related function name
  static bool isMaskTest(const std::string& name);

  /// @brief Get mangled name for transpose function
  /// @param isLoad True if this is load and transpose, false otherwise
  /// @param origVecType Vector type in the original instruction
  /// @param packetWidth Packetization width
  /// @return name
  static std::string getTransposeBuiltinName(bool isLoad, VectorType * origVecType, unsigned int packetWidth);

  /// @brief returns fake builtin name for a given builtin name
  /// @param name - original builtin name
  /// @return the builtin name with a fake builtin name
  static std::string getFakeBuiltinName(const std::string& name);

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

  /// @brief checks whether builtin name is mangled fake builtin name
  /// @param name - name of the function
  /// @return true if has fake builtin prefix
  static bool isFakeBuiltin(const std::string& name);

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
  /// @brief mangling of fake built-ins used for vectorization
  static const std::string fake_builtin_prefix;
  /// @brief mangling fake extract calls used for vectorization of 
  ///        scalar built-ins that return a vector
  static const std::string fake_prefix_extract;
  /// @brief mangling fake isnert calls used for vectorization of 
  ///        scalar built-ins that have vector arguments.
  static const std::string fake_prefix_insert;

public:
  /// @brief mangled name of 'allone function'
  static const std::string name_allOne;
  /// @brief mangled name of 'allzero function'
  static const std::string name_allZero;
};


#endif // __MANGLER_H_
