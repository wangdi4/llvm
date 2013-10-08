/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

// This pass is used for relaxed functions substitution

#include "RelaxedPass.h"
#include "OCLPassSupport.h"
#include <llvm/Pass.h>
#include <llvm/Module.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Constants.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>

#include <list>
#include <map>
#include <string>

#include "CompilationUtils.h"

using namespace llvm;
using namespace std;
using namespace intel;

extern "C" {
    /// @brief Returns an instance of the RelaxedPass pass,
    ///        which will be added to a PassManager and run on a Module.
    void* createRelaxedPass() {
        return new intel::RelaxedPass();
    }
}

#define NAME_RELAXED_2_0 "_rm"

// the length of "_rm" is 3, the length of "cos" is 3, so NAME_RELAXED_2_0_LEN_3 = 6, these
// defines are used in macroses like RELAXED_MATH_2_0_P1_vX and if prefixes/postfixes like "_rm" will be changed 
// to new ones with different length, it will be enough to change only defines NAME_RELAXED_2_0_LEN_3 ... NAME_RELAXED_2_0_LEN_6

#define NAME_RELAXED_2_0_LEN_3 6
#define NAME_RELAXED_2_0_LEN_4 7
#define NAME_RELAXED_2_0_LEN_5 8
#define NAME_RELAXED_2_0_LEN_6 9

