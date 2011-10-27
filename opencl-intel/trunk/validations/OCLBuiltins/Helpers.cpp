/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  Helpers.cpp

\*****************************************************************************/

#include "Helpers.h"
#include "Exception.h"

using namespace llvm;
namespace Validation {
namespace OCLBuiltins {

    llvm::GenericValue UnimplementedBuiltin(const llvm::FunctionType *FT, const std::vector<llvm::GenericValue> &Args)
    {
        throw Exception::NotImplemented("Some OCL built-in function.");
    }

    template<> bool isSignedType<int8_t>() {return true;}
    template<> bool isSignedType<int16_t>() {return true;}
    template<> bool isSignedType<int32_t>() {return true;}
    template<> bool isSignedType<int64_t>() {return true;}
    template<> bool isSignedType<uint8_t>() {return false;}
    template<> bool isSignedType<uint16_t>() {return false;}
    template<> bool isSignedType<uint32_t>() {return false;}
    template<> bool isSignedType<uint64_t>() {return false;}

    template<> float& getRef<float>(llvm::GenericValue& R){return R.FloatVal;}
    template<> double& getRef<double>(llvm::GenericValue& R){return R.DoubleVal;}

    template<> bool predLess<int8_t> (const llvm::APInt& a, const llvm::APInt& b) { return a.slt(b); }
    template<> bool predLess<int16_t>(const llvm::APInt& a, const llvm::APInt& b) { return a.slt(b); }
    template<> bool predLess<int32_t>(const llvm::APInt& a, const llvm::APInt& b) { return a.slt(b); }
    template<> bool predLess<int64_t>(const llvm::APInt& a, const llvm::APInt& b) { return a.slt(b); }
    template<> bool predLess<uint8_t> (const llvm::APInt& a, const llvm::APInt& b) { return a.ult(b); }
    template<> bool predLess<uint16_t>(const llvm::APInt& a, const llvm::APInt& b) { return a.ult(b); }
    template<> bool predLess<uint32_t>(const llvm::APInt& a, const llvm::APInt& b) { return a.ult(b); }
    template<> bool predLess<uint64_t>(const llvm::APInt& a, const llvm::APInt& b) { return a.ult(b); }

    template<> llvm::APInt& getRef<int8_t,1>(llvm::GenericValue &R, int i){return getRef<int8_t>(R);}
    template<> llvm::APInt& getRef<uint8_t,1>(llvm::GenericValue &R, int i){return getRef<uint8_t>(R);}
    template<> llvm::APInt& getRef<int16_t,1>(llvm::GenericValue &R, int i){return getRef<int16_t>(R);}
    template<> llvm::APInt& getRef<uint16_t,1>(llvm::GenericValue &R, int i){return getRef<uint16_t>(R);}
    template<> llvm::APInt& getRef<int32_t,1>(llvm::GenericValue &R, int i){return getRef<int32_t>(R);}
    template<> llvm::APInt& getRef<uint32_t,1>(llvm::GenericValue &R, int i){return getRef<uint32_t>(R);}
    template<> llvm::APInt& getRef<int64_t,1>(llvm::GenericValue &R, int i){return getRef<int64_t>(R);}
    template<> llvm::APInt& getRef<uint64_t,1>(llvm::GenericValue &R, int i){return getRef<uint64_t>(R);}

    template<> float& getRef<float,1>(llvm::GenericValue &R, int i) { return getRef<float>(R);}
    template<> double& getRef<double,1>(llvm::GenericValue &R, int i) { return getRef<double>(R);}

    template<> float derefPointer<float>(float* p) {return *p;}
    template<> double derefPointer<double>(double* p) {return *p;}

