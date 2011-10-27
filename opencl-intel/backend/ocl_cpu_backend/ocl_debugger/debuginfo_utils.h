#ifndef DEBUGINFO_UTILS_H
#define DEBUGINFO_UTILS_H

#include <string>
#include <sstream>

namespace debugservermessages {
    class VarTypeDescriptor;
}


namespace llvm {
    class DIType;
}

// Turn any type supporting the ostream&<< operator into a string
//
template<typename T> inline std::string stringify(const T& x)
{
    std::ostringstream out;
    out << x;
    return out.str();
}

// Specialization for booleans
//
template<> inline std::string stringify(const bool& b)
{
    return b ? "true" : "false";
}

// Specialization for signed and unsigned chars: we want them to be just
// displayed as numbers
//
template<> inline std::string stringify(const unsigned char& c)
{
    return stringify(static_cast<unsigned int>(c));
}


template<> inline std::string stringify(const signed char& c)
{
    return stringify(static_cast<signed int>(c));
}


std::string DescribeVarType(const llvm::DIType& di_type);
std::string DescribeVarValue(const llvm::DIType& di_type, void* addr, std::string type_name = "");


// Translate the internal llvm::DIType into a VarTypeDescriptor suitable for
// transmission to the client.
//
debugservermessages::VarTypeDescriptor GenerateVarTypeDescriptor(
    const llvm::DIType& di_type);


#endif // DEBUGINFO_UTILS_H
