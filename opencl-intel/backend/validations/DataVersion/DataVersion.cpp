/*****************************************************************************\

Copyright (c) Intel Corporation (2013).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  DataVersion.cpp

\*****************************************************************************/
#include "DataVersion.h"
#include "Image.h"
#include "Buffer.h"
#include "llvm/IR/Function.h"
#include "cl_types.h"

#undef DEBUG_TYPE
#define DEBUG_TYPE "DataVersion"
#include <llvm/Support/raw_ostream.h>
#include "llvm/Support/Debug.h"

enum convertEnum
{
    ADDRESS_BASE                                = 0,
    CL_DEV_SAMPLER_ADDRESS_NONE                 = 0,
    CL_DEV_SAMPLER_ADDRESS_CLAMP                = 1 << ADDRESS_BASE ,    //!< Sampler is defined with CLAMP attribute
    CL_DEV_SAMPLER_ADDRESS_CLAMP_TO_EDGE        = 2 << ADDRESS_BASE,    //!< Sampler is defined with CLAMP_TO_EDGE attribute
    CL_DEV_SAMPLER_ADDRESS_REPEAT               = 3 << ADDRESS_BASE,    //!< Sampler is defined with REPEAT attribute
    CL_DEV_SAMPLER_ADDRESS_MIRRORED_REPEAT      = 4 << ADDRESS_BASE,    //!< Sampler is defined with MIRRORED_REPEAT attribute
    ADDRESS_BITS                                = 3,                    //!< number of bits required to represent address info
    ADDRESS_MASK                                = ( (1<<ADDRESS_BITS) -1),

    NORMALIZED_BASE                             = ADDRESS_BITS,
    CL_DEV_SAMPLER_NORMALIZED_COORDS_FALSE      = 0,                        //!< Sampler is defined with normalized coordinates set to FALSE
    CL_DEV_SAMPLER_NORMALIZED_COORDS_TRUE       = 1 << NORMALIZED_BASE,     //!< Sampler is defined with normalized coordinates set to TRUE
    NORMALIZED_BITS                             = 1,                        //!< number of bits required to represent normalize coordinates selection
    NORMALIZED_MASK                             = ( ((1<<NORMALIZED_BITS)-1) << NORMALIZED_BASE ),

    FILTER_BASE                                 = NORMALIZED_BASE + NORMALIZED_BITS,
    CL_DEV_SAMPLER_FILTER_NEAREST               = 0 << FILTER_BASE,        //!< Sampler is defined with filtering set to NEAREST
    CL_DEV_SAMPLER_FILTER_LINEAR                = 1 << FILTER_BASE,        //!< Sampler is defined with filtering set to LINEAR
    FILTER_BITS                                 = 2,                        //!< number of bits required to represent filter info
    FILTER_MASK                                 = ( ((1<<FILTER_BITS)-1) << FILTER_BASE)
};


using namespace Validation;

static void convertSampler(uint32_t* inOutSampler) {
    uint32_t sampler = 0;

    switch((*inOutSampler) & ADDRESS_MASK)
    {
    case CL_DEV_SAMPLER_ADDRESS_NONE:
        sampler |= CLK_ADDRESS_NONE; break;
    case CL_DEV_SAMPLER_ADDRESS_CLAMP:
        sampler |= CLK_ADDRESS_CLAMP; break;
    case CL_DEV_SAMPLER_ADDRESS_CLAMP_TO_EDGE:
        sampler |= CLK_ADDRESS_CLAMP_TO_EDGE; break;
    case CL_DEV_SAMPLER_ADDRESS_REPEAT:
        sampler |= CLK_ADDRESS_REPEAT; break;
    case CL_DEV_SAMPLER_ADDRESS_MIRRORED_REPEAT:
        sampler |= CLK_ADDRESS_MIRRORED_REPEAT; break;
    default:
        throw Exception::InvalidArgument("DataVersion::convertSampler can't convert the old sampler format : addressing mode");
        break;
    }

    switch((*inOutSampler) & FILTER_MASK)
    {
    case CL_DEV_SAMPLER_FILTER_NEAREST:
        sampler |= CLK_FILTER_NEAREST; break;
    case CL_DEV_SAMPLER_FILTER_LINEAR:
        sampler |= CLK_FILTER_LINEAR; break;
    default:
        throw Exception::InvalidArgument("DataVersion::convertSampler can't convert the old sampler format : filter type");
    }

    *inOutSampler = sampler;
}

