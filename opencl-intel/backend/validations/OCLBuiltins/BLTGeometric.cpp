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

File Name:  BLTGeometric.cpp

\*****************************************************************************/

#include "BLTGeometric.h"

using namespace llvm;
using std::map;
using std::string;
using std::vector;
namespace Validation {
namespace OCLBuiltins {

void GeometricMapFiller::addOpenCLBuiltins( map<string, PBLTFunc>& funcNames )
{
    funcNames["lle_X__Z9normalizef"]        = lle_X_normalize<float,1>;
    funcNames["lle_X__Z9normalizeDv2_f"]    = lle_X_normalize<float,2>;
    funcNames["lle_X__Z9normalizeDv3_f"]    = lle_X_normalize<float,3>;
    funcNames["lle_X__Z9normalizeDv4_f"]    = lle_X_normalize<float,4>;
    funcNames["lle_X__Z9normalized"]        = lle_X_normalize<double,1>;
    funcNames["lle_X__Z9normalizeDv2_d"]    = lle_X_normalize<double,2>;
    funcNames["lle_X__Z9normalizeDv3_d"]    = lle_X_normalize<double,3>;
    funcNames["lle_X__Z9normalizeDv4_d"]    = lle_X_normalize<double,4>;

    funcNames["lle_X__Z14fast_normalizef"]     = lle_X_normalize<float,1>;
    funcNames["lle_X__Z14fast_normalizeDv2_f"] = lle_X_normalize<float,2>;
    funcNames["lle_X__Z14fast_normalizeDv3_f"] = lle_X_normalize<float,3>;
    funcNames["lle_X__Z14fast_normalizeDv4_f"] = lle_X_normalize<float,4>;

    funcNames["lle_X__Z3dotff"]         = lle_X_dot<float,1>;
    funcNames["lle_X__Z3dotDv2_fS_"]    = lle_X_dot<float,2>;
    funcNames["lle_X__Z3dotDv3_fS_"]    = lle_X_dot<float,3>;
    funcNames["lle_X__Z3dotDv4_fS_"]    = lle_X_dot<float,4>;
    funcNames["lle_X__Z3dotdd"]         = lle_X_dot<double,1>;
    funcNames["lle_X__Z3dotDv2_dS_"]    = lle_X_dot<double,2>;
    funcNames["lle_X__Z3dotDv3_dS_"]    = lle_X_dot<double,3>;
    funcNames["lle_X__Z3dotDv4_dS_"]    = lle_X_dot<double,4>;

    funcNames["lle_X__Z6lengthf"]     = lle_X_length<float,1>;
    funcNames["lle_X__Z6lengthDv2_f"] = lle_X_length<float,2>;
    funcNames["lle_X__Z6lengthDv3_f"] = lle_X_length<float,3>;
    funcNames["lle_X__Z6lengthDv4_f"] = lle_X_length<float,4>;
    funcNames["lle_X__Z6lengthd"]     = lle_X_length<double,1>;
    funcNames["lle_X__Z6lengthDv2_d"] = lle_X_length<double,2>;
    funcNames["lle_X__Z6lengthDv3_d"] = lle_X_length<double,3>;
    funcNames["lle_X__Z6lengthDv4_d"] = lle_X_length<double,4>;

    funcNames["lle_X__Z11fast_lengthf"]     = lle_X_length<float,1>;
    funcNames["lle_X__Z11fast_lengthDv2_f"] = lle_X_length<float,2>;
    funcNames["lle_X__Z11fast_lengthDv3_f"] = lle_X_length<float,3>;
    funcNames["lle_X__Z11fast_lengthDv4_f"] = lle_X_length<float,4>;

    funcNames["lle_X__Z8distanceff"]      = lle_X_distance<float,1>;
    funcNames["lle_X__Z8distanceDv2_fS_"] = lle_X_distance<float,2>;
    funcNames["lle_X__Z8distanceDv3_fS_"] = lle_X_distance<float,3>;
    funcNames["lle_X__Z8distanceDv4_fS_"] = lle_X_distance<float,4>;
    funcNames["lle_X__Z8distancedd"]      = lle_X_distance<double,1>;
    funcNames["lle_X__Z8distanceDv2_dS_"] = lle_X_distance<double,1>;
    funcNames["lle_X__Z8distanceDv3_dS_"] = lle_X_distance<double,1>;
    funcNames["lle_X__Z8distanceDv4_dS_"] = lle_X_distance<double,1>;

    funcNames["lle_X__Z13fast_distanceff"]      = lle_X_distance<float,1>;
    funcNames["lle_X__Z13fast_distanceDv2_fS_"] = lle_X_distance<float,2>;
    funcNames["lle_X__Z13fast_distanceDv3_fS_"] = lle_X_distance<float,3>;
    funcNames["lle_X__Z13fast_distanceDv4_fS_"] = lle_X_distance<float,4>;

    funcNames["lle_X__Z5crossDv4_fS_"] = lle_X_cross<float,4>;
    funcNames["lle_X__Z5crossDv3_fS_"] = lle_X_cross<float,3>;
    funcNames["lle_X__Z5crossDv4_dS_"] = lle_X_cross<double,4>;
    funcNames["lle_X__Z5crossDv3_dS_"] = lle_X_cross<double,3>;
}

}
}
