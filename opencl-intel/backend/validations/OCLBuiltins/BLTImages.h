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

File Name:  BLTImages.h

\*****************************************************************************/
#ifndef BLT_IMAGES_H
#define BLT_IMAGES_H

#include <vector>
#include <llvm/DerivedTypes.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include "llvm/Support/Debug.h"
#include "Helpers.h"
#include "IBLTMapFiller.h"
#include "RefALU.h"
#include "ImagesALU.h"
// !!!! HACK
// Do not move #include "CL/cl.h" before including <math.h> since on VS2008 it generates
// Removing annoying ‘ceil’ : attributes not present on previous declaration warning C4985
#include "cl_types.h"
#include "CL/cl.h"

#ifdef DEBUG_TYPE
#undef DEBUG_TYPE
#define DEBUG_TYPE "OpenCLReferenceRunner"
#endif

struct _cl_mem_obj_descriptor;
typedef struct _cl_mem_obj_descriptor cl_mem_obj_descriptor;
using namespace llvm;

namespace Validation {
namespace OCLBuiltins {

Conformance::image_descriptor CreateConfImageDesc(const cl_mem_obj_descriptor& in_Desc, cl_image_format& out_ImageFmt);
Conformance::image_sampler_data CreateSamplerData(const uint32_t& in_sampler);
cl_channel_type ConvertChannelDataTypeFromIntelOCLToCL(const cl_channel_type& val );

    /// This class adds references to the implementations of OpenCL built-in functions
    /// from 6.11.13 section Images
    class ImageMapFiller : public IBLTMapFiller
    {
    public:
        void addOpenCLBuiltins(std::map<std::string, PBLTFunc>& funcNames);
    };

    template<typename T>
    llvm::GenericValue lle_X_read_image( llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args )
    {
        llvm::GenericValue gv;
        cl_mem_obj_descriptor * memobj = (cl_mem_obj_descriptor *)Args[0].PointerVal;
        uint32_t sampler = Args[1].IntVal.getZExtValue();

        // datatype of coordinates == float or int
        const llvm::Type::TypeID CoordTy= FT->getParamType(2)->getContainedType(0)->getTypeID();

        // is image 3d
        const bool IsImage3d = (memobj->dim_count == 3);

        // coordinates
        float u = 0.0f, v = 0.0f, w = 0.0f;

        const llvm::GenericValue& CoordGV = Args[2];

        if( llvm::Type::FloatTyID == CoordTy)
        {
            // suppose vector is 4 elements
            // even if vector is 2 elements we don't access out of boundaries
            u = getVal<float, 4>(CoordGV, 0);
            v = getVal<float, 4>(CoordGV, 1);
            if( IsImage3d )
                w = getVal<float, 4>(CoordGV, 2);
        }
        else
        { // int coordinates
            u = (float) getVal<uint32_t, 4>(CoordGV, 0);
            v = (float) getVal<uint32_t, 4>(CoordGV, 1);
            if( IsImage3d )
                w = (float) getVal<uint32_t, 4>(CoordGV, 2);
        }

        //DEBUG(llvm::dbgs() << "Coordinates u=" << u <<" v=" << v << " w=" << w << "\n");

        cl_image_format im_fmt;
        Conformance::image_descriptor desc = CreateConfImageDesc(*memobj, im_fmt);
        Conformance::image_sampler_data imageSampler = CreateSamplerData(sampler);
        T Pixel[4];
        Conformance::sample_image_pixel<T>(
            memobj->pData, // void *imageData,
            &desc, // image_descriptor *imageInfo,
            u,v,w,
            &imageSampler,// image_sampler_data *imageSampler,
            Pixel);

        gv.AggregateVal.resize(4);
        getRef<T,4>(gv, 0) = derefPointer<T>(Pixel+0);
        getRef<T,4>(gv, 1) = derefPointer<T>(Pixel+1);
        getRef<T,4>(gv, 2) = derefPointer<T>(Pixel+2);
        getRef<T,4>(gv, 3) = derefPointer<T>(Pixel+3);

        return gv;
    }


    llvm::GenericValue lle_X_write_image(llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args);

    llvm::GenericValue lle_X_get_image_dim2(llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args);

    llvm::GenericValue lle_X_get_image_width(llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args);

    llvm::GenericValue lle_X_get_image_height(llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args);

    llvm::GenericValue lle_X_get_image_depth(llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args);

    llvm::GenericValue lle_X_get_image_dim3(llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args);

    llvm::GenericValue lle_X_get_image_channel_data_type(llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args);

    llvm::GenericValue lle_X_get_image_channel_order(llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args);

    /// @brief convert from Intel OpenCL enums with CLK_ prefix to CL_ prefix
    cl_channel_order ConvertChannelOrderFromIntelOCLToCL(const cl_channel_order& val );

    /// @brief convert from Intel OpenCL enums with CLK_ prefix to CL_ prefix
    cl_channel_type ConvertChannelDataTypeFromIntelOCLToCL(const cl_channel_type& val );

    /// @brief Create image_descriptor and cl_image_format for calling Conformance::ImagesALU functions
    Conformance::image_descriptor CreateConfImageDesc(const cl_mem_obj_descriptor& in_Desc,
        cl_image_format& out_ImageFmt);

    /// @brief Create sampler data struct for calling Conformance::ImagesALU functions
    Conformance::image_sampler_data CreateSamplerData(const uint32_t& in_sampler);

} // namespace OCLBuiltins
} // namespace Validation

#endif // BLT_IMAGES_H
