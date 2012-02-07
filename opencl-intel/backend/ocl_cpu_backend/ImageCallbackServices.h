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

File Name:  ImageCallbackService.h

\*****************************************************************************/

#pragma once

#include "cl_dev_backend_api.h"
#include "ImageCallbackManager.h"
#include "ICLDevBackendProgram.h"
#include "BackendConfiguration.h"
#include "cl_device_api.h"
#include "cl_types.h"
//#include "cl.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {


/**
 * This interface class is responsible for the image callback services, i.e., 
 * giving the callback module to the device, and supplying the API for image creation/deletion
 * functions in the BE
 ***/

class ImageCallbackService : public ICLDevBackendImageService
{
public:

    /* Initializes the service with the proper options, getting the architecture */
    ImageCallbackService(CompilerConfiguration& config, bool isCpu);

    /**
    returns the size of the auxilary structure for
    **/
    size_t GetAuxilarySize() const {return sizeof(image_aux_data);}

    /**
     * Endues the image object with the auxilary data, and the proper callback functions assigned, according to the architecture
     * with which this service has been initialized
     *  pImageObject - mem_object descriptor. General descriptor of image object
     *  auxObject - Pointer to auxiliary data structure to fill in.
     *              It is initialized with callbacks and set to pImageObject->imageAuxData
     **/
    cl_dev_err_code CreateImageObject(cl_mem_obj_descriptor* pImageObject, void* auxObject) const;

    /** 
    *  Releases the auxilary data from the image object
    **/

    cl_dev_err_code DeleteImageObject(cl_mem_obj_descriptor* pImageObject, void** auxObject) const;

    /**
    *  Returns an array of supported image formats
    ***/

    const cl_image_format* GetSupportedImageFormats(unsigned int *numFormats);
 
    /*releases the services instance. Right now nothing is realeased because all image
     *resources are stored in imageCallbackLibrary singleton class
     */
    void Release(){};

private:
    Intel::ECPU m_ArchId;
    unsigned int m_ArchFeatures;

};

}}} // namespace

