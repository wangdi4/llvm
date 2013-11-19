/* ************************************************************************* *\
#               INTEL CORPORATION PROPRIETARY INFORMATION
#     This software is supplied under the terms of a license agreement or
#     nondisclosure agreement with Intel Corporation and may not be copied
#     or disclosed except in accordance with the terms of that agreement.
#        Copyright (C) 2011 Intel Corporation. All Rights Reserved.
#\* ************************************************************************* */

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <algorithm>
#include <sstream>
#include "stdafx.h"

#include "mic_sys_info.h"
#include "mic_sys_info_internal.h"
#include "mic_device.h"
#include "cl_sys_info.h"
#include "hw_utils.h"
#include "mic_common_macros.h"
#include "mic_device_interface.h"

#ifdef _DEBUG
    #define PRINT_DEBUG(...) fprintf(stderr, ##__VA_ARGS__)
#else
    #define PRINT_DEBUG(...)
#endif

using namespace Intel::OpenCL::MICDevice;

OclSpinMutex*				MICSysInfo::m_mutex = NULL;
MICSysInfo*                 MICSysInfo::m_singleton = NULL;
MICSysInfo::TSku2DevData*   MICSysInfo::m_info_db = NULL;
MICDeviceConfig				MICSysInfo::m_MICDeviceConfig;

MICSysInfo::SInitializer	gSIntializer;

#define __MINUMUM_SUPPORT__
//#define __TEST__
const cl_image_format Intel::OpenCL::MICDevice::suportedImageFormats[] = {
#ifndef __TEST__

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
    {CL_BGRA,    CL_UNORM_INT8},

    // Additional formats required by users
    // CL_INTENCITY
    {CL_INTENSITY,    CL_FLOAT},

    // CL_LUMINANCE
    {CL_LUMINANCE,    CL_FLOAT},
#ifndef __MINUMUM_SUPPORT__
    // CL_R
    {CL_R,        CL_UNORM_INT8},
    {CL_R,        CL_UNORM_INT16},
    {CL_R,        CL_SNORM_INT8},
    {CL_R,        CL_SNORM_INT16},
    {CL_R,        CL_SIGNED_INT8},
    {CL_R,        CL_SIGNED_INT16},
    {CL_R,        CL_SIGNED_INT32},
    {CL_R,        CL_UNSIGNED_INT8},
    {CL_R,        CL_UNSIGNED_INT16},
    {CL_R,        CL_UNSIGNED_INT32},
    //    {CL_R,        CL_HALF_FLOAT},
    {CL_R,        CL_FLOAT},

    // CL_A
    {CL_A,        CL_UNORM_INT8},
    {CL_A,        CL_UNSIGNED_INT8},
    {CL_A,        CL_SNORM_INT8},
    {CL_A,        CL_SIGNED_INT8},
    {CL_A,        CL_UNORM_INT16},
    {CL_A,        CL_UNSIGNED_INT16},
    {CL_A,        CL_SNORM_INT16},
    {CL_A,        CL_SIGNED_INT16},
    {CL_A,        CL_UNSIGNED_INT32},
    {CL_A,        CL_SIGNED_INT32},
    //    {CL_A,        CL_HALF_FLOAT},
    {CL_A,        CL_FLOAT},

    // CL_INTENSITY
    {CL_INTENSITY,    CL_UNORM_INT8},
    {CL_INTENSITY,    CL_UNORM_INT16},
    {CL_INTENSITY,    CL_SNORM_INT8},
    {CL_INTENSITY,    CL_SNORM_INT16},
    //    {CL_INTENSITY,    CL_HALF_FLOAT},
    {CL_INTENSITY,    CL_FLOAT},

    // CL_LUMINANCE
    {CL_LUMINANCE,    CL_UNORM_INT8},
    {CL_LUMINANCE,    CL_UNORM_INT16},
    {CL_LUMINANCE,    CL_SNORM_INT8},
    {CL_LUMINANCE,    CL_SNORM_INT16},
    //    {CL_LUMINANCE,    CL_HALF_FLOAT},
    {CL_LUMINANCE,    CL_FLOAT},

    // CL_RG
    {CL_RG,        CL_UNORM_INT8},
    {CL_RG,        CL_UNSIGNED_INT8},
    {CL_RG,        CL_SNORM_INT8},
    {CL_RG,        CL_SIGNED_INT8},
    {CL_RG,        CL_UNORM_INT16},
    {CL_RG,        CL_UNSIGNED_INT16},
    {CL_RG,        CL_SNORM_INT16},
    {CL_RG,        CL_SIGNED_INT16},
    {CL_RG,        CL_UNSIGNED_INT32},
    {CL_RG,        CL_SIGNED_INT32},
    //    {CL_RG,        CL_HALF_FLOAT},
    {CL_RG,        CL_FLOAT},

    // CL_RA
    {CL_RA,        CL_UNORM_INT8},
    {CL_RA,        CL_UNSIGNED_INT8},
    {CL_RA,        CL_SNORM_INT8},
    {CL_RA,        CL_SIGNED_INT8},
    {CL_RA,        CL_UNORM_INT16},
    {CL_RA,        CL_UNSIGNED_INT16},
    {CL_RA,        CL_SNORM_INT16},
    {CL_RA,        CL_SIGNED_INT16},
    {CL_RA,        CL_UNSIGNED_INT32},
    {CL_RA,        CL_SIGNED_INT32},
    //    {CL_RA,        CL_HALF_FLOAT},
    {CL_RA,        CL_FLOAT},

    // CL_RGB
    {CL_RGB,    CL_UNORM_SHORT_555},
    {CL_RGB,    CL_UNORM_SHORT_565},
    {CL_RGB,    CL_UNORM_INT_101010},

    // CL_RGBA
    {CL_RGBA, CL_SNORM_INT8},
    {CL_RGBA, CL_SNORM_INT16},

    // CL_BGRA
    {CL_BGRA,    CL_SNORM_INT8},
    {CL_BGRA,    CL_SIGNED_INT8},
    {CL_BGRA,    CL_UNSIGNED_INT8},

    // CL_ARGB
    {CL_ARGB,    CL_UNORM_INT8},
    {CL_ARGB,    CL_SNORM_INT8},
    {CL_ARGB,    CL_SIGNED_INT8},
    {CL_ARGB,    CL_UNSIGNED_INT8},
#endif

#else
    //    {CL_RG,        CL_UNORM_INT8},
    {CL_RGB,  CL_UNORM_SHORT_555},
    /*    // CL_INTENSITY
    {CL_INTENSITY,    CL_UNORM_INT8},
    {CL_INTENSITY,    CL_UNORM_INT16},
    {CL_INTENSITY,    CL_SNORM_INT8},
    {CL_INTENSITY,    CL_SNORM_INT16},
    {CL_INTENSITY,    CL_HALF_FLOAT},
    {CL_INTENSITY,    CL_FLOAT},
    */
#endif
};

