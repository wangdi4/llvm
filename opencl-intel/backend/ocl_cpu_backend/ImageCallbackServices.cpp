#include "ImageCallbackServices.h"
#include "cl_types.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {


const cl_image_format supportedImageFormats[] = {
    // Minimum supported image formats
    // CL_RGBA
    {CL_RGBA, CL_UNORM_INT8},
    {CL_RGBA, CL_UNORM_INT16},
    {CL_RGBA, CL_SIGNED_INT8},
    {CL_RGBA, CL_SIGNED_INT16},
    {CL_RGBA, CL_SIGNED_INT32},
    {CL_RGBA, CL_UNSIGNED_INT8},
    {CL_RGBA, CL_UNSIGNED_INT16},
    {CL_RGBA, CL_UNSIGNED_INT32},
    {CL_RGBA, CL_HALF_FLOAT},
    {CL_RGBA, CL_FLOAT},

    // CL_BGRA
    {CL_BGRA,   CL_UNORM_INT8},

    // Additional formats required by users
    // CL_INTENCITY
    {CL_INTENSITY,  CL_FLOAT},
    {CL_INTENSITY,  CL_UNORM_INT8},
    {CL_INTENSITY,  CL_UNORM_INT16},
    {CL_INTENSITY,  CL_HALF_FLOAT},

    // CL_LUMINANCE
    {CL_LUMINANCE,  CL_FLOAT},
    {CL_LUMINANCE,  CL_UNORM_INT8},
    {CL_LUMINANCE,  CL_UNORM_INT16},
    {CL_LUMINANCE,  CL_HALF_FLOAT},


    // CL_R
    {CL_R,      CL_FLOAT},
    {CL_R,      CL_UNORM_INT8},
    {CL_R,      CL_UNORM_INT16},
    {CL_R,      CL_SIGNED_INT8},
    {CL_R,      CL_SIGNED_INT16},
    {CL_R,      CL_SIGNED_INT32},
    {CL_R,      CL_UNSIGNED_INT8},
    {CL_R,      CL_UNSIGNED_INT16},
    {CL_R,      CL_UNSIGNED_INT32},
    {CL_R,      CL_HALF_FLOAT},

    // CL_A
    {CL_A,      CL_UNORM_INT8},
    {CL_A,      CL_UNORM_INT16},
    {CL_A,      CL_HALF_FLOAT},
    {CL_A,      CL_FLOAT},

    // CL_RG
    {CL_RG,     CL_UNORM_INT8},
    {CL_RG,     CL_UNORM_INT16},
    {CL_RG,     CL_SIGNED_INT16},
    {CL_RG,     CL_SIGNED_INT32},
    {CL_RG,     CL_SIGNED_INT8},
    {CL_RG,     CL_UNSIGNED_INT8},
    {CL_RG,     CL_UNSIGNED_INT16},
    {CL_RG,     CL_UNSIGNED_INT32},
    {CL_RG,     CL_HALF_FLOAT},
    {CL_RG,     CL_FLOAT}

};

ImageCallbackService::ImageCallbackService(const CompilerConfig& config, bool isCpu)
{
  ImageCallbackManager::GetInstance()->InitLibrary(config, isCpu, m_CpuId);
}

