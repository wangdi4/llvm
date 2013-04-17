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

    template<typename T>
    llvm::GenericValue local_read_image(const cl_mem_obj_descriptor * memobj, const llvm::GenericValue& CoordGV, const uint32_t sampler, const llvm::Type::TypeID CoordTy) {
        llvm::GenericValue gv;

        const cl_mem_object_type objType = memobj->memObjType;
        // coordinates
        float u = 0.0f, v = 0.0f, w = 0.0f;

        if( llvm::Type::FloatTyID == CoordTy)
        { // float coordinates
            switch(objType) {
            case CL_MEM_OBJECT_IMAGE1D:
            case CL_MEM_OBJECT_IMAGE1D_BUFFER:
                u = getVal<float, 1>(CoordGV, 0);
                break;
            case CL_MEM_OBJECT_IMAGE1D_ARRAY:
            case CL_MEM_OBJECT_IMAGE2D:
                u = getVal<float, 2>(CoordGV, 0);
                v = getVal<float, 2>(CoordGV, 1);
                break;
            case CL_MEM_OBJECT_IMAGE2D_ARRAY:
            case CL_MEM_OBJECT_IMAGE3D:
                u = getVal<float, 4>(CoordGV, 0);
                v = getVal<float, 4>(CoordGV, 1);
                w = getVal<float, 4>(CoordGV, 2);
                break;
            default:
                break;
            }
        }
        else
        { // int coordinates
            switch(objType) {
            case CL_MEM_OBJECT_IMAGE1D:
            case CL_MEM_OBJECT_IMAGE1D_BUFFER:
                 u = (float) getVal<int32_t, 1>(CoordGV, 0);
                break;
            case CL_MEM_OBJECT_IMAGE1D_ARRAY:
            case CL_MEM_OBJECT_IMAGE2D:
                u = (float) getVal<int32_t, 2>(CoordGV, 0);
                v = (float) getVal<int32_t, 2>(CoordGV, 1);
                break;
            case CL_MEM_OBJECT_IMAGE2D_ARRAY:
            case CL_MEM_OBJECT_IMAGE3D:
                u = (float) getVal<int32_t, 4>(CoordGV, 0);
                v = (float) getVal<int32_t, 4>(CoordGV, 1);
                w = (float) getVal<int32_t, 4>(CoordGV, 2);
                break;
            default:
                break;
            }
        }

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


    template<typename T>
    llvm::GenericValue lle_X_read_image( llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args )
    {
        const cl_mem_obj_descriptor * memobj = (cl_mem_obj_descriptor *)Args[0].PointerVal;

        const llvm::GenericValue& CoordGV = Args[2];

        const uint32_t sampler = uint32_t(Args[1].IntVal.getZExtValue());

        // datatype of coordinates == float or int
        llvm::Type::TypeID CoordTy;
        // for 1d image parameter number 2 is scalar, for other image
        // formats it is a vector
        if( FT->getParamType(2)->getNumContainedTypes() > 0 )
            CoordTy = FT->getParamType(2)->getContainedType(0)->getTypeID();
        else
            CoordTy = FT->getParamType(2)->getTypeID();

        return local_read_image<T>(memobj,CoordGV,sampler,CoordTy);
    }

    template<typename T>
    llvm::GenericValue lle_X_read_image_samplerless( llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args )
    {
        const cl_mem_obj_descriptor * memobj = (cl_mem_obj_descriptor *)Args[0].PointerVal;

        const llvm::GenericValue& CoordGV = Args[1];

        const uint32_t sampler = uint32_t(CL_DEV_SAMPLER_ADDRESS_NONE);

        // datatype of coordinates == float or int
        llvm::Type::TypeID CoordTy;
        // for 1d image parameter number 1 is scalar, for other image
        // formats it is a vector
        if( FT->getParamType(1)->getNumContainedTypes() > 0 )
            CoordTy = FT->getParamType(1)->getContainedType(0)->getTypeID();
        else
            CoordTy = FT->getParamType(1)->getTypeID();

        return local_read_image<T>(memobj,CoordGV,sampler,CoordTy);
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

    llvm::GenericValue lle_X_get_image_array_size(FunctionType *FT,
        const std::vector<GenericValue> &Args);

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