const unsigned int Intel::OpenCL::MICDevice::NUM_OF_SUPPORTED_IMAGE_FORMATS =
sizeof(Intel::OpenCL::MICDevice::suportedImageFormats)/sizeof(cl_image_format);

extern "C"
const char* clDevErr2Txt(cl_dev_err_code errorCode)
{
    switch(errorCode)
    {
        case (CL_DEV_ERROR_FAIL): return "CL_DEV_ERROR_FAIL";
        case (CL_DEV_INVALID_VALUE): return "CL_DEV_INVALID_VALUE";
        case (CL_DEV_INVALID_PROPERTIES): return "CL_DEV_INVALID_PROPERTIES";
        case (CL_DEV_OUT_OF_MEMORY): return "CL_DEV_OUT_OF_MEMORY";
        case (CL_DEV_INVALID_COMMAND_LIST): return "CL_DEV_INVALID_COMMAND_LIST";
        case (CL_DEV_INVALID_COMMAND_TYPE): return "CL_DEV_INVALID_COMMAND_TYPE";
        case (CL_DEV_INVALID_MEM_OBJECT): return "CL_DEV_INVALID_MEM_OBJECT";
        case (CL_DEV_INVALID_KERNEL): return "CL_DEV_INVALID_KERNEL";
        case (CL_DEV_INVALID_OPERATION): return "CL_DEV_INVALID_OPERATION";
        case (CL_DEV_INVALID_WRK_DIM): return "CL_DEV_INVALID_WRK_DIM";
        case (CL_DEV_INVALID_WG_SIZE): return "CL_DEV_INVALID_WG_SIZE";
        case (CL_DEV_INVALID_GLB_OFFSET): return "CL_DEV_INVALID_GLB_OFFSET";
        case (CL_DEV_INVALID_WRK_ITEM_SIZE): return "CL_DEV_INVALID_WRK_ITEM_SIZE";
        case (CL_DEV_INVALID_IMG_FORMAT): return "CL_DEV_INVALID_IMG_FORMAT";
        case (CL_DEV_INVALID_IMG_SIZE): return "CL_DEV_INVALID_IMG_SIZE";
        case (CL_DEV_OBJECT_ALLOC_FAIL): return "CL_DEV_INVALID_COMMAND_LIST";
        case (CL_DEV_INVALID_BINARY): return "CL_DEV_INVALID_BINARY";
        case (CL_DEV_INVALID_BUILD_OPTIONS): return "CL_DEV_INVALID_BUILD_OPTIONS";
        case (CL_DEV_INVALID_PROGRAM): return "CL_DEV_INVALID_PROGRAM";
        case (CL_DEV_BUILD_IN_PROGRESS): return "CL_DEV_BUILD_IN_PROGRESS";
        case (CL_DEV_INVALID_KERNEL_NAME): return "CL_DEV_INVALID_KERNEL_NAME";

    default: return "Unknown Error Code";
    }
}

