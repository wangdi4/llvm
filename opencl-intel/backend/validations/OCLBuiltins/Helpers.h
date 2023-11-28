// INTEL CONFIDENTIAL
//
// Copyright 2011 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#ifndef HELPERS_H
#define HELPERS_H

#include "FloatOperations.h"
#include "OCLBuiltinParser.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/Support/DataTypes.h"     // int8_t, int16_t, ... declared here
#include "llvm/Support/ErrorHandling.h" // report_fatal_error declared here

#ifdef VOID
#undef VOID
#endif

namespace Validation {
namespace OCLBuiltins {

llvm::GenericValue
UnimplementedBuiltin(llvm::FunctionType *FT,
                     llvm::ArrayRef<llvm::GenericValue> Args);

// Returns true if integer data type is signed and false otherwise
template <typename T> bool isSignedType() {
  llvm::report_fatal_error("isSignedType: unsupported integer data type.");
  return true;
}

template <> bool isSignedType<int8_t>();
template <> bool isSignedType<int16_t>();
template <> bool isSignedType<int32_t>();
template <> bool isSignedType<int64_t>();
template <> bool isSignedType<uint8_t>();
template <> bool isSignedType<uint16_t>();
template <> bool isSignedType<uint32_t>();
template <> bool isSignedType<uint64_t>();

template <class T> class retType {
public:
  typedef llvm::APInt type;
};

template <> class retType<float> {
public:
  typedef float type;
};

template <> class retType<double> {
public:
  typedef double type;
};

template <typename T> typename retType<T>::type &getRef(llvm::GenericValue &R) {
  return R.IntVal;
}
template <> float &getRef<float>(llvm::GenericValue &R);
template <> double &getRef<double>(llvm::GenericValue &R);

template <typename T>
bool predLess(const typename retType<T>::type &a,
              const typename retType<T>::type &b) {
  return a < b;
}

template <> bool predLess<int8_t>(const llvm::APInt &a, const llvm::APInt &b);
template <> bool predLess<int16_t>(const llvm::APInt &a, const llvm::APInt &b);
template <> bool predLess<int32_t>(const llvm::APInt &a, const llvm::APInt &b);
template <> bool predLess<int64_t>(const llvm::APInt &a, const llvm::APInt &b);
template <> bool predLess<uint8_t>(const llvm::APInt &a, const llvm::APInt &b);
template <> bool predLess<uint16_t>(const llvm::APInt &a, const llvm::APInt &b);
template <> bool predLess<uint32_t>(const llvm::APInt &a, const llvm::APInt &b);
template <> bool predLess<uint64_t>(const llvm::APInt &a, const llvm::APInt &b);

template <typename T, int n>
typename retType<T>::type &getRef(llvm::GenericValue &R, int i) {
  return getRef<T>(R.AggregateVal[i]);
}

template <> llvm::APInt &getRef<int8_t, 1>(llvm::GenericValue &R, int i);
template <> llvm::APInt &getRef<uint8_t, 1>(llvm::GenericValue &R, int i);
template <> llvm::APInt &getRef<int16_t, 1>(llvm::GenericValue &R, int i);
template <> llvm::APInt &getRef<uint16_t, 1>(llvm::GenericValue &R, int i);
template <> llvm::APInt &getRef<int32_t, 1>(llvm::GenericValue &R, int i);
template <> llvm::APInt &getRef<uint32_t, 1>(llvm::GenericValue &R, int i);
template <> llvm::APInt &getRef<int64_t, 1>(llvm::GenericValue &R, int i);
template <> llvm::APInt &getRef<uint64_t, 1>(llvm::GenericValue &R, int i);

template <> float &getRef<float, 1>(llvm::GenericValue &R, int i);
template <> double &getRef<double, 1>(llvm::GenericValue &R, int i);

// Casts generic value into C-type value.
template <typename T> T getVal(const llvm::GenericValue &v) {
  return *reinterpret_cast<const T *>(v.IntVal.getRawData());
}
template <> float getVal<float>(const llvm::GenericValue &v);
template <> double getVal<double>(const llvm::GenericValue &v);

template <typename T, int n> T getVal(const llvm::GenericValue &R, int i) {
  return getVal<T>(R.AggregateVal[i]);
}

template <> int8_t getVal<int8_t, 1>(const llvm::GenericValue &R, int i);
template <> uint8_t getVal<uint8_t, 1>(const llvm::GenericValue &R, int i);
template <> int16_t getVal<int16_t, 1>(const llvm::GenericValue &R, int i);
template <> uint16_t getVal<uint16_t, 1>(const llvm::GenericValue &R, int i);
template <> int32_t getVal<int32_t, 1>(const llvm::GenericValue &R, int i);
template <> uint32_t getVal<uint32_t, 1>(const llvm::GenericValue &R, int i);
template <> int64_t getVal<int64_t, 1>(const llvm::GenericValue &R, int i);
template <> uint64_t getVal<uint64_t, 1>(const llvm::GenericValue &R, int i);

template <> float getVal<float, 1>(const llvm::GenericValue &R, int i);
template <> double getVal<double, 1>(const llvm::GenericValue &R, int i);

// template parameter could be int8_t, uint8_t, int16_t, uint16_t, int32_t,
// uint32_t, int64_t, uint64_t
template <typename T> typename retType<T>::type derefPointer(T *p) {
  uint64_t tmp = 0;
  std::copy(p, p + 1, &tmp);
  return typename retType<T>::type(sizeof(T) * 8, tmp, isSignedType<T>());
}
template <> float derefPointer<float>(float *p);
template <> double derefPointer<double>(double *p);

template <typename T> llvm::OCLBuiltinParser::BasicArgType getBasicType() {
  llvm::report_fatal_error("getBasicType: unsupported data type.");
}
template <> llvm::OCLBuiltinParser::BasicArgType getBasicType<int8_t>();
template <> llvm::OCLBuiltinParser::BasicArgType getBasicType<uint8_t>();
template <> llvm::OCLBuiltinParser::BasicArgType getBasicType<int16_t>();
template <> llvm::OCLBuiltinParser::BasicArgType getBasicType<uint16_t>();
template <> llvm::OCLBuiltinParser::BasicArgType getBasicType<int32_t>();
template <> llvm::OCLBuiltinParser::BasicArgType getBasicType<uint32_t>();
template <> llvm::OCLBuiltinParser::BasicArgType getBasicType<int64_t>();
template <> llvm::OCLBuiltinParser::BasicArgType getBasicType<uint64_t>();
template <> llvm::OCLBuiltinParser::BasicArgType getBasicType<long long>();
template <>
llvm::OCLBuiltinParser::BasicArgType getBasicType<unsigned long long>();
template <> llvm::OCLBuiltinParser::BasicArgType getBasicType<float>();
template <> llvm::OCLBuiltinParser::BasicArgType getBasicType<double>();
template <> llvm::OCLBuiltinParser::BasicArgType getBasicType<long double>();
template <> llvm::OCLBuiltinParser::BasicArgType getBasicType<void>();
template <> llvm::OCLBuiltinParser::BasicArgType getBasicType<bool>();

#define DEFINE_BLT_ONE_ARG(name)                                               \
  template <typename T, int n>                                                 \
  llvm::GenericValue lle_X_##name(llvm::FunctionType *FT,                      \
                                  llvm::ArrayRef<llvm::GenericValue> Args) {   \
    llvm::GenericValue R;                                                      \
    R.AggregateVal.resize(n);                                                  \
    llvm::GenericValue arg0 = Args[0];                                         \
    for (uint32_t i = 0; i < n; ++i) {                                         \
      getRef<T, n>(R, i) = RefALU::name<T>(getVal<T, n>(arg0, i));             \
    }                                                                          \
    return R;                                                                  \
  }

#define DEFINE_BLT_TWO_ARGS(name)                                              \
  template <typename T, int n>                                                 \
  llvm::GenericValue lle_X_##name(llvm::FunctionType *FT,                      \
                                  llvm::ArrayRef<llvm::GenericValue> Args) {   \
    llvm::GenericValue R;                                                      \
    R.AggregateVal.resize(n);                                                  \
    llvm::GenericValue arg0 = Args[0];                                         \
    llvm::GenericValue arg1 = Args[1];                                         \
    for (uint32_t i = 0; i < n; ++i) {                                         \
      getRef<T, n>(R, i) =                                                     \
          RefALU::name<T>(getVal<T, n>(arg0, i), getVal<T, n>(arg1, i));       \
    }                                                                          \
    return R;                                                                  \
  }

