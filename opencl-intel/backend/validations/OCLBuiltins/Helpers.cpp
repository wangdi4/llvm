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

#include "Helpers.h"
#include "Exception.h"
#include <limits>

using namespace llvm;
namespace Validation {
namespace OCLBuiltins {

template <> float getVal<float>(const llvm::GenericValue &R) {
  return R.FloatVal;
}
template <> double getVal<double>(const llvm::GenericValue &R) {
  return R.DoubleVal;
}

template <> int8_t getVal<int8_t, 1>(const llvm::GenericValue &R, int i) {
  return getVal<int8_t>(R);
}
template <> uint8_t getVal<uint8_t, 1>(const llvm::GenericValue &R, int i) {
  return getVal<uint8_t>(R);
}
template <> int16_t getVal<int16_t, 1>(const llvm::GenericValue &R, int i) {
  return getVal<int16_t>(R);
}
template <> uint16_t getVal<uint16_t, 1>(const llvm::GenericValue &R, int i) {
  return getVal<uint16_t>(R);
}
template <> int32_t getVal<int32_t, 1>(const llvm::GenericValue &R, int i) {
  return getVal<int32_t>(R);
}
template <> uint32_t getVal<uint32_t, 1>(const llvm::GenericValue &R, int i) {
  return getVal<uint32_t>(R);
}
template <> int64_t getVal<int64_t, 1>(const llvm::GenericValue &R, int i) {
  return getVal<int64_t>(R);
}
template <> uint64_t getVal<uint64_t, 1>(const llvm::GenericValue &R, int i) {
  return getVal<uint64_t>(R);
}

template <> float getVal<float, 1>(const llvm::GenericValue &R, int i) {
  return getVal<float>(R);
}
template <> double getVal<double, 1>(const llvm::GenericValue &R, int i) {
  return getVal<double>(R);
}

template <> int8_t intMax<int8_t>() { return INT8_MAX; }
template <> int16_t intMax<int16_t>() { return INT16_MAX; }
template <> int32_t intMax<int32_t>() { return INT32_MAX; }
template <> int64_t intMax<int64_t>() { return INT64_MAX; }
template <> uint8_t intMax<uint8_t>() { return UINT8_MAX; }
template <> uint16_t intMax<uint16_t>() { return UINT16_MAX; }
template <> uint32_t intMax<uint32_t>() { return UINT32_MAX; }
template <> uint64_t intMax<uint64_t>() { return UINT64_MAX; }

template <> int8_t intMin<int8_t>() { return INT8_MIN; }
template <> int16_t intMin<int16_t>() { return INT16_MIN; }
template <> int32_t intMin<int32_t>() { return INT32_MIN; }
template <> int64_t intMin<int64_t>() { return INT64_MIN; }
template <> uint8_t intMin<uint8_t>() { return 0; }
template <> uint16_t intMin<uint16_t>() { return 0; }
template <> uint32_t intMin<uint32_t>() { return 0; }
template <> uint64_t intMin<uint64_t>() { return 0; }

template <> llvm::GenericValue initWithMin<int8_t>() {
  llvm::GenericValue gv;
  gv.IntVal = APInt(8, INT8_MIN, true);
  return gv;
}
template <> llvm::GenericValue initWithMin<int16_t>() {
  llvm::GenericValue gv;
  gv.IntVal = APInt(16, INT16_MIN, true);
  return gv;
}
template <> llvm::GenericValue initWithMin<int32_t>() {
  llvm::GenericValue gv;
  gv.IntVal = APInt(32, INT32_MIN, true);
  return gv;
}
template <> llvm::GenericValue initWithMin<int64_t>() {
  llvm::GenericValue gv;
  gv.IntVal = APInt(64, INT64_MIN, true);
  return gv;
}
template <> llvm::GenericValue initWithMin<uint8_t>() {
  llvm::GenericValue gv;
  gv.IntVal = APInt(8, 0, true);
  return gv;
}
template <> llvm::GenericValue initWithMin<uint16_t>() {
  llvm::GenericValue gv;
  gv.IntVal = APInt(16, 0, true);
  return gv;
}
template <> llvm::GenericValue initWithMin<uint32_t>() {
  llvm::GenericValue gv;
  gv.IntVal = APInt(32, 0, true);
  return gv;
}
template <> llvm::GenericValue initWithMin<uint64_t>() {
  llvm::GenericValue gv;
  gv.IntVal = APInt(64, 0, true);
  return gv;
}
template <> llvm::GenericValue initWithMin<float>() {
  llvm::GenericValue gv;
  gv.FloatVal = -std::numeric_limits<float>::infinity();
  return gv;
}
template <> llvm::GenericValue initWithMin<double>() {
  llvm::GenericValue gv;
  gv.DoubleVal = -std::numeric_limits<double>::infinity();
  return gv;
}

template <> llvm::GenericValue initWithMax<int8_t>() {
  llvm::GenericValue gv;
  gv.IntVal = APInt(8, INT8_MAX, true);
  return gv;
}
template <> llvm::GenericValue initWithMax<int16_t>() {
  llvm::GenericValue gv;
  gv.IntVal = APInt(16, INT16_MAX, true);
  return gv;
}
template <> llvm::GenericValue initWithMax<int32_t>() {
  llvm::GenericValue gv;
  gv.IntVal = APInt(32, INT32_MAX, true);
  return gv;
}
template <> llvm::GenericValue initWithMax<int64_t>() {
  llvm::GenericValue gv;
  gv.IntVal = APInt(64, INT64_MAX, true);
  return gv;
}
template <> llvm::GenericValue initWithMax<uint8_t>() {
  llvm::GenericValue gv;
  gv.IntVal = APInt(8, UINT8_MAX, true);
  return gv;
}
template <> llvm::GenericValue initWithMax<uint16_t>() {
  llvm::GenericValue gv;
  gv.IntVal = APInt(16, UINT16_MAX, true);
  return gv;
}
template <> llvm::GenericValue initWithMax<uint32_t>() {
  llvm::GenericValue gv;
  gv.IntVal = APInt(32, UINT32_MAX, true);
  return gv;
}
template <> llvm::GenericValue initWithMax<uint64_t>() {
  llvm::GenericValue gv;
  gv.IntVal = APInt(64, UINT64_MAX, true);
  return gv;
}
template <> llvm::GenericValue initWithMax<float>() {
  llvm::GenericValue gv;
  gv.FloatVal = std::numeric_limits<float>::infinity();
  return gv;
}
template <> llvm::GenericValue initWithMax<double>() {
  llvm::GenericValue gv;
  gv.DoubleVal = std::numeric_limits<double>::infinity();
  return gv;
}

template <> llvm::GenericValue initWithZero<int8_t>() {
  llvm::GenericValue gv;
  gv.IntVal = APInt(8, 0, true);
  return gv;
}
template <> llvm::GenericValue initWithZero<int16_t>() {
  llvm::GenericValue gv;
  gv.IntVal = APInt(16, 0, true);
  return gv;
}
template <> llvm::GenericValue initWithZero<int32_t>() {
  llvm::GenericValue gv;
  gv.IntVal = APInt(32, 0, true);
  return gv;
}
template <> llvm::GenericValue initWithZero<int64_t>() {
  llvm::GenericValue gv;
  gv.IntVal = APInt(64, 0, true);
  return gv;
}
template <> llvm::GenericValue initWithZero<uint8_t>() {
  llvm::GenericValue gv;
  gv.IntVal = APInt(8, 0, true);
  return gv;
}
template <> llvm::GenericValue initWithZero<uint16_t>() {
  llvm::GenericValue gv;
  gv.IntVal = APInt(16, 0, true);
  return gv;
}
template <> llvm::GenericValue initWithZero<uint32_t>() {
  llvm::GenericValue gv;
  gv.IntVal = APInt(32, 0, true);
  return gv;
}
template <> llvm::GenericValue initWithZero<uint64_t>() {
  llvm::GenericValue gv;
  gv.IntVal = APInt(64, 0, true);
  return gv;
}
template <> llvm::GenericValue initWithZero<float>() {
  llvm::GenericValue gv;
  gv.FloatVal = 0.0f;
  return gv;
}
template <> llvm::GenericValue initWithZero<double>() {
  llvm::GenericValue gv;
  gv.DoubleVal = 0.0f;
  return gv;
}

template <> llvm::GenericValue initWithInteger<int8_t>(int i) {
  llvm::GenericValue gv;
  gv.IntVal = APInt(8, i, true);
  return gv;
}
template <> llvm::GenericValue initWithInteger<int16_t>(int i) {
  llvm::GenericValue gv;
  gv.IntVal = APInt(16, i, true);
  return gv;
}
template <> llvm::GenericValue initWithInteger<int32_t>(int i) {
  llvm::GenericValue gv;
  gv.IntVal = APInt(32, i, true);
  return gv;
}
template <> llvm::GenericValue initWithInteger<int64_t>(int i) {
  llvm::GenericValue gv;
  gv.IntVal = APInt(64, i, true);
  return gv;
}
template <> llvm::GenericValue initWithInteger<uint8_t>(int i) {
  llvm::GenericValue gv;
  gv.IntVal = APInt(8, i, true);
  return gv;
}
template <> llvm::GenericValue initWithInteger<uint16_t>(int i) {
  llvm::GenericValue gv;
  gv.IntVal = APInt(16, i, true);
  return gv;
}
template <> llvm::GenericValue initWithInteger<uint32_t>(int i) {
  llvm::GenericValue gv;
  gv.IntVal = APInt(32, i, true);
  return gv;
}
template <> llvm::GenericValue initWithInteger<uint64_t>(int i) {
  llvm::GenericValue gv;
  gv.IntVal = APInt(64, i, true);
  return gv;
}
template <> llvm::GenericValue initWithInteger<float>(int i) {
  llvm::GenericValue gv;
  gv.FloatVal = (float)i;
  return gv;
}
template <> llvm::GenericValue initWithInteger<double>(int i) {
  llvm::GenericValue gv;
  gv.DoubleVal = (float)i;
  return gv;
}

template <> float getOneMinus1ULP<float>() {
  ::Validation::Utils::FloatParts<float> one(1.f);
  one.AddUlps(-1);
  return one.val();
}

template <> double getOneMinus1ULP<double>() {
  ::Validation::Utils::FloatParts<double> one(1.f);
  one.AddUlps(-1);
  return one.val();
}

template <> bool isSignedType<int8_t>() { return true; }
template <> bool isSignedType<int16_t>() { return true; }
template <> bool isSignedType<int32_t>() { return true; }
template <> bool isSignedType<int64_t>() { return true; }
template <> bool isSignedType<uint8_t>() { return false; }
template <> bool isSignedType<uint16_t>() { return false; }
template <> bool isSignedType<uint32_t>() { return false; }
template <> bool isSignedType<uint64_t>() { return false; }

template <> bool predLess<int8_t>(const llvm::APInt &a, const llvm::APInt &b) {
  return a.slt(b);
}
template <> bool predLess<int16_t>(const llvm::APInt &a, const llvm::APInt &b) {
  return a.slt(b);
}
template <> bool predLess<int32_t>(const llvm::APInt &a, const llvm::APInt &b) {
  return a.slt(b);
}
template <> bool predLess<int64_t>(const llvm::APInt &a, const llvm::APInt &b) {
  return a.slt(b);
}
template <> bool predLess<uint8_t>(const llvm::APInt &a, const llvm::APInt &b) {
  return a.ult(b);
}
template <>
bool predLess<uint16_t>(const llvm::APInt &a, const llvm::APInt &b) {
  return a.ult(b);
}
template <>
bool predLess<uint32_t>(const llvm::APInt &a, const llvm::APInt &b) {
  return a.ult(b);
}
template <>
bool predLess<uint64_t>(const llvm::APInt &a, const llvm::APInt &b) {
  return a.ult(b);
}

template <> float &getRef<float>(llvm::GenericValue &R) { return R.FloatVal; }
template <> double &getRef<double>(llvm::GenericValue &R) {
  return R.DoubleVal;
}

template <> llvm::APInt &getRef<int8_t, 1>(llvm::GenericValue &R, int i) {
  return getRef<int8_t>(R);
}
template <> llvm::APInt &getRef<uint8_t, 1>(llvm::GenericValue &R, int i) {
  return getRef<uint8_t>(R);
}
template <> llvm::APInt &getRef<int16_t, 1>(llvm::GenericValue &R, int i) {
  return getRef<int16_t>(R);
}
template <> llvm::APInt &getRef<uint16_t, 1>(llvm::GenericValue &R, int i) {
  return getRef<uint16_t>(R);
}
template <> llvm::APInt &getRef<int32_t, 1>(llvm::GenericValue &R, int i) {
  return getRef<int32_t>(R);
}
template <> llvm::APInt &getRef<uint32_t, 1>(llvm::GenericValue &R, int i) {
  return getRef<uint32_t>(R);
}
template <> llvm::APInt &getRef<int64_t, 1>(llvm::GenericValue &R, int i) {
  return getRef<int64_t>(R);
}
template <> llvm::APInt &getRef<uint64_t, 1>(llvm::GenericValue &R, int i) {
  return getRef<uint64_t>(R);
}

template <> float &getRef<float, 1>(llvm::GenericValue &R, int i) {
  return getRef<float>(R);
}
template <> double &getRef<double, 1>(llvm::GenericValue &R, int i) {
  return getRef<double>(R);
}

template <> float derefPointer<float>(float *p) { return *p; }
template <> double derefPointer<double>(double *p) { return *p; }

template <> llvm::OCLBuiltinParser::BasicArgType getBasicType<int8_t>() {
  return llvm::OCLBuiltinParser::CHAR;
}
template <> llvm::OCLBuiltinParser::BasicArgType getBasicType<uint8_t>() {
  return llvm::OCLBuiltinParser::UCHAR;
}
template <> llvm::OCLBuiltinParser::BasicArgType getBasicType<int16_t>() {
  return llvm::OCLBuiltinParser::SHORT;
}
template <> llvm::OCLBuiltinParser::BasicArgType getBasicType<uint16_t>() {
  return llvm::OCLBuiltinParser::USHORT;
}
template <> llvm::OCLBuiltinParser::BasicArgType getBasicType<int32_t>() {
  return llvm::OCLBuiltinParser::INT;
}
template <> llvm::OCLBuiltinParser::BasicArgType getBasicType<uint32_t>() {
  return llvm::OCLBuiltinParser::UINT;
}
template <> llvm::OCLBuiltinParser::BasicArgType getBasicType<int64_t>() {
  return llvm::OCLBuiltinParser::LONG;
}
template <> llvm::OCLBuiltinParser::BasicArgType getBasicType<uint64_t>() {
  return llvm::OCLBuiltinParser::ULONG;
}
template <> llvm::OCLBuiltinParser::BasicArgType getBasicType<float>() {
  return llvm::OCLBuiltinParser::FLOAT;
}
template <> llvm::OCLBuiltinParser::BasicArgType getBasicType<double>() {
  return llvm::OCLBuiltinParser::DOUBLE;
}
template <> llvm::OCLBuiltinParser::BasicArgType getBasicType<long double>() {
  return llvm::OCLBuiltinParser::LONGDOUBLE;
}
template <> llvm::OCLBuiltinParser::BasicArgType getBasicType<void>() {
  return llvm::OCLBuiltinParser::VOID;
}
template <> llvm::OCLBuiltinParser::BasicArgType getBasicType<bool>() {
  return llvm::OCLBuiltinParser::BOOL;
}

llvm::GenericValue
UnimplementedBuiltin(llvm::FunctionType *FT,
                     llvm::ArrayRef<llvm::GenericValue> Args) {
  throw Exception::NotImplemented("Some OCL built-in function.");
}

llvm::GenericValue lle_X_memcpy(llvm::FunctionType *FT,
                                llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue arg0 = Args[0];
  llvm::GenericValue arg1 = Args[1];
  llvm::GenericValue size = Args[2];
  memcpy(GVTOP(arg0), GVTOP(arg1), size.IntVal.getLimitedValue());
  return llvm::GenericValue();
}
} // namespace OCLBuiltins
} // namespace Validation