MICSysInfo::MICSysInfo()
{
    m_numEngines = 0;
    m_guardedInfoArr = NULL;

	// Initialize MICDeviceConfig
	m_MICDeviceConfig.Initialize(GetConfigFilePath());

	// It is thread safe to allocate this static member here because "m_singleton" the only MICSysInfo instance is guarded.
	m_info_db = new TSku2DevData;
    // add static info tables for supported SKUs
    add_mic_info();
}

MICSysInfo::~MICSysInfo()
{
    if (m_guardedInfoArr)
    {
        for (unsigned int i = 0; i < m_numEngines; i++)
        {
            if (m_guardedInfoArr[i].devInfoStruct)
            {
                delete(m_guardedInfoArr[i].devInfoStruct);
            }
        }
        delete [] m_guardedInfoArr;
        m_guardedInfoArr = NULL;
    }

	if (m_info_db)
	{
		clear_sku_info();
		delete m_info_db;
	}
}

MICSysInfo& MICSysInfo::getInstance()
{
    // if already created
    if (m_singleton != NULL)
    {
        return *m_singleton;
    }

	OclAutoMutex autoMutex(m_mutex);
	// if already created
	if (m_singleton != NULL)
    {
        return *m_singleton;
    } 
	MICSysInfo* tMicSysInfo = new MICSysInfo();
    assert(tMicSysInfo && "MICSysInfo allocation failed");

    COIRESULT result = COI_ERROR;
    uint32_t tNumEngines = 0;

    // Let's make sure there is a MIC device available
    result = COIEngineGetCount(CL_COI_ISA_MIC, &tNumEngines);
    if( result != COI_SUCCESS )
    {
        PRINT_DEBUG("MIC: COIEngineGetCount result %s\n", COIResultGetName(result));
        tNumEngines = 0;
    }

    // If there isn't at least one engine, there is something wrong
    if( tNumEngines < 1)
    {
        //PRINT_DEBUG("MIC: ERROR: Need at least 1 engine\n");
        tNumEngines = 0;
    }

    if (tNumEngines > 0)
    {
		// Read "OFFLOAD_DEVICES" environment variable in order to get the devices IDs that specified as available.
		assert(tMicSysInfo->m_deviceIdToCoiEngineId.size() == 0);
		tMicSysInfo->getAvailableOffloadDevicesFromEnv(tMicSysInfo->m_deviceIdToCoiEngineId, tNumEngines);
		// If "OFFLOAD_DEVICES" is not set or it define illegal IDs than set m_restrictsDeviceIDs with device IDs 0..(tNumEngines-1)
		if (tMicSysInfo->m_deviceIdToCoiEngineId.size() == 0)
		{
			for (unsigned int i = 0; i < tNumEngines; i++)
			{
				tMicSysInfo->m_deviceIdToCoiEngineId.push_back(i);
			}
		}
		assert(tNumEngines > 0);
		// set tNumEngines as the amount of restricts devices
		tNumEngines = tMicSysInfo->m_deviceIdToCoiEngineId.size();

        tMicSysInfo->m_guardedInfoArr = new guardedInfo[tNumEngines];
        assert(tMicSysInfo->m_guardedInfoArr && "m_guardedInfoArr allocation failed");
        for (unsigned int i = 0; i < tNumEngines; i++)
        {
            tMicSysInfo->m_guardedInfoArr[i].devInfoStruct = NULL;
        }
    }

    tMicSysInfo->m_numEngines = tNumEngines;

	m_singleton = tMicSysInfo;
    return *m_singleton;
}