#define INSERT_MAP_TO_NATIVE(map, func, type, length, native_length)        \
    map.insert ( pair<std::string, std::string>("_Z" #length #func type, "_Z" #native_length "native_" #func type) );

#define INSERT_MAP_TO_RELAXED_20(map, func, type, length, ocl20_length)        \
    map.insert ( pair<std::string, std::string>("_Z" #length #func type, "_Z" #ocl20_length #func NAME_RELAXED_2_0 type) );

// native built-ins, one argument, float version
#define    RELAXED_P1_vX(map, func,           length, native_length)     \
    INSERT_MAP_TO_NATIVE(map, func, "f",      length, native_length)     \
    INSERT_MAP_TO_NATIVE(map, func, "Dv2_f",  length, native_length)     \
    INSERT_MAP_TO_NATIVE(map, func, "Dv3_f",  length, native_length)     \
    INSERT_MAP_TO_NATIVE(map, func, "Dv4_f",  length, native_length)     \
    INSERT_MAP_TO_NATIVE(map, func, "Dv8_f",  length, native_length)     \
    INSERT_MAP_TO_NATIVE(map, func, "Dv16_f", length, native_length)

// native built-ins, one argument, double version
#define  RELAXED_P1_vX_D(map, func,           length, native_length)     \
    INSERT_MAP_TO_NATIVE(map, func, "d",      length, native_length)     \
    INSERT_MAP_TO_NATIVE(map, func, "Dv2_d",  length, native_length)     \
    INSERT_MAP_TO_NATIVE(map, func, "Dv3_d",  length, native_length)     \
    INSERT_MAP_TO_NATIVE(map, func, "Dv4_d",  length, native_length)     \
    INSERT_MAP_TO_NATIVE(map, func, "Dv8_d",  length, native_length)     \
    INSERT_MAP_TO_NATIVE(map, func, "Dv16_d", length, native_length)

// native built-ins, two argument, float version
#define RELAXED_P2_vX_vY(map, func,             length, native_length)   \
    INSERT_MAP_TO_NATIVE(map, func, "ff",       length, native_length)   \
    INSERT_MAP_TO_NATIVE(map, func, "Dv2_fS_",  length, native_length)   \
    INSERT_MAP_TO_NATIVE(map, func, "Dv3_fS_",  length, native_length)   \
    INSERT_MAP_TO_NATIVE(map, func, "Dv4_fS_",  length, native_length)   \
    INSERT_MAP_TO_NATIVE(map, func, "Dv8_fS_",  length, native_length)   \
    INSERT_MAP_TO_NATIVE(map, func, "Dv16_fS_", length, native_length)

// native built-ins, two argument, double version
#define RELAXED_P2_vX_vY_D(map, func,             length, native_length)   \
    INSERT_MAP_TO_NATIVE(map,   func, "dd",       length, native_length)   \
    INSERT_MAP_TO_NATIVE(map,   func, "Dv2_dS_",  length, native_length)   \
    INSERT_MAP_TO_NATIVE(map,   func, "Dv3_dS_",  length, native_length)   \
    INSERT_MAP_TO_NATIVE(map,   func, "Dv4_dS_",  length, native_length)   \
    INSERT_MAP_TO_NATIVE(map,   func, "Dv8_dS_",  length, native_length)   \
    INSERT_MAP_TO_NATIVE(map,   func, "Dv16_dS_", length, native_length)

// native built-ins, two argument(vector, scalar), double version
#define RELAXED_P2_vX_sY(map, func,            length, native_length)    \
    INSERT_MAP_TO_NATIVE(map, func, "ff",      length, native_length)    \
    INSERT_MAP_TO_NATIVE(map, func, "Dv2_ff",  length, native_length)    \
    INSERT_MAP_TO_NATIVE(map, func, "Dv3_ff",  length, native_length)    \
    INSERT_MAP_TO_NATIVE(map, func, "Dv4_ff",  length, native_length)    \
    INSERT_MAP_TO_NATIVE(map, func, "Dv8_ff",  length, native_length)    \
    INSERT_MAP_TO_NATIVE(map, func, "Dv16_ff", length, native_length)

// native built-ins, two argument(vector, pointer), float
#define RELAXED_P2_vX_pY(map, func,              length, native_length)  \
    INSERT_MAP_TO_NATIVE(map, func, "fPf",       length, native_length)  \
    INSERT_MAP_TO_NATIVE(map, func, "Dv2_fPS_",  length, native_length)  \
    INSERT_MAP_TO_NATIVE(map, func, "Dv3_fPS_",  length, native_length)  \
    INSERT_MAP_TO_NATIVE(map, func, "Dv4_fPS_",  length, native_length)  \
    INSERT_MAP_TO_NATIVE(map, func, "Dv8_fPS_",  length, native_length)  \
    INSERT_MAP_TO_NATIVE(map, func, "Dv16_fPS_", length, native_length)


// Table 7.2 of OCL 2.0 built-ins, one argument
#define    RELAXED_MATH_2_0_P1_vX(map, func,      length, native_length)     \
    INSERT_MAP_TO_RELAXED_20(map, func, "f",      length, native_length)     \
    INSERT_MAP_TO_RELAXED_20(map, func, "Dv2_f",  length, native_length)     \
    INSERT_MAP_TO_RELAXED_20(map, func, "Dv3_f",  length, native_length)     \
    INSERT_MAP_TO_RELAXED_20(map, func, "Dv4_f",  length, native_length)     \
    INSERT_MAP_TO_RELAXED_20(map, func, "Dv8_f",  length, native_length)     \
    INSERT_MAP_TO_RELAXED_20(map, func, "Dv16_f", length, native_length)

// Table 7.2 of OCL 2.0 built-ins, two argument
#define RELAXED_MATH_2_0_P2_vX_vY(map, func,        length, native_length)   \
    INSERT_MAP_TO_RELAXED_20(map, func, "ff",       length, native_length)   \
    INSERT_MAP_TO_RELAXED_20(map, func, "Dv2_fS_",  length, native_length)   \
    INSERT_MAP_TO_RELAXED_20(map, func, "Dv3_fS_",  length, native_length)   \
    INSERT_MAP_TO_RELAXED_20(map, func, "Dv4_fS_",  length, native_length)   \
    INSERT_MAP_TO_RELAXED_20(map, func, "Dv8_fS_",  length, native_length)   \
    INSERT_MAP_TO_RELAXED_20(map, func, "Dv16_fS_", length, native_length)

// Table 7.2 of OCL 2.0 built-ins, two argument(vector, pointer to vector)
#define RELAXED_MATH_2_0_P2_vX_pvY(map, func,         length, native_length)  \
    INSERT_MAP_TO_RELAXED_20(map, func, "fPU3AS1f",       length, native_length)  \
    INSERT_MAP_TO_RELAXED_20(map, func, "Dv2_fPU3AS1S_",  length, native_length)  \
    INSERT_MAP_TO_RELAXED_20(map, func, "Dv3_fPU3AS1S_",  length, native_length)  \
    INSERT_MAP_TO_RELAXED_20(map, func, "Dv4_fPU3AS1S_",  length, native_length)  \
    INSERT_MAP_TO_RELAXED_20(map, func, "Dv8_fPU3AS1S_",  length, native_length)  \
    INSERT_MAP_TO_RELAXED_20(map, func, "Dv16_fPU3AS1S_", length, native_length)

namespace intel{

    OCL_INITIALIZE_PASS(RelaxedPass, "relaxed-funcs", "This pass is used for relaxed functions substitution", false, false)

    /// @brief Constructor, add mapping for relaxed functions
    RelaxedPass::RelaxedPass() : ModulePass(ID)
    {
        // float version of native built-ins
        RELAXED_P1_vX(m_relaxedFunctions, tan, 3, 10);
        RELAXED_P1_vX(m_relaxedFunctions, sin, 3, 10);
        RELAXED_P1_vX(m_relaxedFunctions, log, 3, 10);
        RELAXED_P1_vX(m_relaxedFunctions, exp, 3, 10);
        RELAXED_P1_vX(m_relaxedFunctions, cos, 3, 10);
        RELAXED_P1_vX(m_relaxedFunctions, logb, 4, 11);
        RELAXED_P1_vX(m_relaxedFunctions, exp2, 4, 11);
        RELAXED_P1_vX(m_relaxedFunctions, log2, 4, 11);
        RELAXED_P1_vX(m_relaxedFunctions, sqrt, 4, 11);
        // RELAXED_P1_vX(m_relaxedFunctions, recip, 5, 12);// there is no recip BI in the spec, there for we can't replace it with native_recip
        RELAXED_P1_vX(m_relaxedFunctions, rsqrt, 5, 12);
        RELAXED_P1_vX(m_relaxedFunctions, log10, 5, 12);
        RELAXED_P1_vX(m_relaxedFunctions, exp10, 5, 12);
        RELAXED_P1_vX(m_relaxedFunctions, ilogb, 5, 12);
        //RELAXED_P1_ALL(m_relaxedFunctions, divide, 6, 13);

        // double version of native built-ins
        RELAXED_P1_vX_D(m_relaxedFunctions, tan, 3, 10);
        RELAXED_P1_vX_D(m_relaxedFunctions, sin, 3, 10);
        RELAXED_P1_vX_D(m_relaxedFunctions, log, 3, 10);
        RELAXED_P1_vX_D(m_relaxedFunctions, exp, 3, 10);
        RELAXED_P1_vX_D(m_relaxedFunctions, cos, 3, 10);
        RELAXED_P1_vX_D(m_relaxedFunctions, exp2, 4, 11);
        RELAXED_P1_vX_D(m_relaxedFunctions, log2, 4, 11);
        RELAXED_P1_vX_D(m_relaxedFunctions, sqrt, 4, 11);
        RELAXED_P1_vX_D(m_relaxedFunctions, rsqrt, 5, 12);
        RELAXED_P1_vX_D(m_relaxedFunctions, log10, 5, 12);
        RELAXED_P1_vX_D(m_relaxedFunctions, exp10, 5, 12);

        RELAXED_P2_vX_vY_D(m_relaxedFunctions, powr, 4, 11);
        RELAXED_P2_vX_vY_D(m_relaxedFunctions, hypot, 5, 12);

        RELAXED_P2_vX_vY(m_relaxedFunctions, fdim, 4, 11);
        RELAXED_P2_vX_vY(m_relaxedFunctions, fmod, 4, 11);
        RELAXED_P2_vX_vY(m_relaxedFunctions, powr, 4, 11);
        RELAXED_P2_vX_vY(m_relaxedFunctions, hypot, 5, 12);

        RELAXED_P2_vX_vY(m_relaxedFunctions, fmax, 4, 11);
        RELAXED_P2_vX_sY(m_relaxedFunctions, fmax, 4, 11);
        RELAXED_P2_vX_vY(m_relaxedFunctions, fmin, 4, 11);
        RELAXED_P2_vX_sY(m_relaxedFunctions, fmin, 4, 11);

        RELAXED_P2_vX_pY(m_relaxedFunctions, fract, 5, 12);

        // non-spec native-BI
        RELAXED_P1_vX(m_relaxedFunctions,   acos, 4, 11);
        RELAXED_P1_vX_D(m_relaxedFunctions, acos, 4, 11);

        RELAXED_P1_vX(m_relaxedFunctions,   acosh, 5, 12);
        RELAXED_P1_vX_D(m_relaxedFunctions, acosh, 5, 12);

        RELAXED_P1_vX(m_relaxedFunctions,   acospi, 6, 13);
        RELAXED_P1_vX_D(m_relaxedFunctions, acospi, 6, 13);

        RELAXED_P1_vX(m_relaxedFunctions,   asin, 4, 11);
        RELAXED_P1_vX_D(m_relaxedFunctions, asin, 4, 11);

        RELAXED_P1_vX(m_relaxedFunctions,   asinh, 5, 12);
        RELAXED_P1_vX_D(m_relaxedFunctions, asinh, 5, 12);

        RELAXED_P1_vX(m_relaxedFunctions,   asinpi, 6, 13);
        RELAXED_P1_vX_D(m_relaxedFunctions, asinpi, 6, 13);

        RELAXED_P1_vX(m_relaxedFunctions,   atan, 4, 11);
        RELAXED_P1_vX_D(m_relaxedFunctions, atan, 4, 11);

        RELAXED_P1_vX(m_relaxedFunctions,   atanh, 5, 12);
        RELAXED_P1_vX_D(m_relaxedFunctions, atanh, 5, 12);

        RELAXED_P1_vX(m_relaxedFunctions,   atanpi, 6, 13);
        RELAXED_P1_vX_D(m_relaxedFunctions, atanpi, 6, 13);

        RELAXED_P2_vX_vY(m_relaxedFunctions,   atan2, 5, 12);
        RELAXED_P2_vX_vY_D(m_relaxedFunctions, atan2, 5, 12);

        RELAXED_P2_vX_vY(m_relaxedFunctions,   atan2pi, 7, 14);
        RELAXED_P2_vX_vY_D(m_relaxedFunctions, atan2pi, 7, 14);

        RELAXED_P1_vX(m_relaxedFunctions,   cbrt, 4, 11);
        RELAXED_P1_vX_D(m_relaxedFunctions, cbrt, 4, 11);

        RELAXED_P1_vX(m_relaxedFunctions,   cospi, 5, 12);
        RELAXED_P1_vX_D(m_relaxedFunctions, cospi, 5, 12);

        RELAXED_P1_vX(m_relaxedFunctions,   erfc, 4, 11);
        RELAXED_P1_vX_D(m_relaxedFunctions, erfc, 4, 11);

        RELAXED_P1_vX(m_relaxedFunctions,   erf, 3, 10);
        RELAXED_P1_vX_D(m_relaxedFunctions, erf, 3, 10);

        RELAXED_P1_vX(m_relaxedFunctions,   expm1, 5, 12);
        RELAXED_P1_vX_D(m_relaxedFunctions, expm1, 5, 12);

        RELAXED_P1_vX(m_relaxedFunctions,   log1p, 5, 12);
        RELAXED_P1_vX_D(m_relaxedFunctions, log1p, 5, 12);

        RELAXED_P2_vX_vY(m_relaxedFunctions,   pow, 3, 10);
        RELAXED_P2_vX_vY_D(m_relaxedFunctions, pow, 3, 10);

        RELAXED_P1_vX(m_relaxedFunctions,   sinpi, 5, 12);
        RELAXED_P1_vX_D(m_relaxedFunctions, sinpi, 5, 12);

        RELAXED_P1_vX(m_relaxedFunctions,   tanpi, 5, 12);
        RELAXED_P1_vX_D(m_relaxedFunctions, tanpi, 5, 12);

        // Table 7.2 of OCL 2.0 built-ins
        RELAXED_MATH_2_0_P1_vX(m_relaxedFunctions_2_0, cos, 3, NAME_RELAXED_2_0_LEN_3);
        RELAXED_MATH_2_0_P1_vX(m_relaxedFunctions_2_0, exp, 3, NAME_RELAXED_2_0_LEN_3);
        RELAXED_MATH_2_0_P1_vX(m_relaxedFunctions_2_0, exp2, 4, NAME_RELAXED_2_0_LEN_4);
        RELAXED_MATH_2_0_P1_vX(m_relaxedFunctions_2_0, exp10, 5, NAME_RELAXED_2_0_LEN_5);
        RELAXED_MATH_2_0_P1_vX(m_relaxedFunctions_2_0, log, 3, NAME_RELAXED_2_0_LEN_3);
        RELAXED_MATH_2_0_P1_vX(m_relaxedFunctions_2_0, log2, 4, NAME_RELAXED_2_0_LEN_4);

        RELAXED_MATH_2_0_P2_vX_vY(m_relaxedFunctions_2_0,   pow, 3, NAME_RELAXED_2_0_LEN_3);
        RELAXED_MATH_2_0_P2_vX_pvY(m_relaxedFunctions_2_0,   sincos, 6, NAME_RELAXED_2_0_LEN_6);

        RELAXED_MATH_2_0_P1_vX(m_relaxedFunctions_2_0, sin, 3, NAME_RELAXED_2_0_LEN_3);
        RELAXED_MATH_2_0_P1_vX(m_relaxedFunctions_2_0, tan, 3, NAME_RELAXED_2_0_LEN_3);

        RELAXED_MATH_2_0_P1_vX(m_relaxedFunctions_2_0, divide, 6, NAME_RELAXED_2_0_LEN_6);

    }

    char RelaxedPass::ID = 0;

    void RelaxedPass::replaceFunctions(std::map<std::string, std::string> &mapIn)
    {
        m_relaxedFunctions = mapIn;
    }


    /// @brief doPassInitialization - For this pass, it removes global symbol table
    ///        entries for primitive types.  These are never used for linking in GCC and
    ///        they make the output uglier to look at, so we nuke them.
    ///        Also, initialize instance variables.
    bool RelaxedPass::runOnModule(Module &M)
    {
        bool changed = false;

        if (Intel::OpenCL::DeviceBackend::CompilationUtils::getCLVersionFromModuleOrDefault(M) ==
            Intel::OpenCL::DeviceBackend::OclVersion::CL_VER_2_0)
            replaceFunctions(m_relaxedFunctions_2_0);

        Module::iterator it,e;
        for (it = M.begin(), e=M.end(); it != e; ++it)
        {
            std::string pFuncName = it->getName().str();
            if ( ( it->isDeclaration() ) && ( m_relaxedFunctions.count( pFuncName ) != 0 ) )
            {
                Function* pFunction = &*it;
                std::string stRelaxedName = m_relaxedFunctions[pFuncName];
                FunctionType* pType = pFunction->getFunctionType();

                Function* pRelaxedFunction = dyn_cast<Function>(M.getOrInsertFunction( stRelaxedName, pType, pFunction->getAttributes() ));
                assert(NULL != pRelaxedFunction);
                pFunction->replaceAllUsesWith(pRelaxedFunction);

                changed = true;
            }
        }

        return changed;
    }


} // namespace intel
