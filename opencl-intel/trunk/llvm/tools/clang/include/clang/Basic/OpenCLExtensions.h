#ifndef LLVM_CLANG_BASIC_OPEN_CL_EXTENSION_H
#define LLVM_CLANG_BASIC_OPEN_CL_EXTENSION_H

#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include <string>

namespace clang {

namespace OpenCL {

#define OPENCLEXT(NAME) NAME,
enum Extension {
    #include "clang/Basic/OpenCLExtensions.def"
    NUM_EXTENSIONS,
    unknownExtension
};

Extension convertExtNameToEnum(const std::string &Name);

llvm::StringRef convertEnumToString(const unsigned ext);

}  // end namespace OpenCL

}  // end namespace clang

#endif