cl_uint MICSysInfo::getNumOfCores(uint32_t deviceId)
{
    if (initializedInfoStruct(deviceId))
    {
        return m_guardedInfoArr[deviceId].devInfoStruct->micDeviceInfoStruct.NumCores;
    }
    return 0;
}

cl_uint MICSysInfo::getNumOfComputeUnits(uint32_t deviceId)
{
    if (initializedInfoStruct(deviceId))
    {
        return m_guardedInfoArr[deviceId].devInfoStruct->micDeviceInfoStruct.NumThreads;
    }
    return 0;
}

cl_uint MICSysInfo::getGlobalMemCachelineSize(uint32_t deviceId)
{
    if (initializedInfoStruct(deviceId))
    {
        return 64;
    }
    return 0;
}

cl_ulong MICSysInfo::getGlobalMemCacheSize(uint32_t deviceId)
{
    if (initializedInfoStruct(deviceId))
    {
        return 256*1024; //(256k)
    }
    return 0;
}

cl_uint MICSysInfo::getMaxClockFrequency(uint32_t deviceId)
{
    if (initializedInfoStruct(deviceId))
    {
        return m_guardedInfoArr[deviceId].devInfoStruct->micDeviceInfoStruct.CoreMaxFrequency;
    }
    return 0;
}

unsigned long long MICSysInfo::getProfilingTimerResolution(uint32_t deviceId)
{
    if (initializedInfoStruct(deviceId))
    {
        return m_guardedInfoArr[deviceId].devInfoStruct->micDeviceInfoStruct.CoreMaxFrequency;
    }
    return 0;
}

unsigned long long MICSysInfo::TotalPhysicalMemSize(uint32_t deviceId)
{
    if (initializedInfoStruct(deviceId))
    {
        return m_guardedInfoArr[deviceId].devInfoStruct->micDeviceInfoStruct.PhysicalMemory;
    }
    return 0;
}

const char* MICSysInfo::getSupportedOclExtensions(uint32_t deviceId)
{
    if (! initializedInfoStruct(deviceId))
    {
        return NULL;
    }

    const InfoType2DataEntry* data = m_guardedInfoArr[deviceId].devInfoStruct->data_table;
    assert( data && "MICDevice: SKU static data table is not initialized" );

    TInfoType2Data::const_iterator it = data->data_map.find( CL_DEVICE_EXTENSIONS );
    if (data->data_map.end() == it)
    {
        // info not found
        return NULL;
    }

    const SYS_INFO_ENTRY* info_entry = it->second;
    assert( VALUE_STRING == info_entry->si_value_type );

    return (const char*)info_entry->const_value;
}

COIENGINE MICSysInfo::getCOIEngineHandle(uint32_t deviceId)
{
    if (! initializedInfoStruct(deviceId))
    {
        return NULL;
    }

    return m_guardedInfoArr[deviceId].devInfoStruct->engine_handle;
}

unsigned int MICSysInfo::getRequiredDeviceDLLs(uint32_t deviceId, const char* const **string_arr)
{
    if (! initializedInfoStruct(deviceId))
    {
        return 0;
    }

    const InfoType2DataEntry* data = m_guardedInfoArr[deviceId].devInfoStruct->data_table;
    assert( data && "MICDevice: SKU static data table is not initialized" );

    assert( NULL != string_arr );
    *string_arr = data->internal_attribs.required_dlls_array;
    return (unsigned int)(data->internal_attribs.required_dlls_count);
}

inline bool process_info_params( size_t required_size,
                                 size_t buf_size, void* buf,
                                 size_t* filled_buf_size )
{
    *filled_buf_size = required_size;
    if(NULL != buf && buf_size < required_size)
    {
        return false;
    }

    return true;
}

