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
#include "Helpers.h"
#include "IBLTMapFiller.h"
#include "RefALU.h"
#include "ImagesALU.h"

struct _cl_mem_obj_descriptor;
typedef struct _cl_mem_obj_descriptor cl_mem_obj_descriptor;

namespace Validation {
namespace OCLBuiltins {

    /// This class adds references to the implementations of OpenCL built-in functions 
    /// from 6.11.13 section Images
    class ImageMapFiller : public IBLTMapFiller
    {
    public:
        void addOpenCLBuiltins(std::map<std::string, PBLTFunc>& funcNames);
    };

    llvm::GenericValue lle_X_write_image(const llvm::FunctionType *FT, 
        const std::vector<llvm::GenericValue> &Args);

    llvm::GenericValue lle_X_get_image_dim2(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args);

    llvm::GenericValue lle_X_get_image_width(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args);

    llvm::GenericValue lle_X_get_image_height(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args);

    llvm::GenericValue lle_X_get_image_depth(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args);

    llvm::GenericValue lle_X_get_image_dim3(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args);

    llvm::GenericValue lle_X_get_image_channel_data_type(const llvm::FunctionType *FT,
        const std::vector<llvm::GenericValue> &Args);

    llvm::GenericValue lle_X_get_image_channel_order(const llvm::FunctionType *FT,
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
