// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "ImageCallbackServices.h"
#include "ImageCallbackManager.h"
#include "cl_types.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {


#define READ_WRITE_FORMATS \
    {CL_RGBA, CL_UNORM_INT8},\
    {CL_RGBA, CL_UNORM_INT16},\
    {CL_RGBA, CL_SNORM_INT8},\
    {CL_RGBA, CL_SNORM_INT16},\
    {CL_RGBA, CL_SIGNED_INT8},\
    {CL_RGBA, CL_SIGNED_INT16},\
    {CL_RGBA, CL_SIGNED_INT32},\
    {CL_RGBA, CL_UNSIGNED_INT8},\
    {CL_RGBA, CL_UNSIGNED_INT16},\
    {CL_RGBA, CL_UNSIGNED_INT32},\
    {CL_RGBA, CL_HALF_FLOAT},\
    {CL_RGBA, CL_FLOAT},\
    \
    {CL_BGRA,   CL_UNORM_INT8},\
    \
    {CL_INTENSITY,  CL_FLOAT},\
    {CL_INTENSITY,  CL_UNORM_INT8},\
    {CL_INTENSITY,  CL_UNORM_INT16},\
    {CL_INTENSITY,  CL_HALF_FLOAT},\
    \
    {CL_LUMINANCE,  CL_FLOAT},\
    {CL_LUMINANCE,  CL_UNORM_INT8},\
    {CL_LUMINANCE,  CL_UNORM_INT16},\
    {CL_LUMINANCE,  CL_HALF_FLOAT},\
    \
    \
    {CL_R,      CL_FLOAT},\
    {CL_R,      CL_UNORM_INT8},\
    {CL_R,      CL_UNORM_INT16},\
    {CL_R,      CL_SNORM_INT8},\
    {CL_R,      CL_SNORM_INT16},\
    {CL_R,      CL_SIGNED_INT8},\
    {CL_R,      CL_SIGNED_INT16},\
    {CL_R,      CL_SIGNED_INT32},\
    {CL_R,      CL_UNSIGNED_INT8},\
    {CL_R,      CL_UNSIGNED_INT16},\
    {CL_R,      CL_UNSIGNED_INT32},\
    {CL_R,      CL_HALF_FLOAT},\
    \
    {CL_A,      CL_UNORM_INT8},\
    {CL_A,      CL_UNORM_INT16},\
    {CL_A,      CL_HALF_FLOAT},\
    {CL_A,      CL_FLOAT},\
    \
    {CL_RG,     CL_UNORM_INT8},\
    {CL_RG,     CL_UNORM_INT16},\
    {CL_RG,     CL_SNORM_INT8},\
    {CL_RG,     CL_SNORM_INT16},\
    {CL_RG,     CL_SIGNED_INT16},\
    {CL_RG,     CL_SIGNED_INT32},\
    {CL_RG,     CL_SIGNED_INT8},\
    {CL_RG,     CL_UNSIGNED_INT8},\
    {CL_RG,     CL_UNSIGNED_INT16},\
    {CL_RG,     CL_UNSIGNED_INT32},\
    {CL_RG,     CL_HALF_FLOAT},\
    {CL_RG,     CL_FLOAT}

#define READ_ONLY_FORMATS\
    {CL_sRGBA,  CL_UNORM_INT8},\
    {CL_sBGRA,  CL_UNORM_INT8}

#define ONLY_2D_FORMATS\
    {CL_DEPTH,  CL_FLOAT},\
    {CL_DEPTH,  CL_UNORM_INT16}

// supportedRWImageFormats contains readable and writable image formats
const cl_image_format supportedRWImageFormats1D3D[] = { READ_WRITE_FORMATS };
const cl_image_format supportedRWImageFormats2D[] = { READ_WRITE_FORMATS, ONLY_2D_FORMATS };

// supportedROImageFormats also contains not writable image formats
const cl_image_format supportedROImageFormats1D3D[] = { READ_WRITE_FORMATS, READ_ONLY_FORMATS };
const cl_image_format supportedROImageFormats2D[] = { READ_WRITE_FORMATS, READ_ONLY_FORMATS, ONLY_2D_FORMATS };

ImageCallbackService::ImageCallbackService(const CompilerConfig& config, bool isCpu)
{
  ImageCallbackManager::GetInstance()->InitLibrary(config, isCpu, m_CpuId);
}