cl_dev_err_code MICSysInfo::get_variable_info(
                                uint32_t        deviceId,
                                cl_device_info  param,
                                size_t          buf_size,
                                void*           buf,
                                size_t*         filled_buf_size)
{
    if (NULL == filled_buf_size)
    {
        return CL_DEV_INVALID_VALUE;
    }

    switch (param)
    {
        // Temporary disable until fission implementation
		//case( CL_DEVICE_PARTITION_MAX_SUB_DEVICES):
        case( CL_DEVICE_MAX_COMPUTE_UNITS):
        {
            if(! process_info_params( sizeof(cl_uint), buf_size, buf, filled_buf_size ))
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != buf)
            {
                // this calculation should mimic the calculation done at device startup in TBB init procedure 
                // but because info may be queued yed before the device was start - we cannot use the data from real device.
                // we also cannot calculate it here as calculation requires configuration file that is created in 
                // device constructor on host. 
                // In any case data end user cannot influence the real calculations as all coniguration switches are hidden.
                // So return the default HW threads count - (real number of HW cores - 1)*4
                *(cl_uint*)buf = MAX((cl_uint)1, (getNumOfComputeUnits(deviceId) - MIC_NATIVE_MAX_THREADS_PER_CORE));
            }
            return CL_DEV_SUCCESS;
        }

        case( CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE):
        {
            if(! process_info_params( sizeof(cl_uint), buf_size, buf, filled_buf_size ))
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != buf)
            {
                *(cl_uint*)buf = (cl_uint)getGlobalMemCachelineSize(deviceId);
            }
            return CL_DEV_SUCCESS;
        }

        case( CL_DEVICE_GLOBAL_MEM_CACHE_SIZE):
        {
            if(! process_info_params( sizeof(cl_ulong), buf_size, buf, filled_buf_size ))
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != buf)
            {
                *(cl_ulong*)buf = (cl_ulong)getGlobalMemCacheSize(deviceId);
            }
            return CL_DEV_SUCCESS;
        }

        case( CL_DEVICE_MAX_CLOCK_FREQUENCY):
        {
            if(! process_info_params( sizeof(cl_uint), buf_size, buf, filled_buf_size ))
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != buf)
            {
                *(cl_uint*)buf = (cl_uint)getMaxClockFrequency(deviceId);
            }
            return CL_DEV_SUCCESS;
        }

        case( CL_DEVICE_PROFILING_TIMER_RESOLUTION):
        {
            if(! process_info_params( sizeof(size_t), buf_size, buf, filled_buf_size ))
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != buf)
            {
                *(size_t*)buf = (size_t)(getProfilingTimerResolution((deviceId)));
            }
            return CL_DEV_SUCCESS;
        }

        case( CL_DEVICE_GLOBAL_MEM_SIZE):
        {
            if(! process_info_params( sizeof(cl_ulong), buf_size, buf, filled_buf_size ))
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != buf)
            {
                *(cl_ulong*)buf = MIC_MAX_GLOBAL_MEM_SIZE(deviceId);
            }
            return CL_DEV_SUCCESS;
        }

        case( CL_DEVICE_MAX_MEM_ALLOC_SIZE):
        {
            if(! process_info_params( sizeof(cl_ulong), buf_size, buf, filled_buf_size ))
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != buf)
            {
                *(cl_ulong*)buf = MIC_MAX_BUFFER_ALLOC_SIZE(deviceId);
            }
            return CL_DEV_SUCCESS;
        }

        default:
            assert( false && "MICDevice: info function invoked for unknown info type" );
            return CL_DEV_INVALID_VALUE;
    };
    return CL_DEV_SUCCESS;

}


bool MICSysInfo::initializedInfoStruct(uint32_t deviceId)
{
    if (m_numEngines == 0)
    {
        getEngineCount();
    }
    if (deviceId >= m_numEngines)
    {
        //PRINT_DEBUG("device ID (%d) is greater than numEngines (%d)\n", deviceId, m_numEngines);
        return false;
    }
    // If already got sys info from device
    if (m_guardedInfoArr[deviceId].devInfoStruct)
    {
        return true;
    }

	OclAutoMutex autoMutex(&(m_guardedInfoArr[deviceId].mutex));
    if (m_guardedInfoArr[deviceId].devInfoStruct == NULL)
    {
        //TODO - Maybe at the future the engine handle will initialized somewhere else... so we can take it from there instead of get it again
        COIRESULT           result = COI_ERROR;
        COIENGINE           engine = NULL;

		assert(deviceId < m_deviceIdToCoiEngineId.size());

        // Get a handle to the specific MIC engine
        result = COIEngineGetHandle(CL_COI_ISA_MIC, m_deviceIdToCoiEngineId[deviceId], &engine);
        if( result != COI_SUCCESS )
        {
            PRINT_DEBUG("MIC: COIEngineGetHandle result %s\n", COIResultGetName(result));
            return false;
        }

        engineInfo* tEngineInfo = new engineInfo;
        assert(tEngineInfo && "COI_ENGINE_INFO allocation failed");
        result = COIEngineGetInfo(engine, sizeof(COI_ENGINE_INFO), &(tEngineInfo->micDeviceInfoStruct));
        if( result != COI_SUCCESS )
        {
            delete tEngineInfo;
            PRINT_DEBUG("MIC: COIEngineGetInfo result %s\n", COIResultGetName(result));
            return false;
        }

        tEngineInfo->engine_handle = engine;

        // find relevant static table
        InfoKeyType sku;
        sku.full_key = 0;
        sku.fields.device_type = tEngineInfo->micDeviceInfoStruct.ISA;
        TSku2DevData::iterator it = m_info_db->find( sku.full_key );
        if (m_info_db->end() == it)
        {
            delete tEngineInfo;
            PRINT_DEBUG("MIC: cannot find static MICSysInfo table for the SKU %lu\n", sku.full_key );
            return false;
        }

        tEngineInfo->data_table = it->second;

        m_guardedInfoArr[deviceId].devInfoStruct = tEngineInfo;
    }

    return true;
}

