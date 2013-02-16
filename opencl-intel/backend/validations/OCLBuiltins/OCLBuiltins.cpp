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
#include "BLTAtomic.h"
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

    // Adding atomic built-in functions.
    AtomicMapFiller atomicBuiltins;
    atomicBuiltins.addOpenCLBuiltins(funcNames);

    // Implementation of 'memcpy' function/intrinsic.
    funcNames["lle_X_memcpy"] = lle_X_memcpy;

#if 0 // temporary switch off because the compilation in Release mode is deadly slow.
    funcNames["barrier"] = UnimplementedBuiltin;
    funcNames["mem_fence"] = UnimplementedBuiltin;
    funcNames["read_mem_fence"] = UnimplementedBuiltin;
    funcNames["write_mem_fence"] = UnimplementedBuiltin;

#endif

}

}
}
