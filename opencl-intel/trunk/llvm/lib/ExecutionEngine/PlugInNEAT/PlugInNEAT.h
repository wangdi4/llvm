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

File Name:  PlugInNEAT.h


\*****************************************************************************/

#ifndef LLI_PLUGIN_NEAT_H
#define LLI_PLUGIN_NEAT_H

#include "llvm/Function.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Support/CallSite.h"
#include "llvm/System/DataTypes.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/InstVisitor.h"
#include "llvm/Support/raw_ostream.h"
#include "InterpreterPlugIn.h"
#include "llvm/Support/InstVisitor.h"

#include "BufferContainerList.h"
#include "IBufferContainer.h"
#include "IMemoryObject.h"
#include "NEATValue.h"
#include "NEATVector.h"
#include "OCLBuiltinParser.h"

namespace llvm {

  class NEATTargetData;

  class NEATStructLayout {
  public:
    uint64_t getElementOffset(unsigned Idx) const {
      assert(Idx < NumElements && "Invalid element idx!");
      return MemberOffsets[Idx];
    }
  private:
    uint64_t StructSize;
    unsigned StructAlignment;
    unsigned NumElements;
    uint64_t MemberOffsets[1];  // variable sized array!
    friend class NEATTargetData;   // Only NEATTargetData can create this class
    NEATStructLayout(const StructType *ST, const NEATTargetData &TD);
  };

  class NEATTargetData
  {
  public:

    /// getTypeStoreSize - Return the maximum number of bytes that may be
    /// overwritten by storing the specified type.  For example, returns 5
    /// for i36 and 10 for x86_fp80.
    uint64_t getTypeStoreSize(const Type *Ty) const;

    /// InitMemory - Initializes memory given by Memory pointer. It calls NEATValue constructor
    /// for all NEATValues
    void InitMemory( void* Memory, const Type* Ty ) const;

    /// getTypeAllocSize - Return the offset in bytes between successive objects
    /// of the specified type, including alignment padding.  This is the amount
    /// that alloca reserves for this type.  For example, returns 12 or 16 for
    /// x86_fp80, depending on alignment.
    uint64_t getTypeAllocSize(const Type* Ty) const;

    static bool IsNEATSupported(const Type *Ty);

    /// getABITypeAlignment - Return the minimum ABI-required alignment for the
    /// specified type.
    unsigned getABITypeAlignment(const Type *Ty) const;

    /// getStructLayout - Return a StructLayout object, indicating the alignment
    /// of the struct, its size, and the offsets of its fields.  Note that this
    /// information is lazily cached.
    const NEATStructLayout *getStructLayout(const StructType *Ty) const;

  };

    /// NEAT Generic Value container struct
    struct NEATGenericValue
    {
        union {
            PointerTy       PointerVal;
            unsigned char   Untyped[8];
        };
        Validation::NEATValue       NEATVal;
        Validation::NEATVector      NEATVec;

        // For aggregate data types
        std::vector<NEATGenericValue> AggregateVal;

        explicit NEATGenericValue(void *V) : PointerVal(V) {}
        NEATGenericValue() : PointerVal(NULL) {}
    };

    inline NEATGenericValue PTONGV(void *P) { return NEATGenericValue(P); }
    inline void* NGVTOP(const NEATGenericValue &GV) { return GV.PointerVal; }

    /// NEATExecutionContext struct - This struct represents one stack frame currently
    /// executing.
    ///
    struct NEATExecutionContext {
        std::map<Value *, NEATGenericValue> Values; // LLVM values used in this invocation
        // todo: decide what to do with VarArgs
        //std::vector<NEATGenericValue>  VarArgs; // Values passed through an ellipsis
        AllocaHolderHandle    Allocas;    // Track memory allocated by alloca
    };

    /// Function creates NEAT "mirror" of OCL program inputs
    /// NEAT Buffers are created with the same size as input Buffers
    /// Input buffers are copied to corresponding NEAT Buffers
    /// NEAT Values are initialized with ACCURATE flag and value from input buffer
    /// general idea is output BufferContainerList(BCL) should have
    /// the same number of buffers as input BCL
    /// it is done since Comparator interface
    /// accepts inputs BCLs with the equal number of buffers
    /// @returns Created BufferContainer object with NEAT
    Validation::IBufferContainer * CreateNEATBufferContainer(
        const Validation::IBufferContainer& BC,
        Validation::IBufferContainerList *);