void MICSysInfo::add_sku_info( uint64_t sku_key, size_t entries, const SYS_INFO_ENTRY* array, const DeviceSKU_InternalAttributes& attribs )
{
    // add Device SKU info to the global info map
    assert( 0 != sku_key && 0 != entries && NULL != array );

    InfoType2DataEntry* type2data = new InfoType2DataEntry;
    assert( type2data && "Cannot allocate std::map" );

    type2data->internal_attribs = attribs;

    for (size_t i = 0; i < entries; ++i)
    {
        const SYS_INFO_ENTRY& entry = array[i];

        assert( 0 != entry.info_id     && "MICDevice: error in static info map - zero info id" );
        assert( 0 != entry.array_count && "MICDevice: error in static info map - zero values count" );

        // scalar
        assert(((VALUE_SCALAR != entry.si_value_type) || (0 != entry.type_size))
                                       && "MICDevice: error in static info map - zero-sized scalar or array value" );

        assert(((VALUE_SCALAR != entry.si_value_type)  || (entry.type_size <= sizeof(uint64_t)))
                                       && "MICDevice: error in static info map - wrong scalar size" );

        assert(((VALUE_SCALAR != entry.si_value_type) || (1 == entry.array_count) || (0 != entry.const_value))
                                       && "MICDevice: error in static info map - zero array pointer" );

        // string
        assert(((VALUE_STRING != entry.si_value_type) || (0 == entry.type_size))
                                       && "MICDevice: error in static info map - string size must be 0" );

        assert(((VALUE_STRING != entry.si_value_type) || (1 == entry.array_count))
                                       && "MICDevice: error in static info map - string count must be 1" );

        assert(((VALUE_STRING != entry.si_value_type) || (0 != entry.const_value))
                                       && "MICDevice: error in static info map - zero string pointer" );

        // function
        assert(((VALUE_FUNCTION != entry.si_value_type) || (0 == entry.type_size))
                                       && "MICDevice: error in static info map - func size must be 0" );

        assert(((VALUE_FUNCTION != entry.si_value_type) || (1 == entry.array_count))
                                       && "MICDevice: error in static info map - func count must be 1" );

        assert(((VALUE_FUNCTION != entry.si_value_type) || (NULL != entry.func_value))
                                       && "MICDevice: error in static info map - zero func pointer" );

        // insert
        assert( type2data->data_map.find( entry.info_id ) == type2data->data_map.end()
                                       && "MICDevice: error in static info map - repeated info ids" );

        type2data->data_map[ entry.info_id ] = &entry;
    }

    assert( m_info_db->find( sku_key ) == m_info_db->end()
                                       && "MICDevice: error in static info map - repeated adding of the same device SKU info" );

    m_info_db->insert(pair<uint64_t, InfoType2DataEntry*>( sku_key, type2data ));
}

void MICSysInfo::clear_sku_info( void )
{
    TSku2DevData::iterator db_it  = m_info_db->begin();
    TSku2DevData::iterator db_end = m_info_db->end();

    for (; db_it != db_end; ++db_it)
    {
        InfoType2DataEntry* type2data = db_it->second;
        delete type2data;
    }

    m_info_db->clear();
}

