#include "Type.h"
#include <string>

#ifndef __MANGLING_UTILS_H__
#define __MANGLING_UTILS_H__

namespace reflection {
const char* mangledString(const Type*);
const char* readableString(const Type*);
primitives::Primitive parseType(const std::string& s);

}

#endif//__MANGLING_UTILS_H__
