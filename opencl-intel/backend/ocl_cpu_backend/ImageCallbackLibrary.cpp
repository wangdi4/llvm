/*****************************************************************************\

Copyright (c) Intel Corporation (2010).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  ImageCallbackLibrary.cpp

\*****************************************************************************/

#include <stdio.h>
#include "ImageCallbackLibrary.h"
#include "exceptions.h"
#include "SystemInfo.h"
#include "ServiceFactory.h"
#include "BitCodeContainer.h"
#include "llvm/Module.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Support/FormattedStream.h"
#include "ProgramContainerMemoryBuffer.h"
#include "llvm/System/DynamicLibrary.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "Compiler.h"
#include <string>

#if defined(_WIN32)
    #include <windows.h>
#else
    #include <linux/limits.h>
    #define MAX_PATH PATH_MAX
#endif


namespace Intel { namespace OpenCL { namespace DeviceBackend {

void ImageCallbackLibrary::Load()
{
    char szModuleName[MAX_PATH];
    char szRTLibName[MAX_PATH];
    std::string strErr;

    Utils::SystemInfo::GetModuleDirectory(szModuleName, MAX_PATH);

    //Klocwork warning - false alarm the Id is always in correct bounds
    const char* pCPUPrefix = Utils::CPUDetect::GetInstance()->GetCPUPrefix(m_CpuId);

    if( Intel::CPU_SANDYBRIDGE == m_CpuId && ((m_CpuFeatures & Intel::CFS_AVX1) == 0))
    {
        // Use SSE4 if AVX1 is not supported
        pCPUPrefix = Utils::CPUDetect::GetInstance()->GetCPUPrefix(Intel::CPU_COREI7);
    }

    // Load LLVM built-ins module
#if defined (_WIN32)
    sprintf_s(szRTLibName, MAX_PATH, "%sclbltfn%s_img_cbk.rtl", szModuleName, pCPUPrefix);
#else
    snprintf(szRTLibName, MAX_PATH, "%sclbltfn%s_img_cbk.rtl", szModuleName, pCPUPrefix);
#endif

    m_pRtlBuffer = llvm::MemoryBuffer::getFile(szRTLibName);
    if( NULL == m_pRtlBuffer )
    {
        throw Exceptions::DeviceBackendExceptionBase(std::string("Failed to load the image callback rtl library"));
    }
}

bool ImageCallbackLibrary::Build()
{
    CompilerBuildOptions buildOptions(false, true, false, true);

    ProgramBuildResult buildResult;  //what is this for?
    m_pModule = m_Compiler->BuildProgram(m_pRtlBuffer,&buildOptions, &buildResult);
    m_ImageFunctions = new ImageCallbackFunctions(m_pModule, m_Compiler);
    return (m_pModule != NULL);
}

// For CPU this should be left empty
// TODO: Add implementation for MIC support
bool ImageCallbackLibrary::LoadExecutable()
{
    return true;
}

ImageCallbackFunctions::ImageCallbackFunctions(llvm::Module* pImagesRTModule, CPUCompiler* pCompiler)
{
    const int32_t FUNCTIONS_COUNT = 91;

    // List of function names to retrieve from images module
    const char* funcNames[FUNCTIONS_COUNT]={
        "_Z34trans_coord_int_NONE_FALSE_NEARESTP10_image2d_tDv4_i",                //1
        "_Z41trans_coord_int_CLAMPTOEDGE_FALSE_NEARESTP10_image2d_tDv4_i",            //2
        "_Z25trans_coord_int_UNDEFINEDP10_image2d_tDv4_i",                            //3

        "_Z36trans_coord_float_NONE_FALSE_NEARESTP10_image2d_tDv4_f",                //4
        "_Z43trans_coord_float_CLAMPTOEDGE_FALSE_NEARESTP10_image2d_tDv4_f",        //5

        "_Z35trans_coord_float_NONE_TRUE_NEARESTP10_image2d_tDv4_f",
        "_Z42trans_coord_float_CLAMPTOEDGE_TRUE_NEARESTP10_image2d_tDv4_f",
        "_Z37trans_coord_float_REPEAT_TRUE_NEARESTP10_image2d_tDv4_f",
        "_Z39trans_coord_float_MIRRORED_TRUE_NEARESTP10_image2d_tDv4_f",

        "_Z42trans_coord_float_float_NONE_FALSE_NEARESTP10_image2d_tDv4_fPDv4_iS3_",    //10
        "_Z49trans_coord_float_float_CLAMPTOEDGE_FALSE_NEARESTP10_image2d_tDv4_fPDv4_iS3_",

        "_Z41trans_coord_float_float_NONE_TRUE_NEARESTP10_image2d_tDv4_fPDv4_iS3_",
        "_Z48trans_coord_float_float_CLAMPTOEDGE_TRUE_NEARESTP10_image2d_tDv4_fPDv4_iS3_",
        "_Z43trans_coord_float_float_REPEAT_TRUE_NEARESTP10_image2d_tDv4_fPDv4_iS3_",
        "_Z45trans_coord_float_float_MIRRORED_TRUE_NEARESTP10_image2d_tDv4_fPDv4_iS3_",   //15

        "_Z35trans_coord_float_NONE_FALSE_LINEARP10_image2d_tDv4_fPDv4_iS3_",
        "_Z42trans_coord_float_CLAMPTOEDGE_FALSE_LINEARP10_image2d_tDv4_fPDv4_iS3_",

        "_Z34trans_coord_float_NONE_TRUE_LINEARP10_image2d_tDv4_fPDv4_iS3_",
        "_Z41trans_coord_float_CLAMPTOEDGE_TRUE_LINEARP10_image2d_tDv4_fPDv4_iS3_",
        "_Z36trans_coord_float_REPEAT_TRUE_LINEARP10_image2d_tDv4_fPDv4_iS3_",                //20
        "_Z38trans_coord_float_MIRRORED_TRUE_LINEARP10_image2d_tDv4_fPDv4_iS3_",            
        "_Z27trans_coord_float_UNDEFINEDP10_image2d_tDv4_f",
        "_Z33trans_coord_float_float_UNDEFINEDP10_image2d_tDv4_fPDv4_iS3_",

        "_Z38read_sample_NEAREST_NOCLAMP_RGBA_UINT8P10_image2d_tDv4_i",
        "_Z36read_sample_NEAREST_CLAMP_RGBA_UINT8P10_image2d_tDv4_i",              //25
        "_Z23write_sample_RGBA_UINT8PvDv4_j",

        "_Z39read_sample_NEAREST_NOCLAMP_RGBA_UINT16P10_image2d_tDv4_i",            
        "_Z37read_sample_NEAREST_CLAMP_RGBA_UINT16P10_image2d_tDv4_i",
        "_Z24write_sample_RGBA_UINT16PvDv4_j",

        "_Z39read_sample_NEAREST_NOCLAMP_RGBA_UINT32P10_image2d_tDv4_i",             //30
        "_Z37read_sample_NEAREST_CLAMP_RGBA_UINT32P10_image2d_tDv4_i",
        "_Z24write_sample_RGBA_UINT32PvDv4_j",                        

        "_Z37read_sample_NEAREST_NOCLAMP_RGBA_INT8P10_image2d_tDv4_i",
        "_Z35read_sample_NEAREST_CLAMP_RGBA_INT8P10_image2d_tDv4_i",
        "_Z22write_sample_RGBA_INT8PvDv4_i",                            //35

        "_Z38read_sample_NEAREST_NOCLAMP_RGBA_INT16P10_image2d_tDv4_i",
        "_Z36read_sample_NEAREST_CLAMP_RGBA_INT16P10_image2d_tDv4_i",                
        "_Z23write_sample_RGBA_INT16PvDv4_i",

        "_Z38read_sample_NEAREST_NOCLAMP_RGBA_INT32P10_image2d_tDv4_i",
        "_Z36read_sample_NEAREST_CLAMP_RGBA_INT32P10_image2d_tDv4_i",                      //40
        "_Z23write_sample_RGBA_INT32PvDv4_i",

        /// Float functions
        "_Z38read_sample_NEAREST_NOCLAMP_RGBA_FLOATP10_image2d_tDv4_i",            
        "_Z36read_sample_NEAREST_CLAMP_RGBA_FLOATP10_image2d_tDv4_i",
        "_Z39read_sample_LINEAR2D_NOCLAMP_RGBA_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z37read_sample_LINEAR2D_CLAMP_RGBA_FLOATP10_image2d_tDv4_iS1_Dv4_f",                  //45
        "_Z39read_sample_LINEAR3D_NOCLAMP_RGBA_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z37read_sample_LINEAR3D_CLAMP_RGBA_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z23write_sample_RGBA_FLOATPvDv4_f",

        "_Z43read_sample_NEAREST_NOCLAMP_INTENSITY_FLOATP10_image2d_tDv4_i",
        "_Z41read_sample_NEAREST_CLAMP_INTENSITY_FLOATP10_image2d_tDv4_i",              //50
        "_Z48read_sample_LINEAR2D_NOCLAMP_CH1_INTENSITY_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z42read_sample_LINEAR2D_CLAMP_INTENSITY_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z44read_sample_LINEAR3D_NOCLAMP_INTENSITY_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z42read_sample_LINEAR3D_CLAMP_INTENSITY_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z28write_sample_INTENSITY_FLOATPvDv4_f",                              //55

        "_Z43read_sample_NEAREST_NOCLAMP_RGBA_UNORM_INT8P10_image2d_tDv4_i",    
        "_Z41read_sample_NEAREST_CLAMP_RGBA_UNORM_INT8P10_image2d_tDv4_i",
        "_Z44read_sample_LINEAR2D_NOCLAMP_RGBA_UNORM_INT8P10_image2d_tDv4_iS1_Dv4_f",
        "_Z42read_sample_LINEAR2D_CLAMP_RGBA_UNORM_INT8P10_image2d_tDv4_iS1_Dv4_f",
        "_Z44read_sample_LINEAR3D_NOCLAMP_RGBA_UNORM_INT8P10_image2d_tDv4_iS1_Dv4_f",           //60
        "_Z42read_sample_LINEAR3D_CLAMP_RGBA_UNORM_INT8P10_image2d_tDv4_iS1_Dv4_f",
        "_Z28write_sample_RGBA_UNORM_INT8PvDv4_f",

        "_Z44read_sample_NEAREST_NOCLAMP_RGBA_UNORM_INT16P10_image2d_tDv4_i",    
        "_Z42read_sample_NEAREST_CLAMP_RGBA_UNORM_INT16P10_image2d_tDv4_i",
        "_Z45read_sample_LINEAR2D_NOCLAMP_RGBA_UNORM_INT16P10_image2d_tDv4_iS1_Dv4_f",          //65
        "_Z43read_sample_LINEAR2D_CLAMP_RGBA_UNORM_INT16P10_image2d_tDv4_iS1_Dv4_f",
        "_Z45read_sample_LINEAR3D_NOCLAMP_RGBA_UNORM_INT16P10_image2d_tDv4_iS1_Dv4_f",
        "_Z43read_sample_LINEAR3D_CLAMP_RGBA_UNORM_INT16P10_image2d_tDv4_iS1_Dv4_f",
        "_Z29write_sample_RGBA_UNORM_INT16PvDv4_f",

        "_Z43read_sample_NEAREST_NOCLAMP_RGBA_HALF_FLOATP10_image2d_tDv4_i",            //70
        "_Z41read_sample_NEAREST_CLAMP_RGBA_HALF_FLOATP10_image2d_tDv4_i",
        "_Z44read_sample_LINEAR2D_NOCLAMP_RGBA_HALF_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z42read_sample_LINEAR2D_CLAMP_RGBA_HALF_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z44read_sample_LINEAR3D_NOCLAMP_RGBA_HALF_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z42read_sample_LINEAR3D_CLAMP_RGBA_HALF_FLOATP10_image2d_tDv4_iS1_Dv4_f",             //75
        "_Z28write_sample_RGBA_HALF_FLOATPvDv4_f",

        "_Z43read_sample_NEAREST_NOCLAMP_LUMINANCE_FLOATP10_image2d_tDv4_i",
        "_Z41read_sample_NEAREST_CLAMP_LUMINANCE_FLOATP10_image2d_tDv4_i",
        "_Z48read_sample_LINEAR2D_NOCLAMP_CH1_LUMINANCE_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z42read_sample_LINEAR2D_CLAMP_LUMINANCE_FLOATP10_image2d_tDv4_iS1_Dv4_f",            //80
        "_Z44read_sample_LINEAR3D_NOCLAMP_LUMINANCE_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z42read_sample_LINEAR3D_CLAMP_LUMINANCE_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z28write_sample_LUMINANCE_FLOATPvDv4_f",

        "_Z43read_sample_NEAREST_NOCLAMP_BGRA_UNORM_INT8P10_image2d_tDv4_i",
        "_Z41read_sample_NEAREST_CLAMP_BGRA_UNORM_INT8P10_image2d_tDv4_i",             //85
        "_Z44read_sample_LINEAR2D_NOCLAMP_BGRA_UNORM_INT8P10_image2d_tDv4_iS1_Dv4_f",
        "_Z42read_sample_LINEAR2D_CLAMP_BGRA_UNORM_INT8P10_image2d_tDv4_iS1_Dv4_f",
        "_Z44read_sample_LINEAR3D_NOCLAMP_BGRA_UNORM_INT8P10_image2d_tDv4_iS1_Dv4_f",
        "_Z42read_sample_LINEAR3D_CLAMP_BGRA_UNORM_INT8P10_image2d_tDv4_iS1_Dv4_f",
        "_Z28write_sample_BGRA_UNORM_INT8PvDv4_f",                             //90

        "_Z26read_sample_UNDEFINED_QUADP10_image2d_tDv4_i"                     //91
    };

    // List of image function pointers
    void* funcPointers[FUNCTIONS_COUNT];

    // Go through all functions and retrieve function pointers
    // functions are JITted as they requested
    // TODO: do lazy comiplation. Only required functions should be JITted inside clCreateImage
    for (int32_t index=0;index<FUNCTIONS_COUNT;index++){
        llvm::StringRef fptr(funcNames[index]);
        llvm::Function *fp  = pImagesRTModule->getFunction(fptr);
        // Request pointer to function. If it is the first request function is being JITted.
        funcPointers[index]=pCompiler->GetPointerToFunction(fp);
        if( NULL == funcPointers[index]){
            throw Exceptions::DeviceBackendExceptionBase(std::string("Internal error occurred while jitting image functions"));
        }
    }

    // Assign JITted function pointers to corresponding members

    //////////////////  Fill in translate callback pointers  //////////////////
    // indexes used here correspond to indexes in list funcNames
    m_fpINoneFalseNearest = funcPointers[0];
    m_fpIClampToEdgeFalseNearest = funcPointers[1];
    m_fpIUndefTrans = funcPointers[2];

    m_fpFNoneFalseNearest = funcPointers[3];
    m_fpFClampToEdgeFalseNearest = funcPointers[4];
    
    m_fpFNoneTrueNearest = funcPointers[5];
    m_fpFClampToEdgeTrueNearest = funcPointers[6];
    m_fpFRepeatTrueNearest = funcPointers[7];
    m_fpFMirroredTrueNearest = funcPointers[8];

    m_fpFFNoneFalseNearest = funcPointers[9];
    m_fpFFClampToEdgeFalseNearest = funcPointers[10];

    m_fpFFNoneTrueNearest = funcPointers[11];
    m_fpFFClampToEdgeTrueNearest = funcPointers[12];
    m_fpFFRepeatTrueNearest = funcPointers[13];
    m_fpFFMirroredTrueNearest = funcPointers[14];

    m_fpFNoneFalseLinear = funcPointers[15];
    m_fpFClampToEdgeFalseLinear = funcPointers[16];

    m_fpFNoneTrueLinear = funcPointers[17];
    m_fpFClampToEdgeTrueLinear = funcPointers[18];
    m_fpFRepeatTrueLinear = funcPointers[19];
    m_fpFMirroredTrueLinear = funcPointers[20];
    m_fpFUndefTrans = funcPointers[21];
    m_fpFFUndefTrans = funcPointers[22];

    ////////////////// Fill in read callback pointers //////////////////

    int32_t InitIndex=23;
    // number of supported image formats
    const int32_t IMG_FORMATS_COUNT = 13;
    int32_t TypeIndex[IMG_FORMATS_COUNT];
    TypeIndex[0]=TYPE_ORDER_TO_INDEX(CLK_UNSIGNED_INT8, CLK_RGBA);
    TypeIndex[1]=TYPE_ORDER_TO_INDEX(CLK_UNSIGNED_INT16, CLK_RGBA);
    TypeIndex[2]=TYPE_ORDER_TO_INDEX(CLK_UNSIGNED_INT32, CLK_RGBA);
    TypeIndex[3]=TYPE_ORDER_TO_INDEX(CLK_SIGNED_INT8, CLK_RGBA);
    TypeIndex[4]=TYPE_ORDER_TO_INDEX(CLK_SIGNED_INT16, CLK_RGBA);
    TypeIndex[5]=TYPE_ORDER_TO_INDEX(CLK_SIGNED_INT32, CLK_RGBA);
    TypeIndex[6]=TYPE_ORDER_TO_INDEX(CLK_FLOAT, CLK_RGBA);
    TypeIndex[7]=TYPE_ORDER_TO_INDEX(CLK_FLOAT, CLK_INTENSITY);
    TypeIndex[8]=TYPE_ORDER_TO_INDEX(CLK_UNORM_INT8, CLK_RGBA);
    TypeIndex[9]=TYPE_ORDER_TO_INDEX(CLK_UNORM_INT16, CLK_RGBA);
    TypeIndex[10]=TYPE_ORDER_TO_INDEX(CLK_HALF_FLOAT, CLK_RGBA);
    TypeIndex[11]=TYPE_ORDER_TO_INDEX(CLK_FLOAT, CLK_LUMINANCE);
    TypeIndex[12]=TYPE_ORDER_TO_INDEX(CLK_UNORM_INT8, CLK_BGRA);

    // Number of integer image formats
    const int32_t INT_FORMATS_COUNT = 6;

    // Fill in integer image callbacks
    for (int32_t i=0;i<INT_FORMATS_COUNT;i++){
        m_fpNearestNoClamp[TypeIndex[i]] = funcPointers[InitIndex++];
        m_fpNearestClamp[TypeIndex[i]] = funcPointers[InitIndex++];
        m_fpWriteImage[TypeIndex[i]] = funcPointers[InitIndex++];
    }

    // Fill in floating point callbacks
    for (int32_t i=INT_FORMATS_COUNT;i<IMG_FORMATS_COUNT;i++){
        m_fpNearestNoClamp[TypeIndex[i]] = funcPointers[InitIndex++];
        m_fpNearestClamp[TypeIndex[i]] = funcPointers[InitIndex++];
        m_fpLinearNoClamp2D[TypeIndex[i]] = funcPointers[InitIndex++];
        m_fpLinearClamp2D[TypeIndex[i]] = funcPointers[InitIndex++];
        m_fpLinearNoClamp3D[TypeIndex[i]] = funcPointers[InitIndex++];
        m_fpLinearClamp3D[TypeIndex[i]] = funcPointers[InitIndex++];
        m_fpWriteImage[TypeIndex[i]] = funcPointers[InitIndex++];
        
    }

    assert(FUNCTIONS_COUNT == ARRAY_SIZE(funcPointers));
    // set undefined float reading callback
    m_fpUndefReadQuad = funcPointers[FUNCTIONS_COUNT-1];
}



}}} // namespace
