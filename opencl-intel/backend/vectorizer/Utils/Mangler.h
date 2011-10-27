#ifndef __MANGLER_H_
#define __MANGLER_H_

#include <string>

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
  static std::string getLoadName();
  /// @brief Get mangled name for store instruction
  /// @return name
  static std::string getStoreName();
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
  /// @brief Is this a special function which checks the mask.
  ///  Is this a call to 'all-one' or 'all-zero' ?
  /// @param name Name of function
  /// @return True if mask-related function name
  static bool isMaskTest(const std::string& name);

  /// @brief returns fake builtin name for a given builtin name
  /// @param name - original builtin name
  /// @return the builtin name with a fake builtin name
  static std::string getFakeBuiltinName(const std::string& name);

  /// @brief recovers origianl builtin name from the mangled fake builtin name
  /// @param name - fake builtin name
  /// @return the original builtin name
  static std::string demangle_fake_builtin(const std::string& name);

  /// @brief checks whether builtin name is mangled fake builtin name
  /// @param name - name of the function
  /// @return true if has fake builtin prefix
  static bool isFakeBuiltin(const std::string& name);

private:
  /// @brief mangling delimiter
  static const std::string mask_delim;
  /// @brief mangling function call prefix
  static const std::string mask_prefix_func;
  /// @brief mangling of load operations
  static const std::string mask_prefix_load;
  /// @brief mangling of store operations
  static const std::string mask_prefix_store;
  /// @brief mangling of store operations
  static const std::string fake_builtin_prefix;

public:
  /// @brief mangled name of 'allone function'
  static const std::string name_allOne;
  /// @brief mangled name of 'allzero function'
  static const std::string name_allZero;
};


#endif // __MANGLER_H_