const cl_image_format* ImageCallbackService::GetSupportedImageFormats(unsigned int *numFormats){
    *numFormats=ARRAY_SIZE(supportedImageFormats);
    return (&supportedImageFormats[0]);
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

cl_dev_err_code ImageCallbackService::CreateImageObject(cl_mem_obj_descriptor* pImageObject, void* auxObject) const
{
  // make sure that object passed here is image. Otherwise
    if (pImageObject->memObjType == CL_MEM_OBJECT_BUFFER){
      pImageObject->imageAuxData=NULL;
      return CL_DEV_ERROR_FAIL;
    }

    pImageObject->imageAuxData = (image_aux_data*)auxObject;
    image_aux_data* pImageAuxData = (image_aux_data*)pImageObject->imageAuxData;  //using this pointer for the clarity of the code 
    pImageAuxData->pData = pImageObject->pData;
    pImageAuxData->dim_count = pImageObject->dim_count;
    pImageAuxData->format = pImageObject->format;
    pImageAuxData->uiElementSize = pImageObject->uiElementSize;
    // workaround for image array
    if(IsImageArray(pImageObject)){
        pImageAuxData->array_size = pImageObject->dimensions.dim[pImageObject->dim_count - 1];
        pImageAuxData->dim_count--;
    }
    else
        pImageAuxData->array_size = -1;

#define IMG_SET_CALLBACK(CALLBACK, FUNCTION) CALLBACK = FUNCTION;
#define SET_IF_NOT_NULL(DST_PTR, SRC_PTR) if(SRC_PTR != NULL) DST_PTR = SRC_PTR;

    //supplementing additional data
    cl_channel_type ch_type  = pImageAuxData->format.image_channel_data_type;
    cl_channel_type ch_order = pImageAuxData->format.image_channel_order;
    memset(pImageAuxData->read_img_callback_int, 0, sizeof(pImageAuxData->read_img_callback_int));
    memset(pImageAuxData->read_img_callback_float, 0, sizeof(pImageAuxData->read_img_callback_float));
    memset(pImageAuxData->coord_translate_f_callback, 0, sizeof(pImageAuxData->coord_translate_f_callback));
    memset(pImageAuxData->dim,0,sizeof(pImageAuxData->dim));
    memset(pImageAuxData->offset,0,sizeof(pImageAuxData->offset));
    memset(pImageAuxData->pitch, 0, sizeof(pImageAuxData->pitch));
    memset(pImageAuxData->dimSub1,0,sizeof(pImageAuxData->dimSub1));
    memset(pImageAuxData->dimf,0,sizeof(pImageAuxData->dimf));
    for (unsigned int i=0;i<pImageAuxData->dim_count;i++){
        pImageAuxData->dim[i] = pImageObject->dimensions.dim[i];
        pImageAuxData->pitch[i] = pImageObject->pitch[i];
        pImageAuxData->dimSub1[i] = pImageAuxData->dim[i]-1;
        pImageAuxData->dimf[i] = pImageAuxData->dim[i];
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
    }

    if(IsIntDataType(pImageAuxData->format.image_channel_data_type))
    {
        // for integer images and float coordinates
        // nearest filter, and false normalized
        pImageAuxData->coord_translate_f_callback[NONE_FALSE_NEAREST] = pImageCallbackFuncs->GetTranslationCbk(INT_CBK, NONE_FALSE_NEAREST);
        pImageAuxData->coord_translate_f_callback[CLAMP_FALSE_NEAREST] = pImageCallbackFuncs->GetTranslationCbk(INT_CBK, NONE_FALSE_NEAREST);
        pImageAuxData->coord_translate_f_callback[CLAMPTOEDGE_FALSE_NEAREST] = pImageCallbackFuncs->GetTranslationCbk(INT_CBK, CLAMPTOEDGE_FALSE_NEAREST);

        //nearest filter, and true normalized
        pImageAuxData->coord_translate_f_callback[NONE_TRUE_NEAREST] = pImageCallbackFuncs->GetTranslationCbk( INT_CBK, NONE_TRUE_NEAREST);
        pImageAuxData->coord_translate_f_callback[CLAMP_TRUE_NEAREST] = pImageCallbackFuncs->GetTranslationCbk( INT_CBK, NONE_TRUE_NEAREST);
        pImageAuxData->coord_translate_f_callback[CLAMPTOEDGE_TRUE_NEAREST] = pImageCallbackFuncs->GetTranslationCbk( INT_CBK, CLAMPTOEDGE_TRUE_NEAREST);
        pImageAuxData->coord_translate_f_callback[REPEAT_TRUE_NEAREST] = pImageCallbackFuncs->GetTranslationCbk( INT_CBK, REPEAT_TRUE_NEAREST);
        pImageAuxData->coord_translate_f_callback[MIRRORED_TRUE_NEAREST] = pImageCallbackFuncs->GetTranslationCbk( INT_CBK, MIRRORED_TRUE_NEAREST);
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

    //bilinear filter is undefined for un/signed integer types

    if (IsIntDataType(pImageAuxData->format.image_channel_data_type)){
        for (unsigned int i=MIRRORED_TRUE_NEAREST+1;i<32;i++)
            pImageAuxData->coord_translate_f_callback[i] = pImageCallbackFuncs->GetUndefinedCbk(TRANS_CBK_UNDEF_FLOAT);
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
    void** read_cbk_ptr = NULL;
    if(IsIntDataType(pImageAuxData->format.image_channel_data_type))
        read_cbk_ptr = pImageAuxData->read_img_callback_int;
    else
        read_cbk_ptr = pImageAuxData->read_img_callback_float;

    for (unsigned int i=NONE_FALSE_NEAREST;i<NONE_FALSE_LINEAR;i++)
    {
        read_cbk_ptr[i] = pImageCallbackFuncs->GetReadingCbk(NO_CLAMP_CBK, ch_order, ch_type, CL_FILTER_NEAREST);
        // Currently we have SOA implementation only for integer images
        if(IsSOASupported(pImageAuxData->format.image_channel_data_type))
        {
            SET_IF_NOT_NULL(pImageAuxData->soa4_read_img_callback_int[i], pImageCallbackFuncs->GetReadingCbk(NO_CLAMP_CBK, ch_order, ch_type, CL_FILTER_NEAREST, CL_MEM_OBJECT_IMAGE2D, SOA4));
            SET_IF_NOT_NULL(pImageAuxData->soa8_read_img_callback_int[i], pImageCallbackFuncs->GetReadingCbk(NO_CLAMP_CBK, ch_order, ch_type, CL_FILTER_NEAREST, CL_MEM_OBJECT_IMAGE2D, SOA8));
        }
    }

    if( !IsIntDataType(pImageAuxData->format.image_channel_data_type))
    {
        if(pImageAuxData->dim_count == 1)
        {
            for (unsigned int i=NONE_FALSE_LINEAR;i<MIRRORED_TRUE_LINEAR+1;i++)
                read_cbk_ptr[i] = pImageCallbackFuncs->GetReadingCbk(NO_CLAMP_CBK, ch_order, ch_type, CL_FILTER_LINEAR, CL_MEM_OBJECT_IMAGE1D);
            read_cbk_ptr[CLAMP_FALSE_LINEAR] = pImageCallbackFuncs->GetReadingCbk(CLAMP_CBK, ch_order, ch_type, CL_FILTER_LINEAR, CL_MEM_OBJECT_IMAGE1D);
            read_cbk_ptr[CLAMP_TRUE_LINEAR]  = pImageCallbackFuncs->GetReadingCbk(CLAMP_CBK, ch_order, ch_type, CL_FILTER_LINEAR, CL_MEM_OBJECT_IMAGE1D);
        } else if(pImageAuxData->dim_count == 2)
        {
            for (unsigned int i=NONE_FALSE_LINEAR;i<MIRRORED_TRUE_LINEAR+1;i++)
                read_cbk_ptr[i] = pImageCallbackFuncs->GetReadingCbk(NO_CLAMP_CBK, ch_order, ch_type, CL_FILTER_LINEAR, CL_MEM_OBJECT_IMAGE2D);
            read_cbk_ptr[CLAMP_FALSE_LINEAR] = pImageCallbackFuncs->GetReadingCbk(CLAMP_CBK, ch_order, ch_type, CL_FILTER_LINEAR, CL_MEM_OBJECT_IMAGE2D);
            read_cbk_ptr[CLAMP_TRUE_LINEAR]  = pImageCallbackFuncs->GetReadingCbk(CLAMP_CBK, ch_order, ch_type, CL_FILTER_LINEAR, CL_MEM_OBJECT_IMAGE2D);
        }
        else
        {
            for (unsigned int i=NONE_FALSE_LINEAR;i<MIRRORED_TRUE_LINEAR+1;i++)
                read_cbk_ptr[i] = pImageCallbackFuncs->GetReadingCbk(NO_CLAMP_CBK, ch_order, ch_type, CL_FILTER_LINEAR, CL_MEM_OBJECT_IMAGE3D);
            read_cbk_ptr[CLAMP_FALSE_LINEAR] = pImageCallbackFuncs->GetReadingCbk(CLAMP_CBK, ch_order, ch_type, CL_FILTER_LINEAR, CL_MEM_OBJECT_IMAGE3D);
            read_cbk_ptr[CLAMP_TRUE_LINEAR] = pImageCallbackFuncs->GetReadingCbk(CLAMP_CBK, ch_order, ch_type, CL_FILTER_LINEAR, CL_MEM_OBJECT_IMAGE3D);
        }
    }
    if(IsSOASupported(pImageAuxData->format.image_channel_data_type))
    {
        SET_IF_NOT_NULL(pImageAuxData->soa4_read_img_callback_int[CLAMP_FALSE_NEAREST], pImageCallbackFuncs->GetReadingCbk(CLAMP_CBK, ch_order, ch_type, CL_FILTER_NEAREST, CL_MEM_OBJECT_IMAGE2D, SOA4));
        SET_IF_NOT_NULL(pImageAuxData->soa8_read_img_callback_int[CLAMP_FALSE_NEAREST], pImageCallbackFuncs->GetReadingCbk(CLAMP_CBK, ch_order, ch_type, CL_FILTER_NEAREST, CL_MEM_OBJECT_IMAGE2D, SOA8));
        SET_IF_NOT_NULL(pImageAuxData->soa4_write_img_callback, pImageCallbackFuncs->GetWritingCbk(ch_order, ch_type, SOA4));
        SET_IF_NOT_NULL(pImageAuxData->soa8_write_img_callback, pImageCallbackFuncs->GetWritingCbk(ch_order, ch_type, SOA8));
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

}}}  //namespace
