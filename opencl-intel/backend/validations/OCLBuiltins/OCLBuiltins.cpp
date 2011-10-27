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

#include "VLoadStore.h"
#include "BLTCommon.h"
#include "BLTMath.h"
#include "BLTInteger.h"
#include "BLTGeometric.h"
#include "BLTConversion.h"
#include "BLTRelational.h"
#include "BLTAsyncCopiesAndPrefetch.h"
#include "BLTImages.h"
#include "OCLBuiltins.h"

using namespace llvm;
using std::map;
using std::string;
using std::vector;

namespace Validation {
namespace OCLBuiltins {

void FillOCLBuiltins (map<string, GenericValue (*)(const FunctionType *, const vector<GenericValue> &)>& funcNames)
{
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
    funcNames["lle_X__Z7shuffleDv2_cDv2_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_cDv2_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_cDv2_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_cDv2_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_hS_"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_hDv2_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_hDv2_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_hDv2_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_sDv2_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_sDv2_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_sDv2_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_sDv2_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_tS_"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_tDv2_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_tDv2_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_tDv2_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_iDv2_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_iDv2_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_iDv2_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_iDv2_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_jS_"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_jDv2_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_jDv2_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_jDv2_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_lDv2_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_lDv2_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_lDv2_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_lDv2_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_mS_"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_mDv2_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_mDv2_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_mDv2_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_fDv2_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_fDv2_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_fDv2_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_fDv2_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_dDv2_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_dDv2_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_dDv2_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_dDv2_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_cDv4_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_cDv4_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_cDv4_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_cDv4_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_hDv4_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_hS_"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_hDv4_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_hDv4_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_sDv4_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_sDv4_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_sDv4_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_sDv4_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_tDv4_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_tS_"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_tDv4_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_tDv4_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_iDv4_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_iDv4_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_iDv4_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_iDv4_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_jDv4_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_jS_"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_jDv4_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_jDv4_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_lDv4_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_lDv4_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_lDv4_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_lDv4_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_mDv4_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_mS_"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_mDv4_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_mDv4_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_fDv4_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_fDv4_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_fDv4_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_fDv4_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_dDv4_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_dDv4_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_dDv4_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_dDv4_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_cDv8_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_cDv8_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_cDv8_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_cDv8_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_hDv8_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_hDv8_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_hS_"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_hDv8_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_sDv8_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_sDv8_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_sDv8_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_sDv8_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_tDv8_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_tDv8_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_tS_"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_tDv8_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_iDv8_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_iDv8_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_iDv8_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_iDv8_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_jDv8_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_jDv8_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_jS_"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_jDv8_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_lDv8_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_lDv8_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_lDv8_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_lDv8_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_mDv8_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_mDv8_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_mS_"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_mDv8_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_fDv8_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_fDv8_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_fDv8_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_fDv8_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_dDv8_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_dDv8_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_dDv8_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_dDv8_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_cDv16_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_cDv16_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_cDv16_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_cDv16_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_hDv16_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_hDv16_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_hDv16_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_hS_"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_sDv16_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_sDv16_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_sDv16_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_sDv16_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_tDv16_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_tDv16_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_tDv16_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_tS_"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_iDv16_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_iDv16_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_iDv16_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_iDv16_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_jDv16_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_jDv16_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_jDv16_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_jS_"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_lDv16_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_lDv16_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_lDv16_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_lDv16_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_mDv16_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_mDv16_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_mDv16_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_mS_"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_fDv16_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_fDv16_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_fDv16_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_fDv16_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv2_dDv16_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv4_dDv16_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv8_dDv16_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z7shuffleDv16_dDv16_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_cS_Dv2_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_cS_Dv2_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_cS_Dv2_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_cS_Dv2_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_hS_S_"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_hS_Dv2_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_hS_Dv2_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_hS_Dv2_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_sS_Dv2_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_sS_Dv2_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_sS_Dv2_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_sS_Dv2_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_tS_S_"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_tS_Dv2_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_tS_Dv2_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_tS_Dv2_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_iS_Dv2_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_iS_Dv2_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_iS_Dv2_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_iS_Dv2_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_jS_S_"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_jS_Dv2_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_jS_Dv2_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_jS_Dv2_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_lS_Dv2_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_lS_Dv2_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_lS_Dv2_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_lS_Dv2_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_mS_S_"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_mS_Dv2_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_mS_Dv2_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_mS_Dv2_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_fS_Dv2_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_fS_Dv2_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_fS_Dv2_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_fS_Dv2_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_dS_Dv2_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_dS_Dv2_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_dS_Dv2_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_dS_Dv2_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_cS_Dv4_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_cS_Dv4_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_cS_Dv4_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_cS_Dv4_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_hS_Dv4_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_hS_S_"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_hS_Dv4_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_hS_Dv4_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_sS_Dv4_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_sS_Dv4_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_sS_Dv4_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_sS_Dv4_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_tS_Dv4_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_tS_S_"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_tS_Dv4_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_tS_Dv4_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_iS_Dv4_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_iS_Dv4_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_iS_Dv4_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_iS_Dv4_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_jS_Dv4_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_jS_S_"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_jS_Dv4_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_jS_Dv4_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_lS_Dv4_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_lS_Dv4_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_lS_Dv4_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_lS_Dv4_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_mS_Dv4_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_mS_S_"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_mS_Dv4_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_mS_Dv4_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_fS_Dv4_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_fS_Dv4_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_fS_Dv4_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_fS_Dv4_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_dS_Dv4_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_dS_Dv4_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_dS_Dv4_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_dS_Dv4_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_cS_Dv8_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_cS_Dv8_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_cS_Dv8_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_cS_Dv8_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_hS_Dv8_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_hS_Dv8_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_hS_S_"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_hS_Dv8_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_sS_Dv8_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_sS_Dv8_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_sS_Dv8_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_sS_Dv8_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_tS_Dv8_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_tS_Dv8_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_tS_S_"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_tS_Dv8_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_iS_Dv8_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_iS_Dv8_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_iS_Dv8_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_iS_Dv8_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_jS_Dv8_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_jS_Dv8_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_jS_S_"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_jS_Dv8_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_lS_Dv8_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_lS_Dv8_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_lS_Dv8_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_lS_Dv8_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_mS_Dv8_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_mS_Dv8_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_mS_S_"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_mS_Dv8_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_fS_Dv8_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_fS_Dv8_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_fS_Dv8_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_fS_Dv8_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_dS_Dv8_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_dS_Dv8_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_dS_Dv8_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_dS_Dv8_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_cS_Dv16_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_cS_Dv16_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_cS_Dv16_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_cS_Dv16_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_hS_Dv16_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_hS_Dv16_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_hS_Dv16_h"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_hS_S_"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_sS_Dv16_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_sS_Dv16_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_sS_Dv16_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_sS_Dv16_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_tS_Dv16_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_tS_Dv16_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_tS_Dv16_t"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_tS_S_"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_iS_Dv16_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_iS_Dv16_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_iS_Dv16_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_iS_Dv16_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_jS_Dv16_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_jS_Dv16_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_jS_Dv16_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_jS_S_"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_lS_Dv16_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_lS_Dv16_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_lS_Dv16_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_lS_Dv16_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_mS_Dv16_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_mS_Dv16_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_mS_Dv16_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_mS_S_"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_fS_Dv16_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_fS_Dv16_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_fS_Dv16_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_fS_Dv16_j"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv2_dS_Dv16_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv4_dS_Dv16_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv8_dS_Dv16_m"] = UnimplementedBuiltin;
    funcNames["lle_X__Z8shuffle2Dv16_dS_Dv16_m"] = UnimplementedBuiltin;

#endif

}

}
}
