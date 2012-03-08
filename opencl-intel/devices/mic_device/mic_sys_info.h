/* ************************************************************************* *\
#               INTEL CORPORATION PROPRIETARY INFORMATION
#     This software is supplied under the terms of a license agreement or
#     nondisclosure agreement with Intel Corporation and may not be copied
#     or disclosed except in accordance with the terms of that agreement.
#        Copyright (C) 2011 Intel Corporation. All Rights Reserved.
#\* ************************************************************************* */

#pragma once

#include "cl_device_api.h"
#include <source/COIEngine_source.h>
#include <pthread.h>
#include <string.h>
#include <map>

namespace Intel { namespace OpenCL { namespace MICDevice {

    class MICSysInfo
    {

    public:

        /* Destructor */
        virtual ~MICSysInfo();

           /* Return singelton instance of MICSysInfo */
        static MICSysInfo& getInstance();

        static cl_dev_err_code clDevGetDeviceInfo(uint32_t deviceId, cl_device_info IN param, size_t IN valSize, void* OUT paramVal, size_t* OUT paramValSizeRet);

    // the following methods may be used internally inside MIC device

        /* Returns the number of engines in the system that match COI_ISA_KNF.
           Calculate it once (Thread safe) */
        uint32_t getEngineCount();

        /* Return the number of parallel compute cores on the device.
           If failed return 1 (According to OpenCL 1.1 spec the minimum value is 1)*/
        cl_uint getNumOfComputeUnits(uint32_t deviceId);

        /* Return maximum number of samplers that can be used in the kernel
           If failed return 16 (According to OpenCL 1.1 spec the minimum value is 16 if CL_DEVICE_IMAGE_SUPPORT is True)*/
        cl_uint getMaxSamplers(uint32_t deviceId);

        /* Return size of global memory cache line in bytes
           If failed return 0.*/
        cl_uint getGlobalMemCachelineSize(uint32_t deviceId);

        /* Return size of global memory cache in bytes
           If failed return 0 */
        cl_ulong getGlobalMemCacheSize(uint32_t deviceId);

        /* Return maximum configured clock frequency of the device in MHz.
        If failed return 0 */
        cl_uint getMaxClockFrequency(uint32_t deviceId);

        /* Return profiling timer resolution.
        If failed return 0 */
        unsigned long long getProfilingTimerResolution(uint32_t deviceId);

        /* Return total physical memory size.
           If failed return 0 */
        unsigned long long TotalPhysicalMemSize(uint32_t deviceId);

        /* Return space delimited string with OpenCL extension names
           If failed return NULL */
        const char* getSupportedOclExtensions(uint32_t deviceId);

        /* Return COI Engine Handle, NULL on error */
        COIENGINE getCOIEngineHandle(uint32_t deviceId);

        /* Return array of required device DLLs
           Return value - count of strings
           string_arr   - pointer to the array of string pointers */
        unsigned int getRequiredDeviceDLLs(uint32_t deviceId, const char* const **string_arr);

        ///////////////////////////////////////////////////////////////////
        //
        // Info Data Base - internal
        //
        ///////////////////////////////////////////////////////////////////

        enum InfoValueType
        {
            VALUE_SCALAR   = 0, // scalar or array or scalars with size < szie_t
            VALUE_STRING,       // pointer to string
            VALUE_FUNCTION      // member function callback
        };

        typedef cl_dev_err_code (MICSysInfo::*TInfoFunc)(
                                 uint32_t deviceId,
                                 cl_device_info param,
                                 size_t buf_size, void* buf,
                                 size_t* filled_buf_size );

        struct SYS_INFO_ENTRY
        {
            cl_device_info info_id;
            size_t         array_count; // if array_count == 1 -> scalar, else array
            uint64_t       type_size;   // scalar size, must be < sizeof(uint64_t)
            InfoValueType  si_value_type;

            // mutulally exclusive values
            size_t         const_value; // either scalar or pointer to array of scalars
            TInfoFunc      func_value;

            // for debug
            const char*    info_id_name;
        };

        cl_dev_err_code get_variable_info(
                                uint32_t deviceId,
                                cl_device_info param,
                                size_t buf_size,
                                void* buf,
                                size_t* filled_buf_size);

        // device SKU implementation attributes, not defined by OpenCL standard
        struct DeviceSKU_InternalAttributes
        {
            size_t              required_dlls_count;
            const char* const*  required_dlls_array;

            DeviceSKU_InternalAttributes() :
                required_dlls_count(0), required_dlls_array(NULL) {};
        };

        // this function must be called at constructor/destructor
        static void add_sku_info( uint64_t sku_key, size_t entries, const SYS_INFO_ENTRY* array,
                                  const DeviceSKU_InternalAttributes& attribs );
        static void clear_sku_info( void );

        // map with all info - used as a key for TSku2DevData
        union InfoKeyType
        {
            struct {
                COI_ISA_TYPE device_type;
                // here will come other SKU values
            } fields;

            uint64_t full_key;
        };

    private:

        // map of cl_device_info -> data entry for specific device SKU
        typedef std::map< cl_device_info, const SYS_INFO_ENTRY*>    TInfoType2Data;

        struct InfoType2DataEntry
        {
            TInfoType2Data                  data_map;
            DeviceSKU_InternalAttributes    internal_attribs;
        };

        // map of all supported SKUs by InfoKeyType
        typedef std::map< uint64_t, InfoType2DataEntry*>            TSku2DevData;

        // the whole data base
        static TSku2DevData                                         m_info_db;

        ///////////////////////////////////////////////////////////////////
        //
        // Dynamic data
        //
        ///////////////////////////////////////////////////////////////////

        struct engineInfo {
            const InfoType2DataEntry*   data_table;
            COIENGINE                   engine_handle;
            COI_ENGINE_INFO             micDeviceInfoStruct;

            engineInfo() : data_table(NULL), engine_handle(NULL) { memset( &micDeviceInfoStruct, 0, sizeof(micDeviceInfoStruct)); };
        };

        struct guardedInfo
        {
            pthread_mutex_t      mutex;
            engineInfo* volatile devInfoStruct;
        };

        /* private constructor because I want single instance */
        MICSysInfo();

        enum SINGLETON_KEEPER_STATE
        {
            BEFORE_INIT = 0,
            CREATION,
            READY
        };
        /* Keeper for singleton creation
           If '0' can create a new instance.
           If '1' in creation process of other thread.
           If '2' already created */
        static volatile unsigned int m_singletonKeeper;

        /* Singelton object of MICSysInfo */
        static MICSysInfo* m_singleton;

        /* Array contains guardedInfo struct for each MIC device*/
        guardedInfo* m_guardedInfoArr;

        /* Contains the num engines set once when calling to getEngineCount() first time */
        volatile uint32_t m_numEngines;

        /* mutex guard */
        pthread_mutex_t m_mutex;

        /* Initialize once the m_micDeviceInfoStruct data structure.
           Return true if succeded and false otherwise.
           If fails, delete the m_micDeviceInfoStruct resources
           (Thread safe) */
        bool initializedInfoStruct(uint32_t deviceId);


    };

}}}