#define DEFINE_BLT_THREE_ARGS(name)                                            \
  template <typename T, int n>                                                 \
  llvm::GenericValue lle_X_##name(llvm::FunctionType *FT,                      \
                                  llvm::ArrayRef<llvm::GenericValue> Args) {   \
    llvm::GenericValue R;                                                      \
    R.AggregateVal.resize(n);                                                  \
    llvm::GenericValue arg0 = Args[0];                                         \
    llvm::GenericValue arg1 = Args[1];                                         \
    llvm::GenericValue arg2 = Args[2];                                         \
    for (uint32_t i = 0; i < n; ++i) {                                         \
      getRef<T, n>(R, i) =                                                     \
          RefALU::name<T>(getVal<T, n>(arg0, i), getVal<T, n>(arg1, i),        \
                          getVal<T, n>(arg2, i));                              \
    }                                                                          \
    return R;                                                                  \
  }

template <typename T> T intMax() {
  llvm::report_fatal_error("intMax: unsupported data type.");
}

template <typename T> T intMin() {
  llvm::report_fatal_error("intMin: unsupported data type.");
}
template <> int8_t intMax<int8_t>();
template <> int16_t intMax<int16_t>();
template <> int32_t intMax<int32_t>();
template <> int64_t intMax<int64_t>();
template <> uint8_t intMax<uint8_t>();
template <> uint16_t intMax<uint16_t>();
template <> uint32_t intMax<uint32_t>();
template <> uint64_t intMax<uint64_t>();