const cl_image_format* ImageCallbackService::GetSupportedImageFormats(unsigned int *numFormats, cl_mem_object_type imageType, cl_mem_flags flags){

    cl_image_format const* ret = nullptr;
    // leave only memory access flags
    flags &= (CL_MEM_WRITE_ONLY | CL_MEM_READ_ONLY | CL_MEM_READ_WRITE | CL_MEM_KERNEL_READ_AND_WRITE);
    // If value specified for flags is 0, the default is used which
    // is CL_MEM_READ_WRITE.
    flags = flags ? flags : CL_MEM_READ_WRITE;

    switch(flags) {
      case CL_MEM_WRITE_ONLY:
      case CL_MEM_READ_WRITE:
      case CL_MEM_KERNEL_READ_AND_WRITE:
      {
        if(imageType == CL_MEM_OBJECT_IMAGE2D ||
           imageType == CL_MEM_OBJECT_IMAGE2D_ARRAY) {
          ret = supportedRWImageFormats2D;
          *numFormats=ARRAY_SIZE(supportedRWImageFormats2D);
        } else {
          ret = supportedRWImageFormats1D3D;
          *numFormats=ARRAY_SIZE(supportedRWImageFormats1D3D);
        }
        break;
      }
      case CL_MEM_READ_ONLY:
      {
        if(imageType == CL_MEM_OBJECT_IMAGE2D ||
           imageType == CL_MEM_OBJECT_IMAGE2D_ARRAY) {
          ret = supportedROImageFormats2D;
          *numFormats=ARRAY_SIZE(supportedROImageFormats2D);
        } else {
          ret = supportedROImageFormats1D3D;
          *numFormats=ARRAY_SIZE(supportedROImageFormats1D3D);
        }
        break;
      }
      default:
        assert(false && "unsupported memory flags");
    }
    return ret;
}

bool IsSOASupported(int dt)
{
    switch(dt)
    {
    case CLK_UNSIGNED_INT8:
    case CLK_UNSIGNED_INT16:
    case CLK_UNSIGNED_INT32:
        return true;
    case CLK_SIGNED_INT8:
    case CLK_SIGNED_INT16:
    case CLK_SIGNED_INT32:
    case CLK_SNORM_INT8:
    case CLK_SNORM_INT16:
    case CLK_UNORM_INT8:
    case CLK_UNORM_INT16:
    case CLK_UNORM_SHORT_565:
    case CLK_UNORM_SHORT_555:
    case CLK_UNORM_INT_101010:
    case CLK_HALF_FLOAT:
    case CLK_FLOAT:
        return false;
    default:
        throw Exceptions::DeviceBackendExceptionBase(std::string("Unkown channel type"));
    }
}

bool IsIntDataType(int dt)
{
    switch(dt)
    {
    case CLK_SIGNED_INT8:
    case CLK_SIGNED_INT16:
    case CLK_SIGNED_INT32:
    case CLK_UNSIGNED_INT8:
    case CLK_UNSIGNED_INT16:
    case CLK_UNSIGNED_INT32:
        return true;
    case CLK_SNORM_INT8:
    case CLK_SNORM_INT16:
    case CLK_UNORM_INT8:
    case CLK_UNORM_INT16:
    case CLK_UNORM_SHORT_565:
    case CLK_UNORM_SHORT_555:
    case CLK_UNORM_INT_101010:
    case CLK_HALF_FLOAT:
    case CLK_FLOAT:
        return false;
    default:
        throw Exceptions::DeviceBackendExceptionBase(std::string("Unkown channel type"));
    }
}

// returns true if specified object is image array. False otherwise
bool IsImageArray(cl_mem_obj_descriptor* pImageObject)
{
    if( (pImageObject->memObjType == CL_MEM_OBJECT_IMAGE2D_ARRAY) ||
        (pImageObject->memObjType == CL_MEM_OBJECT_IMAGE1D_ARRAY) )
    {
        return true;
    }
    else
    {
        return false;
    }
}

void ImageCallbackService::InitializeToTrap(void*& ptr) const
{
    void*const trapFunction =
        ImageCallbackManager::GetInstance()->getCallbackFunctions(m_CpuId)
        ->GetTrapCbk();
    ptr = trapFunction;
}

void ImageCallbackService::InitializeToTrap(void* arr[], size_t size) const
{
    void*const trapFunction =
        ImageCallbackManager::GetInstance()->getCallbackFunctions(m_CpuId)
        ->GetTrapCbk();
    for(size_t i = 0 ; i < size ; ++i)
        arr[i] = trapFunction;
}

