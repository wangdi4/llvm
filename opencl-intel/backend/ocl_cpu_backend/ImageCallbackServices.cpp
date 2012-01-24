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

    // CL_LUMINANCE
    {CL_LUMINANCE,  CL_FLOAT}
};

ImageCallbackService::ImageCallbackService(CompilerConfiguration& config, bool isCpu)
{
  ImageCallbackManager::GetInstance()->InitLibrary(config, isCpu, m_ArchId, m_ArchFeatures);
}

const cl_image_format* ImageCallbackService::GetSupportedImageFormats(unsigned int *numFormats){
    *numFormats=ARRAY_SIZE(supportedImageFormats);
    return (&supportedImageFormats[0]);
}

cl_dev_err_code ImageCallbackService::CreateImageObject(cl_mem_obj_descriptor* pImageObject, void* auxObject) const
{

  // make sure that object passed here is image. Otherwise
    if (pImageObject->dim_count == 1){
      pImageObject->imageAuxData=NULL;
      return CL_DEV_ERROR_FAIL;
    }

    pImageObject->imageAuxData = (image_aux_data*)auxObject;
    image_aux_data* pImageAuxData = (image_aux_data*)pImageObject->imageAuxData;  //using this pointer for the clarity of the code 
    pImageAuxData->pData = pImageObject->pData;
    pImageAuxData->dim_count = pImageObject->dim_count;
    pImageAuxData->format = pImageObject->format;
    pImageAuxData->uiElementSize = pImageObject->uiElementSize;

#define IMG_SET_CALLBACK(CALLBACK, FUNCTION) CALLBACK = FUNCTION;

    //supplementing additional data
    unsigned int TOIndex=TYPE_ORDER_TO_INDEX(pImageAuxData->format.image_channel_data_type, pImageAuxData->format.image_channel_order);
    memset(pImageAuxData->read_img_callback, 0, sizeof(pImageAuxData->read_img_callback));
    memset(pImageAuxData->coord_translate_i_callback, 0, sizeof(pImageAuxData->coord_translate_i_callback));
    memset(pImageAuxData->coord_translate_f_callback, 0, sizeof(pImageAuxData->coord_translate_f_callback));
    memset(pImageAuxData->dim,0,sizeof(pImageAuxData->dim));
    memset(pImageAuxData->pitch, 0, sizeof(pImageAuxData->pitch));
    memset(pImageAuxData->dimSub1,0,sizeof(pImageAuxData->dimSub1));
    memset(pImageAuxData->dimf,0,sizeof(pImageAuxData->dimf));
    for (unsigned int i=0;i<pImageAuxData->dim_count;i++){
        pImageAuxData->dim[i] = pImageObject->dimensions.dim[i];
        pImageAuxData->pitch[i] = pImageObject->pitch[i];
        pImageAuxData->dimSub1[i] = pImageAuxData->dim[i]-1;
        pImageAuxData->dimf[i] = pImageAuxData->dim[i];
    }

    pImageAuxData->offset[0] = pImageAuxData->uiElementSize;
    pImageAuxData->offset[1] = pImageAuxData->pitch[0];
    pImageAuxData->offset[3] = 0;
    
    if (pImageAuxData->dim_count == 3)
        pImageAuxData->offset[2] = pImageAuxData->pitch[1];
    else
        pImageAuxData->offset[2] = 0;

    pImageAuxData->dimmask = (1<<(pImageAuxData->dim_count*4))-1;
    
    ////////////////The integer coordinate callbacks

    ImageCallbackFunctions* pImageCallbackFuncs = ImageCallbackManager::GetInstance()->getCallbackFunctions(m_ArchId, m_ArchFeatures);

    pImageAuxData->coord_translate_i_callback[NONE_FALSE_NEAREST] = pImageCallbackFuncs->m_fpINoneFalseNearest;
    pImageAuxData->coord_translate_i_callback[CLAMP_FALSE_NEAREST] = pImageCallbackFuncs->m_fpINoneFalseNearest;
    pImageAuxData->coord_translate_i_callback[CLAMPTOEDGE_FALSE_NEAREST] = pImageCallbackFuncs->m_fpIClampToEdgeFalseNearest;
    pImageAuxData->coord_translate_i_callback[REPEAT_FALSE_NEAREST] = pImageCallbackFuncs->m_fpIUndefTrans;    //REPEAT+UI COORDINATES MODE IS NOT DEFINED
    pImageAuxData->coord_translate_i_callback[MIRRORED_FALSE_NEAREST] = pImageCallbackFuncs->m_fpIUndefTrans;    //REPEAT+UI COORDINATES MODE IS NOT DEFINED

    //Normalized and bilinear modes are not defined with integer coordinates

    for (unsigned int i=MIRRORED_FALSE_NEAREST+1;i<32;i++)
        pImageAuxData->coord_translate_i_callback[i] = pImageCallbackFuncs->m_fpIUndefTrans;

    //////////////////The float coordinates callbacks

    if ((pImageAuxData->format.image_channel_data_type>=CLK_SIGNED_INT8) && (pImageAuxData->format.image_channel_data_type<=CLK_UNSIGNED_INT32)){
    // for integer images and float coordinates
        //nearest filter, and false normalized
        pImageAuxData->coord_translate_f_callback[NONE_FALSE_NEAREST] = pImageCallbackFuncs->m_fpFNoneFalseNearest;
        pImageAuxData->coord_translate_f_callback[CLAMP_FALSE_NEAREST] = pImageCallbackFuncs->m_fpFNoneFalseNearest;
        pImageAuxData->coord_translate_f_callback[CLAMPTOEDGE_FALSE_NEAREST] = pImageCallbackFuncs->m_fpFClampToEdgeFalseNearest;
        pImageAuxData->coord_translate_f_callback[REPEAT_FALSE_NEAREST] = pImageCallbackFuncs->m_fpFUndefTrans;   //REPEAT + NORMALIZED_FALSE MODE IS NOT DEFINED
        pImageAuxData->coord_translate_f_callback[MIRRORED_FALSE_NEAREST] = pImageCallbackFuncs->m_fpFUndefTrans;    //REPEAT + NORMALIZED_FALSE MODE IS NOT DEFINED

        //nearest filter, and true normalized
        pImageAuxData->coord_translate_f_callback[NONE_TRUE_NEAREST] = pImageCallbackFuncs->m_fpFNoneTrueNearest;
        pImageAuxData->coord_translate_f_callback[CLAMP_TRUE_NEAREST] = pImageCallbackFuncs->m_fpFNoneTrueNearest;
        pImageAuxData->coord_translate_f_callback[CLAMPTOEDGE_TRUE_NEAREST] = pImageCallbackFuncs->m_fpFClampToEdgeTrueNearest;
        pImageAuxData->coord_translate_f_callback[REPEAT_TRUE_NEAREST] = pImageCallbackFuncs->m_fpFRepeatTrueNearest;   
        pImageAuxData->coord_translate_f_callback[MIRRORED_TRUE_NEAREST] = pImageCallbackFuncs->m_fpFMirroredTrueNearest;  
    } else {   //float and unorm images
        pImageAuxData->coord_translate_f_callback[NONE_FALSE_NEAREST] = pImageCallbackFuncs->m_fpFFNoneFalseNearest;
        pImageAuxData->coord_translate_f_callback[CLAMP_FALSE_NEAREST] = pImageCallbackFuncs->m_fpFFNoneFalseNearest;
        pImageAuxData->coord_translate_f_callback[CLAMPTOEDGE_FALSE_NEAREST] = pImageCallbackFuncs->m_fpFFClampToEdgeFalseNearest;
        pImageAuxData->coord_translate_f_callback[REPEAT_FALSE_NEAREST] = pImageCallbackFuncs->m_fpFFUndefTrans;   //REPEAT + NORMALIZED_FALSE MODE IS NOT DEFINED
        pImageAuxData->coord_translate_f_callback[MIRRORED_FALSE_NEAREST] = pImageCallbackFuncs->m_fpFFUndefTrans;    //REPEAT + NORMALIZED_FALSE MODE IS NOT DEFINED

        //nearest filter, and true normalized
        pImageAuxData->coord_translate_f_callback[NONE_TRUE_NEAREST] = pImageCallbackFuncs->m_fpFFNoneTrueNearest;
        pImageAuxData->coord_translate_f_callback[CLAMP_TRUE_NEAREST] = pImageCallbackFuncs->m_fpFFNoneTrueNearest;
        pImageAuxData->coord_translate_f_callback[CLAMPTOEDGE_TRUE_NEAREST] = pImageCallbackFuncs->m_fpFFClampToEdgeTrueNearest;
        pImageAuxData->coord_translate_f_callback[REPEAT_TRUE_NEAREST] = pImageCallbackFuncs->m_fpFFRepeatTrueNearest;   
        pImageAuxData->coord_translate_f_callback[MIRRORED_TRUE_NEAREST] = pImageCallbackFuncs->m_fpFFMirroredTrueNearest;  
    }

    //bilinear filter is undefined for un/signed integer types

    if ((pImageAuxData->format.image_channel_data_type>=CLK_SIGNED_INT8) && (pImageAuxData->format.image_channel_data_type<=CLK_UNSIGNED_INT32)){
        for (unsigned int i=MIRRORED_TRUE_NEAREST+1;i<32;i++)
            pImageAuxData->coord_translate_f_callback[i] = pImageCallbackFuncs->m_fpFUndefTrans;
    } else {
        //nearest filter, and false normalized
        IMG_SET_CALLBACK(pImageAuxData->coord_translate_f_callback[NONE_FALSE_LINEAR], pImageCallbackFuncs->m_fpFNoneFalseLinear);
        IMG_SET_CALLBACK(pImageAuxData->coord_translate_f_callback[CLAMP_FALSE_LINEAR], pImageCallbackFuncs->m_fpFNoneFalseLinear);
        IMG_SET_CALLBACK(pImageAuxData->coord_translate_f_callback[CLAMPTOEDGE_FALSE_LINEAR], pImageCallbackFuncs->m_fpFClampToEdgeFalseLinear);
        IMG_SET_CALLBACK(pImageAuxData->coord_translate_f_callback[REPEAT_FALSE_LINEAR], pImageCallbackFuncs->m_fpFUndefTrans);   //REPEAT + NORMALIZED_FALSE MODE IS NOT DEFINED
        IMG_SET_CALLBACK(pImageAuxData->coord_translate_f_callback[MIRRORED_FALSE_LINEAR], pImageCallbackFuncs->m_fpFUndefTrans);    //REPEAT + NORMALIZED_FALSE MODE IS NOT DEFINED

        //nearest filter, and true normalized
        IMG_SET_CALLBACK(pImageAuxData->coord_translate_f_callback[NONE_TRUE_LINEAR], pImageCallbackFuncs->m_fpFNoneTrueLinear);
        IMG_SET_CALLBACK(pImageAuxData->coord_translate_f_callback[CLAMP_TRUE_LINEAR], pImageCallbackFuncs->m_fpFNoneTrueLinear);
        IMG_SET_CALLBACK(pImageAuxData->coord_translate_f_callback[CLAMPTOEDGE_TRUE_LINEAR], pImageCallbackFuncs->m_fpFClampToEdgeTrueLinear);
        IMG_SET_CALLBACK(pImageAuxData->coord_translate_f_callback[REPEAT_TRUE_LINEAR], pImageCallbackFuncs->m_fpFRepeatTrueLinear);
        IMG_SET_CALLBACK(pImageAuxData->coord_translate_f_callback[MIRRORED_TRUE_LINEAR], pImageCallbackFuncs->m_fpFMirroredTrueLinear);
    }

    ///////////////////////////Read & write image callbacks

    for (unsigned int i=NONE_FALSE_NEAREST;i<NONE_FALSE_LINEAR;i++)
        pImageAuxData->read_img_callback[i] = pImageCallbackFuncs->m_fpNearestNoClamp[TOIndex];
    if(pImageAuxData->dim_count == 2)
    {
        for (unsigned int i=NONE_FALSE_LINEAR;i<MIRRORED_TRUE_LINEAR+1;i++)
            pImageAuxData->read_img_callback[i] = pImageCallbackFuncs->m_fpLinearNoClamp2D[TOIndex];
        IMG_SET_CALLBACK(pImageAuxData->read_img_callback[CLAMP_FALSE_LINEAR], pImageCallbackFuncs->m_fpLinearClamp2D[TOIndex]);
        IMG_SET_CALLBACK(pImageAuxData->read_img_callback[CLAMP_TRUE_LINEAR], pImageCallbackFuncs->m_fpLinearClamp2D[TOIndex]);
    }
    else
    {
        for (unsigned int i=NONE_FALSE_LINEAR;i<MIRRORED_TRUE_LINEAR+1;i++)
            pImageAuxData->read_img_callback[i] = pImageCallbackFuncs->m_fpLinearNoClamp3D[TOIndex];
        IMG_SET_CALLBACK(pImageAuxData->read_img_callback[CLAMP_FALSE_LINEAR], pImageCallbackFuncs->m_fpLinearClamp3D[TOIndex]);
        IMG_SET_CALLBACK(pImageAuxData->read_img_callback[CLAMP_TRUE_LINEAR], pImageCallbackFuncs->m_fpLinearClamp3D[TOIndex]);
    }

    IMG_SET_CALLBACK(pImageAuxData->read_img_callback[CLAMP_FALSE_NEAREST], pImageCallbackFuncs->m_fpNearestClamp[TOIndex]);
    IMG_SET_CALLBACK(pImageAuxData->read_img_callback[CLAMP_TRUE_NEAREST], pImageCallbackFuncs->m_fpNearestClamp[TOIndex]);

    pImageAuxData->write_img_callback = pImageCallbackFuncs->m_fpWriteImage[TOIndex];
    
  return CL_DEV_SUCCESS;
}

cl_dev_err_code ImageCallbackService::DeleteImageObject(cl_mem_obj_descriptor* pImageObject, void** auxObject) const
{
  //this function does nothing meaningful in the meantime, because the allocation is done on the device...
  *auxObject=pImageObject->imageAuxData;
  
  return CL_DEV_SUCCESS;
}

}}}  //namespace
