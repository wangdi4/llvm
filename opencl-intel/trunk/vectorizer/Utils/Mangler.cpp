#include "Mangler.h"
#include "Logger.h"

#include <cassert>
#include <sstream>

const std::string Mangler::mask_delim           = "_";
const std::string Mangler::mask_prefix_func     = "maskedf_";
const std::string Mangler::mask_prefix_load     = "masked_load";
const std::string Mangler::mask_prefix_store    = "masked_store";
const std::string Mangler::name_allOne          = "allOne";
const std::string Mangler::name_allZero         = "allZero";
const std::string Mangler::fake_builtin_prefix  = "_f_v.";

template <class T>
inline std::string toString (const T& elem) {
  std::stringstream ss;
  ss << elem;
  return ss.str();
}

std::string Mangler::mangle(const std::string& name) {
  static unsigned int serial = 0;
  // Attach a serial number to each function decleration
  std::string suffix = toString(serial++);
  return mask_prefix_func+suffix+mask_delim+name;
}

std::string Mangler::getLoadName() {
  static unsigned int serial = 0;
  std::string suffix = toString(serial++);
  return mask_prefix_load+suffix;
}

std::string Mangler::getStoreName() {
  static unsigned int serial = 0;
  std::string suffix = toString(serial++);
  return mask_prefix_store+suffix;
}

std::string Mangler::demangle(const std::string& name) {
  V_ASSERT(name.find(mask_prefix_func) != name.npos && "not a mangled function");
  // Format:
  // masked_83_function
  size_t start = name.find(mask_prefix_func)
    + std::string(mask_prefix_func).length() + 1;
  size_t orig = name.find(mask_delim, start);
  V_ASSERT(orig != std::string::npos && "unable to find original name");
  return name.substr(orig + 1);
}


bool Mangler::isMangledLoad(const std::string& name) {
  return name.find(mask_prefix_load) != std::string::npos;
}

bool Mangler::isMangledStore(const std::string& name) {
  return name.find(mask_prefix_store) != std::string::npos;
}

bool Mangler::isMangledCall(const std::string& name) {
  return name.find(mask_prefix_func) != std::string::npos;
}

bool Mangler::isFakeBuiltin(const std::string& name) {
  return name.find(fake_builtin_prefix) != std::string::npos;
}

std::string Mangler::getFakeBuiltinName(const std::string& name) {
  return fake_builtin_prefix+name;
}

std::string Mangler::demangle_fake_builtin(const std::string& name) {
  V_ASSERT(isFakeBuiltin(name) && "not a mangled fake builtin function");
  // Format:
  // _f_v.function
  size_t start = name.find(fake_builtin_prefix);
  // when demangle_fake_builtin is called fake_builtin_pefix should be at start 
  V_ASSERT(start == 0);
  start += fake_builtin_prefix.length();
  return name.substr(start);
}