    /// create arguments map for NEAT from function in_F and BufferContainer in_BC
    void CreateNEATBufferContainerMap(Function *in_F,
                                      Validation::IBufferContainer& in_BC,
                                      std::map<Value *, NEATGenericValue>& out_NEATArgValues);

    class NEATPlugIn : public InstVisitor<NEATPlugIn>, public InterpreterPlugIn
    {
    public:
        /// ctor
        NEATPlugIn () :
            m_pInterp(NULL), m_pECStack(NULL)
        {
            m_CurEvent = BAD_EVENT;
            m_NECStack.clear();
            m_ArgValues.clear();
            m_GlobalAddressMap.clear();
        }

        /// virtual dtor
        virtual ~NEATPlugIn();

        // map for mapping global variables to NEAT variables
        typedef std::map<const GlobalValue *, void *>
            GlobalAddressMapTy;

        /// set NEAT arguments of runFunction(). in runFunction() execution starts
        void SetArgValues(std::map<Value *, NEATGenericValue> &val)
        {
            m_ArgValues = val;
        }
        /// init before interpreter starts running function
        virtual void handlePreFunctionRun( std::vector<ExecutionContext>&,
            llvm::InterpreterPluggable&);
        /// event after function call
        virtual void handlePostFunctionRun();
        /// handle event prior to instruction execution
        virtual void handlePreInstExecution(Instruction& );
        /// handle event post to instruction execution
        virtual void handlePostInstExecution(Instruction& );

        ///////////////////////////////////////////////////////////////////////
        /// Opcode Implementations

        void visitLoadInst(LoadInst &I);
        void visitStoreInst(StoreInst &I);
        void visitAllocaInst(AllocaInst &I);
        void visitGetElementPtrInst(GetElementPtrInst &I);
        void visitBinaryOperator(BinaryOperator &I);
        void visitReturnInst(ReturnInst &I);
        void visitExtractElementInst(ExtractElementInst &I);
        void visitInsertElementInst(InsertElementInst &I);
        void visitShuffleVectorInst(ShuffleVectorInst &I);
        void visitFCmpInst(FCmpInst &I);
        void visitUIToFPInst(UIToFPInst &I);
        void visitSIToFPInst(SIToFPInst &I);
        void visitSelectInst(SelectInst &I);
        void visitFPTruncInst(FPTruncInst &I);
        void visitFPExtInst(FPExtInst &I);
        void visitBitCastInst(BitCastInst &I);
        void visitExtractValueInst(ExtractValueInst &I);
        void visitInsertValueInst(InsertValueInst &I);
        void visitCallSite(CallSite CS);
        void visitCallInst(CallInst &I) { visitCallSite (CallSite (&I)); }
        void visitInvokeInst(InvokeInst &I) { visitCallSite (CallSite (&I)); }
        void visitBranchInst(BranchInst &I);

        /// getOrEmitGlobalVariable - Return the address of the specified global
        /// variable, possibly emitting it to memory if needed.
        void *getOrEmitGlobalVariable(const GlobalVariable *GV);

        /// updateGlobalMapping - Replace an existing mapping for GV with a new
        /// address.  This updates map as required.
        void *updateGlobalMapping(const GlobalValue *GV, void *Addr);

        /// getPointerToGlobal - This returns the address of the specified global
        /// value.  This may involve code generation if it's a function.
        ///
        void *getPointerToGlobal(const GlobalValue *GV);

    private:

