/*****************************************************************************\

Copyright (c) Intel Corporation (2012).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  BLTAtomic.cpp

\*****************************************************************************/

#include "BLTAtomic.h"

using namespace llvm;
using std::map;
using std::string;
using std::vector;
namespace Validation {
namespace OCLBuiltins {


void AtomicMapFiller::addOpenCLBuiltins( map<string, PBLTFunc>& funcNames )
{
    funcNames["lle_X__Z10atomic_addPVU3AS1ii"] = lle_X_atomic_add<int>;
    funcNames["lle_X__Z10atomic_addPVU3AS1jj"] = lle_X_atomic_add<unsigned int>;
    funcNames["lle_X__Z10atomic_addPVU3AS3ii"] = lle_X_atomic_add<int>;
    funcNames["lle_X__Z10atomic_addPVU3AS3jj"] = lle_X_atomic_add<unsigned int>;
    funcNames["lle_X__Z8atom_addPU3AS1ii"] = lle_X_atomic_add<int>;
    funcNames["lle_X__Z8atom_addPU3AS1jj"] = lle_X_atomic_add<unsigned int>;
    funcNames["lle_X__Z8atom_addPU3AS3ii"] = lle_X_atomic_add<int>;
    funcNames["lle_X__Z8atom_addPU3AS3jj"] = lle_X_atomic_add<unsigned int>;
    funcNames["lle_X__Z8atom_addPU3AS1ll"] = lle_X_atomic_add<long>;
    funcNames["lle_X__Z8atom_addPU3AS1mm"] = lle_X_atomic_add<unsigned long>;
    funcNames["lle_X__Z8atom_addPU3AS3ll"] = lle_X_atomic_add<long>;
    funcNames["lle_X__Z8atom_addPU3AS3mm"] = lle_X_atomic_add<unsigned long>;
    funcNames["lle_X__Z10atomic_subPVU3AS1ii"] = lle_X_atomic_sub<int>;
    funcNames["lle_X__Z10atomic_subPVU3AS1jj"] = lle_X_atomic_sub<unsigned int>;
    funcNames["lle_X__Z10atomic_subPVU3AS3ii"] = lle_X_atomic_sub<int>;
    funcNames["lle_X__Z10atomic_subPVU3AS3jj"] = lle_X_atomic_sub<unsigned int>;
    funcNames["lle_X__Z8atom_subPU3AS1ii"] = lle_X_atomic_sub<int>;
    funcNames["lle_X__Z8atom_subPU3AS1jj"] = lle_X_atomic_sub<unsigned int>;
    funcNames["lle_X__Z8atom_subPU3AS3ii"] = lle_X_atomic_sub<int>;
    funcNames["lle_X__Z8atom_subPU3AS3jj"] = lle_X_atomic_sub<unsigned int>;
    funcNames["lle_X__Z8atom_subPU3AS1ll"] = lle_X_atomic_sub<long>;
    funcNames["lle_X__Z8atom_subPU3AS1mm"] = lle_X_atomic_sub<unsigned long>;
    funcNames["lle_X__Z8atom_subPU3AS3ll"] = lle_X_atomic_sub<long>;
    funcNames["lle_X__Z8atom_subPU3AS3mm"] = lle_X_atomic_sub<unsigned long>;
    funcNames["lle_X__Z11atomic_xchgPVU3AS1ii"] = lle_X_atomic_xchg<int>;
    funcNames["lle_X__Z11atomic_xchgPVU3AS1jj"] = lle_X_atomic_xchg<unsigned int>;
    funcNames["lle_X__Z11atomic_xchgPVU3AS1ff"] = lle_X_atomic_xchg<float>;
    funcNames["lle_X__Z11atomic_xchgPVU3AS3ii"] = lle_X_atomic_xchg<int>;
    funcNames["lle_X__Z11atomic_xchgPVU3AS3jj"] = lle_X_atomic_xchg<unsigned int>;
    funcNames["lle_X__Z11atomic_xchgPVU3AS3ff"] = lle_X_atomic_xchg<float>;
    funcNames["lle_X__Z9atom_xchgPU3AS1ii"] = lle_X_atomic_xchg<int>;
    funcNames["lle_X__Z9atom_xchgPU3AS1jj"] = lle_X_atomic_xchg<unsigned int>;
    funcNames["lle_X__Z9atom_xchgPU3AS3ii"] = lle_X_atomic_xchg<int>;
    funcNames["lle_X__Z9atom_xchgPU3AS3jj"] = lle_X_atomic_xchg<unsigned int>;
    funcNames["lle_X__Z9atom_xchgPU3AS1ll"] = lle_X_atomic_xchg<long>;
    funcNames["lle_X__Z9atom_xchgPU3AS1mm"] = lle_X_atomic_xchg<unsigned long>;
    funcNames["lle_X__Z9atom_xchgPU3AS3ll"] = lle_X_atomic_xchg<long>;
    funcNames["lle_X__Z9atom_xchgPU3AS3mm"] = lle_X_atomic_xchg<unsigned long>;
    funcNames["lle_X__Z10atomic_incPVU3AS1i"] = lle_X_atomic_inc<int>;
    funcNames["lle_X__Z10atomic_incPVU3AS1j"] = lle_X_atomic_inc<unsigned int>;
    funcNames["lle_X__Z10atomic_incPVU3AS3i"] = lle_X_atomic_inc<int>;
    funcNames["lle_X__Z10atomic_incPVU3AS3j"] = lle_X_atomic_inc<unsigned int>;
    funcNames["lle_X__Z8atom_incPU3AS1i"] = lle_X_atomic_inc<int>;
    funcNames["lle_X__Z8atom_incPU3AS1j"] = lle_X_atomic_inc<unsigned int>;
    funcNames["lle_X__Z8atom_incPU3AS3i"] = lle_X_atomic_inc<int>;
    funcNames["lle_X__Z8atom_incPU3AS3j"] = lle_X_atomic_inc<unsigned int>;
    funcNames["lle_X__Z8atom_incPU3AS1l"] = lle_X_atomic_inc<long>;
    funcNames["lle_X__Z8atom_incPU3AS1m"] = lle_X_atomic_inc<unsigned long>;
    funcNames["lle_X__Z8atom_incPU3AS3l"] = lle_X_atomic_inc<long>;
    funcNames["lle_X__Z8atom_incPU3AS3m"] = lle_X_atomic_inc<unsigned long>;
    funcNames["lle_X__Z10atomic_decPVU3AS1i"] = lle_X_atomic_dec<int>;
    funcNames["lle_X__Z10atomic_decPVU3AS1j"] = lle_X_atomic_dec<unsigned int>;
    funcNames["lle_X__Z10atomic_decPVU3AS3i"] = lle_X_atomic_dec<int>;
    funcNames["lle_X__Z10atomic_decPVU3AS3j"] = lle_X_atomic_dec<unsigned int>;
    funcNames["lle_X__Z8atom_decPU3AS1i"] = lle_X_atomic_dec<int>;
    funcNames["lle_X__Z8atom_decPU3AS1j"] = lle_X_atomic_dec<unsigned int>;
    funcNames["lle_X__Z8atom_decPU3AS3i"] = lle_X_atomic_dec<int>;
    funcNames["lle_X__Z8atom_decPU3AS3j"] = lle_X_atomic_dec<unsigned int>;
    funcNames["lle_X__Z8atom_decPU3AS1l"] = lle_X_atomic_dec<long>;
    funcNames["lle_X__Z8atom_decPU3AS1m"] = lle_X_atomic_dec<unsigned long>;
    funcNames["lle_X__Z8atom_decPU3AS3l"] = lle_X_atomic_dec<long>;
    funcNames["lle_X__Z8atom_decPU3AS3m"] = lle_X_atomic_dec<unsigned long>;
    funcNames["lle_X__Z14atomic_cmpxchgPVU3AS1iii"] = lle_X_atomic_cmpxchg<int>;
    funcNames["lle_X__Z14atomic_cmpxchgPVU3AS1jjj"] = lle_X_atomic_cmpxchg<unsigned int>;
    funcNames["lle_X__Z14atomic_cmpxchgPVU3AS3iii"] = lle_X_atomic_cmpxchg<int>;
    funcNames["lle_X__Z14atomic_cmpxchgPVU3AS3jjj"] = lle_X_atomic_cmpxchg<unsigned int>;
    funcNames["lle_X__Z12atom_cmpxchgPU3AS1iii"] = lle_X_atomic_cmpxchg<int>;
    funcNames["lle_X__Z12atom_cmpxchgPU3AS1jjj"] = lle_X_atomic_cmpxchg<unsigned int>;
    funcNames["lle_X__Z12atom_cmpxchgPU3AS3iii"] = lle_X_atomic_cmpxchg<int>;
    funcNames["lle_X__Z12atom_cmpxchgPU3AS3jjj"] = lle_X_atomic_cmpxchg<unsigned int>;
    funcNames["lle_X__Z12atom_cmpxchgPU3AS1lll"] = lle_X_atomic_cmpxchg<long>;
    funcNames["lle_X__Z12atom_cmpxchgPU3AS1mmm"] = lle_X_atomic_cmpxchg<unsigned long>;
    funcNames["lle_X__Z12atom_cmpxchgPU3AS3lll"] = lle_X_atomic_cmpxchg<long>;
    funcNames["lle_X__Z12atom_cmpxchgPU3AS3mmm"] = lle_X_atomic_cmpxchg<unsigned long>;
    funcNames["lle_X__Z10atomic_minPVU3AS1ii"] = lle_X_atomic_min<int>;
    funcNames["lle_X__Z10atomic_minPVU3AS1jj"] = lle_X_atomic_min<unsigned int>;
    funcNames["lle_X__Z10atomic_minPVU3AS3ii"] = lle_X_atomic_min<int>;
    funcNames["lle_X__Z10atomic_minPVU3AS3jj"] = lle_X_atomic_min<unsigned int>;
    funcNames["lle_X__Z8atom_minPU3AS1ii"] = lle_X_atomic_min<int>;
    funcNames["lle_X__Z8atom_minPU3AS1jj"] = lle_X_atomic_min<unsigned int>;
    funcNames["lle_X__Z8atom_minPU3AS3ii"] = lle_X_atomic_min<int>;
    funcNames["lle_X__Z8atom_minPU3AS3jj"] = lle_X_atomic_min<unsigned int>;
    funcNames["lle_X__Z8atom_minPU3AS1ll"] = lle_X_atomic_min<long>;
    funcNames["lle_X__Z8atom_minPU3AS1mm"] = lle_X_atomic_min<unsigned long>;
    funcNames["lle_X__Z8atom_minPU3AS3ll"] = lle_X_atomic_min<long>;
    funcNames["lle_X__Z8atom_minPU3AS3mm"] = lle_X_atomic_min<unsigned long>;
    funcNames["lle_X__Z10atomic_maxPVU3AS1ii"] = lle_X_atomic_max<int>;
    funcNames["lle_X__Z10atomic_maxPVU3AS1jj"] = lle_X_atomic_max<unsigned int>;
    funcNames["lle_X__Z10atomic_maxPVU3AS3ii"] = lle_X_atomic_max<int>;
    funcNames["lle_X__Z10atomic_maxPVU3AS3jj"] = lle_X_atomic_max<unsigned int>;
    funcNames["lle_X__Z8atom_maxPU3AS1ii"] = lle_X_atomic_max<int>;
    funcNames["lle_X__Z8atom_maxPU3AS1jj"] = lle_X_atomic_max<unsigned int>;
    funcNames["lle_X__Z8atom_maxPU3AS3ii"] = lle_X_atomic_max<int>;
    funcNames["lle_X__Z8atom_maxPU3AS3jj"] = lle_X_atomic_max<unsigned int>;
    funcNames["lle_X__Z8atom_maxPU3AS1ll"] = lle_X_atomic_max<long>;
    funcNames["lle_X__Z8atom_maxPU3AS1mm"] = lle_X_atomic_max<unsigned long>;
    funcNames["lle_X__Z8atom_maxPU3AS3ll"] = lle_X_atomic_max<long>;
    funcNames["lle_X__Z8atom_maxPU3AS3mm"] = lle_X_atomic_max<unsigned long>;
    funcNames["lle_X__Z10atomic_andPVU3AS1ii"] = lle_X_atomic_and<int>;
    funcNames["lle_X__Z10atomic_andPVU3AS1jj"] = lle_X_atomic_and<unsigned int>;
    funcNames["lle_X__Z10atomic_andPVU3AS3ii"] = lle_X_atomic_and<int>;
    funcNames["lle_X__Z10atomic_andPVU3AS3jj"] = lle_X_atomic_and<unsigned int>;
    funcNames["lle_X__Z8atom_andPU3AS1ii"] = lle_X_atomic_and<int>;
    funcNames["lle_X__Z8atom_andPU3AS1jj"] = lle_X_atomic_and<unsigned int>;
    funcNames["lle_X__Z8atom_andPU3AS3ii"] = lle_X_atomic_and<int>;
    funcNames["lle_X__Z8atom_andPU3AS3jj"] = lle_X_atomic_and<unsigned int>;
    funcNames["lle_X__Z8atom_andPU3AS1ll"] = lle_X_atomic_and<long>;
    funcNames["lle_X__Z8atom_andPU3AS1mm"] = lle_X_atomic_and<unsigned long>;
    funcNames["lle_X__Z8atom_andPU3AS3ll"] = lle_X_atomic_and<long>;
    funcNames["lle_X__Z8atom_andPU3AS3mm"] = lle_X_atomic_and<unsigned long>;
    funcNames["lle_X__Z9atomic_orPVU3AS1ii"] = lle_X_atomic_or<int>;
    funcNames["lle_X__Z9atomic_orPVU3AS1jj"] = lle_X_atomic_or<unsigned int>;
    funcNames["lle_X__Z9atomic_orPVU3AS3ii"] = lle_X_atomic_or<int>;
    funcNames["lle_X__Z9atomic_orPVU3AS3jj"] = lle_X_atomic_or<unsigned int>;
    funcNames["lle_X__Z7atom_orPU3AS1ii"] = lle_X_atomic_or<int>;
    funcNames["lle_X__Z7atom_orPU3AS1jj"] = lle_X_atomic_or<unsigned int>;
    funcNames["lle_X__Z7atom_orPU3AS3ii"] = lle_X_atomic_or<int>;
    funcNames["lle_X__Z7atom_orPU3AS3jj"] = lle_X_atomic_or<unsigned int>;
    funcNames["lle_X__Z7atom_orPU3AS1ll"] = lle_X_atomic_or<long>;
    funcNames["lle_X__Z7atom_orPU3AS1mm"] = lle_X_atomic_or<unsigned long>;
    funcNames["lle_X__Z7atom_orPU3AS3ll"] = lle_X_atomic_or<long>;
    funcNames["lle_X__Z7atom_orPU3AS3mm"] = lle_X_atomic_or<unsigned long>;
    funcNames["lle_X__Z10atomic_xorPVU3AS1ii"] = lle_X_atomic_xor<int>;
    funcNames["lle_X__Z10atomic_xorPVU3AS1jj"] = lle_X_atomic_xor<unsigned int>;
    funcNames["lle_X__Z10atomic_xorPVU3AS3ii"] = lle_X_atomic_xor<int>;
    funcNames["lle_X__Z10atomic_xorPVU3AS3jj"] = lle_X_atomic_xor<unsigned int>;
    funcNames["lle_X__Z8atom_xorPU3AS1ii"] = lle_X_atomic_xor<int>;
    funcNames["lle_X__Z8atom_xorPU3AS1jj"] = lle_X_atomic_xor<unsigned int>;
    funcNames["lle_X__Z8atom_xorPU3AS3ii"] = lle_X_atomic_xor<int>;
    funcNames["lle_X__Z8atom_xorPU3AS3jj"] = lle_X_atomic_xor<unsigned int>;
    funcNames["lle_X__Z8atom_xorPU3AS1ll"] = lle_X_atomic_xor<long>;
    funcNames["lle_X__Z8atom_xorPU3AS1mm"] = lle_X_atomic_xor<unsigned long>;
    funcNames["lle_X__Z8atom_xorPU3AS3ll"] = lle_X_atomic_xor<long>;
    funcNames["lle_X__Z8atom_xorPU3AS3mm"] = lle_X_atomic_xor<unsigned long>;
}

}
}