// this function returns vector of order numbers of "sampler_t" arguments of kernel
static std::vector<unsigned int> FindSamplers(llvm::NamedMDNode* metadata, std::string kernelNameIn) {
    // TODO: use MetaDataUtils instead of the manual traversal of metadata below
    std::vector<unsigned int> res;

    // return empty vector if there are no metadata
    if(NULL == metadata)
        return res;
    // look thru the all kernels
    for (unsigned int j = 0; j < metadata->getNumOperands(); ++j) {

        llvm::MDNode *elt = metadata->getOperand(j);

        // if elt is NULL we can't find "sampler_t" so return the empty vector
        if((NULL != elt) && (NULL != elt->getOperand(0))) {
            llvm::Function* pKernel =  llvm::mdconst::dyn_extract<llvm::Function>(elt->getOperand(0));
            assert(pKernel  && "FindSamplers : kernel pointer is NULL");
            std::string kernelName = pKernel->stripPointerCasts()->getName().str();
            // skip if it is not the kernel we want to scan
            if(kernelNameIn != kernelName)
                continue;

            //scan metadata for kernel_arg_type
            llvm::MDNode* pTypeNames = NULL;
            for (unsigned int i = 0; i < elt->getNumOperands(); ++i)
            {
                llvm::MDNode* pNode = llvm::dyn_cast<llvm::MDNode>(elt->getOperand(i));
                if (NULL == pNode){
                    continue;
                }

                llvm::MDString* pName = llvm::cast<llvm::MDString>(pNode->getOperand(0));
                if (NULL == pName) {
                    continue;
                }

                if (0 == pName->getString().compare("kernel_arg_type")) {
                    pTypeNames = pNode;
                    break;
                }
            }

            // if pTypeNames is NULL we can't find "sampler_t" so return the empty vector
            if(NULL != pTypeNames) {
                // looking for "sampler_t" thru the list of arguments of kernel, starting
                // from number 1, number 0 is "kernel_arg_type" itself
                for (unsigned int i = 1; i < pTypeNames->getNumOperands(); ++i)
                {
                    llvm::MDString* pTypeName = llvm::cast<llvm::MDString>(pTypeNames->getOperand(i));
                    std::string szTypeName = pTypeName->getString().str();
                    DEBUG(llvm::dbgs() << "kernel argument type " << (i-1) << " " << szTypeName << "\n");
                    if (szTypeName == std::string("sampler_t")) {
                        //the zero operand in the list is "kernel_arg_type", so push number i-1, not i
                        res.push_back(i-1);
                    }
                }
            }
        }
    }
    return res;
}

void ConvertData_v0_to_v1(IBufferContainerList* pContainerList, llvm::NamedMDNode* metadata, std::string kernelName) {

    const IBufferContainer* pBufferContainer = pContainerList->GetBufferContainer(0);

    std::vector<unsigned int > samplerIndxs = FindSamplers(metadata, kernelName);

    for (unsigned int i = 0; i < samplerIndxs.size(); ++i)
    {
        uint32_t *pSampler = (uint32_t*)pBufferContainer->GetMemoryObject(samplerIndxs[i])->GetDataPtr();
        // converting sampler to the new format
        convertSampler(pSampler);
    }
}

typedef void (*ConvertData) (IBufferContainerList* , llvm::NamedMDNode*, std::string);

// the table of converters
ConvertData arrConvertData[] = {&ConvertData_v0_to_v1};


void DataVersion::ConvertData (IBufferContainerList* pContainerList, llvm::NamedMDNode* metadata, std::string kernelName) {
    uint32_t finalVer = GetCurrentDataVersion();
    uint32_t numDataVersions = sizeof(arrConvertData)/sizeof(arrConvertData[0]);

    if(finalVer > numDataVersions)
        throw Exception::InvalidArgument("DataVersion::ConvertData can't convert data : version number is bigger than the highest version of available convertor");
    else {
        // each converter changes the data in the container list
        for (uint32_t i = pContainerList->GetDataVersion(); i < finalVer; i++) {
            arrConvertData[i](pContainerList, metadata, kernelName);
        }
    }
}