void MICSysInfo::getAvailableOffloadDevicesFromEnv(vector<unsigned int>& deviceIdList, unsigned int coiNumEngines)
{
	assert(deviceIdList.size() == 0 && "deviceIdList must be empty list");
	stringstream deviceIdStream(m_MICDeviceConfig.Device_offloadDevices());
	vector<int> idsArr;
	int tDevId = 0;
	while (deviceIdStream >> tDevId)
    {
        idsArr.push_back(tDevId);
        char c = deviceIdStream.peek();
        while (((int)c != -1) && (((int)c < (int)('0')) || ((int)c > (int)('9'))))
        {
                deviceIdStream.ignore(1);
                c = deviceIdStream.peek();
        }
	}
	sort(idsArr.begin(), idsArr.end());
	for (unsigned int i = 0; i < idsArr.size(); i++)
	{
		assert(idsArr[i] >= 0);
		// wrap around amount of COI engines.
		unsigned int candidateId = idsArr[i] % coiNumEngines;
		if ((0 == deviceIdList.size()) || (deviceIdList[deviceIdList.size() - 1] != candidateId))
		{
			deviceIdList.push_back(candidateId);
		}
	}
}

//TODO - AdirD change the MICSysInfo::getInstance().get...(0) from 0 to real device ID.

// Device entry points
//Device Information function prototypes
//
/************************************************************************************************************************
   clDevGetDeviceInfo
    Description
        This function return device specific information defined by cl_device_info enumeration as specified in OCL spec. table 4.3.
    Input
        param                    An enumeration that identifies the device information being queried. It can be one of
                                the following values as specified in OCL spec. table 4.3
        valSize                Specifies the size in bytes of memory pointed to by paramValue. This size in
                                bytes must be >= size of return type
    Output
        paramVal                A pointer to memory location where appropriate values for a given param as specified in OCL spec. table 4.3 will be returned. If paramVal is NULL, it is ignored
        paramValSize_ret        Returns the actual size in bytes of data being queried by paramVal. If paramValSize_ret is NULL, it is ignored
    Returns
        CL_DEV_SUCCESS            If functions is executed successfully.
        CL_DEV_INVALID_VALUE    If param_name is not one of the supported values or if size in bytes specified by paramValSize is < size of return type as specified in OCL spec. table 4.3 and paramVal is not a NULL value
**************************************************************************************************************************/
extern "C"
cl_dev_err_code clDevGetDeviceInfo(unsigned int IN	dev_id, cl_device_info IN param, size_t IN valSize, void* OUT paramVal, size_t* OUT paramValSizeRet)
{
    return MICSysInfo::clDevGetDeviceInfo(dev_id, param, valSize, paramVal, paramValSizeRet);
}

cl_dev_err_code MICSysInfo::clDevGetDeviceInfo(
                uint32_t deviceId,
                cl_device_info IN param, size_t IN valSize, void* OUT paramVal,
                size_t* OUT paramValSizeRet)
{
    size_t  internalRetunedValueSize = valSize;
    size_t  *pinternalRetunedValueSize;

    //if OUT paramValSize_ret is NULL it should be ignored
    if(paramValSizeRet)
    {
        pinternalRetunedValueSize = paramValSizeRet;
    }
    else
    {
        pinternalRetunedValueSize = &internalRetunedValueSize;
    }

    MICSysInfo& object = getInstance();

    if (! object.initializedInfoStruct(deviceId))
    {
        return CL_DEV_ERROR_FAIL;
    }

    const InfoType2DataEntry* data = object.m_guardedInfoArr[deviceId].devInfoStruct->data_table;
    assert( data && "MICDevice: SKU static data table is not initialized" );

    TInfoType2Data::const_iterator it = data->data_map.find( param );
    if (data->data_map.end() == it)
    {
        // info not found
        return CL_DEV_INVALID_VALUE;
    }

    const SYS_INFO_ENTRY* info_entry = it->second;
    assert( info_entry && (info_entry->info_id == param) && "MICDevice: error in static info map" );
    assert( 0 != info_entry->array_count && "MICDevice: error in static info map - zero values count" );

    switch (info_entry->si_value_type)
    {
        case VALUE_SCALAR:
        {
            assert( 0 != info_entry->type_size   && "MICDevice: error in static info map - zero-sized value" );
            assert( info_entry->type_size <= sizeof(uint64_t) &&
                                                    "MICDevice: error in static info map - wrong scalar size" );

            assert(((1 == info_entry->array_count) || (0 != info_entry->const_value)) &&
                                                    "MICDevice: error in static info map - zero array pointer" );

            // here may be 2 cases:
            // 1. real small scalar
            // 2. array of scalars

            size_t required_size = info_entry->array_count * info_entry->type_size;

            if(! process_info_params( required_size, valSize, paramVal, pinternalRetunedValueSize ))
            {
                return CL_DEV_INVALID_VALUE;
            }

            //if OUT paramVal is NULL it should be ignored
            if(NULL == paramVal)
            {
                return CL_DEV_SUCCESS;
            }

            const void* source_ptr = ((1 == info_entry->array_count) || (0 == info_entry->const_value)) ?
                                (const void*)&info_entry->const_value : (const void*)info_entry->const_value;

            MEMCPY_S( paramVal, valSize, source_ptr, required_size );
            return CL_DEV_SUCCESS;
        }

        case VALUE_STRING:
        {
            assert( 0 == info_entry->type_size   && "MICDevice: error in static info map - string size must be 0" );
            assert( 1 == info_entry->array_count && "MICDevice: error in static info map - string count must be 1" );
            assert( 0 != info_entry->const_value && "MICDevice: error in static info map - zero string pointer" );

            const char* str      = (const char*)info_entry->const_value;
            size_t required_size = (NULL == str) ? 0 : (strlen( str ) + 1);

            if(! process_info_params( required_size, valSize, paramVal, pinternalRetunedValueSize ))
            {
                return CL_DEV_INVALID_VALUE;
            }

            //if OUT paramVal is NULL it should be ignored
            //if nothing should be copied and no place to set null-char, return also
            if ((NULL == paramVal) || ((NULL == str) && (valSize == 0)))
            {
                return CL_DEV_SUCCESS;
            }

            if ((NULL == str) && (valSize > 0))
            {
                ((char*)paramVal)[0] = '\0';
            }
            else
            {
                MEMCPY_S( paramVal, valSize, str, required_size );
            }
            return CL_DEV_SUCCESS;
        }

        case VALUE_FUNCTION:
        {
            assert( 0 == info_entry->type_size   && "MICDevice: error in static info map - func size must be 0" );
            assert( 1 == info_entry->array_count && "MICDevice: error in static info map - func count must be 1" );
            assert( NULL != info_entry->func_value && "MICDevice: error in static info map - zero func pointer" );

            TInfoFunc   func = info_entry->func_value;

            if (NULL == func)
            {
                return CL_DEV_ERROR_FAIL;
            }

            return (object.*func)( deviceId, param, valSize, paramVal, pinternalRetunedValueSize );
        }

        default:
            assert( false && "MICDevice: error in static info map - unknown record type" );
            return CL_DEV_INVALID_VALUE;
    }

    return CL_DEV_SUCCESS;

}

