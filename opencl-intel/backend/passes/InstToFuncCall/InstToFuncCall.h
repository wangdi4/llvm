// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#ifndef __INSTTOFUNCCALL_H__
#define __INSTTOFUNCCALL_H__
#include <llvm/Pass.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/DerivedTypes.h>
#include <map>
#include <iostream>

//===----------------------------------------------------------------------===//
//
// This pass replaces LLVM IR instructions with calls to functions
//
//===----------------------------------------------------------------------===//

namespace intel{

    using namespace llvm;
    class Inst2FunctionLookup
    {
    public:
        typedef std::pair<const char*, CallingConv::ID> LookupValue;

        Inst2FunctionLookup(bool isV16Supported) {
            //TODO: move this away from here
            Type2ValueLookup FPToUI_Lookup;
            Type2ValueLookup FPToSI_Lookup;
            Type2ValueLookup UIToFP_Lookup;
            Type2ValueLookup SIToFP_Lookup;
            Type2ValueLookup FPTrunc_Lookup;

            /// Replaces:
            /// %conv = fptoui double %tmp2 to i64
            /// With:
            /// %call_conv = call i64 @_Z13convert_ulongd(double %tmp2) nounwind
            FPToUI_Lookup[std::make_pair(Integer64,Double)] = std::make_pair("_Z13convert_ulongd", CallingConv::C);

            /// Replaces:
            /// %conv = fptoui float %tmp2 to i64
            /// With:
            /// %call_conv = call i64 @_Z13convert_ulongf(float %tmp2) nounwind
            FPToUI_Lookup[std::make_pair(Integer64,Float)] = std::make_pair("_Z13convert_ulongf", CallingConv::C);

            /// Replaces:
            /// %conv = fptoui double %tmp2 to i32
            /// With:
            /// %call_conv = call i32 @_Z12convert_uintd(double %tmp2) nounwind
            FPToUI_Lookup[std::make_pair(Integer32,Double)] = std::make_pair("_Z12convert_uintd", CallingConv::C);

            /// Replaces:
            /// %conv = sitofp i64 %tmp2 to double
            /// With:
            /// %call_conv = call double @_Z14convert_doublel(i64 %tmp2) nounwind
            SIToFP_Lookup[std::make_pair(Double,Integer64)] = std::make_pair("_Z14convert_doublel", CallingConv::C);

            if (isV16Supported)
            {
                /// Replaces:
                /// %conv = fptoui <16 x float> %tmp2 to <16 x i64>
                /// With:
                /// %call_conv = call <16 x i64> @_Z15convert_ulong16Dv16_f(<16 x float> %tmp2) nounwind
                FPToUI_Lookup[std::make_pair(v16xInteger64,v16xFloat)] = std::make_pair("_Z15convert_ulong16Dv16_f", CallingConv::C);

                /// Replaces:
                /// %conv = fptoui <16 x double> %tmp2 to <16 x i64>
                /// With:
                /// %call_conv = call <16 x i64> @_Z15convert_ulong16Dv16_d(<16 x double> %tmp2) nounwind
                FPToUI_Lookup[std::make_pair(v16xInteger64,v16xDouble)] = std::make_pair("_Z15convert_ulong16Dv16_d", CallingConv::C);

                /// Replaces:
                /// %conv = fptosi float %tmp2 to i64
                /// With:
                /// %call_conv = call i64 @_Z12convert_longf(float %tmp2) nounwind
                FPToSI_Lookup[std::make_pair(Integer64,Float)] = std::make_pair("_Z12convert_longf", CallingConv::C);

                /// Replaces:
                /// %conv = fptosi <16 x float> %tmp2 to <16 x i64>
                /// With:
                /// %call_conv = call <16 x i64> @_Z14convert_long16Dv16_f(<16 x float> %tmp2) nounwind
                FPToSI_Lookup[std::make_pair(v16xInteger64,v16xFloat)] = std::make_pair("_Z14convert_long16Dv16_f", CallingConv::C);

                /// Replaces:
                /// %conv = fptosi double %tmp2 to i64
                /// With:
                /// %call_conv = call i64 @_Z12convert_longd(double %tmp2) nounwind
                FPToSI_Lookup[std::make_pair(Integer64,Double)] = std::make_pair("_Z12convert_longd", CallingConv::C);

                /// Replaces:
                /// %conv = fptosi <16 x double> %tmp2 to <16 x i64>
                /// With:
                /// %call_conv = call <16 x i64> @_Z14convert_long16Dv16_d(<16 x double> %tmp2) nounwind
                FPToSI_Lookup[std::make_pair(v16xInteger64,v16xDouble)] = std::make_pair("_Z14convert_long16Dv16_d", CallingConv::C);

                /// Replaces:
                /// %conv = sitofp i64 %tmp2 to float
                /// With:
                /// %call_conv = call float @_Z13convert_floatl(i64 %tmp2) nounwind
                SIToFP_Lookup[std::make_pair(Float,Integer64)] = std::make_pair("_Z13convert_floatl", CallingConv::C);

                /// Replaces:
                /// %conv = sitofp <16 x i64> %tmp2 to <16 x float>
                /// With:
                /// %call_conv = call <16 x float> @_Z15convert_float16Dv16_l(<16 x i64> %tmp2) nounwind
                SIToFP_Lookup[std::make_pair(v16xFloat,v16xInteger64)] = std::make_pair("_Z15convert_float16Dv16_l", CallingConv::C);

                /// Replaces:
                /// %conv = uitofp i64 %tmp2 to double
                /// With:
                /// %call_conv = call double @_Z13convert_doublem(i64 %tmp2) nounwind
                UIToFP_Lookup[std::make_pair(Double,Integer64)] = std::make_pair("_Z14convert_doublem", CallingConv::C);

                /// Replaces:
                /// %conv = uitofp <16 x i64> %tmp2 to <16 x float>
                /// With:
                /// %call_conv = call <16 x float> @_Z15convert_float16Dv16_m(<16 x i64> %tmp2) nounwind
                UIToFP_Lookup[std::make_pair(v16xFloat,v16xInteger64)] = std::make_pair("_Z15convert_float16Dv16_m", CallingConv::C);

                /// Replaces:
                /// %conv = uitofp <16 x i64> %tmp2 to <16 x double>
                /// With:
                /// %call_conv = call <16 x double> @_Z16convert_double16Dv16_m(<16 x i64> %tmp2) nounwind
                UIToFP_Lookup[std::make_pair(v16xDouble,v16xInteger64)] = std::make_pair("_Z16convert_double16Dv16_m", CallingConv::C);

                /// Replaces:
                /// %conv = uitofp i64 %tmp2 to float
                /// With:
                /// %call_conv = call float @_Z13convert_floatm(i64 %tmp2) nounwind
                UIToFP_Lookup[std::make_pair(Float,Integer64)] = std::make_pair("_Z13convert_floatm", CallingConv::C);
            }

            FPTrunc_Lookup[std::make_pair(Half,Double)] = std::make_pair("convert_halfd", CallingConv::C);

            m_Lookup[Instruction::UIToFP] = UIToFP_Lookup;
            m_Lookup[Instruction::SIToFP] = SIToFP_Lookup;
            m_Lookup[Instruction::FPToUI] = FPToUI_Lookup;
            m_Lookup[Instruction::FPToSI] = FPToSI_Lookup;
            m_Lookup[Instruction::FPTrunc] = FPTrunc_Lookup;
        }

