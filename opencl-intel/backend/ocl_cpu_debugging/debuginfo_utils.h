#ifndef DEBUGINFO_UTILS_H
#define DEBUGINFO_UTILS_H

#include <string>
#include <sstream>
#include <cl_utils.h>

namespace debugservermessages {
    class VarTypeDescriptor;
}


namespace llvm {
    class DIType;
}

std::string DescribeVarType(const llvm::DIType& di_type);
std::string DescribeVarValue(const llvm::DIType& di_type, void* addr, std::string type_name = "");


// Translate the internal llvm::DIType into a VarTypeDescriptor suitable for
// transmission to the client.
//
debugservermessages::VarTypeDescriptor GenerateVarTypeDescriptor(
    const llvm::DIType& di_type);


#endif // DEBUGINFO_UTILS_H
