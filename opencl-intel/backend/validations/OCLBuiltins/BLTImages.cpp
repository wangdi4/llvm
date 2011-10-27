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

File Name:  BLTImages.cpp

\*****************************************************************************/
#define DEBUG_TYPE "OpenCLReferenceRunner"


// debug macros
#include <llvm/Type.h>
#include <llvm/Support/raw_ostream.h>
#include "llvm/Support/Debug.h"

#include "Helpers.h"
#include "BLTImages.h"

// !!!! HACK 
// Do not move #include "CL/cl.h" before including <math.h> since on VS2008 it generates
// Removing annoying ‘ceil’ : attributes not present on previous declaration warning C4985
#include "cl_types.h"
#include "CL/cl.h"


using namespace llvm;
using std::map;
using std::string;
using std::vector;
namespace Validation {
namespace OCLBuiltins {

    template<typename T>
    llvm::GenericValue lle_X_read_image( const llvm::FunctionType *FT, 
        const std::vector<llvm::GenericValue> &Args )
    {
        GenericValue gv;
        cl_mem_obj_descriptor * memobj = (cl_mem_obj_descriptor *)Args[0].PointerVal;
        uint32_t sampler = Args[1].IntVal.getZExtValue();

        // datatype of coordinates == float or int
        const Type::TypeID CoordTy= FT->getParamType(2)->getContainedType(0)->getTypeID();

        // is image 3d
        const bool IsImage3d = (memobj->dim_count == 3);

        // coordinates
        float u = 0.0f, v = 0.0f, w = 0.0f;

        const GenericValue& CoordGV = Args[2];

        if( Type::FloatTyID == CoordTy)
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

        DEBUG(dbgs() << "Coordinates u=" << u <<" v=" << v << " w=" << w << "\n");

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

void ImageMapFiller::addOpenCLBuiltins( map<string, PBLTFunc>& funcNames )
{
    funcNames["lle_X__Z11read_imagefP10_image2d_tjDv2_i"] = lle_X_read_image<float>;
    funcNames["lle_X__Z11read_imagefP10_image2d_tjDv2_f"] = lle_X_read_image<float>;
    funcNames["lle_X__Z11read_imageiP10_image2d_tjDv2_i"] = lle_X_read_image<int32_t>;
    funcNames["lle_X__Z11read_imageiP10_image2d_tjDv2_f"] = lle_X_read_image<int32_t>;
    funcNames["lle_X__Z12read_imageuiP10_image2d_tjDv2_i"] = lle_X_read_image<uint32_t>;
    funcNames["lle_X__Z12read_imageuiP10_image2d_tjDv2_f"] = lle_X_read_image<uint32_t>;
    funcNames["lle_X__Z12write_imagefP10_image2d_tDv2_iDv4_f"] = lle_X_write_image;
    funcNames["lle_X__Z12write_imageiP10_image2d_tDv2_iDv4_i"] = lle_X_write_image;
    funcNames["lle_X__Z13write_imageuiP10_image2d_tDv2_iDv4_j"] = lle_X_write_image;
    funcNames["lle_X__Z11read_imagefP10_image3d_tjDv4_i"] = lle_X_read_image<float>;
    funcNames["lle_X__Z11read_imagefP10_image3d_tjDv4_f"] = lle_X_read_image<float>;
    funcNames["lle_X__Z11read_imageiP10_image3d_tjDv4_i"] = lle_X_read_image<int32_t>;
    funcNames["lle_X__Z11read_imageiP10_image3d_tjDv4_f"] = lle_X_read_image<int32_t>;
    funcNames["lle_X__Z12read_imageuiP10_image3d_tjDv4_i"] = lle_X_read_image<uint32_t>;
    funcNames["lle_X__Z12read_imageuiP10_image3d_tjDv4_f"] = lle_X_read_image<uint32_t>;
    funcNames["lle_X__Z15get_image_widthP10_image2d_t"] = lle_X_get_image_width;
    funcNames["lle_X__Z15get_image_widthP10_image3d_t"] = lle_X_get_image_width;
    funcNames["lle_X__Z16get_image_heightP10_image2d_t"] = lle_X_get_image_height;
    funcNames["lle_X__Z16get_image_heightP10_image3d_t"] = lle_X_get_image_height;
    funcNames["lle_X__Z15get_image_depthP10_image3d_t"] = lle_X_get_image_depth;
    funcNames["lle_X__Z27get_image_channel_data_typeP10_image2d_t"] = lle_X_get_image_channel_data_type;
    funcNames["lle_X__Z27get_image_channel_data_typeP10_image3d_t"] = lle_X_get_image_channel_data_type;
    funcNames["lle_X__Z23get_image_channel_orderP10_image2d_t"] = lle_X_get_image_channel_order;
    funcNames["lle_X__Z23get_image_channel_orderP10_image3d_t"] = lle_X_get_image_channel_order;
    funcNames["lle_X__Z13get_image_dimP10_image2d_t"] = lle_X_get_image_dim2;
    funcNames["lle_X__Z13get_image_dimP10_image3d_t"] = lle_X_get_image_dim3;
}

// convert from Intel OpenCL enums with CLK_ prefix to CL_ prefix
cl_channel_order ConvertChannelOrderFromIntelOCLToCL(const cl_channel_order& val )
{
    cl_channel_order ret;
    switch (val)
    {
    case CLK_R:          ret = CL_R; break;
    // todo: add CL_Rx
    //case OpenCL_Rx:         ret = CL_Rx; throw; break;
    case CLK_A:          ret = CL_A; break;
    case CLK_INTENSITY:  ret = CL_INTENSITY; break;
    case CLK_LUMINANCE:  ret = CL_LUMINANCE; break;
    case CLK_RG:         ret = CL_RG; break;
    // todo: add CL_RGx
    //case OpenCL_RGx:        ret = CL_RGx; throw; break;
    case CLK_RA:         ret = CL_RA; break;
    case CLK_RGB:        ret = CL_RGB; break;
    // todo: add CL_RGBx
    //case OpenCL_RGBx:       ret = CL_RGBx; throw; break;
    case CLK_RGBA:       ret = CL_RGBA; break;
    case CLK_ARGB:       ret = CL_ARGB; break;
    case CLK_BGRA:       ret = CL_BGRA; break;
    default: throw Exception::InvalidArgument("ConvertChannelOrderFromIntelOCLToCL:: Unknown Image pixel data type");
    }
    return ret;
}

// convert from Intel OpenCL enums with CLK_ prefix to CL_ prefix
cl_channel_type ConvertChannelDataTypeFromIntelOCLToCL(const cl_channel_type& val )
{
    cl_channel_type ret;
    switch (val)
    {
    case CLK_SNORM_INT8:         ret = CL_SNORM_INT8; break;
    case CLK_SNORM_INT16:        ret = CL_SNORM_INT16; break;
    case CLK_UNORM_INT8:         ret = CL_UNORM_INT8; break;
    case CLK_UNORM_INT16:        ret = CL_UNORM_INT16; break;
    case CLK_UNORM_SHORT_565:    ret = CL_UNORM_SHORT_565; break;
    case CLK_UNORM_SHORT_555:    ret = CL_UNORM_SHORT_555; break;
    case CLK_UNORM_INT_101010:   ret = CL_UNORM_INT_101010; break;
    case CLK_SIGNED_INT8:        ret = CL_SIGNED_INT8; break;
    case CLK_SIGNED_INT16:       ret = CL_SIGNED_INT16; break;
    case CLK_SIGNED_INT32:       ret = CL_SIGNED_INT32; break;
    case CLK_UNSIGNED_INT8:      ret = CL_UNSIGNED_INT8; break;
    case CLK_UNSIGNED_INT16:     ret = CL_UNSIGNED_INT16; break;
    case CLK_UNSIGNED_INT32:     ret = CL_UNSIGNED_INT32; break;
    case CLK_HALF_FLOAT:         ret = CL_HALF_FLOAT; break;
    case CLK_FLOAT:              ret = CL_FLOAT; break;
    default: throw Exception::InvalidArgument("ConvertChannelDataTypeFromIntelOCLToCL:: Unknown Image channel data type");
    }
    return ret;
}

Conformance::image_descriptor CreateConfImageDesc(const cl_mem_obj_descriptor& in_Desc, 
                                                         cl_image_format& out_ImageFmt)
{
    Conformance::image_descriptor retDesc;
    const bool Is3D = (in_Desc.dim_count > 2);
    
    retDesc.width = (size_t) in_Desc.dimensions.dim[0];
    retDesc.height = (size_t) in_Desc.dimensions.dim[1];
    retDesc.depth = (size_t) (Is3D ? in_Desc.dimensions.dim[1] : 0);
    retDesc.rowPitch = (size_t) in_Desc.pitch[0];
    retDesc.slicePitch = (size_t) (Is3D ? in_Desc.pitch[1] : 0);
    
    out_ImageFmt.image_channel_data_type = 
        ConvertChannelDataTypeFromIntelOCLToCL(in_Desc.format.image_channel_data_type);
    out_ImageFmt.image_channel_order = 
        ConvertChannelOrderFromIntelOCLToCL(in_Desc.format.image_channel_order);
    retDesc.format = &out_ImageFmt;
    return retDesc;
}

Conformance::image_sampler_data CreateSamplerData(const uint32_t& in_sampler)
{
    Conformance::image_sampler_data retSampler;
    cl_addressing_mode dst_adr;
    switch(in_sampler & __ADDRESS_MASK)
    {
    case CL_DEV_SAMPLER_ADDRESS_NONE:
        dst_adr = CL_ADDRESS_NONE; break;
    case CL_DEV_SAMPLER_ADDRESS_CLAMP:
        dst_adr = CL_ADDRESS_CLAMP; break;
    case CL_DEV_SAMPLER_ADDRESS_CLAMP_TO_EDGE:
        dst_adr = CL_ADDRESS_CLAMP_TO_EDGE; break;
    case CL_DEV_SAMPLER_ADDRESS_REPEAT:
        dst_adr = CL_ADDRESS_REPEAT; break;
    case CL_DEV_SAMPLER_ADDRESS_MIRRORED_REPEAT:
        dst_adr = CL_ADDRESS_MIRRORED_REPEAT; break;
    default:
        throw Exception::InvalidArgument("Not supported sampler addressing mode");
        break;
    }
    
    cl_filter_mode dst_filt;
    switch(in_sampler & __FILTER_MASK)
    {
    case CL_DEV_SAMPLER_FILTER_NEAREST:
        dst_filt = CL_FILTER_NEAREST; break;
    case CL_DEV_SAMPLER_FILTER_LINEAR:
        dst_filt = CL_FILTER_LINEAR; break;
    default:
        throw Exception::InvalidArgument("Not supported sampler filter type");
    }

    retSampler.addressing_mode = dst_adr;
    retSampler.filter_mode = dst_filt;
    retSampler.normalized_coords = (in_sampler & __NORMALIZED_MASK);
    return retSampler;
}

GenericValue lle_X_get_image_dim2(const FunctionType *FT,
                                  const std::vector<GenericValue> &Args) 
{

    GenericValue gv;
    cl_mem_obj_descriptor * memobj = (cl_mem_obj_descriptor *)Args[0].PointerVal;
    const uint32_t width = (uint32_t)memobj->dimensions.dim[0];
    const uint32_t height = (uint32_t)memobj->dimensions.dim[1];
    
    gv.AggregateVal.resize(2);
    gv.AggregateVal[0].IntVal = APInt(32, width);
    gv.AggregateVal[1].IntVal = APInt(32, height);

    return gv;
}

GenericValue lle_X_get_image_width(const FunctionType *FT,
                                   const std::vector<GenericValue> &Args) 
{

    GenericValue gv;
    cl_mem_obj_descriptor * memobj = (cl_mem_obj_descriptor *)Args[0].PointerVal;
    const uint32_t width = (uint32_t)memobj->dimensions.dim[0];
    gv.IntVal = APInt(32, width);
    return gv;
}

GenericValue lle_X_get_image_height(const FunctionType *FT,
                                    const std::vector<GenericValue> &Args) 
{

    GenericValue gv;
    cl_mem_obj_descriptor * memobj = (cl_mem_obj_descriptor *)Args[0].PointerVal;
    const uint32_t height = (uint32_t)memobj->dimensions.dim[1];
    gv.IntVal = APInt(32, height);
    return gv;
}

GenericValue lle_X_get_image_depth(const FunctionType *FT,
                                    const std::vector<GenericValue> &Args) 
{

    GenericValue gv;
    cl_mem_obj_descriptor * memobj = (cl_mem_obj_descriptor *)Args[0].PointerVal;
    const uint32_t depth = (uint32_t)memobj->dimensions.dim[2];
    gv.IntVal = APInt(32, depth);
    return gv;
}

GenericValue lle_X_get_image_dim3(const FunctionType *FT,
                                  const std::vector<GenericValue> &Args) 
{

    GenericValue gv;
    cl_mem_obj_descriptor * memobj = (cl_mem_obj_descriptor *)Args[0].PointerVal;
    const uint32_t width = (uint32_t)memobj->dimensions.dim[0];
    const uint32_t height = (uint32_t)memobj->dimensions.dim[1];
    const uint32_t depth = (uint32_t)memobj->dimensions.dim[2];
    
    gv.AggregateVal.resize(4);
    gv.AggregateVal[0].IntVal = APInt(32, width);
    gv.AggregateVal[1].IntVal = APInt(32, height);
    gv.AggregateVal[2].IntVal = APInt(32, depth);
    gv.AggregateVal[3].IntVal = APInt(32, 0);

    return gv;
}

GenericValue lle_X_get_image_channel_data_type(const FunctionType *FT,
                                  const std::vector<GenericValue> &Args) 
{
    GenericValue gv;
    cl_mem_obj_descriptor * memobj = (cl_mem_obj_descriptor *)Args[0].PointerVal;
    const int32_t image_channel_data_type = memobj->format.image_channel_data_type;
    gv.IntVal = APInt( 32, image_channel_data_type );
    return gv;
}

GenericValue lle_X_get_image_channel_order(const FunctionType *FT,
                                               const std::vector<GenericValue> &Args) 
{
    GenericValue gv;
    cl_mem_obj_descriptor * memobj = (cl_mem_obj_descriptor *)Args[0].PointerVal;
    const int32_t image_channel_order = memobj->format.image_channel_order;
    gv.IntVal = APInt( 32, image_channel_order );
    return gv;
}

llvm::GenericValue lle_X_write_image( const llvm::FunctionType *FT, 
                                     const std::vector<llvm::GenericValue> &Args )
{
    GenericValue gv;
    cl_mem_obj_descriptor * memobj = (cl_mem_obj_descriptor *)Args[0].PointerVal;
    
    cl_image_format im_fmt;
    Conformance::image_descriptor desc = CreateConfImageDesc(*memobj, im_fmt);

    // coordinates
    const GenericValue& CoordGV = Args[1];
    const int32_t u = getVal<uint32_t, 4>(CoordGV, 0);
    const int32_t v = getVal<uint32_t, 4>(CoordGV, 1);

    // datatype of pixel 
    const GenericValue& PixelGV = Args[2];
    const llvm::Type::TypeID PixelTy= FT->getParamType(2)->getContainedType(0)->getTypeID();

    if (PixelTy == llvm::Type::FloatTyID)
    {   // pixel data type is float
        float val[4] = 
                       {PixelGV.AggregateVal[0].FloatVal, 
                        PixelGV.AggregateVal[1].FloatVal,
                        PixelGV.AggregateVal[2].FloatVal, 
                        PixelGV.AggregateVal[3].FloatVal };
        write_image_pixel_float(memobj->pData, &desc, u, v, val);    
    }
    else if(PixelTy == llvm::Type::IntegerTyID && CoordGV.AggregateVal[0].IntVal.isSignedIntN(32))
    {   // pixel is signed int
        int32_t val[4] = 
        {   PixelGV.AggregateVal[0].IntVal.getSExtValue(), 
            PixelGV.AggregateVal[1].IntVal.getSExtValue(),
            PixelGV.AggregateVal[2].IntVal.getSExtValue(), 
            PixelGV.AggregateVal[3].IntVal.getSExtValue()};

        write_image_pixel_int(memobj->pData, &desc, u, v, val);    
    }
    else if(PixelTy == llvm::Type::IntegerTyID && !CoordGV.AggregateVal[0].IntVal.isSignedIntN(32))
    {   // pixel is unsigned int
        uint32_t val[4] = 
        {   PixelGV.AggregateVal[0].IntVal.getZExtValue(), 
            PixelGV.AggregateVal[1].IntVal.getZExtValue(),
            PixelGV.AggregateVal[2].IntVal.getZExtValue(), 
            PixelGV.AggregateVal[3].IntVal.getZExtValue()};
        write_image_pixel_uint(memobj->pData, &desc, u, v, val);    
    }
    else throw Exception::InvalidArgument("lle_X_write_image::Invalid data type of pixel data");

    return gv;

}

} // namespace OCLBuiltins
} // namespace Validation
