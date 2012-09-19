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

File Name:  OCLBuiltins.cpp

\*****************************************************************************/

#include "BLTWorkItem.h"
#include "VLoadStore.h"
#include "BLTCommon.h"
#include "BLTMath.h"
#include "BLTInteger.h"
#include "BLTGeometric.h"
#include "BLTConversion.h"
#include "BLTRelational.h"
#include "BLTAsyncCopiesAndPrefetch.h"
#include "BLTImages.h"
#include "BLTMiscellaneousVector.h"
#include "OCLBuiltins.h"

using namespace llvm;
using std::map;
using std::string;
using std::vector;

namespace Validation {
namespace OCLBuiltins {

void FillOCLBuiltins (map<string, GenericValue (*)(FunctionType *, const vector<GenericValue> &)>& funcNames)
{
    // Adding work-item built-in functions to the map.
    WorkItemMapFiller wiBuiltins;
    wiBuiltins.addOpenCLBuiltins(funcNames);

    // Adding vector load/store built-in functions to the map.
    VLoadStoreMapFiller vlsBuiltins;
    vlsBuiltins.addOpenCLBuiltins(funcNames);

    CommonMapFiller commonBuiltins;
    commonBuiltins.addOpenCLBuiltins(funcNames);

    // Adding math built-in functions.
    MathMapFiller mathBuiltins;
    mathBuiltins.addOpenCLBuiltins(funcNames);

    // Adding integer built-in functions.
    IntegerMapFiller integerBuiltins;
    integerBuiltins.addOpenCLBuiltins(funcNames);

    // Adding geometric built-in functions.
    GeometricMapFiller geometricBuiltins;
    geometricBuiltins.addOpenCLBuiltins(funcNames);

    // Adding conversion built-in functions.
    ConversionMapFiller conversionBuiltins;
    conversionBuiltins.addOpenCLBuiltins(funcNames);

    AsyncCopiesAndPrefetchMapFiller asyncBuiltins;
    asyncBuiltins.addOpenCLBuiltins(funcNames);

    RelationalMapFiller relationalBuiltins;
    relationalBuiltins.addOpenCLBuiltins(funcNames);

    // Adding image built-in functions.
    ImageMapFiller imagesBuiltins;
    imagesBuiltins.addOpenCLBuiltins(funcNames);

    // Adding Miscellaneous Vector built-in functions.
    MiscellaneousVectorFiller miscVectorBuiltins;
    miscVectorBuiltins.addOpenCLBuiltins(funcNames);

    // Implementation of 'memcpy' function/intrinsic.
    funcNames["lle_X_memcpy"] = lle_X_memcpy;

#if 0 // temporary switch off because the compilation in Release mode is deadly slow.
    funcNames["barrier"] = UnimplementedBuiltin;
    funcNames["mem_fence"] = UnimplementedBuiltin;
    funcNames["read_mem_fence"] = UnimplementedBuiltin;
    funcNames["write_mem_fence"] = UnimplementedBuiltin;
    funcNames["lle_X__Z10atomic_addPVU3AS1ii"] = UnimplementedBuiltin;
    funcNames["lle_X__Z10atomic_addPVU3AS1jj"] = UnimplementedBuiltin;
    funcNames["lle_X__Z10atomic_addPVU3AS3ii"] = UnimplementedBuiltin;
    funcNames["lle_X__Z10atomic_addPVU3AS3jj"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_addPU3AS1ii"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_addPU3AS1jj"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_addPU3AS3ii"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_addPU3AS3jj"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_addPU3AS1ll"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_addPU3AS1mm"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_addPU3AS3ll"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_addPU3AS3mm"] = UnimplementedBuiltin;
    funcNames["lle_X__Z10atomic_subPVU3AS1ii"] = UnimplementedBuiltin;
    funcNames["lle_X__Z10atomic_subPVU3AS1jj"] = UnimplementedBuiltin;
    funcNames["lle_X__Z10atomic_subPVU3AS3ii"] = UnimplementedBuiltin;
    funcNames["lle_X__Z10atomic_subPVU3AS3jj"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_subPU3AS1ii"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_subPU3AS1jj"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_subPU3AS3ii"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_subPU3AS3jj"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_subPU3AS1ll"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_subPU3AS1mm"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_subPU3AS3ll"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_subPU3AS3mm"] = UnimplementedBuiltin;
    funcNames["lle_X__Z11atomic_xchgPVU3AS1ii"] = UnimplementedBuiltin;
    funcNames["lle_X__Z11atomic_xchgPVU3AS1jj"] = UnimplementedBuiltin;
    funcNames["lle_X__Z11atomic_xchgPVU3AS1ff"] = UnimplementedBuiltin;
    funcNames["lle_X__Z11atomic_xchgPVU3AS3ii"] = UnimplementedBuiltin;
    funcNames["lle_X__Z11atomic_xchgPVU3AS3jj"] = UnimplementedBuiltin;
    funcNames["lle_X__Z11atomic_xchgPVU3AS3ff"] = UnimplementedBuiltin;
    funcNames["lle_X__Z9atom_xchgPU3AS1ii"] = UnimplementedBuiltin;
    funcNames["lle_X__Z9atom_xchgPU3AS1jj"] = UnimplementedBuiltin;
    funcNames["lle_X__Z9atom_xchgPU3AS3ii"] = UnimplementedBuiltin;
    funcNames["lle_X__Z9atom_xchgPU3AS3jj"] = UnimplementedBuiltin;
    funcNames["lle_X__Z9atom_xchgPU3AS1ll"] = UnimplementedBuiltin;
    funcNames["lle_X__Z9atom_xchgPU3AS1mm"] = UnimplementedBuiltin;
    funcNames["lle_X__Z9atom_xchgPU3AS3ll"] = UnimplementedBuiltin;
    funcNames["lle_X__Z9atom_xchgPU3AS3mm"] = UnimplementedBuiltin;
    funcNames["lle_X__Z10atomic_incPVU3AS1i"] = UnimplementedBuiltin;
    funcNames["lle_X__Z10atomic_incPVU3AS1j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z10atomic_incPVU3AS3i"] = UnimplementedBuiltin;
    funcNames["lle_X__Z10atomic_incPVU3AS3j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_incPU3AS1i"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_incPU3AS1j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_incPU3AS3i"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_incPU3AS3j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_incPU3AS1l"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_incPU3AS1m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_incPU3AS3l"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_incPU3AS3m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z10atomic_decPVU3AS1i"] = UnimplementedBuiltin;
    funcNames["lle_X__Z10atomic_decPVU3AS1j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z10atomic_decPVU3AS3i"] = UnimplementedBuiltin;
    funcNames["lle_X__Z10atomic_decPVU3AS3j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_decPU3AS1i"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_decPU3AS1j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_decPU3AS3i"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_decPU3AS3j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_decPU3AS1l"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_decPU3AS1m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_decPU3AS3l"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_decPU3AS3m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z14atomic_cmpxchgPVU3AS1iii"] = UnimplementedBuiltin;
    funcNames["lle_X__Z14atomic_cmpxchgPVU3AS1jjj"] = UnimplementedBuiltin;
    funcNames["lle_X__Z14atomic_cmpxchgPVU3AS3iii"] = UnimplementedBuiltin;
    funcNames["lle_X__Z14atomic_cmpxchgPVU3AS3jjj"] = UnimplementedBuiltin;
    funcNames["lle_X__Z12atom_cmpxchgPU3AS1iii"] = UnimplementedBuiltin;
    funcNames["lle_X__Z12atom_cmpxchgPU3AS1jjj"] = UnimplementedBuiltin;
    funcNames["lle_X__Z12atom_cmpxchgPU3AS3iii"] = UnimplementedBuiltin;
    funcNames["lle_X__Z12atom_cmpxchgPU3AS3jjj"] = UnimplementedBuiltin;
    funcNames["lle_X__Z12atom_cmpxchgPU3AS1lll"] = UnimplementedBuiltin;
    funcNames["lle_X__Z12atom_cmpxchgPU3AS1mmm"] = UnimplementedBuiltin;
    funcNames["lle_X__Z12atom_cmpxchgPU3AS3lll"] = UnimplementedBuiltin;
    funcNames["lle_X__Z12atom_cmpxchgPU3AS3mmm"] = UnimplementedBuiltin;
    funcNames["lle_X__Z10atomic_minPVU3AS1ii"] = UnimplementedBuiltin;
    funcNames["lle_X__Z10atomic_minPVU3AS1jj"] = UnimplementedBuiltin;
    funcNames["lle_X__Z10atomic_minPVU3AS3ii"] = UnimplementedBuiltin;
    funcNames["lle_X__Z10atomic_minPVU3AS3jj"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_minPU3AS1ii"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_minPU3AS1jj"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_minPU3AS3ii"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_minPU3AS3jj"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_minPU3AS1ll"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_minPU3AS1mm"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_minPU3AS3ll"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_minPU3AS3mm"] = UnimplementedBuiltin;
    funcNames["lle_X__Z10atomic_maxPVU3AS1ii"] = UnimplementedBuiltin;
    funcNames["lle_X__Z10atomic_maxPVU3AS1jj"] = UnimplementedBuiltin;
    funcNames["lle_X__Z10atomic_maxPVU3AS3ii"] = UnimplementedBuiltin;
    funcNames["lle_X__Z10atomic_maxPVU3AS3jj"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_maxPU3AS1ii"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_maxPU3AS1jj"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_maxPU3AS3ii"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_maxPU3AS3jj"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_maxPU3AS1ll"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_maxPU3AS1mm"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_maxPU3AS3ll"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_maxPU3AS3mm"] = UnimplementedBuiltin;
    funcNames["lle_X__Z10atomic_andPVU3AS1ii"] = UnimplementedBuiltin;
    funcNames["lle_X__Z10atomic_andPVU3AS1jj"] = UnimplementedBuiltin;
    funcNames["lle_X__Z10atomic_andPVU3AS3ii"] = UnimplementedBuiltin;
    funcNames["lle_X__Z10atomic_andPVU3AS3jj"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_andPU3AS1ii"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_andPU3AS1jj"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_andPU3AS3ii"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_andPU3AS3jj"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_andPU3AS1ll"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_andPU3AS1mm"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_andPU3AS3ll"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_andPU3AS3mm"] = UnimplementedBuiltin;
    funcNames["lle_X__Z9atomic_orPVU3AS1ii"] = UnimplementedBuiltin;
    funcNames["lle_X__Z9atomic_orPVU3AS1jj"] = UnimplementedBuiltin;
    funcNames["lle_X__Z9atomic_orPVU3AS3ii"] = UnimplementedBuiltin;
    funcNames["lle_X__Z9atomic_orPVU3AS3jj"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7atom_orPU3AS1ii"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7atom_orPU3AS1jj"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7atom_orPU3AS3ii"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7atom_orPU3AS3jj"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7atom_orPU3AS1ll"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7atom_orPU3AS1mm"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7atom_orPU3AS3ll"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7atom_orPU3AS3mm"] = UnimplementedBuiltin;
    funcNames["lle_X__Z10atomic_xorPVU3AS1ii"] = UnimplementedBuiltin;
    funcNames["lle_X__Z10atomic_xorPVU3AS1jj"] = UnimplementedBuiltin;
    funcNames["lle_X__Z10atomic_xorPVU3AS3ii"] = UnimplementedBuiltin;
    funcNames["lle_X__Z10atomic_xorPVU3AS3jj"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_xorPU3AS1ii"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_xorPU3AS1jj"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_xorPU3AS3ii"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_xorPU3AS3jj"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_xorPU3AS1ll"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_xorPU3AS1mm"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_xorPU3AS3ll"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8atom_xorPU3AS3mm"] = UnimplementedBuiltin;

#endif

}

}
}
