#include "clang/Basic/OpenCLExtensions.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include <string>

namespace clang {

namespace OpenCL {

Extension convertExtNameToEnum(const std::string &Name) {
#define OPENCLEXT(NAME) .Case(#NAME, NAME)
  Extension ext = llvm::StringSwitch<Extension>(Name)
    #include "clang/Basic/OpenCLExtensions.def"
    .Default(unknownExtension);

  return ext;
}

llvm::StringRef convertEnumToString(const unsigned ext) {
#define OPENCLEXT(NAME) \
  if (ext == NAME) \
    return #NAME;
  #include "clang/Basic/OpenCLExtensions.def"

  return NULL;
}

}  // end namespace OpenCL

}  // end namespace clang
