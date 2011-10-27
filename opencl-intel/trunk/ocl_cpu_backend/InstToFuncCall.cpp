#include "InstToFuncCall.h"
#include <llvm/Pass.h>
#include <llvm/Module.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Instructions.h>
#include <llvm/Constants.h>

#include <map>
using namespace llvm;

//===----------------------------------------------------------------------===//
//
// This pass replaces LLVM IR instructions with calls to functions
//
//===----------------------------------------------------------------------===//


namespace Intel { namespace OpenCL { namespace DeviceBackend {

   
    class Inst2FunctionLookup 
    {
    public:
        typedef std::pair<const char*, CallingConv::ID> LookupValue;

        Inst2FunctionLookup() {		
            //TODO: move this away from here
            Type2ValueLookup FPToUI_Lookup;
            Type2ValueLookup UIToFP_Lookup;
            Type2ValueLookup SIToFP_Lookup;

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
            /// %conv = sitofp i64 %tmp2 to float
            /// With:
            /// %call_conv = call float @_Z13convert_floatl(i64 %tmp2) nounwind
            SIToFP_Lookup[std::make_pair(Float,Integer64)] = std::make_pair("_Z13convert_floatl", CallingConv::C);

            /// Replaces:
            /// %conv = sitofp i64 %tmp2 to double
            /// With:
            /// %call_conv = call double @_Z14convert_doublel(i64 %tmp2) nounwind
            SIToFP_Lookup[std::make_pair(Double,Integer64)] = std::make_pair("_Z14convert_doublel", CallingConv::C);

            /// Replaces:
            /// %conv = uitofp i64 %tmp2 to float
            /// With:
            /// %call_conv = call float @_Z13convert_floatm(i64 %tmp2) nounwind
            UIToFP_Lookup[std::make_pair(Float,Integer64)] = std::make_pair("_Z13convert_floatm", CallingConv::C);


            m_Lookup[Instruction::UIToFP] = UIToFP_Lookup;
            m_Lookup[Instruction::SIToFP] = SIToFP_Lookup;
            m_Lookup[Instruction::FPToUI] = FPToUI_Lookup;
        }

        const LookupValue *operator [](const Instruction &inst) const {
            Opcode2T2VLookup::const_iterator iter = m_Lookup.find(inst.getOpcode());
            if (iter == m_Lookup.end()) {
                return 0;
            }

            const Type2ValueLookup &Lookup2 = ( *iter ).second;

            Value* in = inst.getOperand(0);
            const Type* inType = in->getType();
            TypeInfo TI = getTypeInfo(inType);
            if (Unknown == TI) return 0;

            const Type* outType = inst.getType();
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
            Double = 4
        };
        Opcode2T2VLookup m_Lookup;

        static TypeInfo getTypeInfo(const Type *type) 
        {
            if (const IntegerType *tp = dyn_cast<IntegerType>(type)) 
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

            return Unknown;
        }
    };

    // Update kernel assembly to match our execution environment
    class InstToFuncCall : public ModulePass
    {
    public:
        InstToFuncCall();
        bool runOnModule(Module &M);

    private:
        static char ID; // Pass identification, replacement for typeid
        Inst2FunctionLookup m_I2F;

        static void replaceInstWithCall(Function *func, Instruction* inst, const char* funcName, CallingConv::ID CC);

    };

    InstToFuncCall::InstToFuncCall() : ModulePass(ID) {}

    /// Replaces instruction 'inst' with call to function 'funcName' which has a 
	/// calling convention 'CC'.
    void InstToFuncCall::replaceInstWithCall(Function *func, 
        Instruction* inst, const char* funcName, CallingConv::ID CC) 
    {
		
		Value *Op0 = inst->getOperand(0);
		
		std::vector<Value*>      args (1, Op0); // arguments
		std::vector<const Type*> types(1, Op0->getType()); // type of args
		
		FunctionType *proto = FunctionType::get(inst->getType(), types, false);
		Constant* new_f = func->getParent()->getOrInsertFunction(funcName, proto);
		CallInst* call = CallInst::Create(new_f, args.begin(), args.end(), "call_conv", inst);
		call->setCallingConv(CC);
		// replace all users with new function call, DCE will take care of it
		inst->replaceAllUsesWith(call);
	}
    
    bool InstToFuncCall::runOnModule(Module &M) 
    {
        bool changed = false;
        
        // for each function
        
        Module::FunctionListType &FL = M.getFunctionList();
        for (Module::iterator fn = FL.begin(), fne = FL.end(); fn != fne; ++fn) 
        {
            // for each bb
            for (Function::iterator bb = fn->begin(), bbe = fn->end(); bb != bbe; ++bb) {
                // for each instruction
                for(BasicBlock::iterator it = bb->begin(), e = bb->end(); it != e ; ++it) {				
                    // See if a mapping exists for replacing this instruction class
                    const Inst2FunctionLookup::LookupValue *LV = m_I2F[*it];				
                    if (0 == LV) {
                        continue;
                    }
                    replaceInstWithCall(&(*fn), &(*it), LV->first, LV->second);
                    changed = true;
                }
            }
        
        }

        return changed;
    }

    ModulePass *createInstToFuncCallPass() { return new InstToFuncCall(); }

    char InstToFuncCall::ID;

    }}} // namespace
