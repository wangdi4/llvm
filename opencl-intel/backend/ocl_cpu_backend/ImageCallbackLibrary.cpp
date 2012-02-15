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
#include "llvm/Support/DynamicLibrary.h"
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

//    m_pRtlBuffer = llvm::MemoryBuffer::getFile(szRTLibName);
    llvm::error_code ret = llvm::MemoryBuffer::getFile(szRTLibName, m_pRtlBuffer);
    if( NULL == m_pRtlBuffer )
    {
        throw Exceptions::DeviceBackendExceptionBase(std::string("Failed to load the image callback rtl library"));
    }
}

bool ImageCallbackLibrary::Build()
{
    CompilerBuildOptions buildOptions(false, true, false, true);

    ProgramBuildResult buildResult;  //what is this for?
    m_pModule = m_Compiler->BuildProgram(m_pRtlBuffer.get(),&buildOptions, &buildResult);
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
    // Total number of coordinate translation callbacks
    const int TRANS_CBK_COUNT = 20;
    // last index should be equal to
    const int INT_FORMATS_COUNT = 18;
    const int INT_CBK_PER_FORMAT = 3;
    // Float formats count
    const int FLOAT_FORMATS_COUNT = 25;
    const int FLOAT_CBK_PER_FORMAT = 7;
    // Total number of functions
    const int32_t FUNCTIONS_COUNT = TRANS_CBK_COUNT + // count translate callbacks first
             INT_FORMATS_COUNT * INT_CBK_PER_FORMAT + // add integer callbacks
             FLOAT_FORMATS_COUNT * FLOAT_CBK_PER_FORMAT + // add float callbacks
             1; // and add one undefined reading callback

    // List of function names to retrieve from images module
    const char* funcNames[FUNCTIONS_COUNT]={
        "_Z36trans_coord_float_NONE_FALSE_NEARESTP10_image2d_tDv4_f",                //1
        "_Z43trans_coord_float_CLAMPTOEDGE_FALSE_NEARESTP10_image2d_tDv4_f",        //2

        "_Z35trans_coord_float_NONE_TRUE_NEARESTP10_image2d_tDv4_f",
        "_Z42trans_coord_float_CLAMPTOEDGE_TRUE_NEARESTP10_image2d_tDv4_f",
        "_Z37trans_coord_float_REPEAT_TRUE_NEARESTP10_image2d_tDv4_f",
        "_Z39trans_coord_float_MIRRORED_TRUE_NEARESTP10_image2d_tDv4_f",

        "_Z42trans_coord_float_float_NONE_FALSE_NEARESTP10_image2d_tDv4_fPDv4_iS3_",    //7
        "_Z49trans_coord_float_float_CLAMPTOEDGE_FALSE_NEARESTP10_image2d_tDv4_fPDv4_iS3_",

        "_Z41trans_coord_float_float_NONE_TRUE_NEARESTP10_image2d_tDv4_fPDv4_iS3_",
        "_Z48trans_coord_float_float_CLAMPTOEDGE_TRUE_NEARESTP10_image2d_tDv4_fPDv4_iS3_",
        "_Z43trans_coord_float_float_REPEAT_TRUE_NEARESTP10_image2d_tDv4_fPDv4_iS3_",
        "_Z45trans_coord_float_float_MIRRORED_TRUE_NEARESTP10_image2d_tDv4_fPDv4_iS3_",   //12

        "_Z35trans_coord_float_NONE_FALSE_LINEARP10_image2d_tDv4_fPDv4_iS3_",
        "_Z42trans_coord_float_CLAMPTOEDGE_FALSE_LINEARP10_image2d_tDv4_fPDv4_iS3_",

        "_Z34trans_coord_float_NONE_TRUE_LINEARP10_image2d_tDv4_fPDv4_iS3_",
        "_Z41trans_coord_float_CLAMPTOEDGE_TRUE_LINEARP10_image2d_tDv4_fPDv4_iS3_",
        "_Z36trans_coord_float_REPEAT_TRUE_LINEARP10_image2d_tDv4_fPDv4_iS3_",                //17
        "_Z38trans_coord_float_MIRRORED_TRUE_LINEARP10_image2d_tDv4_fPDv4_iS3_",            
        "_Z27trans_coord_float_UNDEFINEDP10_image2d_tDv4_f",
        "_Z33trans_coord_float_float_UNDEFINEDP10_image2d_tDv4_fPDv4_iS3_",

        "_Z38read_sample_NEAREST_NOCLAMP_RGBA_UINT8P10_image2d_tDv4_i",
        "_Z36read_sample_NEAREST_CLAMP_RGBA_UINT8P10_image2d_tDv4_i",              //22
        "_Z23write_sample_RGBA_UINT8PvDv4_j",

        "_Z39read_sample_NEAREST_NOCLAMP_RGBA_UINT16P10_image2d_tDv4_i",            
        "_Z37read_sample_NEAREST_CLAMP_RGBA_UINT16P10_image2d_tDv4_i",
        "_Z24write_sample_RGBA_UINT16PvDv4_j",

        "_Z39read_sample_NEAREST_NOCLAMP_RGBA_UINT32P10_image2d_tDv4_i",             //27
        "_Z37read_sample_NEAREST_CLAMP_RGBA_UINT32P10_image2d_tDv4_i",
        "_Z24write_sample_RGBA_UINT32PvDv4_j",                        

        "_Z37read_sample_NEAREST_NOCLAMP_RGBA_INT8P10_image2d_tDv4_i",
        "_Z35read_sample_NEAREST_CLAMP_RGBA_INT8P10_image2d_tDv4_i",
        "_Z22write_sample_RGBA_INT8PvDv4_i",                            //32

        "_Z38read_sample_NEAREST_NOCLAMP_RGBA_INT16P10_image2d_tDv4_i",
        "_Z36read_sample_NEAREST_CLAMP_RGBA_INT16P10_image2d_tDv4_i",                
        "_Z23write_sample_RGBA_INT16PvDv4_i",

        "_Z38read_sample_NEAREST_NOCLAMP_RGBA_INT32P10_image2d_tDv4_i",
        "_Z36read_sample_NEAREST_CLAMP_RGBA_INT32P10_image2d_tDv4_i",                      //37
        "_Z23write_sample_RGBA_INT32PvDv4_i",

        "_Z35read_sample_NEAREST_NOCLAMP_R_UINT8P10_image2d_tDv4_i",
        "_Z33read_sample_NEAREST_CLAMP_R_UINT8P10_image2d_tDv4_i",
        "_Z20write_sample_R_UINT8PvDv4_j",

        "_Z36read_sample_NEAREST_NOCLAMP_R_UINT16P10_image2d_tDv4_i",
        "_Z34read_sample_NEAREST_CLAMP_R_UINT16P10_image2d_tDv4_i",
        "_Z21write_sample_R_UINT16PvDv4_j",

        "_Z36read_sample_NEAREST_NOCLAMP_R_UINT32P10_image2d_tDv4_i",
        "_Z34read_sample_NEAREST_CLAMP_R_UINT32P10_image2d_tDv4_i",
        "_Z21write_sample_R_UINT32PvDv4_j",

        "_Z34read_sample_NEAREST_NOCLAMP_R_INT8P10_image2d_tDv4_i",
        "_Z32read_sample_NEAREST_CLAMP_R_INT8P10_image2d_tDv4_i",
        "_Z19write_sample_R_INT8PvDv4_i",

        "_Z35read_sample_NEAREST_NOCLAMP_R_INT16P10_image2d_tDv4_i",
        "_Z33read_sample_NEAREST_CLAMP_R_INT16P10_image2d_tDv4_i",
        "_Z20write_sample_R_INT16PvDv4_i",

        "_Z35read_sample_NEAREST_NOCLAMP_R_INT32P10_image2d_tDv4_i",
        "_Z33read_sample_NEAREST_CLAMP_R_INT32P10_image2d_tDv4_i",
        "_Z20write_sample_R_INT32PvDv4_i",

        "_Z36read_sample_NEAREST_NOCLAMP_RG_UINT8P10_image2d_tDv4_i",
        "_Z34read_sample_NEAREST_CLAMP_RG_UINT8P10_image2d_tDv4_i",
        "_Z21write_sample_RG_UINT8PvDv4_j",

        "_Z37read_sample_NEAREST_NOCLAMP_RG_UINT16P10_image2d_tDv4_i",
        "_Z35read_sample_NEAREST_CLAMP_RG_UINT16P10_image2d_tDv4_i",
        "_Z22write_sample_RG_UINT16PvDv4_j",

        "_Z37read_sample_NEAREST_NOCLAMP_RG_UINT32P10_image2d_tDv4_i",
        "_Z35read_sample_NEAREST_CLAMP_RG_UINT32P10_image2d_tDv4_i",
        "_Z22write_sample_RG_UINT32PvDv4_j",

        "_Z35read_sample_NEAREST_NOCLAMP_RG_INT8P10_image2d_tDv4_i",
        "_Z33read_sample_NEAREST_CLAMP_RG_INT8P10_image2d_tDv4_i",
        "_Z20write_sample_RG_INT8PvDv4_i",

        "_Z36read_sample_NEAREST_NOCLAMP_RG_INT16P10_image2d_tDv4_i",
        "_Z34read_sample_NEAREST_CLAMP_RG_INT16P10_image2d_tDv4_i",
        "_Z21write_sample_RG_INT16PvDv4_i",

        "_Z36read_sample_NEAREST_NOCLAMP_RG_INT32P10_image2d_tDv4_i",
        "_Z34read_sample_NEAREST_CLAMP_RG_INT32P10_image2d_tDv4_i",
        "_Z21write_sample_RG_INT32PvDv4_i",

        /// Float functions
        "_Z38read_sample_NEAREST_NOCLAMP_RGBA_FLOATP10_image2d_tDv4_i",
        "_Z36read_sample_NEAREST_CLAMP_RGBA_FLOATP10_image2d_tDv4_i",
        "_Z39read_sample_LINEAR2D_NOCLAMP_RGBA_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z37read_sample_LINEAR2D_CLAMP_RGBA_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z39read_sample_LINEAR3D_NOCLAMP_RGBA_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z37read_sample_LINEAR3D_CLAMP_RGBA_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z23write_sample_RGBA_FLOATPvDv4_f",

        "_Z43read_sample_NEAREST_NOCLAMP_INTENSITY_FLOATP10_image2d_tDv4_i",
        "_Z41read_sample_NEAREST_CLAMP_INTENSITY_FLOATP10_image2d_tDv4_i",
        "_Z48read_sample_LINEAR2D_NOCLAMP_CH1_INTENSITY_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z42read_sample_LINEAR2D_CLAMP_INTENSITY_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z44read_sample_LINEAR3D_NOCLAMP_INTENSITY_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z42read_sample_LINEAR3D_CLAMP_INTENSITY_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z28write_sample_INTENSITY_FLOATPvDv4_f",

        "_Z48read_sample_NEAREST_NOCLAMP_INTENSITY_UNORM_INT8P10_image2d_tDv4_i",
        "_Z46read_sample_NEAREST_CLAMP_INTENSITY_UNORM_INT8P10_image2d_tDv4_i",
        "_Z53read_sample_LINEAR2D_NOCLAMP_CH1_INTENSITY_UNORM_INT8P10_image2d_tDv4_iS1_Dv4_f",
        "_Z47read_sample_LINEAR2D_CLAMP_INTENSITY_UNORM_INT8P10_image2d_tDv4_iS1_Dv4_f",
        "_Z49read_sample_LINEAR3D_NOCLAMP_INTENSITY_UNORM_INT8P10_image2d_tDv4_iS1_Dv4_f",
        "_Z47read_sample_LINEAR3D_CLAMP_INTENSITY_UNORM_INT8P10_image2d_tDv4_iS1_Dv4_f",
        "_Z33write_sample_INTENSITY_UNORM_INT8PvDv4_f",

        "_Z49read_sample_NEAREST_NOCLAMP_INTENSITY_UNORM_INT16P10_image2d_tDv4_i",
        "_Z47read_sample_NEAREST_CLAMP_INTENSITY_UNORM_INT16P10_image2d_tDv4_i",
        "_Z54read_sample_LINEAR2D_NOCLAMP_CH1_INTENSITY_UNORM_INT16P10_image2d_tDv4_iS1_Dv4_f",
        "_Z48read_sample_LINEAR2D_CLAMP_INTENSITY_UNORM_INT16P10_image2d_tDv4_iS1_Dv4_f",
        "_Z50read_sample_LINEAR3D_NOCLAMP_INTENSITY_UNORM_INT16P10_image2d_tDv4_iS1_Dv4_f",
        "_Z48read_sample_LINEAR3D_CLAMP_INTENSITY_UNORM_INT16P10_image2d_tDv4_iS1_Dv4_f",
        "_Z34write_sample_INTENSITY_UNORM_INT16PvDv4_f",

        "_Z48read_sample_NEAREST_NOCLAMP_INTENSITY_HALF_FLOATP10_image2d_tDv4_i",
        "_Z46read_sample_NEAREST_CLAMP_INTENSITY_HALF_FLOATP10_image2d_tDv4_i",
        "_Z53read_sample_LINEAR2D_NOCLAMP_CH1_INTENSITY_HALF_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z47read_sample_LINEAR2D_CLAMP_INTENSITY_HALF_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z49read_sample_LINEAR3D_NOCLAMP_INTENSITY_HALF_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z47read_sample_LINEAR3D_CLAMP_INTENSITY_HALF_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z33write_sample_INTENSITY_HALF_FLOATPvDv4_f",

        "_Z43read_sample_NEAREST_NOCLAMP_RGBA_UNORM_INT8P10_image2d_tDv4_i",    
        "_Z41read_sample_NEAREST_CLAMP_RGBA_UNORM_INT8P10_image2d_tDv4_i",
        "_Z44read_sample_LINEAR2D_NOCLAMP_RGBA_UNORM_INT8P10_image2d_tDv4_iS1_Dv4_f",
        "_Z42read_sample_LINEAR2D_CLAMP_RGBA_UNORM_INT8P10_image2d_tDv4_iS1_Dv4_f",
        "_Z44read_sample_LINEAR3D_NOCLAMP_RGBA_UNORM_INT8P10_image2d_tDv4_iS1_Dv4_f",
        "_Z42read_sample_LINEAR3D_CLAMP_RGBA_UNORM_INT8P10_image2d_tDv4_iS1_Dv4_f",
        "_Z28write_sample_RGBA_UNORM_INT8PvDv4_f",

        "_Z44read_sample_NEAREST_NOCLAMP_RGBA_UNORM_INT16P10_image2d_tDv4_i",
        "_Z42read_sample_NEAREST_CLAMP_RGBA_UNORM_INT16P10_image2d_tDv4_i",
        "_Z45read_sample_LINEAR2D_NOCLAMP_RGBA_UNORM_INT16P10_image2d_tDv4_iS1_Dv4_f",
        "_Z43read_sample_LINEAR2D_CLAMP_RGBA_UNORM_INT16P10_image2d_tDv4_iS1_Dv4_f",
        "_Z45read_sample_LINEAR3D_NOCLAMP_RGBA_UNORM_INT16P10_image2d_tDv4_iS1_Dv4_f",
        "_Z43read_sample_LINEAR3D_CLAMP_RGBA_UNORM_INT16P10_image2d_tDv4_iS1_Dv4_f",
        "_Z29write_sample_RGBA_UNORM_INT16PvDv4_f",

        "_Z43read_sample_NEAREST_NOCLAMP_RGBA_HALF_FLOATP10_image2d_tDv4_i",
        "_Z41read_sample_NEAREST_CLAMP_RGBA_HALF_FLOATP10_image2d_tDv4_i",
        "_Z44read_sample_LINEAR2D_NOCLAMP_RGBA_HALF_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z42read_sample_LINEAR2D_CLAMP_RGBA_HALF_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z44read_sample_LINEAR3D_NOCLAMP_RGBA_HALF_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z42read_sample_LINEAR3D_CLAMP_RGBA_HALF_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z28write_sample_RGBA_HALF_FLOATPvDv4_f",

        "_Z43read_sample_NEAREST_NOCLAMP_LUMINANCE_FLOATP10_image2d_tDv4_i",
        "_Z41read_sample_NEAREST_CLAMP_LUMINANCE_FLOATP10_image2d_tDv4_i",
        "_Z48read_sample_LINEAR2D_NOCLAMP_CH1_LUMINANCE_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z42read_sample_LINEAR2D_CLAMP_LUMINANCE_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z44read_sample_LINEAR3D_NOCLAMP_LUMINANCE_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z42read_sample_LINEAR3D_CLAMP_LUMINANCE_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z28write_sample_LUMINANCE_FLOATPvDv4_f",

        "_Z48read_sample_NEAREST_NOCLAMP_LUMINANCE_UNORM_INT8P10_image2d_tDv4_i",
        "_Z46read_sample_NEAREST_CLAMP_LUMINANCE_UNORM_INT8P10_image2d_tDv4_i",
        "_Z53read_sample_LINEAR2D_NOCLAMP_CH1_LUMINANCE_UNORM_INT8P10_image2d_tDv4_iS1_Dv4_f",
        "_Z47read_sample_LINEAR2D_CLAMP_LUMINANCE_UNORM_INT8P10_image2d_tDv4_iS1_Dv4_f",
        "_Z49read_sample_LINEAR3D_NOCLAMP_LUMINANCE_UNORM_INT8P10_image2d_tDv4_iS1_Dv4_f",
        "_Z47read_sample_LINEAR3D_CLAMP_LUMINANCE_UNORM_INT8P10_image2d_tDv4_iS1_Dv4_f",
        "_Z33write_sample_LUMINANCE_UNORM_INT8PvDv4_f",

        "_Z49read_sample_NEAREST_NOCLAMP_LUMINANCE_UNORM_INT16P10_image2d_tDv4_i",
        "_Z47read_sample_NEAREST_CLAMP_LUMINANCE_UNORM_INT16P10_image2d_tDv4_i",
        "_Z54read_sample_LINEAR2D_NOCLAMP_CH1_LUMINANCE_UNORM_INT16P10_image2d_tDv4_iS1_Dv4_f",
        "_Z48read_sample_LINEAR2D_CLAMP_LUMINANCE_UNORM_INT16P10_image2d_tDv4_iS1_Dv4_f",
        "_Z50read_sample_LINEAR3D_NOCLAMP_LUMINANCE_UNORM_INT16P10_image2d_tDv4_iS1_Dv4_f",
        "_Z48read_sample_LINEAR3D_CLAMP_LUMINANCE_UNORM_INT16P10_image2d_tDv4_iS1_Dv4_f",
        "_Z34write_sample_LUMINANCE_UNORM_INT16PvDv4_f",

        "_Z48read_sample_NEAREST_NOCLAMP_LUMINANCE_HALF_FLOATP10_image2d_tDv4_i",
        "_Z46read_sample_NEAREST_CLAMP_LUMINANCE_HALF_FLOATP10_image2d_tDv4_i",
        "_Z53read_sample_LINEAR2D_NOCLAMP_CH1_LUMINANCE_HALF_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z47read_sample_LINEAR2D_CLAMP_LUMINANCE_HALF_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z49read_sample_LINEAR3D_NOCLAMP_LUMINANCE_HALF_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z47read_sample_LINEAR3D_CLAMP_LUMINANCE_HALF_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z33write_sample_LUMINANCE_HALF_FLOATPvDv4_f",

        "_Z43read_sample_NEAREST_NOCLAMP_BGRA_UNORM_INT8P10_image2d_tDv4_i",
        "_Z41read_sample_NEAREST_CLAMP_BGRA_UNORM_INT8P10_image2d_tDv4_i",
        "_Z44read_sample_LINEAR2D_NOCLAMP_BGRA_UNORM_INT8P10_image2d_tDv4_iS1_Dv4_f",
        "_Z42read_sample_LINEAR2D_CLAMP_BGRA_UNORM_INT8P10_image2d_tDv4_iS1_Dv4_f",
        "_Z44read_sample_LINEAR3D_NOCLAMP_BGRA_UNORM_INT8P10_image2d_tDv4_iS1_Dv4_f",
        "_Z42read_sample_LINEAR3D_CLAMP_BGRA_UNORM_INT8P10_image2d_tDv4_iS1_Dv4_f",
        "_Z28write_sample_BGRA_UNORM_INT8PvDv4_f",

        "_Z35read_sample_NEAREST_NOCLAMP_R_FLOATP10_image2d_tDv4_i",
        "_Z33read_sample_NEAREST_CLAMP_R_FLOATP10_image2d_tDv4_i",
        "_Z36read_sample_LINEAR2D_NOCLAMP_R_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z34read_sample_LINEAR2D_CLAMP_R_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z36read_sample_LINEAR3D_NOCLAMP_R_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z34read_sample_LINEAR3D_CLAMP_R_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z20write_sample_R_FLOATPvDv4_f",

        "_Z40read_sample_NEAREST_NOCLAMP_R_UNORM_INT8P10_image2d_tDv4_i",
        "_Z38read_sample_NEAREST_CLAMP_R_UNORM_INT8P10_image2d_tDv4_i",
        "_Z41read_sample_LINEAR2D_NOCLAMP_R_UNORM_INT8P10_image2d_tDv4_iS1_Dv4_f",
        "_Z39read_sample_LINEAR2D_CLAMP_R_UNORM_INT8P10_image2d_tDv4_iS1_Dv4_f",
        "_Z41read_sample_LINEAR3D_NOCLAMP_R_UNORM_INT8P10_image2d_tDv4_iS1_Dv4_f",
        "_Z39read_sample_LINEAR3D_CLAMP_R_UNORM_INT8P10_image2d_tDv4_iS1_Dv4_f",
        "_Z25write_sample_R_UNORM_INT8PvDv4_f",

        "_Z41read_sample_NEAREST_NOCLAMP_R_UNORM_INT16P10_image2d_tDv4_i",
        "_Z39read_sample_NEAREST_CLAMP_R_UNORM_INT16P10_image2d_tDv4_i",
        "_Z42read_sample_LINEAR2D_NOCLAMP_R_UNORM_INT16P10_image2d_tDv4_iS1_Dv4_f",
        "_Z40read_sample_LINEAR2D_CLAMP_R_UNORM_INT16P10_image2d_tDv4_iS1_Dv4_f",
        "_Z42read_sample_LINEAR3D_NOCLAMP_R_UNORM_INT16P10_image2d_tDv4_iS1_Dv4_f",
        "_Z40read_sample_LINEAR3D_CLAMP_R_UNORM_INT16P10_image2d_tDv4_iS1_Dv4_f",
        "_Z26write_sample_R_UNORM_INT16PvDv4_f",

        "_Z40read_sample_NEAREST_NOCLAMP_R_HALF_FLOATP10_image2d_tDv4_i",
        "_Z38read_sample_NEAREST_CLAMP_R_HALF_FLOATP10_image2d_tDv4_i",
        "_Z41read_sample_LINEAR2D_NOCLAMP_R_HALF_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z39read_sample_LINEAR2D_CLAMP_R_HALF_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z41read_sample_LINEAR3D_NOCLAMP_R_HALF_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z39read_sample_LINEAR3D_CLAMP_R_HALF_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z25write_sample_R_HALF_FLOATPvDv4_f",

        "_Z35read_sample_NEAREST_NOCLAMP_A_FLOATP10_image2d_tDv4_i",
        "_Z33read_sample_NEAREST_CLAMP_A_FLOATP10_image2d_tDv4_i",
        "_Z36read_sample_LINEAR2D_NOCLAMP_A_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z34read_sample_LINEAR2D_CLAMP_A_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z36read_sample_LINEAR3D_NOCLAMP_A_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z34read_sample_LINEAR3D_CLAMP_A_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z20write_sample_A_FLOATPvDv4_f",

        "_Z40read_sample_NEAREST_NOCLAMP_A_UNORM_INT8P10_image2d_tDv4_i",
        "_Z38read_sample_NEAREST_CLAMP_A_UNORM_INT8P10_image2d_tDv4_i",
        "_Z41read_sample_LINEAR2D_NOCLAMP_A_UNORM_INT8P10_image2d_tDv4_iS1_Dv4_f",
        "_Z39read_sample_LINEAR2D_CLAMP_A_UNORM_INT8P10_image2d_tDv4_iS1_Dv4_f",
        "_Z41read_sample_LINEAR3D_NOCLAMP_A_UNORM_INT8P10_image2d_tDv4_iS1_Dv4_f",
        "_Z39read_sample_LINEAR3D_CLAMP_A_UNORM_INT8P10_image2d_tDv4_iS1_Dv4_f",
        "_Z25write_sample_A_UNORM_INT8PvDv4_f",

        "_Z41read_sample_NEAREST_NOCLAMP_A_UNORM_INT16P10_image2d_tDv4_i",
        "_Z39read_sample_NEAREST_CLAMP_A_UNORM_INT16P10_image2d_tDv4_i",
        "_Z42read_sample_LINEAR2D_NOCLAMP_A_UNORM_INT16P10_image2d_tDv4_iS1_Dv4_f",
        "_Z40read_sample_LINEAR2D_CLAMP_A_UNORM_INT16P10_image2d_tDv4_iS1_Dv4_f",
        "_Z42read_sample_LINEAR3D_NOCLAMP_A_UNORM_INT16P10_image2d_tDv4_iS1_Dv4_f",
        "_Z40read_sample_LINEAR3D_CLAMP_A_UNORM_INT16P10_image2d_tDv4_iS1_Dv4_f",
        "_Z26write_sample_A_UNORM_INT16PvDv4_f",

        "_Z40read_sample_NEAREST_NOCLAMP_A_HALF_FLOATP10_image2d_tDv4_i",
        "_Z38read_sample_NEAREST_CLAMP_A_HALF_FLOATP10_image2d_tDv4_i",
        "_Z41read_sample_LINEAR2D_NOCLAMP_A_HALF_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z39read_sample_LINEAR2D_CLAMP_A_HALF_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z41read_sample_LINEAR3D_NOCLAMP_A_HALF_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z39read_sample_LINEAR3D_CLAMP_A_HALF_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z25write_sample_A_HALF_FLOATPvDv4_f",
        ///CL_RG
        "_Z36read_sample_NEAREST_NOCLAMP_RG_FLOATP10_image2d_tDv4_i",
        "_Z34read_sample_NEAREST_CLAMP_RG_FLOATP10_image2d_tDv4_i",
        "_Z37read_sample_LINEAR2D_NOCLAMP_RG_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z35read_sample_LINEAR2D_CLAMP_RG_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z37read_sample_LINEAR3D_NOCLAMP_RG_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z35read_sample_LINEAR3D_CLAMP_RG_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z21write_sample_RG_FLOATPvDv4_f",

        "_Z41read_sample_NEAREST_NOCLAMP_RG_UNORM_INT8P10_image2d_tDv4_i",
        "_Z39read_sample_NEAREST_CLAMP_RG_UNORM_INT8P10_image2d_tDv4_i",
        "_Z42read_sample_LINEAR2D_NOCLAMP_RG_UNORM_INT8P10_image2d_tDv4_iS1_Dv4_f",
        "_Z40read_sample_LINEAR2D_CLAMP_RG_UNORM_INT8P10_image2d_tDv4_iS1_Dv4_f",
        "_Z42read_sample_LINEAR3D_NOCLAMP_RG_UNORM_INT8P10_image2d_tDv4_iS1_Dv4_f",
        "_Z40read_sample_LINEAR3D_CLAMP_RG_UNORM_INT8P10_image2d_tDv4_iS1_Dv4_f",
        "_Z26write_sample_RG_UNORM_INT8PvDv4_f",

        "_Z42read_sample_NEAREST_NOCLAMP_RG_UNORM_INT16P10_image2d_tDv4_i",
        "_Z40read_sample_NEAREST_CLAMP_RG_UNORM_INT16P10_image2d_tDv4_i",
        "_Z43read_sample_LINEAR2D_NOCLAMP_RG_UNORM_INT16P10_image2d_tDv4_iS1_Dv4_f",
        "_Z41read_sample_LINEAR2D_CLAMP_RG_UNORM_INT16P10_image2d_tDv4_iS1_Dv4_f",
        "_Z43read_sample_LINEAR3D_NOCLAMP_RG_UNORM_INT16P10_image2d_tDv4_iS1_Dv4_f",
        "_Z41read_sample_LINEAR3D_CLAMP_RG_UNORM_INT16P10_image2d_tDv4_iS1_Dv4_f",
        "_Z27write_sample_RG_UNORM_INT16PvDv4_f",

        "_Z41read_sample_NEAREST_NOCLAMP_RG_HALF_FLOATP10_image2d_tDv4_i",
        "_Z39read_sample_NEAREST_CLAMP_RG_HALF_FLOATP10_image2d_tDv4_i",
        "_Z42read_sample_LINEAR2D_NOCLAMP_RG_HALF_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z40read_sample_LINEAR2D_CLAMP_RG_HALF_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z42read_sample_LINEAR3D_NOCLAMP_RG_HALF_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z40read_sample_LINEAR3D_CLAMP_RG_HALF_FLOATP10_image2d_tDv4_iS1_Dv4_f",
        "_Z26write_sample_RG_HALF_FLOATPvDv4_f",

        "_Z26read_sample_UNDEFINED_QUADP10_image2d_tDv4_i"
    };

    // Make sure the last string is read_sample_undefined
    assert(strcmp(funcNames[FUNCTIONS_COUNT - 1], "_Z26read_sample_UNDEFINED_QUADP10_image2d_tDv4_i") == 0);

    // List of image function pointers
    llvm::Function* funcPointers[FUNCTIONS_COUNT];

    for (int32_t index=0;index<FUNCTIONS_COUNT;index++){
        llvm::StringRef fName(funcNames[index]);
        funcPointers[index] = pImagesRTModule->getFunction(fName);
        if(funcPointers[index] == NULL)
            throw Exceptions::DeviceBackendExceptionBase(std::string("Image function wasn't found in the module. Make sure image libraries are valid."));
    }

    // Assign JITted function pointers to corresponding members

    //////////////////  Fill in translate callback pointers  //////////////////
    // indexes used here correspond to indexes in list funcNames
    m_fpFNoneFalseNearest.Init(funcPointers[0]);
    m_fpFClampToEdgeFalseNearest.Init(funcPointers[1]);
    
    m_fpFNoneTrueNearest.Init(funcPointers[2]);
    m_fpFClampToEdgeTrueNearest.Init(funcPointers[3]);
    m_fpFRepeatTrueNearest.Init(funcPointers[4]);
    m_fpFMirroredTrueNearest.Init(funcPointers[5]);

    m_fpFFNoneFalseNearest.Init(funcPointers[6]);
    m_fpFFClampToEdgeFalseNearest.Init(funcPointers[7]);

    m_fpFFNoneTrueNearest.Init(funcPointers[8]);
    m_fpFFClampToEdgeTrueNearest.Init(funcPointers[9]);
    m_fpFFRepeatTrueNearest.Init(funcPointers[10]);
    m_fpFFMirroredTrueNearest.Init(funcPointers[11]);

    m_fpFNoneFalseLinear.Init(funcPointers[12]);
    m_fpFClampToEdgeFalseLinear.Init(funcPointers[13]);

    m_fpFNoneTrueLinear.Init(funcPointers[14]);
    m_fpFClampToEdgeTrueLinear.Init(funcPointers[15]);
    m_fpFRepeatTrueLinear.Init(funcPointers[16]);
    m_fpFMirroredTrueLinear.Init(funcPointers[17]);
    m_fpFUndefTrans.Init(funcPointers[18]);
    m_fpFFUndefTrans.Init(funcPointers[19]);
    assert(TRANS_CBK_COUNT == 20);

    ////////////////// Fill in read callback pointers //////////////////

    int32_t InitIndex = TRANS_CBK_COUNT;

    // number of supported image formats
    const int32_t IMG_FORMATS_COUNT = INT_FORMATS_COUNT + FLOAT_FORMATS_COUNT;
    int32_t TypeIndex[IMG_FORMATS_COUNT];
    TypeIndex[0]=TYPE_ORDER_TO_INDEX(CLK_UNSIGNED_INT8, CLK_RGBA);
    TypeIndex[1]=TYPE_ORDER_TO_INDEX(CLK_UNSIGNED_INT16, CLK_RGBA);
    TypeIndex[2]=TYPE_ORDER_TO_INDEX(CLK_UNSIGNED_INT32, CLK_RGBA);
    TypeIndex[3]=TYPE_ORDER_TO_INDEX(CLK_SIGNED_INT8, CLK_RGBA);
    TypeIndex[4]=TYPE_ORDER_TO_INDEX(CLK_SIGNED_INT16, CLK_RGBA);
    TypeIndex[5]=TYPE_ORDER_TO_INDEX(CLK_SIGNED_INT32, CLK_RGBA);
    TypeIndex[6]=TYPE_ORDER_TO_INDEX(CLK_UNSIGNED_INT8, CLK_R);
    TypeIndex[7]=TYPE_ORDER_TO_INDEX(CLK_UNSIGNED_INT16, CLK_R);
    TypeIndex[8]=TYPE_ORDER_TO_INDEX(CLK_UNSIGNED_INT32, CLK_R);
    TypeIndex[9]=TYPE_ORDER_TO_INDEX(CLK_SIGNED_INT8, CLK_R);
    TypeIndex[10]=TYPE_ORDER_TO_INDEX(CLK_SIGNED_INT16, CLK_R);
    TypeIndex[11]=TYPE_ORDER_TO_INDEX(CLK_SIGNED_INT32, CLK_R);
    TypeIndex[12]=TYPE_ORDER_TO_INDEX(CLK_UNSIGNED_INT8, CLK_RG);
    TypeIndex[13]=TYPE_ORDER_TO_INDEX(CLK_UNSIGNED_INT16, CLK_RG);
    TypeIndex[14]=TYPE_ORDER_TO_INDEX(CLK_UNSIGNED_INT32, CLK_RG);
    TypeIndex[15]=TYPE_ORDER_TO_INDEX(CLK_SIGNED_INT8, CLK_RG);
    TypeIndex[16]=TYPE_ORDER_TO_INDEX(CLK_SIGNED_INT16, CLK_RG);
    TypeIndex[17]=TYPE_ORDER_TO_INDEX(CLK_SIGNED_INT32, CLK_RG);
    assert(INT_FORMATS_COUNT == 18);

    TypeIndex[18]=TYPE_ORDER_TO_INDEX(CLK_FLOAT, CLK_RGBA);
    TypeIndex[19]=TYPE_ORDER_TO_INDEX(CLK_FLOAT, CLK_INTENSITY);
    TypeIndex[20]=TYPE_ORDER_TO_INDEX(CLK_UNORM_INT8, CLK_INTENSITY);
    TypeIndex[21]=TYPE_ORDER_TO_INDEX(CLK_UNORM_INT16, CLK_INTENSITY);
    TypeIndex[22]=TYPE_ORDER_TO_INDEX(CLK_HALF_FLOAT, CLK_INTENSITY);
    TypeIndex[23]=TYPE_ORDER_TO_INDEX(CLK_UNORM_INT8, CLK_RGBA);
    TypeIndex[24]=TYPE_ORDER_TO_INDEX(CLK_UNORM_INT16, CLK_RGBA);
    TypeIndex[25]=TYPE_ORDER_TO_INDEX(CLK_HALF_FLOAT, CLK_RGBA);
    TypeIndex[26]=TYPE_ORDER_TO_INDEX(CLK_FLOAT, CLK_LUMINANCE);
    TypeIndex[27]=TYPE_ORDER_TO_INDEX(CLK_UNORM_INT8, CLK_LUMINANCE);
    TypeIndex[28]=TYPE_ORDER_TO_INDEX(CLK_UNORM_INT16, CLK_LUMINANCE);
    TypeIndex[29]=TYPE_ORDER_TO_INDEX(CLK_HALF_FLOAT, CLK_LUMINANCE);
    TypeIndex[30]=TYPE_ORDER_TO_INDEX(CLK_UNORM_INT8, CLK_BGRA);
    TypeIndex[31]=TYPE_ORDER_TO_INDEX(CLK_FLOAT, CLK_R);
    TypeIndex[32]=TYPE_ORDER_TO_INDEX(CLK_UNORM_INT8, CLK_R);
    TypeIndex[33]=TYPE_ORDER_TO_INDEX(CLK_UNORM_INT16, CLK_R);
    TypeIndex[34]=TYPE_ORDER_TO_INDEX(CLK_HALF_FLOAT, CLK_R);
    TypeIndex[35]=TYPE_ORDER_TO_INDEX(CLK_FLOAT, CLK_A);
    TypeIndex[36]=TYPE_ORDER_TO_INDEX(CLK_UNORM_INT8, CLK_A);
    TypeIndex[37]=TYPE_ORDER_TO_INDEX(CLK_UNORM_INT16, CLK_A);
    TypeIndex[38]=TYPE_ORDER_TO_INDEX(CLK_HALF_FLOAT, CLK_A);
    TypeIndex[39]=TYPE_ORDER_TO_INDEX(CLK_FLOAT, CLK_RG);
    TypeIndex[40]=TYPE_ORDER_TO_INDEX(CLK_UNORM_INT8, CLK_RG);
    TypeIndex[41]=TYPE_ORDER_TO_INDEX(CLK_UNORM_INT16, CLK_RG);
    TypeIndex[42]=TYPE_ORDER_TO_INDEX(CLK_HALF_FLOAT, CLK_RG);
    assert(IMG_FORMATS_COUNT == 43);
    assert(INT_FORMATS_COUNT + FLOAT_FORMATS_COUNT == IMG_FORMATS_COUNT);

    // Fill in integer image callbacks
    for (int32_t i=0;i<INT_FORMATS_COUNT;i++){
        m_fpNearestNoClamp[TypeIndex[i]].Init(funcPointers[InitIndex++]);
        m_fpNearestClamp[TypeIndex[i]].Init(funcPointers[InitIndex++]);
        m_fpWriteImage[TypeIndex[i]].Init(funcPointers[InitIndex++]);
    }

    // Fill in floating point callbacks
    for (int32_t i=INT_FORMATS_COUNT;i<IMG_FORMATS_COUNT;i++){
        m_fpNearestNoClamp[TypeIndex[i]].Init(funcPointers[InitIndex++]);
        m_fpNearestClamp[TypeIndex[i]].Init(funcPointers[InitIndex++]);
        m_fpLinearNoClamp2D[TypeIndex[i]].Init(funcPointers[InitIndex++]);
        m_fpLinearClamp2D[TypeIndex[i]].Init(funcPointers[InitIndex++]);
        m_fpLinearNoClamp3D[TypeIndex[i]].Init(funcPointers[InitIndex++]);
        m_fpLinearClamp3D[TypeIndex[i]].Init(funcPointers[InitIndex++]);
        m_fpWriteImage[TypeIndex[i]].Init(funcPointers[InitIndex++]);
        
    }

    assert(InitIndex == FUNCTIONS_COUNT - 1);
    // set undefined float reading callback
    m_fpUndefReadQuad.Init(funcPointers[FUNCTIONS_COUNT-1]);

    m_pCompiler = pCompiler;
}



}}} // namespace