cl_dev_err_code ImageCallbackService::CreateImageObject(cl_mem_obj_descriptor* pImageObject, void* auxObject) const
{
    // make sure that object passed here is image. Otherwise
    if (pImageObject->memObjType == CL_MEM_OBJECT_BUFFER){
      pImageObject->imageAuxData = nullptr;
      return CL_DEV_ERROR_FAIL;
    }

    pImageObject->imageAuxData = (image_aux_data*)auxObject;
    image_aux_data* pImageAuxData = (image_aux_data*)pImageObject->imageAuxData;  //using this pointer for the clarity of the code
    pImageAuxData->pData = pImageObject->pData;
    pImageAuxData->dim_count = pImageObject->dim_count;
    pImageAuxData->format = pImageObject->format;
    pImageAuxData->uiElementSize = pImageObject->uiElementSize;

#define ARRAY_AND_SIZE(ARR) ARR, sizeof(ARR)/sizeof(ARR[0])
    // Initializing all callback functions to trap.
    InitializeToTrap(ARRAY_AND_SIZE(pImageAuxData->coord_translate_f_callback));
    InitializeToTrap(ARRAY_AND_SIZE(pImageAuxData->read_img_callback_int));
    InitializeToTrap(ARRAY_AND_SIZE(pImageAuxData->read_img_callback_float));
    InitializeToTrap(ARRAY_AND_SIZE(pImageAuxData->soa4_read_img_callback_int));
    InitializeToTrap(ARRAY_AND_SIZE(pImageAuxData->soa8_read_img_callback_int));
    InitializeToTrap(ARRAY_AND_SIZE(pImageAuxData->soa16_read_img_callback_int));

    InitializeToTrap(pImageAuxData->write_img_callback);
    InitializeToTrap(pImageAuxData->soa4_write_img_callback);
    InitializeToTrap(pImageAuxData->soa8_write_img_callback);
    InitializeToTrap(pImageAuxData->soa16_write_img_callback);

    // workaround for image array
    if(IsImageArray(pImageObject)){
        pImageAuxData->array_size = pImageObject->dimensions.dim[pImageObject->dim_count - 1];
        pImageAuxData->dim_count--;
    }
    else
        pImageAuxData->array_size = -1;

#define SET_IF_NOT_NULL(DST_PTR, SRC_PTR) if (SRC_PTR) DST_PTR = SRC_PTR

    //supplementing additional data
    cl_channel_type ch_type  = pImageAuxData->format.image_channel_data_type;
    cl_channel_type ch_order = pImageAuxData->format.image_channel_order;
    memset(pImageAuxData->dim,0,sizeof(pImageAuxData->dim));
    memset(pImageAuxData->offset,0,sizeof(pImageAuxData->offset));
    memset(pImageAuxData->pitch, 0, sizeof(pImageAuxData->pitch));
    memset(pImageAuxData->dimSub1,0,sizeof(pImageAuxData->dimSub1));
    memset(pImageAuxData->dimf,0,sizeof(pImageAuxData->dimf));

    switch (pImageAuxData->dim_count) {
      default :
        // Can't be anything except 1, 2 or 3.
        break;
      case 3:
        pImageAuxData->dim[2]     = pImageObject->dimensions.dim[2];
        pImageAuxData->dimSub1[2] = pImageAuxData->dim[2] - 1;
        pImageAuxData->dimf[2]    = pImageAuxData->dim[2];
      case 2:
        pImageAuxData->dim[1]     = pImageObject->dimensions.dim[1];
        pImageAuxData->pitch[1]   = pImageObject->pitch[1];
        pImageAuxData->dimSub1[1] = pImageAuxData->dim[1] - 1;
        pImageAuxData->dimf[1]    = pImageAuxData->dim[1];
      case 1:
        pImageAuxData->dim[0]     = pImageObject->dimensions.dim[0];
        pImageAuxData->pitch[0]   = pImageObject->pitch[0];
        pImageAuxData->dimSub1[0] = pImageAuxData->dim[0] - 1;
        pImageAuxData->dimf[0]    = pImageAuxData->dim[0];
    }

    // Offset represents offset in byte within a dimension used
    // to compute pixel pointer in image access routines
    // For image arrays it represents offset within the image

    pImageAuxData->offset[0] = pImageAuxData->uiElementSize;
    if(pImageObject->memObjType != CL_MEM_OBJECT_IMAGE1D_ARRAY && pImageObject->memObjType != CL_MEM_OBJECT_IMAGE1D)
        pImageAuxData->offset[1] = pImageAuxData->pitch[0];

    if (pImageObject->memObjType != CL_MEM_OBJECT_IMAGE2D_ARRAY)
        pImageAuxData->offset[2] = pImageAuxData->pitch[1];

    pImageAuxData->dimmask = (1<<(pImageAuxData->dim_count*4))-1;

    ImageCallbackFunctions* pImageCallbackFuncs = ImageCallbackManager::GetInstance()->getCallbackFunctions(m_CpuId);

    for(unsigned int i = 0; i<ARRAY_SIZE(pImageAuxData->read_img_callback_int); i++)
    {
        pImageAuxData->read_img_callback_float[i] = pImageCallbackFuncs->GetUndefinedCbk(READ_CBK_UNDEF_FLOAT);
        pImageAuxData->read_img_callback_int[i]   = pImageCallbackFuncs->GetUndefinedCbk(READ_CBK_UNDEF_INT);
        pImageAuxData->soa4_read_img_callback_int[i]   = pImageCallbackFuncs->GetUndefinedCbk(READ_CBK_UNDEF_INT, SOA4);
        pImageAuxData->soa8_read_img_callback_int[i]   = pImageCallbackFuncs->GetUndefinedCbk(READ_CBK_UNDEF_INT, SOA8);
        pImageAuxData->soa16_read_img_callback_int[i]   = pImageCallbackFuncs->GetUndefinedCbk(READ_CBK_UNDEF_INT, SOA16);
    }

    if(IsIntDataType(pImageAuxData->format.image_channel_data_type))
    {
        // for integer images and float coordinates
        // nearest filter, and false normalized
        pImageAuxData->coord_translate_f_callback[NONE_FALSE_NEAREST] = pImageCallbackFuncs->GetTranslationCbk(INT_CBK, NONE_FALSE_NEAREST);
        pImageAuxData->coord_translate_f_callback[CLAMP_FALSE_NEAREST] = pImageCallbackFuncs->GetTranslationCbk(INT_CBK, NONE_FALSE_NEAREST);
        pImageAuxData->coord_translate_f_callback[CLAMPTOEDGE_FALSE_NEAREST] = pImageCallbackFuncs->GetTranslationCbk(INT_CBK, CLAMPTOEDGE_FALSE_NEAREST);

        //nearest filter, and true normalized
        pImageAuxData->coord_translate_f_callback[NONE_TRUE_NEAREST] = pImageCallbackFuncs->GetTranslationCbk(INT_CBK, NONE_TRUE_NEAREST);
        pImageAuxData->coord_translate_f_callback[CLAMP_TRUE_NEAREST] = pImageCallbackFuncs->GetTranslationCbk(INT_CBK, NONE_TRUE_NEAREST);
        pImageAuxData->coord_translate_f_callback[CLAMPTOEDGE_TRUE_NEAREST] = pImageCallbackFuncs->GetTranslationCbk(INT_CBK, CLAMPTOEDGE_TRUE_NEAREST);
        pImageAuxData->coord_translate_f_callback[REPEAT_TRUE_NEAREST] = pImageCallbackFuncs->GetTranslationCbk(INT_CBK, REPEAT_TRUE_NEAREST);
        pImageAuxData->coord_translate_f_callback[MIRRORED_TRUE_NEAREST] = pImageCallbackFuncs->GetTranslationCbk(INT_CBK, MIRRORED_TRUE_NEAREST);
    } else {   //float and unorm images
        // nearest filter, and false normalized
        pImageAuxData->coord_translate_f_callback[NONE_FALSE_NEAREST] = pImageCallbackFuncs->GetTranslationCbk(FLT_CBK, NONE_FALSE_NEAREST);
        pImageAuxData->coord_translate_f_callback[CLAMP_FALSE_NEAREST] = pImageCallbackFuncs->GetTranslationCbk(FLT_CBK, NONE_FALSE_NEAREST);
        pImageAuxData->coord_translate_f_callback[CLAMPTOEDGE_FALSE_NEAREST] = pImageCallbackFuncs->GetTranslationCbk(FLT_CBK, CLAMPTOEDGE_FALSE_NEAREST);

        //nearest filter, and true normalized
        pImageAuxData->coord_translate_f_callback[NONE_TRUE_NEAREST] = pImageCallbackFuncs->GetTranslationCbk( FLT_CBK, NONE_TRUE_NEAREST);//pImageCallbackFuncs->GetFNoneTrueNearest();
        pImageAuxData->coord_translate_f_callback[CLAMP_TRUE_NEAREST] = pImageCallbackFuncs->GetTranslationCbk( FLT_CBK, NONE_TRUE_NEAREST);//pImageCallbackFuncs->GetFNoneTrueNearest();
        pImageAuxData->coord_translate_f_callback[CLAMPTOEDGE_TRUE_NEAREST] = pImageCallbackFuncs->GetTranslationCbk( FLT_CBK, CLAMPTOEDGE_TRUE_NEAREST);//pImageCallbackFuncs->GetFClampToEdgeTrueNearest();
        pImageAuxData->coord_translate_f_callback[REPEAT_TRUE_NEAREST] = pImageCallbackFuncs->GetTranslationCbk( FLT_CBK, REPEAT_TRUE_NEAREST);//pImageCallbackFuncs->GetFRepeatTrueNearest();
        pImageAuxData->coord_translate_f_callback[MIRRORED_TRUE_NEAREST] = pImageCallbackFuncs->GetTranslationCbk( FLT_CBK, MIRRORED_TRUE_NEAREST);//pImageCallbackFuncs->GetFMirroredTrueNearest();
    }

    static const unsigned int nearestSamplers[] =
    {
        NONE_FALSE_NEAREST,
        CLAMP_FALSE_NEAREST,
        CLAMPTOEDGE_FALSE_NEAREST,
        REPEAT_FALSE_NEAREST,
        MIRRORED_FALSE_NEAREST,
        NONE_TRUE_NEAREST,
        CLAMP_TRUE_NEAREST,
        CLAMPTOEDGE_TRUE_NEAREST,
        REPEAT_TRUE_NEAREST,
        MIRRORED_TRUE_NEAREST
    };

    static const unsigned int linearSamplers[] =
    {
        NONE_FALSE_LINEAR,
        CLAMP_FALSE_LINEAR,
        CLAMPTOEDGE_FALSE_LINEAR,
        REPEAT_FALSE_LINEAR,
        MIRRORED_FALSE_LINEAR,
        NONE_TRUE_LINEAR,
        CLAMP_TRUE_LINEAR,
        CLAMPTOEDGE_TRUE_LINEAR,
        REPEAT_TRUE_LINEAR,
        MIRRORED_TRUE_LINEAR
    };

    //bilinear filter is undefined for un/signed integer types
    if (IsIntDataType(pImageAuxData->format.image_channel_data_type)){
        for(unsigned int i = 0; i<ARRAY_SIZE(linearSamplers); i++)
            pImageAuxData->coord_translate_f_callback[linearSamplers[i]] = pImageCallbackFuncs->GetUndefinedCbk(TRANS_CBK_UNDEF_FLOAT);
    } else {
        // linear filter, and false normalized
        pImageAuxData->coord_translate_f_callback[NONE_FALSE_LINEAR] = pImageCallbackFuncs->GetTranslationCbk(FLT_CBK, NONE_FALSE_LINEAR);
        pImageAuxData->coord_translate_f_callback[CLAMP_FALSE_LINEAR] = pImageCallbackFuncs->GetTranslationCbk(FLT_CBK, NONE_FALSE_LINEAR);
        pImageAuxData->coord_translate_f_callback[CLAMPTOEDGE_FALSE_LINEAR] = pImageCallbackFuncs->GetTranslationCbk(FLT_CBK, CLAMPTOEDGE_FALSE_LINEAR);
        pImageAuxData->coord_translate_f_callback[REPEAT_FALSE_LINEAR] = pImageCallbackFuncs->GetUndefinedCbk(TRANS_CBK_UNDEF_FLOAT_FLOAT);
        pImageAuxData->coord_translate_f_callback[MIRRORED_FALSE_LINEAR] = pImageCallbackFuncs->GetUndefinedCbk(TRANS_CBK_UNDEF_FLOAT_FLOAT);

        //linear filter, and true normalized
        pImageAuxData->coord_translate_f_callback[NONE_TRUE_LINEAR] = pImageCallbackFuncs->GetTranslationCbk( FLT_CBK, NONE_TRUE_LINEAR);//pImageCallbackFuncs->GetFNoneTrueNearest();
        pImageAuxData->coord_translate_f_callback[CLAMP_TRUE_LINEAR] = pImageCallbackFuncs->GetTranslationCbk( FLT_CBK, NONE_TRUE_LINEAR);//pImageCallbackFuncs->GetFNoneTrueNearest();
        pImageAuxData->coord_translate_f_callback[CLAMPTOEDGE_TRUE_LINEAR] = pImageCallbackFuncs->GetTranslationCbk( FLT_CBK, CLAMPTOEDGE_TRUE_LINEAR);//pImageCallbackFuncs->GetFClampToEdgeTrueNearest();
        pImageAuxData->coord_translate_f_callback[REPEAT_TRUE_LINEAR] = pImageCallbackFuncs->GetTranslationCbk( FLT_CBK, REPEAT_TRUE_LINEAR);//pImageCallbackFuncs->GetFRepeatTrueNearest();
        pImageAuxData->coord_translate_f_callback[MIRRORED_TRUE_LINEAR] = pImageCallbackFuncs->GetTranslationCbk( FLT_CBK, MIRRORED_TRUE_LINEAR);//pImageCallbackFuncs->GetFMirroredTrueNearest();
    }

    /////////////////////////////Read & write image callbacks
    void** read_cbk_ptr = nullptr;
    if(IsIntDataType(pImageAuxData->format.image_channel_data_type))
        read_cbk_ptr = pImageAuxData->read_img_callback_int;
    else
        read_cbk_ptr = pImageAuxData->read_img_callback_float;

    for(unsigned int i = 0; i<ARRAY_SIZE(nearestSamplers); i++)
    {
        read_cbk_ptr[nearestSamplers[i]] = pImageCallbackFuncs->GetReadingCbk(NO_CLAMP_CBK, ch_order, ch_type, CL_FILTER_NEAREST);
        // Currently we have SOA implementation only for integer images
        if(IsSOASupported(pImageAuxData->format.image_channel_data_type))
        {
            void* cbFunc = pImageCallbackFuncs->GetReadingCbk(NO_CLAMP_CBK, ch_order, ch_type, CL_FILTER_NEAREST, CL_MEM_OBJECT_IMAGE2D, SOA4);
            SET_IF_NOT_NULL(pImageAuxData->soa4_read_img_callback_int[nearestSamplers[i]], cbFunc);
            cbFunc = pImageCallbackFuncs->GetReadingCbk(NO_CLAMP_CBK, ch_order, ch_type, CL_FILTER_NEAREST, CL_MEM_OBJECT_IMAGE2D, SOA8);
            SET_IF_NOT_NULL(pImageAuxData->soa8_read_img_callback_int[nearestSamplers[i]], cbFunc);
            cbFunc = pImageCallbackFuncs->GetReadingCbk(NO_CLAMP_CBK, ch_order, ch_type, CL_FILTER_NEAREST, CL_MEM_OBJECT_IMAGE2D, SOA16);
            SET_IF_NOT_NULL(pImageAuxData->soa16_read_img_callback_int[nearestSamplers[i]], cbFunc);
        }
    }

    if( !IsIntDataType(pImageAuxData->format.image_channel_data_type))
    {
        if(pImageAuxData->dim_count == 1)
        {
            for(unsigned int i = 0; i<ARRAY_SIZE(linearSamplers); i++)
                read_cbk_ptr[linearSamplers[i]] = pImageCallbackFuncs->GetReadingCbk(NO_CLAMP_CBK, ch_order, ch_type, CL_FILTER_LINEAR, CL_MEM_OBJECT_IMAGE1D);
            read_cbk_ptr[CLAMP_FALSE_LINEAR] = pImageCallbackFuncs->GetReadingCbk(CLAMP_CBK, ch_order, ch_type, CL_FILTER_LINEAR, CL_MEM_OBJECT_IMAGE1D);
            read_cbk_ptr[CLAMP_TRUE_LINEAR]  = pImageCallbackFuncs->GetReadingCbk(CLAMP_CBK, ch_order, ch_type, CL_FILTER_LINEAR, CL_MEM_OBJECT_IMAGE1D);
        } else if(pImageAuxData->dim_count == 2)
        {
            for(unsigned int i = 0; i<ARRAY_SIZE(linearSamplers); i++)
                read_cbk_ptr[linearSamplers[i]] = pImageCallbackFuncs->GetReadingCbk(NO_CLAMP_CBK, ch_order, ch_type, CL_FILTER_LINEAR, CL_MEM_OBJECT_IMAGE2D);
            read_cbk_ptr[CLAMP_FALSE_LINEAR] = pImageCallbackFuncs->GetReadingCbk(CLAMP_CBK, ch_order, ch_type, CL_FILTER_LINEAR, CL_MEM_OBJECT_IMAGE2D);
            read_cbk_ptr[CLAMP_TRUE_LINEAR]  = pImageCallbackFuncs->GetReadingCbk(CLAMP_CBK, ch_order, ch_type, CL_FILTER_LINEAR, CL_MEM_OBJECT_IMAGE2D);
        }
        else
        {
            for(unsigned int i = 0; i<ARRAY_SIZE(linearSamplers); i++)
                read_cbk_ptr[linearSamplers[i]] = pImageCallbackFuncs->GetReadingCbk(NO_CLAMP_CBK, ch_order, ch_type, CL_FILTER_LINEAR, CL_MEM_OBJECT_IMAGE3D);
            read_cbk_ptr[CLAMP_FALSE_LINEAR] = pImageCallbackFuncs->GetReadingCbk(CLAMP_CBK, ch_order, ch_type, CL_FILTER_LINEAR, CL_MEM_OBJECT_IMAGE3D);
            read_cbk_ptr[CLAMP_TRUE_LINEAR]  = pImageCallbackFuncs->GetReadingCbk(CLAMP_CBK, ch_order, ch_type, CL_FILTER_LINEAR, CL_MEM_OBJECT_IMAGE3D);
        }
    }
    if(IsSOASupported(pImageAuxData->format.image_channel_data_type))
    {
        void* cbFunc = pImageCallbackFuncs->GetReadingCbk(CLAMP_CBK, ch_order, ch_type, CL_FILTER_NEAREST, CL_MEM_OBJECT_IMAGE2D, SOA4);
        SET_IF_NOT_NULL(pImageAuxData->soa4_read_img_callback_int[CLAMP_FALSE_NEAREST], cbFunc);
        cbFunc = pImageCallbackFuncs->GetReadingCbk(CLAMP_CBK, ch_order, ch_type, CL_FILTER_NEAREST, CL_MEM_OBJECT_IMAGE2D, SOA8);
        SET_IF_NOT_NULL(pImageAuxData->soa8_read_img_callback_int[CLAMP_FALSE_NEAREST], cbFunc);
        cbFunc = pImageCallbackFuncs->GetReadingCbk(CLAMP_CBK, ch_order, ch_type, CL_FILTER_NEAREST, CL_MEM_OBJECT_IMAGE2D, SOA16);
        SET_IF_NOT_NULL(pImageAuxData->soa16_read_img_callback_int[CLAMP_FALSE_NEAREST], cbFunc);
        cbFunc = pImageCallbackFuncs->GetWritingCbk(ch_order, ch_type, SOA4);
        SET_IF_NOT_NULL(pImageAuxData->soa4_write_img_callback, cbFunc);
        cbFunc = pImageCallbackFuncs->GetWritingCbk(ch_order, ch_type, SOA8);
        SET_IF_NOT_NULL(pImageAuxData->soa8_write_img_callback, cbFunc);
        cbFunc = pImageCallbackFuncs->GetWritingCbk(ch_order, ch_type, SOA16);
        SET_IF_NOT_NULL(pImageAuxData->soa16_write_img_callback, cbFunc);
    }

    read_cbk_ptr[CLAMP_FALSE_NEAREST] = pImageCallbackFuncs->GetReadingCbk(CLAMP_CBK, ch_order, ch_type, CL_FILTER_NEAREST);
    read_cbk_ptr[CLAMP_TRUE_NEAREST] = pImageCallbackFuncs->GetReadingCbk(CLAMP_CBK, ch_order, ch_type, CL_FILTER_NEAREST);

    pImageAuxData->write_img_callback = pImageCallbackFuncs->GetWritingCbk(ch_order, ch_type);

    return CL_DEV_SUCCESS;
}

cl_dev_err_code ImageCallbackService::DeleteImageObject(cl_mem_obj_descriptor* pImageObject, void** auxObject) const
{
  //this function does nothing meaningful in the meantime, because the allocation is done on the device...
  *auxObject=pImageObject->imageAuxData;

  return CL_DEV_SUCCESS;
}


void ImageCallbackService::Release()
{
    delete this;
}

}}}  //namespace
