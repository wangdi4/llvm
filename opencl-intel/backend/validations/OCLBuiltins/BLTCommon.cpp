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

File Name:  BLTCommon.cpp

\*****************************************************************************/

#include "BLTCommon.h"

using namespace llvm;
using std::map;
using std::string;
using std::vector;
namespace Validation {
namespace OCLBuiltins {

void CommonMapFiller::addOpenCLBuiltins( map<string, PBLTFunc>& funcNames )
{
    // double
    funcNames["lle_X__Z3maxdd"]             = lle_X_max<double, 1, 1>;
    funcNames["lle_X__Z3maxDv2_dS_"]        = lle_X_max<double, 2, 2>;
    funcNames["lle_X__Z3maxDv3_dS_"]        = lle_X_max<double, 3, 3>;
    funcNames["lle_X__Z3maxDv4_dS_"]        = lle_X_max<double, 4, 4>;
    funcNames["lle_X__Z3maxDv8_dS_"]        = lle_X_max<double, 8, 8>;
    funcNames["lle_X__Z3maxDv16_dS_"]       = lle_X_max<double, 16, 16>;
    funcNames["lle_X__Z3maxDv2_dd"]         = lle_X_max<double, 2, 1>;
    funcNames["lle_X__Z3maxDv3_dd"]         = lle_X_max<double, 3, 1>;
    funcNames["lle_X__Z3maxDv4_dd"]         = lle_X_max<double, 4, 1>;
    funcNames["lle_X__Z3maxDv8_dd"]         = lle_X_max<double, 8, 1>;
    funcNames["lle_X__Z3maxDv16_dd"]        = lle_X_max<double, 16, 1>;
    // float
    funcNames["lle_X__Z3maxff"]             = lle_X_max<float, 1, 1>;
    funcNames["lle_X__Z3maxDv2_fS_"]        = lle_X_max<float, 2, 2>;
    funcNames["lle_X__Z3maxDv3_fS_"]        = lle_X_max<float, 3, 3>;
    funcNames["lle_X__Z3maxDv4_fS_"]        = lle_X_max<float, 4, 4>;
    funcNames["lle_X__Z3maxDv8_fS_"]        = lle_X_max<float, 8, 8>;
    funcNames["lle_X__Z3maxDv16_fS_"]       = lle_X_max<float, 16, 16>;
    funcNames["lle_X__Z3maxDv2_ff"]         = lle_X_max<float, 2, 1>;
    funcNames["lle_X__Z3maxDv3_ff"]         = lle_X_max<float, 3, 1>;
    funcNames["lle_X__Z3maxDv4_ff"]         = lle_X_max<float, 4, 1>;
    funcNames["lle_X__Z3maxDv8_ff"]         = lle_X_max<float, 8, 1>;
    funcNames["lle_X__Z3maxDv16_ff"]        = lle_X_max<float, 16, 1>;

    // double
    funcNames["lle_X__Z3mindd"]             = lle_X_min<double, 1, 1>;
    funcNames["lle_X__Z3minDv2_dS_"]        = lle_X_min<double, 2, 2>;
    funcNames["lle_X__Z3minDv3_dS_"]        = lle_X_min<double, 3, 3>;
    funcNames["lle_X__Z3minDv4_dS_"]        = lle_X_min<double, 4, 4>;
    funcNames["lle_X__Z3minDv8_dS_"]        = lle_X_min<double, 8, 8>;
    funcNames["lle_X__Z3minDv16_dS_"]       = lle_X_min<double, 16, 16>;
    funcNames["lle_X__Z3minDv2_dd"]         = lle_X_min<double, 2, 1>;
    funcNames["lle_X__Z3minDv3_dd"]         = lle_X_min<double, 3, 1>;
    funcNames["lle_X__Z3minDv4_dd"]         = lle_X_min<double, 4, 1>;
    funcNames["lle_X__Z3minDv8_dd"]         = lle_X_min<double, 8, 1>;
    funcNames["lle_X__Z3minDv16_dd"]        = lle_X_min<double, 16, 1>;
    // float
    funcNames["lle_X__Z3minff"]             = lle_X_min<float, 1, 1>;
    funcNames["lle_X__Z3minDv2_fS_"]        = lle_X_min<float, 2, 2>;
    funcNames["lle_X__Z3minDv3_fS_"]        = lle_X_min<float, 3, 3>;
    funcNames["lle_X__Z3minDv4_fS_"]        = lle_X_min<float, 4, 4>;
    funcNames["lle_X__Z3minDv8_fS_"]        = lle_X_min<float, 8, 8>;
    funcNames["lle_X__Z3minDv16_fS_"]       = lle_X_min<float, 16, 16>;
    funcNames["lle_X__Z3minDv2_ff"]         = lle_X_min<float, 2, 1>;
    funcNames["lle_X__Z3minDv3_ff"]         = lle_X_min<float, 3, 1>;
    funcNames["lle_X__Z3minDv4_ff"]         = lle_X_min<float, 4, 1>;
    funcNames["lle_X__Z3minDv8_ff"]         = lle_X_min<float, 8, 1>;
    funcNames["lle_X__Z3minDv16_ff"]        = lle_X_min<float, 16, 1>;

    funcNames["lle_X__Z3mixfff"]        = lle_X_mix<float, 1, 1>;
    funcNames["lle_X__Z3mixDv2_fS_S_"]  = lle_X_mix<float, 2, 2>;
    funcNames["lle_X__Z3mixDv3_fS_S_"]  = lle_X_mix<float, 3, 3>;
    funcNames["lle_X__Z3mixDv4_fS_S_"]  = lle_X_mix<float, 4, 4>;
    funcNames["lle_X__Z3mixDv8_fS_S_"]  = lle_X_mix<float, 8, 8>;
    funcNames["lle_X__Z3mixDv16_fS_S_"] = lle_X_mix<float, 16, 16>;
    funcNames["lle_X__Z3mixDv2_fS_f"]   = lle_X_mix<float, 2, 1>;
    funcNames["lle_X__Z3mixDv3_fS_f"]   = lle_X_mix<float, 3, 1>;
    funcNames["lle_X__Z3mixDv4_fS_f"]   = lle_X_mix<float, 4, 1>;
    funcNames["lle_X__Z3mixDv8_fS_f"]   = lle_X_mix<float, 8, 1>;
    funcNames["lle_X__Z3mixDv16_fS_f"]  = lle_X_mix<float, 16, 1>;
    funcNames["lle_X__Z3mixddd"]        = lle_X_mix<double, 1, 1>;
    funcNames["lle_X__Z3mixDv2_dS_S_"]  = lle_X_mix<double, 2, 2>;
    funcNames["lle_X__Z3mixDv3_dS_S_"]  = lle_X_mix<double, 3, 3>;
    funcNames["lle_X__Z3mixDv4_dS_S_"]  = lle_X_mix<double, 4, 4>;
    funcNames["lle_X__Z3mixDv8_dS_S_"]  = lle_X_mix<double, 8, 8>;
    funcNames["lle_X__Z3mixDv16_dS_S_"] = lle_X_mix<double, 16, 16>;
    funcNames["lle_X__Z3mixDv2_dS_d"]   = lle_X_mix<double, 2, 1>;
    funcNames["lle_X__Z3mixDv3_dS_d"]   = lle_X_mix<double, 3, 1>;
    funcNames["lle_X__Z3mixDv4_dS_d"]   = lle_X_mix<double, 4, 1>;
    funcNames["lle_X__Z3mixDv8_dS_d"]   = lle_X_mix<double, 8, 1>;
    funcNames["lle_X__Z3mixDv16_dS_d"]  = lle_X_mix<double, 16, 1>;

    funcNames["lle_X__Z5clampfff"]          = lle_X_clamp<float, 1, 1>;
    funcNames["lle_X__Z5clampDv2_fS_S_"]    = lle_X_clamp<float, 2, 2>;
    funcNames["lle_X__Z5clampDv3_fS_S_"]    = lle_X_clamp<float, 3, 3>;
    funcNames["lle_X__Z5clampDv4_fS_S_"]    = lle_X_clamp<float, 4, 4>;
    funcNames["lle_X__Z5clampDv8_fS_S_"]    = lle_X_clamp<float, 8, 8>;
    funcNames["lle_X__Z5clampDv16_fS_S_"]   = lle_X_clamp<float, 16, 16>;
    funcNames["lle_X__Z5clampDv2_fff"]      = lle_X_clamp<float, 2, 1>;
    funcNames["lle_X__Z5clampDv3_fff"]      = lle_X_clamp<float, 3, 1>;
    funcNames["lle_X__Z5clampDv4_fff"]      = lle_X_clamp<float, 4, 1>;
    funcNames["lle_X__Z5clampDv8_fff"]      = lle_X_clamp<float, 8, 1>;
    funcNames["lle_X__Z5clampDv16_fff"]     = lle_X_clamp<float, 16, 1>;
    funcNames["lle_X__Z5clampddd"]          = lle_X_clamp<double, 1, 1>;
    funcNames["lle_X__Z5clampDv2_dS_S_"]    = lle_X_clamp<double, 2, 2>;
    funcNames["lle_X__Z5clampDv3_dS_S_"]    = lle_X_clamp<double, 3, 3>;
    funcNames["lle_X__Z5clampDv4_dS_S_"]    = lle_X_clamp<double, 4, 4>;
    funcNames["lle_X__Z5clampDv8_dS_S_"]    = lle_X_clamp<double, 8, 8>;
    funcNames["lle_X__Z5clampDv16_dS_S_"]   = lle_X_clamp<double, 16, 16>;
    funcNames["lle_X__Z5clampDv2_ddd"]      = lle_X_clamp<double, 2, 1>;
    funcNames["lle_X__Z5clampDv3_ddd"]      = lle_X_clamp<double, 3, 1>;
    funcNames["lle_X__Z5clampDv4_ddd"]      = lle_X_clamp<double, 4, 1>;
    funcNames["lle_X__Z5clampDv8_ddd"]      = lle_X_clamp<double, 8, 1>;
    funcNames["lle_X__Z5clampDv16_ddd"]     = lle_X_clamp<double, 16, 1>;

    funcNames["lle_X__Z4stepff"]            = lle_X_step<float, 1, 1>;
    funcNames["lle_X__Z4stepDv2_fS_"]       = lle_X_step<float, 2, 2>;
    funcNames["lle_X__Z4stepDv3_fS_"]       = lle_X_step<float, 3, 3>;
    funcNames["lle_X__Z4stepDv4_fS_"]       = lle_X_step<float, 4, 4>;
    funcNames["lle_X__Z4stepDv8_fS_"]       = lle_X_step<float, 8, 8>;
    funcNames["lle_X__Z4stepDv16_fS_"]      = lle_X_step<float, 16, 16>;
    funcNames["lle_X__Z4stepfDv2_f"]        = lle_X_step<float, 1, 2>;
    funcNames["lle_X__Z4stepfDv3_f"]        = lle_X_step<float, 1, 3>;
    funcNames["lle_X__Z4stepfDv4_f"]        = lle_X_step<float, 1, 4>;
    funcNames["lle_X__Z4stepfDv8_f"]        = lle_X_step<float, 1, 8>;
    funcNames["lle_X__Z4stepfDv16_f"]       = lle_X_step<float, 1, 16>;
    funcNames["lle_X__Z4stepdd"]            = lle_X_step<double, 1, 1>;
    funcNames["lle_X__Z4stepDv2_dS_"]       = lle_X_step<double, 2, 2>;
    funcNames["lle_X__Z4stepDv3_dS_"]       = lle_X_step<double, 3, 3>;
    funcNames["lle_X__Z4stepDv4_dS_"]       = lle_X_step<double, 4, 4>;
    funcNames["lle_X__Z4stepDv8_dS_"]       = lle_X_step<double, 8, 8>;
    funcNames["lle_X__Z4stepDv16_dS_"]      = lle_X_step<double, 16, 16>;
    funcNames["lle_X__Z4stepdDv2_d"]        = lle_X_step<double, 1, 2>;
    funcNames["lle_X__Z4stepdDv3_d"]        = lle_X_step<double, 1, 3>;
    funcNames["lle_X__Z4stepdDv4_d"]        = lle_X_step<double, 1, 4>;
    funcNames["lle_X__Z4stepdDv8_d"]        = lle_X_step<double, 1, 8>;
    funcNames["lle_X__Z4stepdDv16_d"]       = lle_X_step<double, 1, 16>;

    funcNames["lle_X__Z7radiansf"]          = lle_X_radians<float, 1>;
    funcNames["lle_X__Z7radiansDv2_f"]      = lle_X_radians<float, 2>;
    funcNames["lle_X__Z7radiansDv3_f"]      = lle_X_radians<float, 3>;
    funcNames["lle_X__Z7radiansDv4_f"]      = lle_X_radians<float, 4>;
    funcNames["lle_X__Z7radiansDv8_f"]      = lle_X_radians<float, 8>;
    funcNames["lle_X__Z7radiansDv16_f"]     = lle_X_radians<float, 16>;
    funcNames["lle_X__Z7radiansd"]          = lle_X_radians<double, 1>;
    funcNames["lle_X__Z7radiansDv2_d"]      = lle_X_radians<double, 2>;
    funcNames["lle_X__Z7radiansDv3_d"]      = lle_X_radians<double, 3>;
    funcNames["lle_X__Z7radiansDv4_d"]      = lle_X_radians<double, 4>;
    funcNames["lle_X__Z7radiansDv8_d"]      = lle_X_radians<double, 8>;
    funcNames["lle_X__Z7radiansDv16_d"]     = lle_X_radians<double, 16>;

    funcNames["lle_X__Z7degreesf"]          = lle_X_degrees<float, 1>;
    funcNames["lle_X__Z7degreesDv2_f"]      = lle_X_degrees<float, 2>;
    funcNames["lle_X__Z7degreesDv3_f"]      = lle_X_degrees<float, 3>;
    funcNames["lle_X__Z7degreesDv4_f"]      = lle_X_degrees<float, 4>;
    funcNames["lle_X__Z7degreesDv8_f"]      = lle_X_degrees<float, 8>;
    funcNames["lle_X__Z7degreesDv16_f"]     = lle_X_degrees<float, 16>;
    funcNames["lle_X__Z7degreesd"]          = lle_X_degrees<double, 1>;
    funcNames["lle_X__Z7degreesDv2_d"]      = lle_X_degrees<double, 2>;
    funcNames["lle_X__Z7degreesDv3_d"]      = lle_X_degrees<double, 3>;
    funcNames["lle_X__Z7degreesDv4_d"]      = lle_X_degrees<double, 4>;
    funcNames["lle_X__Z7degreesDv8_d"]      = lle_X_degrees<double, 8>;
    funcNames["lle_X__Z7degreesDv16_d"]     = lle_X_degrees<double, 16>;

    funcNames["lle_X__Z10smoothstepfff"]        = lle_X_smoothstep<float, 1, 1>;
    funcNames["lle_X__Z10smoothstepDv2_fS_S_"]  = lle_X_smoothstep<float, 2, 2>;
    funcNames["lle_X__Z10smoothstepDv3_fS_S_"]  = lle_X_smoothstep<float, 3, 3>;
    funcNames["lle_X__Z10smoothstepDv4_fS_S_"]  = lle_X_smoothstep<float, 4, 4>;
    funcNames["lle_X__Z10smoothstepDv8_fS_S_"]  = lle_X_smoothstep<float, 8, 8>;
    funcNames["lle_X__Z10smoothstepDv16_fS_S_"] = lle_X_smoothstep<float, 16, 16>;
    funcNames["lle_X__Z10smoothstepffDv2_f"]    = lle_X_smoothstep<float, 1, 2>;
    funcNames["lle_X__Z10smoothstepffDv3_f"]    = lle_X_smoothstep<float, 1, 3>;
    funcNames["lle_X__Z10smoothstepffDv4_f"]    = lle_X_smoothstep<float, 1, 4>;
    funcNames["lle_X__Z10smoothstepffDv8_f"]    = lle_X_smoothstep<float, 1, 8>;
    funcNames["lle_X__Z10smoothstepffDv16_f"]   = lle_X_smoothstep<float, 1, 16>;
    funcNames["lle_X__Z10smoothstepddd"]        = lle_X_smoothstep<double, 1, 1>;
    funcNames["lle_X__Z10smoothstepddDv2_d"]    = lle_X_smoothstep<double, 1, 2>;
    funcNames["lle_X__Z10smoothstepddDv3_d"]    = lle_X_smoothstep<double, 1, 3>;
    funcNames["lle_X__Z10smoothstepddDv4_d"]    = lle_X_smoothstep<double, 1, 4>;
    funcNames["lle_X__Z10smoothstepddDv8_d"]    = lle_X_smoothstep<double, 1, 8>;
    funcNames["lle_X__Z10smoothstepddDv16_d"]   = lle_X_smoothstep<double, 1, 16>;
    funcNames["lle_X__Z10smoothstepDv2_dS_S_"]  = lle_X_smoothstep<double, 2, 2>;
    funcNames["lle_X__Z10smoothstepDv3_dS_S_"]  = lle_X_smoothstep<double, 3, 3>;
    funcNames["lle_X__Z10smoothstepDv4_dS_S_"]  = lle_X_smoothstep<double, 4, 4>;
    funcNames["lle_X__Z10smoothstepDv8_dS_S_"]  = lle_X_smoothstep<double, 5, 8>;
    funcNames["lle_X__Z10smoothstepDv16_dS_S_"] = lle_X_smoothstep<double, 6, 16>;

    funcNames["lle_X__Z4signf"]      = lle_X_sign<float, 1>;
    funcNames["lle_X__Z4signDv2_f"]  = lle_X_sign<float, 2>;
    funcNames["lle_X__Z4signDv3_f"]  = lle_X_sign<float, 3>;
    funcNames["lle_X__Z4signDv4_f"]  = lle_X_sign<float, 4>;
    funcNames["lle_X__Z4signDv8_f"]  = lle_X_sign<float, 8>;
    funcNames["lle_X__Z4signDv16_f"] = lle_X_sign<float, 16>;
    funcNames["lle_X__Z4signd"]      = lle_X_sign<double, 1>;
    funcNames["lle_X__Z4signDv2_d"]  = lle_X_sign<double, 2>;
    funcNames["lle_X__Z4signDv3_d"]  = lle_X_sign<double, 3>;
    funcNames["lle_X__Z4signDv4_d"]  = lle_X_sign<double, 4>;
    funcNames["lle_X__Z4signDv8_d"]  = lle_X_sign<double, 8>;
    funcNames["lle_X__Z4signDv16_d"] = lle_X_sign<double, 16>;
}

}
}
