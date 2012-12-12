#include "Utils.h"
#include <cassert>

namespace reflection{

//string represenration for the primitive types
static const char* PrimitiveNames[NUM_TYPES] ={
  "bool",
  "uchar",
  "char",
  "ushort",
  "short",
  "uint",
  "int",
  "ulong",
  "long",
  "half",
  "float",
  "double",
  "void",
  "NONE",
};

const char* mangledTypes[NUM_TYPES] = {
  "b", //BOOL
  "h", //UCHAR
  "c", //CHAR
  "t", //USHORT
  "s", //SHORT
  "j", //UINT
  "i", //INT
  "m", //ULONG
  "l", //LONG
  "Dh", //HALF
  "f", //FLOAT
  "d", //DOUBLE
  "v"  //VOID
};

//BOOL is the first type enum, and its not neccesrily valed zero.
const char* mangledString(const reflection::Type* t){
  assert(t && "null pointer");
  assert(
    (t->getPrimitive() > reflection::primitives::FIRST) &&
    (t->getPrimitive() <= reflection::primitives::NONE) &&
    "invalid primitive type"
  );
  return mangledTypes[t->getPrimitive()-reflection::primitives::BOOL];
}

const char* readableString(const reflection::Type* t){
  assert(t && "null pointer");
  assert(
    (t->getPrimitive() > reflection::primitives::FIRST) &&
    (t->getPrimitive() <= reflection::primitives::NONE) &&
    "invalid primitive type"
  );
  if (reflection::primitives::NONE == t->getPrimitive())
    return t->toString().c_str();
  return PrimitiveNames[t->getPrimitive()-reflection::primitives::BOOL];
}

primitives::Primitive parseType(const std::string& s){
  for (int i=0 ; i<NUM_TYPES ; ++i)
    if (0==s.compare(PrimitiveNames[i]))
      return static_cast<primitives::Primitive>(primitives::BOOL+i);
  assert(false && "unreachable code: not a valid type");
  return static_cast<primitives::Primitive>(0);
}

}