        void SwitchToNewBasicBlock(BasicBlock *Dest, ExecutionContext &SF, NEATExecutionContext &NSF);
        /// Obtain integer argument for built-in functions.
        GenericValue GetGenericArg(size_t ArgIdx);
        // Macro to declare methods like execute_namen
        // e.g. execute_vload2, execute_vload3 and so on.
#define DECLARE_EXECUTE_N(_name, n)                     \
        void execute_ ## _name ## n (Function *F,      \
            const std::map<Value *, NEATGenericValue> &ArgVals, \
            NEATGenericValue& Result,                           \
            const OCLBuiltinParser::ArgVector& ArgList)

#define DECLARE_EXECUTE_ALL_N(blt_name)          \
        DECLARE_EXECUTE_N(blt_name, 2);          \
        DECLARE_EXECUTE_N(blt_name, 3);          \
        DECLARE_EXECUTE_N(blt_name, 4);          \
        DECLARE_EXECUTE_N(blt_name, 8);          \
        DECLARE_EXECUTE_N(blt_name, 16)

#define DECLARE_EXECUTE_N_WITH_SUFFIX(_name, n, suffix)                     \
        void execute_ ## _name ## n ## suffix(Function *F,      \
            const std::map<Value *, NEATGenericValue> &ArgVals, \
            NEATGenericValue& Result,                           \
            const OCLBuiltinParser::ArgVector& ArgList)

#define DECLARE_EXECUTE_ALL_N_WITH_SUFFIX(blt_name, suffix)          \
        DECLARE_EXECUTE_N_WITH_SUFFIX(blt_name, 2, suffix);          \
        DECLARE_EXECUTE_N_WITH_SUFFIX(blt_name, 3, suffix);          \
        DECLARE_EXECUTE_N_WITH_SUFFIX(blt_name, 4, suffix);          \
        DECLARE_EXECUTE_N_WITH_SUFFIX(blt_name, 8, suffix);          \
        DECLARE_EXECUTE_N_WITH_SUFFIX(blt_name, 16, suffix)

        DECLARE_EXECUTE_ALL_N(convert_float);
        DECLARE_EXECUTE_ALL_N(convert_double);
        DECLARE_EXECUTE_ALL_N(vload);
        DECLARE_EXECUTE_ALL_N(vload_half);
        DECLARE_EXECUTE_ALL_N(vloada_half);
        DECLARE_EXECUTE_ALL_N(vstore);
        DECLARE_EXECUTE_ALL_N(vstore_half);
        DECLARE_EXECUTE_ALL_N(vstorea_half);
        DECLARE_EXECUTE_ALL_N_WITH_SUFFIX(vstore_half,_rte);
        DECLARE_EXECUTE_ALL_N_WITH_SUFFIX(vstore_half,_rtz);
        DECLARE_EXECUTE_ALL_N_WITH_SUFFIX(vstore_half,_rtp);
        DECLARE_EXECUTE_ALL_N_WITH_SUFFIX(vstore_half,_rtn);
        DECLARE_EXECUTE_ALL_N_WITH_SUFFIX(vstorea_half,_rte);
        DECLARE_EXECUTE_ALL_N_WITH_SUFFIX(vstorea_half,_rtz);
        DECLARE_EXECUTE_ALL_N_WITH_SUFFIX(vstorea_half,_rtp);
        DECLARE_EXECUTE_ALL_N_WITH_SUFFIX(vstorea_half,_rtn);

