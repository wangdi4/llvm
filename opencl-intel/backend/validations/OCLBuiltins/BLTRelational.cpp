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

File Name:  BLTRelational.cpp

\*****************************************************************************/

#include "BLTRelational.h"

using namespace llvm;
using std::map;
using std::string;
using std::vector;
namespace Validation {
namespace OCLBuiltins {

void RelationalMapFiller::addOpenCLBuiltins( map<string, PBLTFunc>& funcNames )
{
    funcNames["lle_X__Z5isinff"]        = lle_X_isinf<float>;
    funcNames["lle_X__Z5isinfDv2_f"]    = lle_X_isinf<float, 2>;
    funcNames["lle_X__Z5isinfDv3_f"]    = lle_X_isinf<float, 3>;
    funcNames["lle_X__Z5isinfDv4_f"]    = lle_X_isinf<float, 4>;
    funcNames["lle_X__Z5isinfDv8_f"]    = lle_X_isinf<float, 8>;
    funcNames["lle_X__Z5isinfDv16_f"]   = lle_X_isinf<float, 16>;
    funcNames["lle_X__Z5isinfd"]        = lle_X_isinf<double>;
    funcNames["lle_X__Z5isinfDv2_d"]    = lle_X_isinf<double, 2>;
    funcNames["lle_X__Z5isinfDv3_d"]    = lle_X_isinf<double, 3>;
    funcNames["lle_X__Z5isinfDv4_d"]    = lle_X_isinf<double, 4>;
    funcNames["lle_X__Z5isinfDv8_d"]    = lle_X_isinf<double, 8>;
    funcNames["lle_X__Z5isinfDv16_d"]   = lle_X_isinf<double, 16>;
    funcNames["lle_X__Z8isnormalf"]         = lle_X_isnormal<float>;
    funcNames["lle_X__Z8isnormalDv2_f"]     = lle_X_isnormal<float, 2>;
    funcNames["lle_X__Z8isnormalDv3_f"]     = lle_X_isnormal<float, 3>;
    funcNames["lle_X__Z8isnormalDv4_f"]     = lle_X_isnormal<float, 4>;
    funcNames["lle_X__Z8isnormalDv8_f"]     = lle_X_isnormal<float, 8>;
    funcNames["lle_X__Z8isnormalDv16_f"]    = lle_X_isnormal<float, 16>;
    funcNames["lle_X__Z8isnormald"]         = lle_X_isnormal<double>;
    funcNames["lle_X__Z8isnormalDv2_d"]     = lle_X_isnormal<double, 2>;
    funcNames["lle_X__Z8isnormalDv3_d"]     = lle_X_isnormal<double, 3>;
    funcNames["lle_X__Z8isnormalDv4_d"]     = lle_X_isnormal<double, 4>;
    funcNames["lle_X__Z8isnormalDv8_d"]     = lle_X_isnormal<double, 8>;
    funcNames["lle_X__Z8isnormalDv16_d"]    = lle_X_isnormal<double, 16>;
    funcNames["lle_X__Z5isnanf"]        = lle_X_isnan<float>;
    funcNames["lle_X__Z5isnanDv2_f"]    = lle_X_isnan<float, 2>;
    funcNames["lle_X__Z5isnanDv3_f"]    = lle_X_isnan<float, 3>;
    funcNames["lle_X__Z5isnanDv4_f"]    = lle_X_isnan<float, 4>;
    funcNames["lle_X__Z5isnanDv8_f"]    = lle_X_isnan<float, 8>;
    funcNames["lle_X__Z5isnanDv16_f"]   = lle_X_isnan<float, 16>;
    funcNames["lle_X__Z5isnand"]        = lle_X_isnan<double>;
    funcNames["lle_X__Z5isnanDv2_d"]    = lle_X_isnan<double, 2>;
    funcNames["lle_X__Z5isnanDv3_d"]    = lle_X_isnan<double, 3>;
    funcNames["lle_X__Z5isnanDv4_d"]    = lle_X_isnan<double, 4>;
    funcNames["lle_X__Z5isnanDv8_d"]    = lle_X_isnan<double, 8>;
    funcNames["lle_X__Z5isnanDv16_d"]   = lle_X_isnan<double, 16>;
    funcNames["lle_X__Z9isgreaterff"]         = lle_X_isgreater<float>;
    funcNames["lle_X__Z9isgreaterDv2_fS_"]    = lle_X_isgreater<float, 2>;
    funcNames["lle_X__Z9isgreaterDv3_fS_"]    = lle_X_isgreater<float, 3>;
    funcNames["lle_X__Z9isgreaterDv4_fS_"]    = lle_X_isgreater<float, 4>;
    funcNames["lle_X__Z9isgreaterDv8_fS_"]    = lle_X_isgreater<float, 8>;
    funcNames["lle_X__Z9isgreaterDv16_fS_"]   = lle_X_isgreater<float, 16>;
    funcNames["lle_X__Z9isgreaterdd"]         = lle_X_isgreater<double>;
    funcNames["lle_X__Z9isgreaterDv2_dS_"]    = lle_X_isgreater<double, 2>;
    funcNames["lle_X__Z9isgreaterDv3_dS_"]    = lle_X_isgreater<double, 3>;
    funcNames["lle_X__Z9isgreaterDv4_dS_"]    = lle_X_isgreater<double, 4>;
    funcNames["lle_X__Z9isgreaterDv8_dS_"]    = lle_X_isgreater<double, 8>;
    funcNames["lle_X__Z9isgreaterDv16_dS_"]   = lle_X_isgreater<double, 16>;
    funcNames["lle_X__Z7isequalff"]           = lle_X_isequal<float>;
    funcNames["lle_X__Z7isequalDv2_fS_"]      = lle_X_isequal<float, 2>;
    funcNames["lle_X__Z7isequalDv3_fS_"]      = lle_X_isequal<float, 3>;
    funcNames["lle_X__Z7isequalDv4_fS_"]      = lle_X_isequal<float, 4>;
    funcNames["lle_X__Z7isequalDv8_fS_"]      = lle_X_isequal<float, 8>;
    funcNames["lle_X__Z7isequalDv16_fS_"]     = lle_X_isequal<float, 16>;
    funcNames["lle_X__Z7isequaldd"]           = lle_X_isequal<double>;
    funcNames["lle_X__Z7isequalDv2_dS_"]      = lle_X_isequal<double, 2>;
    funcNames["lle_X__Z7isequalDv3_dS_"]      = lle_X_isequal<double, 3>;
    funcNames["lle_X__Z7isequalDv4_dS_"]      = lle_X_isequal<double, 4>;
    funcNames["lle_X__Z7isequalDv8_dS_"]      = lle_X_isequal<double, 8>;
    funcNames["lle_X__Z7isequalDv16_dS_"]     = lle_X_isequal<double, 16>;
    funcNames["lle_X__Z14isgreaterequalff"]       = lle_X_isgreaterequal<float>;
    funcNames["lle_X__Z14isgreaterequalDv2_fS_"]  = lle_X_isgreaterequal<float, 2>;
    funcNames["lle_X__Z14isgreaterequalDv3_fS_"]  = lle_X_isgreaterequal<float, 3>;
    funcNames["lle_X__Z14isgreaterequalDv4_fS_"]  = lle_X_isgreaterequal<float, 4>;
    funcNames["lle_X__Z14isgreaterequalDv8_fS_"]  = lle_X_isgreaterequal<float, 8>;
    funcNames["lle_X__Z14isgreaterequalDv16_fS_"] = lle_X_isgreaterequal<float, 16>;
    funcNames["lle_X__Z14isgreaterequaldd"]       = lle_X_isgreaterequal<double>;
    funcNames["lle_X__Z14isgreaterequalDv2_dS_"]  = lle_X_isgreaterequal<double, 2>;
    funcNames["lle_X__Z14isgreaterequalDv3_dS_"]  = lle_X_isgreaterequal<double, 3>;
    funcNames["lle_X__Z14isgreaterequalDv4_dS_"]  = lle_X_isgreaterequal<double, 4>;
    funcNames["lle_X__Z14isgreaterequalDv8_dS_"]  = lle_X_isgreaterequal<double, 8>;
    funcNames["lle_X__Z14isgreaterequalDv16_dS_"] = lle_X_isgreaterequal<double, 16>;
    funcNames["lle_X__Z6islessff"]            = lle_X_isless<float>;
    funcNames["lle_X__Z6islessDv2_fS_"]       = lle_X_isless<float, 2>;
    funcNames["lle_X__Z6islessDv3_fS_"]       = lle_X_isless<float, 3>;
    funcNames["lle_X__Z6islessDv4_fS_"]       = lle_X_isless<float, 4>;
    funcNames["lle_X__Z6islessDv8_fS_"]       = lle_X_isless<float, 8>;
    funcNames["lle_X__Z6islessDv16_fS_"]      = lle_X_isless<float, 16>;
    funcNames["lle_X__Z6islessdd"]            = lle_X_isless<double>;
    funcNames["lle_X__Z6islessDv2_dS_"]       = lle_X_isless<double, 2>;
    funcNames["lle_X__Z6islessDv3_dS_"]       = lle_X_isless<double, 3>;
    funcNames["lle_X__Z6islessDv4_dS_"]       = lle_X_isless<double, 4>;
    funcNames["lle_X__Z6islessDv8_dS_"]       = lle_X_isless<double, 8>;
    funcNames["lle_X__Z6islessDv16_dS_"]      = lle_X_isless<double, 16>;
    funcNames["lle_X__Z11islessequalff"]          = lle_X_islessequal<float>;
    funcNames["lle_X__Z11islessequalDv2_fS_"]     = lle_X_islessequal<float, 2>;
    funcNames["lle_X__Z11islessequalDv3_fS_"]     = lle_X_islessequal<float, 3>;
    funcNames["lle_X__Z11islessequalDv4_fS_"]     = lle_X_islessequal<float, 4>;
    funcNames["lle_X__Z11islessequalDv8_fS_"]     = lle_X_islessequal<float, 8>;
    funcNames["lle_X__Z11islessequalDv16_fS_"]    = lle_X_islessequal<float, 16>;
    funcNames["lle_X__Z11islessequaldd"]          = lle_X_islessequal<double>;
    funcNames["lle_X__Z11islessequalDv2_dS_"]     = lle_X_islessequal<double, 2>;
    funcNames["lle_X__Z11islessequalDv3_dS_"]     = lle_X_islessequal<double, 3>;
    funcNames["lle_X__Z11islessequalDv4_dS_"]     = lle_X_islessequal<double, 4>;
    funcNames["lle_X__Z11islessequalDv8_dS_"]     = lle_X_islessequal<double, 8>;
    funcNames["lle_X__Z11islessequalDv16_dS_"]    = lle_X_islessequal<double, 16>;
    funcNames["lle_X__Z13islessgreaterff"]        = lle_X_islessgreater<float>;
    funcNames["lle_X__Z13islessgreaterDv2_fS_"]   = lle_X_islessgreater<float, 2>;
    funcNames["lle_X__Z13islessgreaterDv3_fS_"]   = lle_X_islessgreater<float, 3>;
    funcNames["lle_X__Z13islessgreaterDv4_fS_"]   = lle_X_islessgreater<float, 4>;
    funcNames["lle_X__Z13islessgreaterDv8_fS_"]   = lle_X_islessgreater<float, 8>;
    funcNames["lle_X__Z13islessgreaterDv16_fS_"]  = lle_X_islessgreater<float, 16>;
    funcNames["lle_X__Z13islessgreaterdd"]        = lle_X_islessgreater<double>;
    funcNames["lle_X__Z13islessgreaterDv2_dS_"]   = lle_X_islessgreater<double, 2>;
    funcNames["lle_X__Z13islessgreaterDv3_dS_"]   = lle_X_islessgreater<double, 3>;
    funcNames["lle_X__Z13islessgreaterDv4_dS_"]   = lle_X_islessgreater<double, 4>;
    funcNames["lle_X__Z13islessgreaterDv8_dS_"]   = lle_X_islessgreater<double, 8>;
    funcNames["lle_X__Z13islessgreaterDv16_dS_"]  = lle_X_islessgreater<double, 16>;
    funcNames["lle_X__Z10isnotequalff"]           = lle_X_isnotequal<float>;
    funcNames["lle_X__Z10isnotequalDv2_fS_"]      = lle_X_isnotequal<float, 2>;
    funcNames["lle_X__Z10isnotequalDv3_fS_"]      = lle_X_isnotequal<float, 3>;
    funcNames["lle_X__Z10isnotequalDv4_fS_"]      = lle_X_isnotequal<float, 4>;
    funcNames["lle_X__Z10isnotequalDv8_fS_"]      = lle_X_isnotequal<float, 8>;
    funcNames["lle_X__Z10isnotequalDv16_fS_"]     = lle_X_isnotequal<float, 16>;
    funcNames["lle_X__Z10isnotequaldd"]           = lle_X_isnotequal<double>;
    funcNames["lle_X__Z10isnotequalDv2_dS_"]      = lle_X_isnotequal<double, 2>;
    funcNames["lle_X__Z10isnotequalDv3_dS_"]      = lle_X_isnotequal<double, 3>;
    funcNames["lle_X__Z10isnotequalDv4_dS_"]      = lle_X_isnotequal<double, 4>;
    funcNames["lle_X__Z10isnotequalDv8_dS_"]      = lle_X_isnotequal<double, 8>;
    funcNames["lle_X__Z10isnotequalDv16_dS_"]     = lle_X_isnotequal<double, 16>;
    funcNames["lle_X__Z9isorderedff"]             = lle_X_isordered<float>;
    funcNames["lle_X__Z9isorderedDv2_fS_"]        = lle_X_isordered<float, 2>;
    funcNames["lle_X__Z9isorderedDv3_fS_"]        = lle_X_isordered<float, 3>;
    funcNames["lle_X__Z9isorderedDv4_fS_"]        = lle_X_isordered<float, 4>;
    funcNames["lle_X__Z9isorderedDv8_fS_"]        = lle_X_isordered<float, 8>;
    funcNames["lle_X__Z9isorderedDv16_fS_"]       = lle_X_isordered<float, 16>;
    funcNames["lle_X__Z9isordereddd"]             = lle_X_isordered<double>;
    funcNames["lle_X__Z9isorderedDv2_dS_"]        = lle_X_isordered<double, 2>;
    funcNames["lle_X__Z9isorderedDv3_dS_"]        = lle_X_isordered<double, 3>;
    funcNames["lle_X__Z9isorderedDv4_dS_"]        = lle_X_isordered<double, 4>;
    funcNames["lle_X__Z9isorderedDv8_dS_"]        = lle_X_isordered<double, 8>;
    funcNames["lle_X__Z9isorderedDv16_dS_"]       = lle_X_isordered<double, 16>;
    funcNames["lle_X__Z11isunorderedff"]          = lle_X_isunordered<float>;
    funcNames["lle_X__Z11isunorderedDv2_fS_"]     = lle_X_isunordered<float, 2>;
    funcNames["lle_X__Z11isunorderedDv3_fS_"]     = lle_X_isunordered<float, 3>;
    funcNames["lle_X__Z11isunorderedDv4_fS_"]     = lle_X_isunordered<float, 4>;
    funcNames["lle_X__Z11isunorderedDv8_fS_"]     = lle_X_isunordered<float, 8>;
    funcNames["lle_X__Z11isunorderedDv16_fS_"]    = lle_X_isunordered<float, 16>;
    funcNames["lle_X__Z11isunordereddd"]          = lle_X_isunordered<double>;
    funcNames["lle_X__Z11isunorderedDv2_dS_"]     = lle_X_isunordered<double, 2>;
    funcNames["lle_X__Z11isunorderedDv3_dS_"]     = lle_X_isunordered<double, 3>;
    funcNames["lle_X__Z11isunorderedDv4_dS_"]     = lle_X_isunordered<double, 4>;
    funcNames["lle_X__Z11isunorderedDv8_dS_"]     = lle_X_isunordered<double, 8>;
    funcNames["lle_X__Z11isunorderedDv16_dS_"]    = lle_X_isunordered<double, 16>;
    funcNames["lle_X__Z8isfinitef"]               = lle_X_isfinite<float>;
    funcNames["lle_X__Z8isfiniteDv2_f"]           = lle_X_isfinite<float, 2>;
    funcNames["lle_X__Z8isfiniteDv3_f"]           = lle_X_isfinite<float, 3>;
    funcNames["lle_X__Z8isfiniteDv4_f"]           = lle_X_isfinite<float, 4>;
    funcNames["lle_X__Z8isfiniteDv8_f"]           = lle_X_isfinite<float, 8>;
    funcNames["lle_X__Z8isfiniteDv16_f"]          = lle_X_isfinite<float, 16>;
    funcNames["lle_X__Z8isfinited"]               = lle_X_isfinite<double>;
    funcNames["lle_X__Z8isfiniteDv2_d"]           = lle_X_isfinite<double, 2>;
    funcNames["lle_X__Z8isfiniteDv3_d"]           = lle_X_isfinite<double, 3>;
    funcNames["lle_X__Z8isfiniteDv4_d"]           = lle_X_isfinite<double, 4>;
    funcNames["lle_X__Z8isfiniteDv8_d"]           = lle_X_isfinite<double, 8>;
    funcNames["lle_X__Z8isfiniteDv16_d"]          = lle_X_isfinite<double, 16>;
    funcNames["lle_X__Z7signbitf"]                = lle_X_signbit<float>;
    funcNames["lle_X__Z7signbitDv2_f"]            = lle_X_signbit<float, 2>;
    funcNames["lle_X__Z7signbitDv3_f"]            = lle_X_signbit<float, 3>;
    funcNames["lle_X__Z7signbitDv4_f"]            = lle_X_signbit<float, 4>;
    funcNames["lle_X__Z7signbitDv8_f"]            = lle_X_signbit<float, 8>;
    funcNames["lle_X__Z7signbitDv16_f"]           = lle_X_signbit<float, 16>;
    funcNames["lle_X__Z7signbitd"]                = lle_X_signbit<double>;
    funcNames["lle_X__Z7signbitDv2_d"]            = lle_X_signbit<double, 2>;
    funcNames["lle_X__Z7signbitDv3_d"]            = lle_X_signbit<double, 3>;
    funcNames["lle_X__Z7signbitDv4_d"]            = lle_X_signbit<double, 4>;
    funcNames["lle_X__Z7signbitDv8_d"]            = lle_X_signbit<double, 8>;
    funcNames["lle_X__Z7signbitDv16_d"]           = lle_X_signbit<double, 16>;
    funcNames["lle_X__Z3anyc"]                    = lle_X_any<int8_t>;
    funcNames["lle_X__Z3anyDv2_c"]                = lle_X_any<int8_t, 2>;
    funcNames["lle_X__Z3anyDv3_c"]                = lle_X_any<int8_t, 3>;
    funcNames["lle_X__Z3anyDv4_c"]                = lle_X_any<int8_t, 4>;
    funcNames["lle_X__Z3anyDv8_c"]                = lle_X_any<int8_t, 8>;
    funcNames["lle_X__Z3anyDv16_c"]               = lle_X_any<int8_t, 16>;
    funcNames["lle_X__Z3anys"]                    = lle_X_any<int16_t>;
    funcNames["lle_X__Z3anyDv2_s"]                = lle_X_any<int16_t, 2>;
    funcNames["lle_X__Z3anyDv3_s"]                = lle_X_any<int16_t, 3>;
    funcNames["lle_X__Z3anyDv4_s"]                = lle_X_any<int16_t, 4>;
    funcNames["lle_X__Z3anyDv8_s"]                = lle_X_any<int16_t, 8>;
    funcNames["lle_X__Z3anyDv16_s"]               = lle_X_any<int16_t, 16>;
    funcNames["lle_X__Z3anyi"]                    = lle_X_any<int32_t>;
    funcNames["lle_X__Z3anyDv2_i"]                = lle_X_any<int32_t, 2>;
    funcNames["lle_X__Z3anyDv3_i"]                = lle_X_any<int32_t, 3>;
    funcNames["lle_X__Z3anyDv4_i"]                = lle_X_any<int32_t, 4>;
    funcNames["lle_X__Z3anyDv8_i"]                = lle_X_any<int32_t, 8>;
    funcNames["lle_X__Z3anyDv16_i"]               = lle_X_any<int32_t, 16>;
    funcNames["lle_X__Z3anyl"]                    = lle_X_any<int64_t>;
    funcNames["lle_X__Z3anyDv2_l"]                = lle_X_any<int64_t, 2>;
    funcNames["lle_X__Z3anyDv3_l"]                = lle_X_any<int64_t, 3>;
    funcNames["lle_X__Z3anyDv4_l"]                = lle_X_any<int64_t, 4>;
    funcNames["lle_X__Z3anyDv8_l"]                = lle_X_any<int64_t, 8>;
    funcNames["lle_X__Z3anyDv16_l"]               = lle_X_any<int64_t, 16>;
    funcNames["lle_X__Z3allc"]                    = lle_X_all<int8_t>;
    funcNames["lle_X__Z3allDv2_c"]                = lle_X_all<int8_t, 2>;
    funcNames["lle_X__Z3allDv3_c"]                = lle_X_all<int8_t, 3>;
    funcNames["lle_X__Z3allDv4_c"]                = lle_X_all<int8_t, 4>;
    funcNames["lle_X__Z3allDv8_c"]                = lle_X_all<int8_t, 8>;
    funcNames["lle_X__Z3allDv16_c"]               = lle_X_all<int8_t, 16>;
    funcNames["lle_X__Z3alls"]                    = lle_X_all<int16_t>;
    funcNames["lle_X__Z3allDv2_s"]                = lle_X_all<int16_t, 2>;
    funcNames["lle_X__Z3allDv3_s"]                = lle_X_all<int16_t, 3>;
    funcNames["lle_X__Z3allDv4_s"]                = lle_X_all<int16_t, 4>;
    funcNames["lle_X__Z3allDv8_s"]                = lle_X_all<int16_t, 8>;
    funcNames["lle_X__Z3allDv16_s"]               = lle_X_all<int16_t, 16>;
    funcNames["lle_X__Z3alli"]                    = lle_X_all<int32_t>;
    funcNames["lle_X__Z3allDv2_i"]                = lle_X_all<int32_t, 2>;
    funcNames["lle_X__Z3allDv3_i"]                = lle_X_all<int32_t, 3>;
    funcNames["lle_X__Z3allDv4_i"]                = lle_X_all<int32_t, 4>;
    funcNames["lle_X__Z3allDv8_i"]                = lle_X_all<int32_t, 8>;
    funcNames["lle_X__Z3allDv16_i"]               = lle_X_all<int32_t, 16>;
    funcNames["lle_X__Z3alll"]                    = lle_X_all<int64_t>;
    funcNames["lle_X__Z3allDv2_l"]                = lle_X_all<int64_t, 2>;
    funcNames["lle_X__Z3allDv3_l"]                = lle_X_all<int64_t, 3>;
    funcNames["lle_X__Z3allDv4_l"]                = lle_X_all<int64_t, 4>;
    funcNames["lle_X__Z3allDv8_l"]                = lle_X_all<int64_t, 8>;
    funcNames["lle_X__Z3allDv16_l"]               = lle_X_all<int64_t, 16>;
    funcNames["lle_X__Z9bitselectccc"]            = lle_X_bitselect<int8_t>;
    funcNames["lle_X__Z9bitselecthhh"]            = lle_X_bitselect<uint8_t>;
    funcNames["lle_X__Z9bitselectDv2_cS_S_"]      = lle_X_bitselect<int8_t,  2>;
    funcNames["lle_X__Z9bitselectDv2_hS_S_"]      = lle_X_bitselect<uint8_t, 2>;
    funcNames["lle_X__Z9bitselectDv3_cS_S_"]      = lle_X_bitselect<int8_t,  3>;
    funcNames["lle_X__Z9bitselectDv3_hS_S_"]      = lle_X_bitselect<uint8_t, 3>;
    funcNames["lle_X__Z9bitselectDv4_cS_S_"]      = lle_X_bitselect<int8_t,  4>;
    funcNames["lle_X__Z9bitselectDv4_hS_S_"]      = lle_X_bitselect<uint8_t, 4>;
    funcNames["lle_X__Z9bitselectDv8_cS_S_"]      = lle_X_bitselect<int8_t,  8>;
    funcNames["lle_X__Z9bitselectDv8_hS_S_"]      = lle_X_bitselect<uint8_t, 8>;
    funcNames["lle_X__Z9bitselectDv16_cS_S_"]     = lle_X_bitselect<int8_t,  16>;
    funcNames["lle_X__Z9bitselectDv16_hS_S_"]     = lle_X_bitselect<uint8_t, 16>;
    funcNames["lle_X__Z9bitselectsss"]            = lle_X_bitselect<int16_t>;
    funcNames["lle_X__Z9bitselectttt"]            = lle_X_bitselect<uint16_t>;
    funcNames["lle_X__Z9bitselectDv2_sS_S_"]      = lle_X_bitselect<int16_t,  2>;
    funcNames["lle_X__Z9bitselectDv2_tS_S_"]      = lle_X_bitselect<uint16_t, 2>;
    funcNames["lle_X__Z9bitselectDv3_sS_S_"]      = lle_X_bitselect<int16_t,  3>;
    funcNames["lle_X__Z9bitselectDv3_tS_S_"]      = lle_X_bitselect<uint16_t, 3>;
    funcNames["lle_X__Z9bitselectDv4_sS_S_"]      = lle_X_bitselect<int16_t,  4>;
    funcNames["lle_X__Z9bitselectDv4_tS_S_"]      = lle_X_bitselect<uint16_t, 4>;
    funcNames["lle_X__Z9bitselectDv8_sS_S_"]      = lle_X_bitselect<int16_t,  8>;
    funcNames["lle_X__Z9bitselectDv8_tS_S_"]      = lle_X_bitselect<uint16_t, 8>;
    funcNames["lle_X__Z9bitselectDv16_sS_S_"]     = lle_X_bitselect<int16_t,  16>;
    funcNames["lle_X__Z9bitselectDv16_tS_S_"]     = lle_X_bitselect<uint16_t, 16>;
    funcNames["lle_X__Z9bitselectiii"]            = lle_X_bitselect<int32_t>;
    funcNames["lle_X__Z9bitselectjjj"]            = lle_X_bitselect<uint32_t>;
    funcNames["lle_X__Z9bitselectDv2_iS_S_"]      = lle_X_bitselect<int32_t,  2>;
    funcNames["lle_X__Z9bitselectDv2_jS_S_"]      = lle_X_bitselect<uint32_t, 2>;
    funcNames["lle_X__Z9bitselectDv3_iS_S_"]      = lle_X_bitselect<int32_t,  3>;
    funcNames["lle_X__Z9bitselectDv3_jS_S_"]      = lle_X_bitselect<uint32_t, 3>;
    funcNames["lle_X__Z9bitselectDv4_iS_S_"]      = lle_X_bitselect<int32_t,  4>;
    funcNames["lle_X__Z9bitselectDv4_jS_S_"]      = lle_X_bitselect<uint32_t, 4>;
    funcNames["lle_X__Z9bitselectDv8_iS_S_"]      = lle_X_bitselect<int32_t,  8>;
    funcNames["lle_X__Z9bitselectDv8_jS_S_"]      = lle_X_bitselect<uint32_t, 8>;
    funcNames["lle_X__Z9bitselectDv16_iS_S_"]     = lle_X_bitselect<int32_t,  16>;
    funcNames["lle_X__Z9bitselectDv16_jS_S_"]     = lle_X_bitselect<uint32_t, 16>;
    funcNames["lle_X__Z9bitselectlll"]            = lle_X_bitselect<int64_t>;
    funcNames["lle_X__Z9bitselectmmm"]            = lle_X_bitselect<uint64_t>;
    funcNames["lle_X__Z9bitselectDv2_lS_S_"]      = lle_X_bitselect<int64_t,  2>;
    funcNames["lle_X__Z9bitselectDv2_mS_S_"]      = lle_X_bitselect<uint64_t, 2>;
    funcNames["lle_X__Z9bitselectDv3_lS_S_"]      = lle_X_bitselect<int64_t,  3>;
    funcNames["lle_X__Z9bitselectDv3_mS_S_"]      = lle_X_bitselect<uint64_t, 3>;
    funcNames["lle_X__Z9bitselectDv4_lS_S_"]      = lle_X_bitselect<int64_t,  4>;
    funcNames["lle_X__Z9bitselectDv4_mS_S_"]      = lle_X_bitselect<uint64_t, 4>;
    funcNames["lle_X__Z9bitselectDv8_lS_S_"]      = lle_X_bitselect<int64_t,  8>;
    funcNames["lle_X__Z9bitselectDv8_mS_S_"]      = lle_X_bitselect<uint64_t, 8>;
    funcNames["lle_X__Z9bitselectDv16_lS_S_"]     = lle_X_bitselect<int64_t,  16>;
    funcNames["lle_X__Z9bitselectDv16_mS_S_"]     = lle_X_bitselect<uint64_t, 16>;
    funcNames["lle_X__Z9bitselectfff"]            = lle_X_bitselect<float>;
    funcNames["lle_X__Z9bitselectDv2_fS_S_"]      = lle_X_bitselect<float, 2>;
    funcNames["lle_X__Z9bitselectDv3_fS_S_"]      = lle_X_bitselect<float, 3>;
    funcNames["lle_X__Z9bitselectDv4_fS_S_"]      = lle_X_bitselect<float, 4>;
    funcNames["lle_X__Z9bitselectDv8_fS_S_"]      = lle_X_bitselect<float, 8>;
    funcNames["lle_X__Z9bitselectDv16_fS_S_"]     = lle_X_bitselect<float, 16>;
    funcNames["lle_X__Z9bitselectddd"]            = lle_X_bitselect<double>;
    funcNames["lle_X__Z9bitselectDv2_dS_S_"]      = lle_X_bitselect<double, 2>;
    funcNames["lle_X__Z9bitselectDv3_dS_S_"]      = lle_X_bitselect<double, 3>;
    funcNames["lle_X__Z9bitselectDv4_dS_S_"]      = lle_X_bitselect<double, 4>;
    funcNames["lle_X__Z9bitselectDv8_dS_S_"]      = lle_X_bitselect<double, 8>;
    funcNames["lle_X__Z9bitselectDv16_dS_S_"]     = lle_X_bitselect<double, 16>;

    funcNames["lle_X__Z6selectccc"]               = lle_X_select<int8_t,   int8_t>;
    funcNames["lle_X__Z6selecthhc"]               = lle_X_select<uint8_t,  uint8_t>;
    funcNames["lle_X__Z6selectDv2_cS_S_"]         = lle_X_select<int8_t,   int8_t, 2>;
    funcNames["lle_X__Z6selectDv2_hS_Dv2_c"]      = lle_X_select<uint8_t,  int8_t, 2>;
    funcNames["lle_X__Z6selectDv3_cS_S_"]         = lle_X_select<int8_t,   int8_t, 3>;
    funcNames["lle_X__Z6selectDv3_hS_Dv3_c"]      = lle_X_select<uint8_t,  int8_t, 3>;
    funcNames["lle_X__Z6selectDv4_cS_S_"]         = lle_X_select<int8_t,   int8_t, 4>;
    funcNames["lle_X__Z6selectDv4_hS_Dv4_c"]      = lle_X_select<uint8_t,  int8_t, 4>;
    funcNames["lle_X__Z6selectDv8_cS_S_"]         = lle_X_select<int8_t,   int8_t, 8>;
    funcNames["lle_X__Z6selectDv8_hS_Dv8_c"]      = lle_X_select<uint8_t,  int8_t, 8>;
    funcNames["lle_X__Z6selectDv16_cS_S_"]        = lle_X_select<int8_t,   int8_t, 16>;
    funcNames["lle_X__Z6selectDv16_hS_Dv16_c"]    = lle_X_select<uint8_t,  int8_t, 16>;

    funcNames["lle_X__Z6selectssc"]               = lle_X_select<int16_t,  int8_t>;
    funcNames["lle_X__Z6selectttc"]               = lle_X_select<uint16_t, int8_t>;
    funcNames["lle_X__Z6selectDv2_sS_Dv2_c"]      = lle_X_select<int16_t,  int8_t, 2>;
    funcNames["lle_X__Z6selectDv2_tS_Dv2_c"]      = lle_X_select<uint16_t, int8_t, 2>;
    funcNames["lle_X__Z6selectDv3_sS_Dv3_c"]      = lle_X_select<int16_t,  int8_t, 3>;
    funcNames["lle_X__Z6selectDv3_tS_Dv3_c"]      = lle_X_select<uint16_t, int8_t, 3>;
    funcNames["lle_X__Z6selectDv4_sS_Dv4_c"]      = lle_X_select<int16_t,  int8_t, 4>;
    funcNames["lle_X__Z6selectDv4_tS_Dv4_c"]      = lle_X_select<uint16_t, int8_t, 4>;
    funcNames["lle_X__Z6selectDv8_sS_Dv8_c"]      = lle_X_select<int16_t,  int8_t, 8>;
    funcNames["lle_X__Z6selectDv8_tS_Dv8_c"]      = lle_X_select<uint16_t, int8_t, 8>;
    funcNames["lle_X__Z6selectDv16_sS_Dv16_c"]    = lle_X_select<int16_t,  int8_t, 16>;
    funcNames["lle_X__Z6selectDv16_tS_Dv16_c"]    = lle_X_select<uint16_t, int8_t, 16>;

    funcNames["lle_X__Z6selectiic"]               = lle_X_select<int32_t,  int8_t>;
    funcNames["lle_X__Z6selectjjc"]               = lle_X_select<uint32_t, int8_t>;
    funcNames["lle_X__Z6selectDv2_iS_Dv2_c"]      = lle_X_select<int32_t,  int8_t, 2>;
    funcNames["lle_X__Z6selectDv2_jS_Dv2_c"]      = lle_X_select<uint32_t, int8_t, 2>;
    funcNames["lle_X__Z6selectDv3_iS_Dv3_c"]      = lle_X_select<int32_t,  int8_t, 3>;
    funcNames["lle_X__Z6selectDv3_jS_Dv3_c"]      = lle_X_select<uint32_t, int8_t, 3>;
    funcNames["lle_X__Z6selectDv4_iS_Dv4_c"]      = lle_X_select<int32_t,  int8_t, 4>;
    funcNames["lle_X__Z6selectDv4_jS_Dv4_c"]      = lle_X_select<uint32_t, int8_t, 4>;
    funcNames["lle_X__Z6selectDv8_iS_Dv8_c"]      = lle_X_select<int32_t,  int8_t, 8>;
    funcNames["lle_X__Z6selectDv8_jS_Dv8_c"]      = lle_X_select<uint32_t, int8_t, 8>;
    funcNames["lle_X__Z6selectDv16_iS_Dv16_c"]    = lle_X_select<int32_t,  int8_t, 16>;
    funcNames["lle_X__Z6selectDv16_jS_Dv16_c"]    = lle_X_select<uint32_t, int8_t, 16>;

    funcNames["lle_X__Z6selectllc"]               = lle_X_select<int64_t,  int8_t>;
    funcNames["lle_X__Z6selectmmc"]               = lle_X_select<uint64_t, int8_t>;
    funcNames["lle_X__Z6selectDv2_lS_Dv2_c"]      = lle_X_select<int64_t,  int8_t, 2>;
    funcNames["lle_X__Z6selectDv2_mS_Dv2_c"]      = lle_X_select<uint64_t, int8_t, 2>;
    funcNames["lle_X__Z6selectDv3_lS_Dv3_c"]      = lle_X_select<int64_t,  int8_t, 3>;
    funcNames["lle_X__Z6selectDv3_mS_Dv3_c"]      = lle_X_select<uint64_t, int8_t, 3>;
    funcNames["lle_X__Z6selectDv4_lS_Dv4_c"]      = lle_X_select<int64_t,  int8_t, 4>;
    funcNames["lle_X__Z6selectDv4_mS_Dv4_c"]      = lle_X_select<uint64_t, int8_t, 4>;
    funcNames["lle_X__Z6selectDv8_lS_Dv8_c"]      = lle_X_select<int64_t,  int8_t, 8>;
    funcNames["lle_X__Z6selectDv8_mS_Dv8_c"]      = lle_X_select<uint64_t, int8_t, 8>;
    funcNames["lle_X__Z6selectDv16_lS_Dv16_c"]    = lle_X_select<int64_t,  int8_t, 16>;
    funcNames["lle_X__Z6selectDv16_mS_Dv16_c"]    = lle_X_select<uint64_t, int8_t, 16>;

    funcNames["lle_X__Z6selectffc"]               = lle_X_select<float, int8_t>;
    funcNames["lle_X__Z6selectDv2_fS_Dv2_c"]      = lle_X_select<float, int8_t, 2>;
    funcNames["lle_X__Z6selectDv3_fS_Dv3_c"]      = lle_X_select<float, int8_t, 3>;
    funcNames["lle_X__Z6selectDv4_fS_Dv4_c"]      = lle_X_select<float, int8_t, 4>;
    funcNames["lle_X__Z6selectDv8_fS_Dv8_c"]      = lle_X_select<float, int8_t, 8>;
    funcNames["lle_X__Z6selectDv16_fS_Dv16_c"]    = lle_X_select<float, int8_t, 16>;

    funcNames["lle_X__Z6selectccs"]               = lle_X_select<int8_t,  int16_t>;
    funcNames["lle_X__Z6selecthhs"]               = lle_X_select<uint8_t, int16_t>;
    funcNames["lle_X__Z6selectDv2_cS_Dv2_s"]      = lle_X_select<int8_t,  int16_t, 2>;
    funcNames["lle_X__Z6selectDv2_hS_Dv2_s"]      = lle_X_select<uint8_t, int16_t, 2>;
    funcNames["lle_X__Z6selectDv3_cS_Dv3_s"]      = lle_X_select<int8_t,  int16_t, 3>;
    funcNames["lle_X__Z6selectDv3_hS_Dv3_s"]      = lle_X_select<uint8_t, int16_t, 3>;
    funcNames["lle_X__Z6selectDv4_cS_Dv4_s"]      = lle_X_select<int8_t,  int16_t, 4>;
    funcNames["lle_X__Z6selectDv4_hS_Dv4_s"]      = lle_X_select<uint8_t, int16_t, 4>;
    funcNames["lle_X__Z6selectDv8_cS_Dv8_s"]      = lle_X_select<int8_t,  int16_t, 8>;
    funcNames["lle_X__Z6selectDv8_hS_Dv8_s"]      = lle_X_select<uint8_t, int16_t, 8>;
    funcNames["lle_X__Z6selectDv16_cS_Dv16_s"]    = lle_X_select<int8_t,  int16_t, 16>;
    funcNames["lle_X__Z6selectDv16_hS_Dv16_s"]    = lle_X_select<uint8_t, int16_t, 16>;

    funcNames["lle_X__Z6selectsss"]               = lle_X_select<int16_t,  int16_t>;
    funcNames["lle_X__Z6selecttts"]               = lle_X_select<uint16_t, int16_t>;
    funcNames["lle_X__Z6selectDv2_sS_S_"]         = lle_X_select<int16_t,  int16_t, 2>;
    funcNames["lle_X__Z6selectDv2_tS_Dv2_s"]      = lle_X_select<uint16_t, int16_t, 2>;
    funcNames["lle_X__Z6selectDv3_sS_S_"]         = lle_X_select<int16_t,  int16_t, 3>;
    funcNames["lle_X__Z6selectDv3_tS_Dv3_s"]      = lle_X_select<uint16_t, int16_t, 3>;
    funcNames["lle_X__Z6selectDv4_sS_S_"]         = lle_X_select<int16_t,  int16_t, 4>;
    funcNames["lle_X__Z6selectDv4_tS_Dv4_s"]      = lle_X_select<uint16_t, int16_t, 4>;
    funcNames["lle_X__Z6selectDv8_sS_S_"]         = lle_X_select<int16_t,  int16_t, 8>;
    funcNames["lle_X__Z6selectDv8_tS_Dv8_s"]      = lle_X_select<uint16_t, int16_t, 8>;
    funcNames["lle_X__Z6selectDv16_sS_S_"]        = lle_X_select<int16_t,  int16_t, 16>;
    funcNames["lle_X__Z6selectDv16_tS_Dv16_s"]    = lle_X_select<uint16_t, int16_t, 16>;

    funcNames["lle_X__Z6selectiis"]               = lle_X_select<int32_t,  int16_t>;
    funcNames["lle_X__Z6selectjjs"]               = lle_X_select<uint32_t, int16_t>;
    funcNames["lle_X__Z6selectDv2_iS_Dv2_s"]      = lle_X_select<int32_t,  int16_t, 2>;
    funcNames["lle_X__Z6selectDv2_jS_Dv2_s"]      = lle_X_select<uint32_t, int16_t, 2>;
    funcNames["lle_X__Z6selectDv3_iS_Dv3_s"]      = lle_X_select<int32_t,  int16_t, 3>;
    funcNames["lle_X__Z6selectDv3_jS_Dv3_s"]      = lle_X_select<uint32_t, int16_t, 3>;
    funcNames["lle_X__Z6selectDv4_iS_Dv4_s"]      = lle_X_select<int32_t,  int16_t, 4>;
    funcNames["lle_X__Z6selectDv4_jS_Dv4_s"]      = lle_X_select<uint32_t, int16_t, 4>;
    funcNames["lle_X__Z6selectDv8_iS_Dv8_s"]      = lle_X_select<int32_t,  int16_t, 8>;
    funcNames["lle_X__Z6selectDv8_jS_Dv8_s"]      = lle_X_select<uint32_t, int16_t, 8>;
    funcNames["lle_X__Z6selectDv16_iS_Dv16_s"]    = lle_X_select<int32_t,  int16_t, 16>;
    funcNames["lle_X__Z6selectDv16_jS_Dv16_s"]    = lle_X_select<uint32_t, int16_t, 16>;

    funcNames["lle_X__Z6selectlls"]               = lle_X_select<int64_t,  int16_t>;
    funcNames["lle_X__Z6selectmms"]               = lle_X_select<uint64_t, int16_t>;
    funcNames["lle_X__Z6selectDv2_lS_Dv2_s"]      = lle_X_select<int64_t,  int16_t, 2>;
    funcNames["lle_X__Z6selectDv2_mS_Dv2_s"]      = lle_X_select<uint64_t, int16_t, 2>;
    funcNames["lle_X__Z6selectDv3_lS_Dv3_s"]      = lle_X_select<int64_t,  int16_t, 3>;
    funcNames["lle_X__Z6selectDv3_mS_Dv3_s"]      = lle_X_select<uint64_t, int16_t, 3>;
    funcNames["lle_X__Z6selectDv4_lS_Dv4_s"]      = lle_X_select<int64_t,  int16_t, 4>;
    funcNames["lle_X__Z6selectDv4_mS_Dv4_s"]      = lle_X_select<uint64_t, int16_t, 4>;
    funcNames["lle_X__Z6selectDv8_lS_Dv8_s"]      = lle_X_select<int64_t,  int16_t, 8>;
    funcNames["lle_X__Z6selectDv8_mS_Dv8_s"]      = lle_X_select<uint64_t, int16_t, 8>;
    funcNames["lle_X__Z6selectDv16_lS_Dv16_s"]    = lle_X_select<int64_t,  int16_t, 16>;
    funcNames["lle_X__Z6selectDv16_mS_Dv16_s"]    = lle_X_select<uint64_t, int16_t, 16>;

    funcNames["lle_X__Z6selectffs"]               = lle_X_select<float, int16_t>;
    funcNames["lle_X__Z6selectDv2_fS_Dv2_c"]      = lle_X_select<float, int8_t,  2>;
    funcNames["lle_X__Z6selectDv2_fS_Dv2_s"]      = lle_X_select<float, int16_t, 2>;
    funcNames["lle_X__Z6selectDv3_fS_Dv3_s"]      = lle_X_select<float, int16_t, 3>;
    funcNames["lle_X__Z6selectDv4_fS_Dv4_s"]      = lle_X_select<float, int16_t, 4>;
    funcNames["lle_X__Z6selectDv8_fS_Dv8_s"]      = lle_X_select<float, int16_t, 8>;
    funcNames["lle_X__Z6selectDv16_fS_Dv16_s"]    = lle_X_select<float, int16_t, 16>;

    funcNames["lle_X__Z6selectcci"]               = lle_X_select<int8_t,  int32_t>;
    funcNames["lle_X__Z6selecthhi"]               = lle_X_select<uint8_t, int32_t>;
    funcNames["lle_X__Z6selectDv2_cS_Dv2_i"]      = lle_X_select<int8_t,  int32_t, 2>;
    funcNames["lle_X__Z6selectDv2_hS_Dv2_i"]      = lle_X_select<uint8_t, int32_t, 2>;
    funcNames["lle_X__Z6selectDv3_cS_Dv3_i"]      = lle_X_select<int8_t,  int32_t, 3>;
    funcNames["lle_X__Z6selectDv3_hS_Dv3_i"]      = lle_X_select<uint8_t, int32_t, 3>;
    funcNames["lle_X__Z6selectDv4_cS_Dv4_i"]      = lle_X_select<int8_t,  int32_t, 4>;
    funcNames["lle_X__Z6selectDv4_hS_Dv4_i"]      = lle_X_select<uint8_t, int32_t, 4>;
    funcNames["lle_X__Z6selectDv8_cS_Dv8_i"]      = lle_X_select<int8_t,  int32_t, 8>;
    funcNames["lle_X__Z6selectDv8_hS_Dv8_i"]      = lle_X_select<uint8_t, int32_t, 8>;
    funcNames["lle_X__Z6selectDv16_cS_Dv16_i"]    = lle_X_select<int8_t,  int32_t, 16>;
    funcNames["lle_X__Z6selectDv16_hS_Dv16_i"]    = lle_X_select<uint8_t, int32_t, 16>;

    funcNames["lle_X__Z6selectssi"]               = lle_X_select<int16_t,  int32_t>;
    funcNames["lle_X__Z6selecttti"]               = lle_X_select<uint16_t, int32_t>;
    funcNames["lle_X__Z6selectDv2_sS_Dv2_i"]      = lle_X_select<int16_t,  int32_t, 2>;
    funcNames["lle_X__Z6selectDv2_tS_Dv2_i"]      = lle_X_select<uint16_t, int32_t, 2>;
    funcNames["lle_X__Z6selectDv3_sS_Dv3_i"]      = lle_X_select<int16_t,  int32_t, 3>;
    funcNames["lle_X__Z6selectDv3_tS_Dv3_i"]      = lle_X_select<uint16_t, int32_t, 3>;
    funcNames["lle_X__Z6selectDv4_sS_Dv4_i"]      = lle_X_select<int16_t,  int32_t, 4>;
    funcNames["lle_X__Z6selectDv4_tS_Dv4_i"]      = lle_X_select<uint16_t, int32_t, 4>;
    funcNames["lle_X__Z6selectDv8_sS_Dv8_i"]      = lle_X_select<int16_t,  int32_t, 8>;
    funcNames["lle_X__Z6selectDv8_tS_Dv8_i"]      = lle_X_select<uint16_t, int32_t, 8>;
    funcNames["lle_X__Z6selectDv16_sS_Dv16_i"]    = lle_X_select<int16_t,  int32_t, 16>;
    funcNames["lle_X__Z6selectDv16_tS_Dv16_i"]    = lle_X_select<uint16_t, int32_t, 16>;

    funcNames["lle_X__Z6selectiii"]               = lle_X_select<int32_t,  int32_t>;
    funcNames["lle_X__Z6selectjji"]               = lle_X_select<uint32_t, int32_t>;
    funcNames["lle_X__Z6selectDv2_iS_S_"]         = lle_X_select<int32_t,  int32_t, 2>;
    funcNames["lle_X__Z6selectDv2_jS_Dv2_i"]      = lle_X_select<uint32_t, int32_t, 2>;
    funcNames["lle_X__Z6selectDv3_iS_S_"]         = lle_X_select<int32_t,  int32_t, 3>;
    funcNames["lle_X__Z6selectDv3_jS_Dv3_i"]      = lle_X_select<uint32_t, int32_t, 3>;
    funcNames["lle_X__Z6selectDv4_iS_S_"]         = lle_X_select<int32_t,  int32_t, 4>;
    funcNames["lle_X__Z6selectDv4_jS_Dv4_i"]      = lle_X_select<uint32_t, int32_t, 4>;
    funcNames["lle_X__Z6selectDv8_iS_S_"]         = lle_X_select<int32_t,  int32_t, 8>;
    funcNames["lle_X__Z6selectDv8_jS_Dv8_i"]      = lle_X_select<uint32_t, int32_t, 8>;
    funcNames["lle_X__Z6selectDv16_iS_S_"]        = lle_X_select<int32_t,  int32_t, 16>;
    funcNames["lle_X__Z6selectDv16_jS_Dv16_i"]    = lle_X_select<uint32_t, int32_t, 16>;

    funcNames["lle_X__Z6selectlli"]               = lle_X_select<int64_t,  int32_t>;
    funcNames["lle_X__Z6selectmmi"]               = lle_X_select<uint64_t, int32_t>;
    funcNames["lle_X__Z6selectDv2_lS_Dv2_i"]      = lle_X_select<int64_t,  int32_t, 2>;
    funcNames["lle_X__Z6selectDv2_mS_Dv2_i"]      = lle_X_select<uint64_t, int32_t, 2>;
    funcNames["lle_X__Z6selectDv3_lS_Dv3_i"]      = lle_X_select<int64_t,  int32_t, 3>;
    funcNames["lle_X__Z6selectDv3_mS_Dv3_i"]      = lle_X_select<uint64_t, int32_t, 3>;
    funcNames["lle_X__Z6selectDv4_lS_Dv4_i"]      = lle_X_select<int64_t,  int32_t, 4>;
    funcNames["lle_X__Z6selectDv4_mS_Dv4_i"]      = lle_X_select<uint64_t, int32_t, 4>;
    funcNames["lle_X__Z6selectDv8_lS_Dv8_i"]      = lle_X_select<int64_t,  int32_t, 8>;
    funcNames["lle_X__Z6selectDv8_mS_Dv8_i"]      = lle_X_select<uint64_t, int32_t, 8>;
    funcNames["lle_X__Z6selectDv16_lS_Dv16_i"]    = lle_X_select<int64_t,  int32_t, 16>;
    funcNames["lle_X__Z6selectDv16_mS_Dv16_i"]    = lle_X_select<uint64_t, int32_t, 16>;

    funcNames["lle_X__Z6selectffi"]               = lle_X_select<float, int32_t>;
    funcNames["lle_X__Z6selectDv2_fS_Dv2_i"]      = lle_X_select<float, int32_t, 2>;
    funcNames["lle_X__Z6selectDv3_fS_Dv3_i"]      = lle_X_select<float, int32_t, 3>;
    funcNames["lle_X__Z6selectDv4_fS_Dv4_i"]      = lle_X_select<float, int32_t, 4>;
    funcNames["lle_X__Z6selectDv8_fS_Dv8_i"]      = lle_X_select<float, int32_t, 8>;
    funcNames["lle_X__Z6selectDv16_fS_Dv16_i"]    = lle_X_select<float, int32_t, 16>;

    funcNames["lle_X__Z6selectccl"]               = lle_X_select<int8_t,  int64_t>;
    funcNames["lle_X__Z6selecthhl"]               = lle_X_select<uint8_t, int64_t>;
    funcNames["lle_X__Z6selectDv2_cS_Dv2_l"]      = lle_X_select<int8_t,  int64_t, 2>;
    funcNames["lle_X__Z6selectDv2_hS_Dv2_l"]      = lle_X_select<uint8_t, int64_t, 2>;
    funcNames["lle_X__Z6selectDv3_cS_Dv3_l"]      = lle_X_select<int8_t,  int64_t, 3>;
    funcNames["lle_X__Z6selectDv3_hS_Dv3_l"]      = lle_X_select<uint8_t, int64_t, 3>;
    funcNames["lle_X__Z6selectDv4_cS_Dv4_l"]      = lle_X_select<int8_t,  int64_t, 4>;
    funcNames["lle_X__Z6selectDv4_hS_Dv4_l"]      = lle_X_select<uint8_t, int64_t, 4>;
    funcNames["lle_X__Z6selectDv8_cS_Dv8_l"]      = lle_X_select<int8_t,  int64_t, 8>;
    funcNames["lle_X__Z6selectDv8_hS_Dv8_l"]      = lle_X_select<uint8_t, int64_t, 8>;
    funcNames["lle_X__Z6selectDv16_cS_Dv16_l"]    = lle_X_select<int8_t,  int64_t, 16>;
    funcNames["lle_X__Z6selectDv16_hS_Dv16_l"]    = lle_X_select<uint8_t, int64_t, 16>;

    funcNames["lle_X__Z6selectssl"]               = lle_X_select<int16_t,  int64_t>;
    funcNames["lle_X__Z6selectttl"]               = lle_X_select<uint16_t, int64_t>;
    funcNames["lle_X__Z6selectDv2_sS_Dv2_l"]      = lle_X_select<int16_t,  int64_t, 2>;
    funcNames["lle_X__Z6selectDv2_tS_Dv2_l"]      = lle_X_select<uint16_t, int64_t, 2>;
    funcNames["lle_X__Z6selectDv3_sS_Dv3_l"]      = lle_X_select<int16_t,  int64_t, 3>;
    funcNames["lle_X__Z6selectDv3_tS_Dv3_l"]      = lle_X_select<uint16_t, int64_t, 3>;
    funcNames["lle_X__Z6selectDv4_sS_Dv4_l"]      = lle_X_select<int16_t,  int64_t, 4>;
    funcNames["lle_X__Z6selectDv4_tS_Dv4_l"]      = lle_X_select<uint16_t, int64_t, 4>;
    funcNames["lle_X__Z6selectDv8_sS_Dv8_l"]      = lle_X_select<int16_t,  int64_t, 8>;
    funcNames["lle_X__Z6selectDv8_tS_Dv8_l"]      = lle_X_select<uint16_t, int64_t, 8>;
    funcNames["lle_X__Z6selectDv16_sS_Dv16_l"]    = lle_X_select<int16_t,  int64_t, 16>;
    funcNames["lle_X__Z6selectDv16_tS_Dv16_l"]    = lle_X_select<uint16_t, int64_t, 16>;

    funcNames["lle_X__Z6selectiil"]               = lle_X_select<int32_t,  int64_t>;
    funcNames["lle_X__Z6selectjjl"]               = lle_X_select<uint32_t, int64_t>;
    funcNames["lle_X__Z6selectDv2_iS_Dv2_l"]      = lle_X_select<int32_t,  int64_t, 2>;
    funcNames["lle_X__Z6selectDv2_jS_Dv2_l"]      = lle_X_select<uint32_t, int64_t, 2>;
    funcNames["lle_X__Z6selectDv3_iS_Dv3_l"]      = lle_X_select<int32_t,  int64_t, 3>;
    funcNames["lle_X__Z6selectDv3_jS_Dv3_l"]      = lle_X_select<uint32_t, int64_t, 3>;
    funcNames["lle_X__Z6selectDv4_iS_Dv4_l"]      = lle_X_select<int32_t,  int64_t, 4>;
    funcNames["lle_X__Z6selectDv4_jS_Dv4_l"]      = lle_X_select<uint32_t, int64_t, 4>;
    funcNames["lle_X__Z6selectDv8_iS_Dv8_l"]      = lle_X_select<int32_t,  int64_t, 8>;
    funcNames["lle_X__Z6selectDv8_jS_Dv8_l"]      = lle_X_select<uint32_t, int64_t, 8>;
    funcNames["lle_X__Z6selectDv16_iS_Dv16_l"]    = lle_X_select<int32_t,  int64_t, 16>;
    funcNames["lle_X__Z6selectDv16_jS_Dv16_l"]    = lle_X_select<uint32_t, int64_t, 16>;

    funcNames["lle_X__Z6selectlll"]               = lle_X_select<int64_t,  int64_t>;
    funcNames["lle_X__Z6selectmml"]               = lle_X_select<uint64_t, int64_t>;
    funcNames["lle_X__Z6selectDv2_lS_S_"]         = lle_X_select<int64_t,  int64_t, 2>;
    funcNames["lle_X__Z6selectDv2_mS_Dv2_l"]      = lle_X_select<uint64_t, int64_t, 2>;
    funcNames["lle_X__Z6selectDv3_lS_S_"]         = lle_X_select<int64_t,  int64_t, 3>;
    funcNames["lle_X__Z6selectDv3_mS_Dv3_l"]      = lle_X_select<uint64_t, int64_t, 3>;
    funcNames["lle_X__Z6selectDv4_lS_S_"]         = lle_X_select<int64_t,  int64_t, 4>;
    funcNames["lle_X__Z6selectDv4_mS_Dv4_l"]      = lle_X_select<uint64_t, int64_t, 4>;
    funcNames["lle_X__Z6selectDv8_lS_S_"]         = lle_X_select<int64_t,  int64_t, 8>;
    funcNames["lle_X__Z6selectDv8_mS_Dv8_l"]      = lle_X_select<uint64_t, int64_t, 8>;
    funcNames["lle_X__Z6selectDv16_lS_S_"]        = lle_X_select<int64_t,  int64_t, 16>;
    funcNames["lle_X__Z6selectDv16_mS_Dv16_l"]    = lle_X_select<uint64_t, int64_t, 16>;

    funcNames["lle_X__Z6selectffl"]               = lle_X_select<float, int64_t>;
    funcNames["lle_X__Z6selectDv2_fS_Dv2_l"]      = lle_X_select<float, int64_t, 2>;
    funcNames["lle_X__Z6selectDv3_fS_Dv3_l"]      = lle_X_select<float, int64_t, 3>;
    funcNames["lle_X__Z6selectDv4_fS_Dv4_l"]      = lle_X_select<float, int64_t, 4>;
    funcNames["lle_X__Z6selectDv8_fS_Dv8_l"]      = lle_X_select<float, int64_t, 8>;
    funcNames["lle_X__Z6selectDv16_fS_Dv16_l"]    = lle_X_select<float, int64_t, 16>;

    funcNames["lle_X__Z6selectcch"]               = lle_X_select<int8_t,  uint8_t>;
    funcNames["lle_X__Z6selecthhh"]               = lle_X_select<uint8_t, uint8_t>;
    funcNames["lle_X__Z6selectDv2_cS_Dv2_h"]      = lle_X_select<int8_t,  uint8_t, 2>;
    funcNames["lle_X__Z6selectDv2_hS_S_"]         = lle_X_select<uint8_t, uint8_t, 2>;
    funcNames["lle_X__Z6selectDv3_cS_Dv3_h"]      = lle_X_select<int8_t,  uint8_t, 3>;
    funcNames["lle_X__Z6selectDv3_hS_S_"]         = lle_X_select<uint8_t, uint8_t, 3>;
    funcNames["lle_X__Z6selectDv4_cS_Dv4_h"]      = lle_X_select<int8_t,  uint8_t, 4>;
    funcNames["lle_X__Z6selectDv4_hS_S_"]         = lle_X_select<uint8_t, uint8_t, 4>;
    funcNames["lle_X__Z6selectDv8_cS_Dv8_h"]      = lle_X_select<int8_t,  uint8_t, 8>;
    funcNames["lle_X__Z6selectDv8_hS_S_"]         = lle_X_select<uint8_t, uint8_t, 8>;
    funcNames["lle_X__Z6selectDv16_cS_Dv16_h"]    = lle_X_select<int8_t,  uint8_t, 16>;
    funcNames["lle_X__Z6selectDv16_hS_S_"]        = lle_X_select<uint8_t, uint8_t, 16>;

    funcNames["lle_X__Z6selectssh"]               = lle_X_select<int16_t,  uint8_t>;
    funcNames["lle_X__Z6selecttth"]               = lle_X_select<uint16_t, uint8_t>;
    funcNames["lle_X__Z6selectDv2_sS_Dv2_h"]      = lle_X_select<int16_t,  uint8_t, 2>;
    funcNames["lle_X__Z6selectDv2_tS_Dv2_h"]      = lle_X_select<uint16_t, uint8_t, 2>;
    funcNames["lle_X__Z6selectDv3_sS_Dv3_h"]      = lle_X_select<int16_t,  uint8_t, 3>;
    funcNames["lle_X__Z6selectDv3_tS_Dv3_h"]      = lle_X_select<uint16_t, uint8_t, 3>;
    funcNames["lle_X__Z6selectDv4_sS_Dv4_h"]      = lle_X_select<int16_t,  uint8_t, 4>;
    funcNames["lle_X__Z6selectDv4_tS_Dv4_h"]      = lle_X_select<uint16_t, uint8_t, 4>;
    funcNames["lle_X__Z6selectDv8_sS_Dv8_h"]      = lle_X_select<int16_t,  uint8_t, 8>;
    funcNames["lle_X__Z6selectDv8_tS_Dv8_h"]      = lle_X_select<uint16_t, uint8_t, 8>;
    funcNames["lle_X__Z6selectDv16_sS_Dv16_h"]    = lle_X_select<int16_t,  uint8_t, 16>;
    funcNames["lle_X__Z6selectDv16_tS_Dv16_h"]    = lle_X_select<uint16_t, uint8_t, 16>;

    funcNames["lle_X__Z6selectiih"]               = lle_X_select<int32_t,  uint8_t>;
    funcNames["lle_X__Z6selectjjh"]               = lle_X_select<uint32_t, uint8_t>;
    funcNames["lle_X__Z6selectDv2_iS_Dv2_h"]      = lle_X_select<int32_t,  uint8_t, 2>;
    funcNames["lle_X__Z6selectDv2_jS_Dv2_h"]      = lle_X_select<uint32_t, uint8_t, 2>;
    funcNames["lle_X__Z6selectDv3_iS_Dv3_h"]      = lle_X_select<int32_t,  uint8_t, 3>;
    funcNames["lle_X__Z6selectDv3_jS_Dv3_h"]      = lle_X_select<uint32_t, uint8_t, 3>;
    funcNames["lle_X__Z6selectDv4_iS_Dv4_h"]      = lle_X_select<int32_t,  uint8_t, 4>;
    funcNames["lle_X__Z6selectDv4_jS_Dv4_h"]      = lle_X_select<uint32_t, uint8_t, 4>;
    funcNames["lle_X__Z6selectDv8_iS_Dv8_h"]      = lle_X_select<int32_t,  uint8_t, 8>;
    funcNames["lle_X__Z6selectDv8_jS_Dv8_h"]      = lle_X_select<uint32_t, uint8_t, 8>;
    funcNames["lle_X__Z6selectDv16_iS_Dv16_h"]    = lle_X_select<int32_t,  uint8_t, 16>;
    funcNames["lle_X__Z6selectDv16_jS_Dv16_h"]    = lle_X_select<uint32_t, uint8_t, 16>;

    funcNames["lle_X__Z6selectllh"]               = lle_X_select<int64_t,  uint8_t>;
    funcNames["lle_X__Z6selectmmh"]               = lle_X_select<uint64_t, uint8_t>;
    funcNames["lle_X__Z6selectDv2_lS_Dv2_h"]      = lle_X_select<int64_t,  uint8_t, 2>;
    funcNames["lle_X__Z6selectDv2_mS_Dv2_h"]      = lle_X_select<uint64_t, uint8_t, 2>;
    funcNames["lle_X__Z6selectDv3_lS_Dv3_h"]      = lle_X_select<int64_t,  uint8_t, 3>;
    funcNames["lle_X__Z6selectDv3_mS_Dv3_h"]      = lle_X_select<uint64_t, uint8_t, 3>;
    funcNames["lle_X__Z6selectDv4_lS_Dv4_h"]      = lle_X_select<int64_t,  uint8_t, 4>;
    funcNames["lle_X__Z6selectDv4_mS_Dv4_h"]      = lle_X_select<uint64_t, uint8_t, 4>;
    funcNames["lle_X__Z6selectDv8_lS_Dv8_h"]      = lle_X_select<int64_t,  uint8_t, 8>;
    funcNames["lle_X__Z6selectDv8_mS_Dv8_h"]      = lle_X_select<uint64_t, uint8_t, 8>;
    funcNames["lle_X__Z6selectDv16_lS_Dv16_h"]    = lle_X_select<int64_t,  uint8_t, 16>;
    funcNames["lle_X__Z6selectDv16_mS_Dv16_h"]    = lle_X_select<uint64_t, uint8_t, 16>;

    funcNames["lle_X__Z6selectffh"]               = lle_X_select<float, uint8_t>;
    funcNames["lle_X__Z6selectDv2_fS_Dv2_h"]      = lle_X_select<float, uint8_t, 2>;
    funcNames["lle_X__Z6selectDv3_fS_Dv3_h"]      = lle_X_select<float, uint8_t, 3>;
    funcNames["lle_X__Z6selectDv4_fS_Dv4_h"]      = lle_X_select<float, uint8_t, 4>;
    funcNames["lle_X__Z6selectDv8_fS_Dv8_h"]      = lle_X_select<float, uint8_t, 8>;
    funcNames["lle_X__Z6selectDv16_fS_Dv16_h"]    = lle_X_select<float, uint8_t, 16>;

    funcNames["lle_X__Z6selectcct"]               = lle_X_select<int8_t,  uint16_t>;
    funcNames["lle_X__Z6selecthht"]               = lle_X_select<uint8_t, uint16_t>;
    funcNames["lle_X__Z6selectDv2_cS_Dv2_t"]      = lle_X_select<int8_t,  uint16_t, 2>;
    funcNames["lle_X__Z6selectDv2_hS_Dv2_t"]      = lle_X_select<uint8_t, uint16_t, 2>;
    funcNames["lle_X__Z6selectDv3_cS_Dv3_t"]      = lle_X_select<int8_t,  uint16_t, 3>;
    funcNames["lle_X__Z6selectDv3_hS_Dv3_t"]      = lle_X_select<uint8_t, uint16_t, 3>;
    funcNames["lle_X__Z6selectDv4_cS_Dv4_t"]      = lle_X_select<int8_t,  uint16_t, 4>;
    funcNames["lle_X__Z6selectDv4_hS_Dv4_t"]      = lle_X_select<uint8_t, uint16_t, 4>;
    funcNames["lle_X__Z6selectDv8_cS_Dv8_t"]      = lle_X_select<int8_t,  uint16_t, 8>;
    funcNames["lle_X__Z6selectDv8_hS_Dv8_t"]      = lle_X_select<uint8_t, uint16_t, 8>;
    funcNames["lle_X__Z6selectDv16_cS_Dv16_t"]    = lle_X_select<int8_t,  uint16_t, 16>;
    funcNames["lle_X__Z6selectDv16_hS_Dv16_t"]    = lle_X_select<uint8_t, uint16_t, 16>;

    funcNames["lle_X__Z6selectsst"]               = lle_X_select<int16_t,  uint16_t>;
    funcNames["lle_X__Z6selectttt"]               = lle_X_select<uint16_t, uint16_t>;
    funcNames["lle_X__Z6selectDv2_sS_Dv2_t"]      = lle_X_select<int16_t,  uint16_t, 2>;
    funcNames["lle_X__Z6selectDv2_tS_S_"]         = lle_X_select<uint16_t, uint16_t, 2>;
    funcNames["lle_X__Z6selectDv3_sS_Dv3_t"]      = lle_X_select<int16_t,  uint16_t, 3>;
    funcNames["lle_X__Z6selectDv3_tS_S_"]         = lle_X_select<uint16_t, uint16_t, 3>;
    funcNames["lle_X__Z6selectDv4_sS_Dv4_t"]      = lle_X_select<int16_t,  uint16_t, 4>;
    funcNames["lle_X__Z6selectDv4_tS_S_"]         = lle_X_select<uint16_t, uint16_t, 4>;
    funcNames["lle_X__Z6selectDv8_sS_Dv8_t"]      = lle_X_select<int16_t,  uint16_t, 8>;
    funcNames["lle_X__Z6selectDv8_tS_S_"]         = lle_X_select<uint16_t, uint16_t, 8>;
    funcNames["lle_X__Z6selectDv16_sS_Dv16_t"]    = lle_X_select<int16_t,  uint16_t, 16>;
    funcNames["lle_X__Z6selectDv16_tS_S_"]        = lle_X_select<uint16_t, uint16_t, 16>;

    funcNames["lle_X__Z6selectiit"]               = lle_X_select<int32_t,  uint16_t>;
    funcNames["lle_X__Z6selectjjt"]               = lle_X_select<uint32_t, uint16_t>;
    funcNames["lle_X__Z6selectDv2_iS_Dv2_t"]      = lle_X_select<int32_t,  uint16_t ,2>;
    funcNames["lle_X__Z6selectDv2_jS_Dv2_t"]      = lle_X_select<uint32_t, uint16_t, 2>;
    funcNames["lle_X__Z6selectDv3_iS_Dv3_t"]      = lle_X_select<int32_t,  uint16_t, 3>;
    funcNames["lle_X__Z6selectDv3_jS_Dv3_t"]      = lle_X_select<uint32_t, uint16_t, 3>;
    funcNames["lle_X__Z6selectDv4_iS_Dv4_t"]      = lle_X_select<int32_t,  uint16_t, 4>;
    funcNames["lle_X__Z6selectDv4_jS_Dv4_t"]      = lle_X_select<uint32_t, uint16_t, 4>;
    funcNames["lle_X__Z6selectDv8_iS_Dv8_t"]      = lle_X_select<int32_t,  uint16_t, 8>;
    funcNames["lle_X__Z6selectDv8_jS_Dv8_t"]      = lle_X_select<uint32_t, uint16_t, 8>;
    funcNames["lle_X__Z6selectDv16_iS_Dv16_t"]    = lle_X_select<int32_t,  uint16_t, 16>;
    funcNames["lle_X__Z6selectDv16_jS_Dv16_t"]    = lle_X_select<uint32_t, uint16_t, 16>;

    funcNames["lle_X__Z6selectllt"]               = lle_X_select<int64_t,  uint16_t>;
    funcNames["lle_X__Z6selectmmt"]               = lle_X_select<uint64_t, uint16_t>;
    funcNames["lle_X__Z6selectDv2_lS_Dv2_t"]      = lle_X_select<int64_t,  uint16_t ,2>;
    funcNames["lle_X__Z6selectDv2_mS_Dv2_t"]      = lle_X_select<uint64_t, uint16_t, 2>;
    funcNames["lle_X__Z6selectDv3_lS_Dv3_t"]      = lle_X_select<int64_t,  uint16_t, 3>;
    funcNames["lle_X__Z6selectDv3_mS_Dv3_t"]      = lle_X_select<uint64_t, uint16_t, 3>;
    funcNames["lle_X__Z6selectDv4_lS_Dv4_t"]      = lle_X_select<int64_t,  uint16_t, 4>;
    funcNames["lle_X__Z6selectDv4_mS_Dv4_t"]      = lle_X_select<uint64_t, uint16_t, 4>;
    funcNames["lle_X__Z6selectDv8_lS_Dv8_t"]      = lle_X_select<int64_t,  uint16_t, 8>;
    funcNames["lle_X__Z6selectDv8_mS_Dv8_t"]      = lle_X_select<uint64_t, uint16_t, 8>;
    funcNames["lle_X__Z6selectDv16_lS_Dv16_t"]    = lle_X_select<int64_t,  uint16_t, 16>;
    funcNames["lle_X__Z6selectDv16_mS_Dv16_t"]    = lle_X_select<uint64_t, uint16_t, 16>;

    funcNames["lle_X__Z6selectfft"]               = lle_X_select<float, uint16_t>;
    funcNames["lle_X__Z6selectDv2_fS_Dv2_t"]      = lle_X_select<float, uint16_t, 2>;
    funcNames["lle_X__Z6selectDv3_fS_Dv3_t"]      = lle_X_select<float, uint16_t, 3>;
    funcNames["lle_X__Z6selectDv4_fS_Dv4_t"]      = lle_X_select<float, uint16_t, 4>;
    funcNames["lle_X__Z6selectDv8_fS_Dv8_t"]      = lle_X_select<float, uint16_t, 8>;
    funcNames["lle_X__Z6selectDv16_fS_Dv16_t"]    = lle_X_select<float, uint16_t, 16>;

    funcNames["lle_X__Z6selectccj"]               = lle_X_select<int8_t,  uint32_t>;
    funcNames["lle_X__Z6selecthhj"]               = lle_X_select<uint8_t, uint32_t>;
    funcNames["lle_X__Z6selectDv2_cS_Dv2_j"]      = lle_X_select<int8_t,  uint32_t ,2>;
    funcNames["lle_X__Z6selectDv2_hS_Dv2_j"]      = lle_X_select<uint8_t, uint32_t, 2>;
    funcNames["lle_X__Z6selectDv3_cS_Dv3_j"]      = lle_X_select<int8_t,  uint32_t, 3>;
    funcNames["lle_X__Z6selectDv3_hS_Dv3_j"]      = lle_X_select<uint8_t, uint32_t, 3>;
    funcNames["lle_X__Z6selectDv4_cS_Dv4_j"]      = lle_X_select<int8_t,  uint32_t, 4>;
    funcNames["lle_X__Z6selectDv4_hS_Dv4_j"]      = lle_X_select<uint8_t, uint32_t, 4>;
    funcNames["lle_X__Z6selectDv8_cS_Dv8_j"]      = lle_X_select<int8_t,  uint32_t, 8>;
    funcNames["lle_X__Z6selectDv8_hS_Dv8_j"]      = lle_X_select<uint8_t, uint32_t, 8>;
    funcNames["lle_X__Z6selectDv16_cS_Dv16_j"]    = lle_X_select<int8_t,  uint32_t, 16>;
    funcNames["lle_X__Z6selectDv16_hS_Dv16_j"]    = lle_X_select<uint8_t, uint32_t, 16>;

    funcNames["lle_X__Z6selectssj"]               = lle_X_select<int16_t,  uint32_t>;
    funcNames["lle_X__Z6selectttj"]               = lle_X_select<uint16_t, uint32_t>;
    funcNames["lle_X__Z6selectDv2_sS_Dv2_j"]      = lle_X_select<int16_t,  uint32_t, 2>;
    funcNames["lle_X__Z6selectDv2_tS_Dv2_j"]      = lle_X_select<uint16_t, uint32_t, 2>;
    funcNames["lle_X__Z6selectDv3_sS_Dv3_j"]      = lle_X_select<int16_t,  uint32_t, 3>;
    funcNames["lle_X__Z6selectDv3_tS_Dv3_j"]      = lle_X_select<uint16_t, uint32_t, 3>;
    funcNames["lle_X__Z6selectDv4_sS_Dv4_j"]      = lle_X_select<int16_t,  uint32_t, 4>;
    funcNames["lle_X__Z6selectDv4_tS_Dv4_j"]      = lle_X_select<uint16_t, uint32_t, 4>;
    funcNames["lle_X__Z6selectDv8_sS_Dv8_j"]      = lle_X_select<int16_t,  uint32_t, 8>;
    funcNames["lle_X__Z6selectDv8_tS_Dv8_j"]      = lle_X_select<uint16_t, uint32_t, 8>;
    funcNames["lle_X__Z6selectDv16_sS_Dv16_j"]    = lle_X_select<int16_t,  uint32_t, 16>;
    funcNames["lle_X__Z6selectDv16_tS_Dv16_j"]    = lle_X_select<uint16_t, uint32_t, 16>;

    funcNames["lle_X__Z6selectiij"]               = lle_X_select<int32_t,  uint32_t>;
    funcNames["lle_X__Z6selectjjj"]               = lle_X_select<uint32_t, uint32_t>;
    funcNames["lle_X__Z6selectDv2_iS_Dv2_j"]      = lle_X_select<int32_t,  uint32_t, 2>;
    funcNames["lle_X__Z6selectDv2_jS_S_"]         = lle_X_select<uint32_t, uint32_t, 2>;
    funcNames["lle_X__Z6selectDv3_iS_Dv3_j"]      = lle_X_select<int32_t,  uint32_t, 3>;
    funcNames["lle_X__Z6selectDv3_jS_S_"]         = lle_X_select<uint32_t, uint32_t, 3>;
    funcNames["lle_X__Z6selectDv4_iS_Dv4_j"]      = lle_X_select<int32_t,  uint32_t, 4>;
    funcNames["lle_X__Z6selectDv4_jS_S_"]         = lle_X_select<uint32_t, uint32_t, 4>;
    funcNames["lle_X__Z6selectDv8_iS_Dv8_j"]      = lle_X_select<int32_t,  uint32_t, 8>;
    funcNames["lle_X__Z6selectDv8_jS_S_"]         = lle_X_select<uint32_t, uint32_t, 8>;
    funcNames["lle_X__Z6selectDv16_iS_Dv16_j"]    = lle_X_select<int32_t,  uint32_t, 16>;
    funcNames["lle_X__Z6selectDv16_jS_S_"]        = lle_X_select<uint32_t, uint32_t, 16>;

    funcNames["lle_X__Z6selectllj"]               = lle_X_select<int64_t,  uint32_t>;
    funcNames["lle_X__Z6selectmmj"]               = lle_X_select<uint64_t, uint32_t>;
    funcNames["lle_X__Z6selectDv2_lS_Dv2_j"]      = lle_X_select<int64_t,  uint32_t, 2>;
    funcNames["lle_X__Z6selectDv2_mS_Dv2_j"]      = lle_X_select<uint64_t, uint32_t, 2>;
    funcNames["lle_X__Z6selectDv3_lS_Dv3_j"]      = lle_X_select<int64_t,  uint32_t, 3>;
    funcNames["lle_X__Z6selectDv3_mS_Dv3_j"]      = lle_X_select<uint64_t, uint32_t, 3>;
    funcNames["lle_X__Z6selectDv4_lS_Dv4_j"]      = lle_X_select<int64_t,  uint32_t, 4>;
    funcNames["lle_X__Z6selectDv4_mS_Dv4_j"]      = lle_X_select<uint64_t, uint32_t, 4>;
    funcNames["lle_X__Z6selectDv8_lS_Dv8_j"]      = lle_X_select<int64_t,  uint32_t, 8>;
    funcNames["lle_X__Z6selectDv8_mS_Dv8_j"]      = lle_X_select<uint64_t, uint32_t, 8>;
    funcNames["lle_X__Z6selectDv16_lS_Dv16_j"]    = lle_X_select<int64_t,  uint32_t, 16>;
    funcNames["lle_X__Z6selectDv16_mS_Dv16_j"]    = lle_X_select<uint64_t, uint32_t, 16>;

    funcNames["lle_X__Z6selectffj"]               = lle_X_select<float, uint32_t>;
    funcNames["lle_X__Z6selectDv2_fS_Dv2_j"]      = lle_X_select<float, uint32_t, 2>;
    funcNames["lle_X__Z6selectDv3_fS_Dv3_j"]      = lle_X_select<float, uint32_t, 3>;
    funcNames["lle_X__Z6selectDv4_fS_Dv4_j"]      = lle_X_select<float, uint32_t, 4>;
    funcNames["lle_X__Z6selectDv8_fS_Dv8_j"]      = lle_X_select<float, uint32_t, 8>;
    funcNames["lle_X__Z6selectDv16_fS_Dv16_j"]    = lle_X_select<float, uint32_t, 16>;

    funcNames["lle_X__Z6selectccm"]               = lle_X_select<int8_t,  uint64_t>;
    funcNames["lle_X__Z6selecthhm"]               = lle_X_select<uint8_t, uint64_t>;
    funcNames["lle_X__Z6selectDv2_cS_Dv2_m"]      = lle_X_select<int8_t,  uint64_t, 2>;
    funcNames["lle_X__Z6selectDv2_hS_Dv2_m"]      = lle_X_select<uint8_t, uint64_t, 2>;
    funcNames["lle_X__Z6selectDv3_cS_Dv3_m"]      = lle_X_select<int8_t,  uint64_t, 3>;
    funcNames["lle_X__Z6selectDv3_hS_Dv3_m"]      = lle_X_select<uint8_t, uint64_t, 3>;
    funcNames["lle_X__Z6selectDv4_cS_Dv4_m"]      = lle_X_select<int8_t,  uint64_t, 4>;
    funcNames["lle_X__Z6selectDv4_hS_Dv4_m"]      = lle_X_select<uint8_t, uint64_t, 4>;
    funcNames["lle_X__Z6selectDv8_cS_Dv8_m"]      = lle_X_select<int8_t,  uint64_t, 8>;
    funcNames["lle_X__Z6selectDv8_hS_Dv8_m"]      = lle_X_select<uint8_t, uint64_t, 8>;
    funcNames["lle_X__Z6selectDv16_cS_Dv16_m"]    = lle_X_select<int8_t,  uint64_t, 16>;
    funcNames["lle_X__Z6selectDv16_hS_Dv16_m"]    = lle_X_select<uint8_t, uint64_t, 16>;

    funcNames["lle_X__Z6selectssm"]               = lle_X_select<int16_t,  uint64_t>;
    funcNames["lle_X__Z6selectttm"]               = lle_X_select<uint16_t, uint64_t>;
    funcNames["lle_X__Z6selectDv2_sS_Dv2_m"]      = lle_X_select<int16_t,  uint64_t, 2>;
    funcNames["lle_X__Z6selectDv2_tS_Dv2_m"]      = lle_X_select<uint16_t, uint64_t, 2>;
    funcNames["lle_X__Z6selectDv3_sS_Dv3_m"]      = lle_X_select<int16_t,  uint64_t, 3>;
    funcNames["lle_X__Z6selectDv3_tS_Dv3_m"]      = lle_X_select<uint16_t, uint64_t, 3>;
    funcNames["lle_X__Z6selectDv4_sS_Dv4_m"]      = lle_X_select<int16_t,  uint64_t, 4>;
    funcNames["lle_X__Z6selectDv4_tS_Dv4_m"]      = lle_X_select<uint16_t, uint64_t, 4>;
    funcNames["lle_X__Z6selectDv8_sS_Dv8_m"]      = lle_X_select<int16_t,  uint64_t, 8>;
    funcNames["lle_X__Z6selectDv8_tS_Dv8_m"]      = lle_X_select<uint16_t, uint64_t, 8>;
    funcNames["lle_X__Z6selectDv16_sS_Dv16_m"]    = lle_X_select<int16_t,  uint64_t, 16>;
    funcNames["lle_X__Z6selectDv16_tS_Dv16_m"]    = lle_X_select<uint16_t, uint64_t, 16>;

    funcNames["lle_X__Z6selectiim"]               = lle_X_select<int32_t,  uint64_t>;
    funcNames["lle_X__Z6selectjjm"]               = lle_X_select<uint32_t, uint64_t>;
    funcNames["lle_X__Z6selectDv2_iS_Dv2_m"]      = lle_X_select<int32_t,  uint64_t, 2>;
    funcNames["lle_X__Z6selectDv2_jS_Dv2_m"]      = lle_X_select<uint32_t, uint64_t, 2>;
    funcNames["lle_X__Z6selectDv3_iS_Dv3_m"]      = lle_X_select<int32_t,  uint64_t, 3>;
    funcNames["lle_X__Z6selectDv3_jS_Dv3_m"]      = lle_X_select<uint32_t, uint64_t, 3>;
    funcNames["lle_X__Z6selectDv4_iS_Dv4_m"]      = lle_X_select<int32_t,  uint64_t, 4>;
    funcNames["lle_X__Z6selectDv4_jS_Dv4_m"]      = lle_X_select<uint32_t, uint64_t, 4>;
    funcNames["lle_X__Z6selectDv8_iS_Dv8_m"]      = lle_X_select<int32_t,  uint64_t, 8>;
    funcNames["lle_X__Z6selectDv8_jS_Dv8_m"]      = lle_X_select<uint32_t, uint64_t, 8>;
    funcNames["lle_X__Z6selectDv16_iS_Dv16_m"]    = lle_X_select<int32_t,  uint64_t, 16>;
    funcNames["lle_X__Z6selectDv16_jS_Dv16_m"]    = lle_X_select<uint32_t, uint64_t, 16>;

    funcNames["lle_X__Z6selectllm"]               = lle_X_select<int64_t,  uint64_t>;
    funcNames["lle_X__Z6selectmmm"]               = lle_X_select<uint64_t, uint64_t>;
    funcNames["lle_X__Z6selectDv2_lS_Dv2_m"]      = lle_X_select<int64_t,  uint64_t, 2>;
    funcNames["lle_X__Z6selectDv2_mS_S_"]         = lle_X_select<uint64_t, uint64_t, 2>;
    funcNames["lle_X__Z6selectDv3_lS_Dv3_m"]      = lle_X_select<int64_t,  uint64_t, 3>;
    funcNames["lle_X__Z6selectDv3_mS_S_"]         = lle_X_select<uint64_t, uint64_t, 3>;
    funcNames["lle_X__Z6selectDv4_lS_Dv4_m"]      = lle_X_select<int64_t,  uint64_t, 4>;
    funcNames["lle_X__Z6selectDv4_mS_S_"]         = lle_X_select<uint64_t, uint64_t, 4>;
    funcNames["lle_X__Z6selectDv8_lS_Dv8_m"]      = lle_X_select<int64_t,  uint64_t, 8>;
    funcNames["lle_X__Z6selectDv8_mS_S_"]         = lle_X_select<uint64_t, uint64_t, 8>;
    funcNames["lle_X__Z6selectDv16_lS_Dv16_m"]    = lle_X_select<int64_t,  uint64_t, 16>;
    funcNames["lle_X__Z6selectDv16_mS_S_"]        = lle_X_select<uint64_t, uint64_t, 16>;

    funcNames["lle_X__Z6selectffm"]               = lle_X_select<float, uint64_t>;
    funcNames["lle_X__Z6selectDv2_fS_Dv2_m"]      = lle_X_select<float, uint64_t, 2>;
    funcNames["lle_X__Z6selectDv3_fS_Dv3_m"]      = lle_X_select<float, uint64_t, 3>;
    funcNames["lle_X__Z6selectDv4_fS_Dv4_m"]      = lle_X_select<float, uint64_t, 4>;
    funcNames["lle_X__Z6selectDv8_fS_Dv8_m"]      = lle_X_select<float, uint64_t, 8>;
    funcNames["lle_X__Z6selectDv16_fS_Dv16_m"]    = lle_X_select<float, uint64_t, 16>;

    funcNames["lle_X__Z6selectddl"]               = lle_X_select<double, int64_t>;
    funcNames["lle_X__Z6selectDv2_dS_Dv2_l"]      = lle_X_select<double, int64_t, 2>;
    funcNames["lle_X__Z6selectDv3_dS_Dv3_l"]      = lle_X_select<double, int64_t, 3>;
    funcNames["lle_X__Z6selectDv4_dS_Dv4_l"]      = lle_X_select<double, int64_t, 4>;
    funcNames["lle_X__Z6selectDv8_dS_Dv8_l"]      = lle_X_select<double, int64_t, 8>;
    funcNames["lle_X__Z6selectDv16_dS_Dv16_l"]    = lle_X_select<double, int64_t, 16>;

    funcNames["lle_X__Z6selectddm"]               = lle_X_select<double, uint64_t>;
    funcNames["lle_X__Z6selectDv2_dS_Dv2_m"]      = lle_X_select<double, uint64_t, 2>;
    funcNames["lle_X__Z6selectDv3_dS_Dv3_m"]      = lle_X_select<double, uint64_t, 3>;
    funcNames["lle_X__Z6selectDv4_dS_Dv4_m"]      = lle_X_select<double, uint64_t, 4>;
    funcNames["lle_X__Z6selectDv8_dS_Dv8_m"]      = lle_X_select<double, uint64_t, 8>;
    funcNames["lle_X__Z6selectDv16_dS_Dv16_m"]    = lle_X_select<double, uint64_t, 16>;
}
    template<>
    llvm::GenericValue localBitselect( float inA, float inB, float inC )
    {
        llvm::GenericValue R;
        union {uint32_t u; float f;} a, b, c, out;
        a.f = inA;
        b.f = inB;
        c.f = inC;
        out.u = ( a.u & ~c.u ) | ( b.u & c.u );
        getRef<float>(R) = out.f;
        return R;
    }
    template<>
    llvm::GenericValue localBitselect( double inA, double inB, double inC )
    {
        llvm::GenericValue R;
        union {uint64_t u; double f;} a, b, c, out;
        a.f = inA;
        b.f = inB;
        c.f = inC;
        out.u = ( a.u & c.u ) | ( b.u & c.u );
        getRef<double>(R) = out.f;
        return R;
    }

    template<>
    llvm::GenericValue selectResult( float inC )
    {
        llvm::GenericValue R;
        getRef<float>(R) = inC;
        return R;
    }
    template<>
    llvm::GenericValue selectResult( double inC )
    {
        llvm::GenericValue R;
        getRef<double>(R) = inC;
        return R;
    }

}
}