template <> int8_t intMin<int8_t>();
template <> int16_t intMin<int16_t>();
template <> int32_t intMin<int32_t>();
template <> int64_t intMin<int64_t>();
template <> uint8_t intMin<uint8_t>();
template <> uint16_t intMin<uint16_t>();
template <> uint32_t intMin<uint32_t>();
template <> uint64_t intMin<uint64_t>();

template <typename T> llvm::GenericValue initWithMin() {
  llvm::report_fatal_error("initWithMin: unsupported data type.");
}
template <> llvm::GenericValue initWithMin<int8_t>();
template <> llvm::GenericValue initWithMin<int16_t>();
template <> llvm::GenericValue initWithMin<int32_t>();
template <> llvm::GenericValue initWithMin<int64_t>();
template <> llvm::GenericValue initWithMin<uint8_t>();
template <> llvm::GenericValue initWithMin<uint16_t>();
template <> llvm::GenericValue initWithMin<uint32_t>();
template <> llvm::GenericValue initWithMin<uint64_t>();
template <> llvm::GenericValue initWithMin<float>();
template <> llvm::GenericValue initWithMin<double>();

template <typename T> llvm::GenericValue initWithMax() {
  llvm::report_fatal_error("initWithMax: unsupported data type.");
}
template <> llvm::GenericValue initWithMax<int8_t>();
template <> llvm::GenericValue initWithMax<int16_t>();
template <> llvm::GenericValue initWithMax<int32_t>();
template <> llvm::GenericValue initWithMax<int64_t>();
template <> llvm::GenericValue initWithMax<uint8_t>();
template <> llvm::GenericValue initWithMax<uint16_t>();
template <> llvm::GenericValue initWithMax<uint32_t>();
template <> llvm::GenericValue initWithMax<uint64_t>();
template <> llvm::GenericValue initWithMax<float>();
template <> llvm::GenericValue initWithMax<double>();