#define DECLARE_EXECUTE(_name)                              \
        void execute_ ## _name(Function *F,                 \
        const std::map<Value *, NEATGenericValue> &ArgVals, \
            NEATGenericValue& Result,                       \
            const OCLBuiltinParser::ArgVector& ArgList)
        DECLARE_EXECUTE(convert_float);
        DECLARE_EXECUTE(convert_double);
        DECLARE_EXECUTE(async_work_group_copy);
        DECLARE_EXECUTE(async_work_group_strided_copy);
        DECLARE_EXECUTE(ldexp);
        DECLARE_EXECUTE(rootn);
        DECLARE_EXECUTE(read_imagef);
        DECLARE_EXECUTE(vload_half);
        DECLARE_EXECUTE(vloada_half);
        DECLARE_EXECUTE(vstore_half);
        DECLARE_EXECUTE(vstorea_half);
        DECLARE_EXECUTE(vstore_half_rte);
        DECLARE_EXECUTE(vstore_half_rtz);
        DECLARE_EXECUTE(vstore_half_rtp);
        DECLARE_EXECUTE(vstore_half_rtn);
        DECLARE_EXECUTE(vstorea_half_rte);
        DECLARE_EXECUTE(vstorea_half_rtz);
        DECLARE_EXECUTE(vstorea_half_rtp);
        DECLARE_EXECUTE(vstorea_half_rtn);
        DECLARE_EXECUTE(ISequal);
        DECLARE_EXECUTE(ISnotequal);
        DECLARE_EXECUTE(ISgreater);
        DECLARE_EXECUTE(ISgreaterequal);
        DECLARE_EXECUTE(ISless);
        DECLARE_EXECUTE(ISlessequal);
        DECLARE_EXECUTE(ISlessgreater);
        DECLARE_EXECUTE(ISfinite);
        DECLARE_EXECUTE(ISinf);
        DECLARE_EXECUTE(ISnan);
        DECLARE_EXECUTE(ISnormal);
        DECLARE_EXECUTE(ISordered);
        DECLARE_EXECUTE(ISunordered);
        DECLARE_EXECUTE(signbit);
        DECLARE_EXECUTE(ilogb);
        DECLARE_EXECUTE(pown);
        DECLARE_EXECUTE(nan);
        DECLARE_EXECUTE(smoothstep);
        DECLARE_EXECUTE(step);
        DECLARE_EXECUTE(clamp);
        DECLARE_EXECUTE(select); // built-in, not instruction

#undef DECLARE_EXECUTE_ALL_N
#undef DECLARE_EXECUTE_N
#undef DECLARE_EXECUTE_ALL_N_WITH_SUFFIX
#undef DECLARE_EXECUTE_N_WITH_SUFFIX
#undef DECLARE_EXECUTE

        /// obtain operant from NEAT context
        NEATGenericValue getOperandValue(Value *V, NEATExecutionContext &SF);
        /// loads NEAT data from memory
        void LoadValueFromMemory(NEATGenericValue &Result,
            NEATGenericValue *NEATPtr, const Type *Ty) ;
        /// stores NEAT data to memory
        void StoreValueToMemory(const NEATGenericValue &Val,
            NEATGenericValue *Ptr, const Type *Ty);
        /// workhorse for getelementptr
        NEATGenericValue executeGEPOperation(Value *Ptr, gep_type_iterator I,
            gep_type_iterator E, NEATExecutionContext &SF);
        /// setup function call
        void callFunction( Function *F,
            const std::map<Value *, NEATGenericValue> &ArgVals );
        /// ptr to interpreter
        InterpreterPluggable *m_pInterp;
        /// ptr to interpreter context
        std::vector<ExecutionContext> * m_pECStack;
        /// mapping of program input arguments to NEATGenericValue
        std::map<Value *, NEATGenericValue> m_ArgValues;
        /// NEAT execution context
        std::vector<NEATExecutionContext> m_NECStack;
        /// helper class to count storage bytes of NEAT values
        NEATTargetData m_NTD;
        /// current event being handled by NEAT
        enum CurEvent {PRE_INST, POST_INST, PRE_FUNC, POST_FUNC, BAD_EVENT} m_CurEvent;
        /// get current event
        CurEvent GetCurEvent() const { return m_CurEvent; }
        /// set current Event
        void SetCurEvent(const CurEvent& val) { m_CurEvent = val; }
        void popStackAndReturnValueToCaller(const Type *RetTy, NEATGenericValue Result);
        /// Detect OpenCL builtin supported by NEAT and execute it if detected
        /// @param F ptr to builtin function which is called
        /// @param ArgVals - map of arguments to function supported by NEAT
        /// @param Result - result of executing builtin
        /// @return true if OCL Detected and executed, false if F is not OCL builtin
        bool DetectAndExecuteOCLBuiltins( Function *F,
            const std::map<Value *, NEATGenericValue> &ArgVals,
            NEATGenericValue& Result );

        // Global address map
        GlobalAddressMapTy m_GlobalAddressMap;
        /// getPointerToGlobalIfAvailable - This returns the address of the specified
        /// global value if it is has already been codegen'd, otherwise it returns null.
        void *getPointerToGlobalIfAvailable(const GlobalValue *GV);
    };
}

#endif // LLI_PLUGIN_NEAT_H