//! This function return IDs list for all devices in the same device type.
/*!
    \param[in]  deviceListSize          Specifies the size of memory pointed to by deviceIdsList.(in term of amount of IDs it can store)
	                                    If deviceIdsList != NULL that deviceListSize must be greater than 0.
    \param[out] deviceIdsList           A pointer to memory location where appropriate values for each device ID will be store. If paramVal is NULL, it is ignored
    \param[out] deviceIdsListSizeRet    If deviceIdsList!= NULL it store the actual amount of IDs being store in deviceIdsList. 
	                                    If deviceIdsList == NULL and deviceIdsListSizeRet than it store the amount of available devices.
										If deviceIdsListSizeRet is NULL, it is ignored.
    \retval     CL_DEV_SUCCESS          If function is executed successfully.
    \retval     CL_DEV_ERROR_FAIL	    If function failed to figure the IDs of the devices.
*/
extern "C" cl_dev_err_code clDevGetAvailableDeviceList(size_t IN deviceListSize,
                        unsigned int*   OUT deviceIdsList,
                        size_t*   OUT deviceIdsListSizeRet)
{
	return MICSysInfo::clDevGetAvailableDeviceList(deviceListSize, deviceIdsList, deviceIdsListSizeRet);
}

cl_dev_err_code MICSysInfo::clDevGetAvailableDeviceList(size_t IN deviceListSize, unsigned int* OUT deviceIdsList, size_t* OUT deviceIdsListSizeRet)
{
	if (((NULL != deviceIdsList) && (0 == deviceListSize)) || ((NULL == deviceIdsList) && (NULL == deviceIdsListSizeRet)))
	{
		return CL_DEV_ERROR_FAIL;
	}
	MICSysInfo& tMicSysInfo = getInstance();
	if (deviceIdsList)
	{
		unsigned int i;
		for (i = 0; (i < deviceListSize) && (i < tMicSysInfo.getEngineCount()); ++i)
		{
			deviceIdsList[i] = i;
		}
		*deviceIdsListSizeRet = i;
	}
	else if (deviceIdsListSizeRet)
	{
		*deviceIdsListSizeRet = tMicSysInfo.getEngineCount();
	}
	return CL_DEV_SUCCESS;
}
