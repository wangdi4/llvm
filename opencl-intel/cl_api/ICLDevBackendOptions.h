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

#ifndef ICLDevBackendOptions_H
#define ICLDevBackendOptions_H

#include <cstdlib>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

/*! \enum cl_dev_backend_options
 * Defines possible values of the backend options
 */
enum cl_dev_backend_options                            // Description                           | Type                   | Values/Format/Exampl
{                                                      // --------------------------------------+------------------------+----------------------
    CL_DEV_BACKEND_OPTION_DEVICE,                      //!< Device selection                    |DeviceMode              | "CPU_DEVICE","FPGA_EMU_DEVICE","EYEQ_EMU_DEVICE"
    CL_DEV_BACKEND_OPTION_SUBDEVICE,                   //!< Sub-device selection                |string                  | "corei7","sandybride" for cpu or "knc" for mic
    CL_DEV_BACKEND_OPTION_SUBDEVICE_FEATURES,          //!< Enable/Disable specific CPU features|string                  | "+avx,-avx256"
    CL_DEV_BACKEND_OPTION_TRANSPOSE_SIZE,              //!< Transpose size use in vectorizer    |ETranposeSize           |
    CL_DEV_BACKEND_OPTION_RT_LOOP_UNROLL_FACTOR,       //!< Loop Unroll Factor                  |int                     |
    CL_DEV_BACKEND_OPTION_USE_VTUNE,                   //!< VTune support                       |boolean                 |
    CL_DEV_BACKEND_OPTION_LOGGER_CALLBACK,             //!< Pointer to the logger callback      |pointer                 |
    CL_DEV_BACKEND_OPTION_JIT_ALLOCATOR,               //!< Pointer to the JIT mem. allocator   |pointer                 | (Used in serialization)
    CL_DEV_BACKEND_OPTION_DUMP_IR_DIR,                 //!< Directory for dumping the IR files  |string                  |
    CL_DEV_BACKEND_OPTION_TARGET_DESC_BLOB,            //!< Target Description Blob             |buffer                  |
    CL_DEV_BACKEND_OPTION_TARGET_DESC_SIZE,            //!< Target Description Blob Size        |int                     |
    CL_DEV_BACKEND_OPTION_TIME_PASSES,                 //!< Filename for dumping time passes    |string                  |
    CL_DEV_BACKEND_OPTION_DISABLE_STACKDUMP,           //!< Disables stack dump on crash        |boolean                 |
    CL_DEV_BACKEND_OPTION_DUMP_HEURISTIC_IR,           //!< Print IR input to heuristic         |boolean                 |
    CL_DEV_BACKEND_OPTION_INJECTED_OBJECT,             //!< Pointer to preloaded object file    |pointer                 |
    CL_DEV_BACKEND_OPTION_DUMPFILE,                    //!< file name to store the dump into    |string                  |
    CL_DEV_BACKEND_OPTION_APF_LEVEL,                   //!< select auto-prefetch level          |int                     | 0 - 3
    CL_DEV_BACKEND_OPTION_STOP_BEFORE_JIT,             //!< Stops compilation before JITing     |boolean                 | (Used for early AVX3 development)
    CL_DEV_BACKEND_OPTION_IDEVICE_COMMAND_MANAGER,     //!<a pointer to IDeviceCommandManager   | IDeviceCommandManager* | pointer
    CL_DEV_BACKEND_OPTION_FORCED_PRIVATE_MEMORY_SIZE,  //!< Set additional memory size          |size_t                  |
    CL_DEV_BACKEND_OPTION_CHANNEL_DEPTH_EMULATION_MODE,//!< Set channel depth emulation mode    |int                     |
    CL_DEV_BACKEND_OPTION_VECTORIZER_TYPE,             //!< Select vectorizer                   |VectorizerType          | VOLCANO_VECTORIZER,VPO_VECTORIZER
    CL_DEV_BACKEND_OPTION_NATIVE_SUBGROUPS,            //!< Enable native subgroups             |boolean                 |
    CL_DEV_BACKEND_OPTION_FORCED_WORK_GROUP_SIZE       //!< Set additional work group size      |size_t                  |
};

/*! \enum cl_dev_backend_dump_options
 * Defines the options ids used during the compile service dump method
 */
/*
enum cl_dev_backend_dump_options
{
    CL_DEV_BACKEND_OPTION_DUMPFILE,  //!< string - file name to store the dump into. If not specified - use stdout
    CL_DEV_BACKEND_OPTION_DUMPTYPE   //!< int - dump type. Possible values are not defined currently
};
*/

/*! \enum ETransposeSize
 * Defines possible values for backend vectorization support
 */
enum ETransposeSize
{
    TRANSPOSE_SIZE_AUTO = 0, // Automatic selection. Backend will selectet if and how to vectorize based on heuristics
    TRANSPOSE_SIZE_1  = 1,   // Scalar mode. Do not use the vectorizer
    TRANSPOSE_SIZE_4  = 4,   // Force the vectorization with packetization size of 4
    TRANSPOSE_SIZE_8  = 8,   // Force the vectorization with packetization size of 8
    TRANSPOSE_SIZE_16 = 16,  // Force the vectorization with packetization size of 16
    TRANSPOSE_SIZE_NOT_SET
};

/*! \dump_IR_options
 * Defines the options for dumping IR - before or after a certain pass
 */
enum dump_IR_options
{
    OPTION_IR_DUMPTYPE_BEFORE = 1,
    OPTION_IR_DUMPTYPE_AFTER
};

/*! \enum IRDumpOptions
 * Defines possible places for printing IR, after/before certain optimizations
 */
enum IRDumpOptions
{
    DUMP_IR_ALL = 1,        // Printing after/before each optimization
    DUMP_IR_TARGERT_DATA,   // Printing after/before target data pass
    DUMP_IR_VECTORIZER      // Printing after/before vectorizer pass
};

/**
 * This interface represents the options which can be passed from the User to the Backend
 */
class ICLDevBackendOptions
{
public:
    virtual ~ICLDevBackendOptions() {}

    /**
     * returns the specified option as (boolean\int\string\void*) value
     *
     * @param optionId specified which option
     * @param default value to return
     */
    virtual bool GetBooleanValue(
        int optionId,
        bool defaultValue) const = 0;

    virtual int GetIntValue(
        int optionId,
        int defaultValue) const = 0;

    virtual const char* GetStringValue(
        int optionId,
        const char* defaultValue) const = 0;

    virtual bool GetValue(
        int optionId,
        void* Value,
        size_t* pSize) const = 0;
};

}}} // namespace

#endif // ICLDevBackendOptions_H