template <typename T> llvm::GenericValue initWithZero() {
  llvm::report_fatal_error("initWithZero: unsupported data type.");
}
template <> llvm::GenericValue initWithZero<int8_t>();
template <> llvm::GenericValue initWithZero<int16_t>();
template <> llvm::GenericValue initWithZero<int32_t>();
template <> llvm::GenericValue initWithZero<int64_t>();
template <> llvm::GenericValue initWithZero<uint8_t>();
template <> llvm::GenericValue initWithZero<uint16_t>();
template <> llvm::GenericValue initWithZero<uint32_t>();
template <> llvm::GenericValue initWithZero<uint64_t>();
template <> llvm::GenericValue initWithZero<float>();
template <> llvm::GenericValue initWithZero<double>();

template <typename T> llvm::GenericValue initWithInteger(int i) {
  llvm::report_fatal_error("initWithInteger: unsupported data type.");
}
template <> llvm::GenericValue initWithInteger<int8_t>(int i);
template <> llvm::GenericValue initWithInteger<int16_t>(int i);
template <> llvm::GenericValue initWithInteger<int32_t>(int i);
template <> llvm::GenericValue initWithInteger<int64_t>(int i);
template <> llvm::GenericValue initWithInteger<uint8_t>(int i);
template <> llvm::GenericValue initWithInteger<uint16_t>(int i);
template <> llvm::GenericValue initWithInteger<uint32_t>(int i);
template <> llvm::GenericValue initWithInteger<uint64_t>(int i);
template <> llvm::GenericValue initWithInteger<float>(int i);
template <> llvm::GenericValue initWithInteger<double>(int i);

template <typename T> class superT;

template <> class superT<int8_t> {
public:
  typedef int16_t type;
};

template <> class superT<uint8_t> {
public:
  typedef uint16_t type;
};

template <> class superT<int16_t> {
public:
  typedef int32_t type;
};

template <> class superT<uint16_t> {
public:
  typedef uint32_t type;
};

template <> class superT<int32_t> {
public:
  typedef int64_t type;
};

template <> class superT<uint32_t> {
public:
  typedef uint64_t type;
};

template <typename T> class unsignedT;

template <> class unsignedT<int8_t> {
public:
  typedef uint8_t type;
};

template <> class unsignedT<int16_t> {
public:
  typedef uint16_t type;
};

template <> class unsignedT<int32_t> {
public:
  typedef uint32_t type;
};

template <> class unsignedT<int64_t> {
public:
  typedef uint64_t type;
};

template <> class unsignedT<uint8_t> {
public:
  typedef uint8_t type;
};

template <> class unsignedT<uint16_t> {
public:
  typedef uint16_t type;
};

template <> class unsignedT<uint32_t> {
public:
  typedef uint32_t type;
};

template <> class unsignedT<uint64_t> {
public:
  typedef uint64_t type;
};

template <typename T> class signedT;

template <> class signedT<int8_t> {
public:
  typedef int8_t type;
};

template <> class signedT<int16_t> {
public:
  typedef int16_t type;
};

template <> class signedT<int32_t> {
public:
  typedef int32_t type;
};

template <> class signedT<int64_t> {
public:
  typedef int64_t type;
};

template <> class signedT<uint8_t> {
public:
  typedef int8_t type;
};

template <> class signedT<uint16_t> {
public:
  typedef int16_t type;
};

template <> class signedT<uint32_t> {
public:
  typedef int32_t type;
};

template <> class signedT<uint64_t> {
public:
  typedef int64_t type;
};

template <typename T> T getOneMinus1ULP();

template <> float getOneMinus1ULP<float>();
template <> double getOneMinus1ULP<double>();

llvm::GenericValue lle_X_memcpy(llvm::FunctionType *FT,
                                llvm::ArrayRef<llvm::GenericValue> Args);
} // namespace OCLBuiltins
} // namespace Validation

#endif // HELPERS_H