        const LookupValue *operator [](const Instruction &inst) const {
            Opcode2T2VLookup::const_iterator iter = m_Lookup.find(inst.getOpcode());
            if (iter == m_Lookup.end()) {
                return 0;
            }

            const Type2ValueLookup &Lookup2 = ( *iter ).second;

            Value* in = inst.getOperand(0);
            Type* inType = in->getType();
            TypeInfo TI = getTypeInfo(inType);
            if (Unknown == TI) return 0;

            Type* outType = inst.getType();
            TypeInfo TO = getTypeInfo(outType);
            if (Unknown == TO) return 0;

            Type2ValueLookup::const_iterator iter2 = Lookup2.find(std::make_pair(TO,TI));
            // If we are here, the lookup must have the mapping.
            if (iter2 == Lookup2.end()) {
                return 0;
            }
            return &( *iter2 ).second;
        }
    private:
        // maps [DstType,SrcType] --> LookupValue
        typedef std::map< std::pair<unsigned,unsigned>, LookupValue> Type2ValueLookup;

        // maps instruction opcode --> Type2ValueLookup
        typedef std::map<unsigned, Type2ValueLookup> Opcode2T2VLookup;

        enum TypeInfo {
            Unknown = 0,
            Integer32 = 1,
            Integer64 = 2,
            Float = 3,
            Double = 4,
            Half = 5,
            v16xInteger64 = 6,
            v16xFloat = 7,
            v16xDouble = 8
        };
        Opcode2T2VLookup m_Lookup;

        static TypeInfo getTypeInfo(Type *type)
        {
            if (IntegerType *tp = dyn_cast<IntegerType>(type))
            {
                switch (tp->getBitWidth())
                {
                default: return Unknown;
                case 32: return Integer32;
                case 64: return Integer64;
                }
            }

            if (type->isFloatTy())
                return Float;
            if (type->isDoubleTy())
                return Double;
            if (type->isHalfTy())
                return Half;
            if (type->isVectorTy())
            {
                VectorType *vt = cast<VectorType>(type);
                if (vt->getNumElements() != 16) return Unknown;
                Type *et = vt->getElementType();
                if (et->isFloatTy())
                    return v16xFloat;
                if (et->isDoubleTy())
                    return v16xDouble;
                if (IntegerType * it = dyn_cast<IntegerType>(et))
                    if (it->getBitWidth() == 64)
                        return v16xInteger64;
            }

            return Unknown;
        }
    };

    // Update kernel assembly to match our execution environment
    class InstToFuncCall : public ModulePass
    {
    public:
        static char ID; // Pass identification, replacement for typeid

        InstToFuncCall(bool isV16Supported = true);

        bool runOnModule(Module &M);

    private:
        Inst2FunctionLookup m_I2F;

        static void replaceInstWithCall(Function *func,
                                        BasicBlock::iterator &II,
                                        const char* funcName,
                                        CallingConv::ID CC);

    };

/// Returns an instance of the Inst2Func pass,
/// which will be added to a PassManager and run on a Module.
    llvm::ModulePass *createInstToFuncCallPass(bool isV16Supported);

}

#endif