    template<> llvm::OCLBuiltinParser::BasicArgType getBasicType<int8_t>(){return llvm::OCLBuiltinParser::CHAR;}
    template<> llvm::OCLBuiltinParser::BasicArgType getBasicType<uint8_t>(){return llvm::OCLBuiltinParser::UCHAR;}
    template<> llvm::OCLBuiltinParser::BasicArgType getBasicType<int16_t>(){return llvm::OCLBuiltinParser::SHORT;}
    template<> llvm::OCLBuiltinParser::BasicArgType getBasicType<uint16_t>(){return llvm::OCLBuiltinParser::USHORT;}
    template<> llvm::OCLBuiltinParser::BasicArgType getBasicType<int32_t>(){return llvm::OCLBuiltinParser::INT;}
    template<> llvm::OCLBuiltinParser::BasicArgType getBasicType<uint32_t>(){return llvm::OCLBuiltinParser::UINT;}
    template<> llvm::OCLBuiltinParser::BasicArgType getBasicType<int64_t>(){return llvm::OCLBuiltinParser::LONG;}
    template<> llvm::OCLBuiltinParser::BasicArgType getBasicType<uint64_t>(){return llvm::OCLBuiltinParser::ULONG;}
    template<> llvm::OCLBuiltinParser::BasicArgType getBasicType<float>(){return llvm::OCLBuiltinParser::FLOAT;}
    template<> llvm::OCLBuiltinParser::BasicArgType getBasicType<double>(){return llvm::OCLBuiltinParser::DOUBLE;}
    template<> llvm::OCLBuiltinParser::BasicArgType getBasicType<long double>(){return llvm::OCLBuiltinParser::LONGDOUBLE;}
    template<> llvm::OCLBuiltinParser::BasicArgType getBasicType<void>(){return llvm::OCLBuiltinParser::VOID;}
    template<> llvm::OCLBuiltinParser::BasicArgType getBasicType<bool>(){return llvm::OCLBuiltinParser::BOOL;}

    template<> float getVal<float>(const llvm::GenericValue& R){return R.FloatVal;}
    template<> double getVal<double>(const llvm::GenericValue& R){return R.DoubleVal;}

    template<> int8_t getVal<int8_t,1>(const llvm::GenericValue &R, int i){return getVal<int8_t>(R);}
    template<> uint8_t getVal<uint8_t,1>(const llvm::GenericValue &R, int i){return getVal<uint8_t>(R);}
    template<> int16_t getVal<int16_t,1>(const llvm::GenericValue &R, int i){return getVal<int16_t>(R);}
    template<> uint16_t getVal<uint16_t,1>(const llvm::GenericValue &R, int i){return getVal<uint16_t>(R);}
    template<> int32_t getVal<int32_t,1>(const llvm::GenericValue &R, int i){return getVal<int32_t>(R);}
    template<> uint32_t getVal<uint32_t,1>(const llvm::GenericValue &R, int i){return getVal<uint32_t>(R);}
    template<> int64_t getVal<int64_t,1>(const llvm::GenericValue &R, int i){return getVal<int64_t>(R);}
    template<> uint64_t getVal<uint64_t,1>(const llvm::GenericValue &R, int i){return getVal<uint64_t>(R);}

    template<> float getVal<float,1>(const llvm::GenericValue &R, int i) { return getVal<float>(R);}
    template<> double getVal<double,1>(const llvm::GenericValue &R, int i) { return getVal<double>(R);}

    template<> int8_t   intMax<int8_t>() {return INT8_MAX;}
    template<> int16_t  intMax<int16_t>() {return INT16_MAX;}
    template<> int32_t  intMax<int32_t>() {return INT32_MAX;}
    template<> int64_t  intMax<int64_t>() {return INT64_MAX;}
    template<> uint8_t  intMax<uint8_t>() {return UINT8_MAX;}
    template<> uint16_t intMax<uint16_t>() {return UINT16_MAX;}
    template<> uint32_t intMax<uint32_t>() {return UINT32_MAX;}
    template<> uint64_t intMax<uint64_t>() {return UINT64_MAX;}

    template<> int8_t   intMin<int8_t>() {return INT8_MIN;}
    template<> int16_t  intMin<int16_t>() {return INT16_MIN;}
    template<> int32_t  intMin<int32_t>() {return INT32_MIN;}
    template<> int64_t  intMin<int64_t>() {return INT64_MIN;}
    template<> uint8_t  intMin<uint8_t>() {return 0;}
    template<> uint16_t intMin<uint16_t>() {return 0;}
    template<> uint32_t intMin<uint32_t>() {return 0;}
    template<> uint64_t intMin<uint64_t>() {return 0;}

    template<> float getOneMinus1ULP<float>() {
        ::Validation::Utils::FloatParts<float> one(1.f);
        one.AddUlps(-1);
        return one.val();
    }

    template<> double getOneMinus1ULP<double>() {
        ::Validation::Utils::FloatParts<double> one(1.f);
        one.AddUlps(-1);
        return one.val();
    }

    llvm::GenericValue lle_X_memcpy(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args)
    {
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        llvm::GenericValue size = Args[2];
        memcpy(GVTOP(arg0), GVTOP(arg1), size.IntVal.getLimitedValue());
        return llvm::GenericValue();
    }
}
}
