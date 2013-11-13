// Copyright (c) 2006-20013 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

/*
*
* File cpu_device.h
* declares C++ interface between the device and the Open CL frame work.
*
*/
#pragma once

#include "cl_config.h"
using namespace Intel::OpenCL::Utils;

/**************************************************************************************************
* Configuration keys
**************************************************************************************************/

// General configuration
#define CL_CONFIG_USE_ITT_API     "CL_CONFIG_USE_ITT_API"     // bool
#define CL_CONFIG_USE_VECTORIZER  "CL_CONFIG_USE_VECTORIZER"  // bool
#define CL_CONFIG_USE_VTUNE       "CL_CONFIG_USE_VTUNE"       // bool


// device setup
#define    CL_CONFIG_MIC_DEVICE_STOP_AT_LOAD				    "CL_CONFIG_MIC_DEVICE_STOP_AT_LOAD"           // bool
#define    CL_CONFIG_MIC_DEVICE_USE_AFFINITY				    "CL_CONFIG_MIC_DEVICE_USE_AFFINITY"           // bool
#define    CL_CONFIG_MIC_DEVICE_THREADS_PER_CORE        "CL_CONFIG_MIC_DEVICE_THREADS_PER_CORE"       // unsigned int
#define    CL_CONFIG_MIC_DEVICE_NUM_CORES					      "CL_CONFIG_MIC_DEVICE_NUM_CORES"	            // unsigned int
#define    CL_CONFIG_MIC_DEVICE_IGNORE_CORE_0				    "CL_CONFIG_MIC_DEVICE_IGNORE_CORE_0"          // bool
#define    CL_CONFIG_MIC_DEVICE_IGNORE_LAST_CORE		    "CL_CONFIG_MIC_DEVICE_IGNORE_LAST_CORE"       // bool
#define    CL_CONFIG_MIC_DEVICE_2MB_BUF_MINSIZE_KB	    "CL_CONFIG_MIC_DEVICE_2MB_BUF_MINSIZE_KB"     // size_t in KB
#define    CL_CONFIG_MIC_DEVICE_TBB_GRAIN_SIZE				  "CL_CONFIG_MIC_DEVICE_TBB_GRAIN_SIZE"         // unsigned int
#define    CL_CONFIG_MIC_DEVICE_TBB_SCHEDULER				    "CL_CONFIG_MIC_DEVICE_TBB_SCHEDULER"          // string
#define    CL_CONFIG_MIC_DEVICE_TBB_BLOCK_OPTIMIZATION  "CL_CONFIG_MIC_DEVICE_TBB_BLOCK_OPTIMIZATION" // string
#define    CL_CONFIG_MIC_DEVICE_TBB_TRAP_WORKERS	      "CL_CONFIG_MIC_DEVICE_TBB_TRAP_WORKERS"       // bool
#define    CL_CONFIG_MIC_DEVICE_LAZY_TRANSFER				    "CL_CONFIG_MIC_DEVICE_LAZY_TRANSFER"          // unsigned int
#define    CL_CONFIG_MIC_DEVICE_SAFE_KERNEL_EXECUTION		"CL_CONFIG_MIC_DEVICE_SAFE_KERNEL_EXECUTION"  // bool
#ifdef __MIC_DA_OMP__
	#define    CL_CONFIG_MIC_DEVICE_OMP_SCHEDULE  "CL_CONFIG_MIC_DEVICE_OMP_SCHED" // string
	#define    CL_CONFIG_MIC_DEVICE_OMP_KMP_AFFINITY  "CL_CONFIG_MIC_DEVICE_OMP_KMP_AFFINITY" // string
	#define    CL_CONFIG_MIC_DEVICE_OMP_KMP_BLOCKTIME  "CL_CONFIG_MIC_DEVICE_OMP_KMP_BLOCKTIME" // string
#endif
#define    CL_CONFIG_MIC_DEVICE_PARALLEL_FILL_BUFFER_FROM_SIZE_BYTE    "CL_CONFIG_MIC_DEVICE_PARALLEL_FILL_BUFFER_FROM_SIZE_BYTE"  // int
#define    CL_CONFIG_MIC_DEVICE_PARALLEL_FILL_MAX_TASK_PER_WORKER    "CL_CONFIG_MIC_DEVICE_PARALLEL_FILL_MAX_TASK_PER_WORKER"  // int
#define    CL_CONFIG_MIC_DEVICE_PARALLEL_FILL_MAX_WORKERS    "CL_CONFIG_MIC_DEVICE_PARALLEL_FILL_MAX_WORKERS"  // int

#define    CL_CONFIG_MIC_DEVICE_PRINT_CONFIG             "CL_CONFIG_MIC_DEVICE_PRINT_CONFIG"          // bool

#define    OFFLOAD_DEVICES									             "OFFLOAD_DEVICES"							              // string

namespace Intel { namespace OpenCL { namespace MICDevice {
class MICDeviceConfig
{
public:

    MICDeviceConfig();
    ~MICDeviceConfig();

    cl_err_code    Initialize(string file_name);
    void           Release();

		bool           Device_PrintConfig() const { return m_pConfigFile->Read<bool>(CL_CONFIG_MIC_DEVICE_PRINT_CONFIG, false); }


    bool           UseITT() const { return m_pConfigFile->Read<bool>(CL_CONFIG_USE_ITT_API, false); }
    bool           UseVectorizer() const { return m_pConfigFile->Read<bool>(CL_CONFIG_USE_VECTORIZER, true ); }
    bool           UseVTune()      const { return m_pConfigFile->Read<bool>(CL_CONFIG_USE_VTUNE,      false); }

		// Device performance setup
		bool           Device_StopAtLoad()      const { return m_pConfigFile->Read<bool>(CL_CONFIG_MIC_DEVICE_STOP_AT_LOAD, false); }
		bool           Device_UseAffinity()     const { return m_pConfigFile->Read<bool>(CL_CONFIG_MIC_DEVICE_USE_AFFINITY, true); }
		unsigned int   Device_ThreadsPerCore()  const { return m_pConfigFile->Read<unsigned int>(CL_CONFIG_MIC_DEVICE_THREADS_PER_CORE, 0); }
		unsigned int   Device_NumCores()        const { return m_pConfigFile->Read<unsigned int>(CL_CONFIG_MIC_DEVICE_NUM_CORES, 0); }
		bool           Device_IgnoreCore0()     const { return m_pConfigFile->Read<bool>(CL_CONFIG_MIC_DEVICE_IGNORE_CORE_0, false); }
		bool           Device_IgnoreLastCore()  const { return m_pConfigFile->Read<bool>(CL_CONFIG_MIC_DEVICE_IGNORE_LAST_CORE, true); }
		size_t         Device_2MB_BufferMinSizeInKB() const { return m_pConfigFile->Read<size_t>(CL_CONFIG_MIC_DEVICE_2MB_BUF_MINSIZE_KB, 512); }
		unsigned int   Device_TbbGrainSize()    const { return m_pConfigFile->Read<unsigned int>(CL_CONFIG_MIC_DEVICE_TBB_GRAIN_SIZE, 1); }
		string         Device_TbbScheduler()    const { return m_pConfigFile->Read<string>(CL_CONFIG_MIC_DEVICE_TBB_SCHEDULER, "auto"); }
		string         Device_TbbBlockOptimization() const { return m_pConfigFile->Read<string>(CL_CONFIG_MIC_DEVICE_TBB_BLOCK_OPTIMIZATION, "rows"); }
		bool           Device_TbbTrapWorkers()  const { return m_pConfigFile->Read<bool>(CL_CONFIG_MIC_DEVICE_TBB_TRAP_WORKERS, false); }
		bool           Device_LazyTransfer()    const { return m_pConfigFile->Read<bool>(CL_CONFIG_MIC_DEVICE_LAZY_TRANSFER, false); }
		// Device safe mode setup
		bool           Device_safeKernelExecution() const { return m_pConfigFile->Read<bool>(CL_CONFIG_MIC_DEVICE_SAFE_KERNEL_EXECUTION, true); }

#ifdef __MIC_DA_OMP__
		string         Device_OmpSchedule() const { return m_pConfigFile->Read<string>(CL_CONFIG_MIC_DEVICE_OMP_SCHEDULE, "static"); }
		string         Device_OmpKmpAffinity() const { return m_pConfigFile->Read<string>(CL_CONFIG_MIC_DEVICE_OMP_KMP_AFFINITY, "granularity=fine,balanced"); }
		string         Device_OmpKmpBlockTime() const { return m_pConfigFile->Read<string>(CL_CONFIG_MIC_DEVICE_OMP_KMP_BLOCKTIME, "infinite"); }
#endif

		int   Device_ParallelFillBufferFromSize() const { return m_pConfigFile->Read<unsigned int>(CL_CONFIG_MIC_DEVICE_PARALLEL_FILL_BUFFER_FROM_SIZE_BYTE, 32768); }
		int   Device_ParallelFillMaxTaskPerWorker() const { return m_pConfigFile->Read<unsigned int>(CL_CONFIG_MIC_DEVICE_PARALLEL_FILL_MAX_TASK_PER_WORKER, -1); }
		int   Device_ParallelFillMaxWorkers() const { return m_pConfigFile->Read<unsigned int>(CL_CONFIG_MIC_DEVICE_PARALLEL_FILL_MAX_WORKERS, -1); }

		string		     Device_offloadDevices() const { return m_pConfigFile->Read<string>(OFFLOAD_DEVICES, ""); }

private:

    #define MICDeviceConfigPrintKey( name, func, help_msg ) std::cout << std::endl << "# " << help_msg << std::endl << name << "=" << func() << std::endl

    void PrintConfiguration()
    {
        std::cout << std::endl;
        std::cout << "-------------MIC Device OpenCL Configuration------------" << std::endl;

        std::cout << std::endl;
        MICDeviceConfigPrintKey( CL_CONFIG_MIC_DEVICE_PRINT_CONFIG, Device_PrintConfig, "1 - print current configuration at MIC device startup" );
        std::cout << std::endl;

        MICDeviceConfigPrintKey( CL_CONFIG_USE_ITT_API, UseITT, "1 - gather info for ITT interface" );
        MICDeviceConfigPrintKey( CL_CONFIG_USE_VECTORIZER, UseVectorizer, "1 - try to vectorize kernel during compilation" );
        MICDeviceConfigPrintKey( CL_CONFIG_USE_VTUNE, UseVTune , "1 - connect to VTune on device");

        std::cout << std::endl;

        MICDeviceConfigPrintKey( CL_CONFIG_MIC_DEVICE_STOP_AT_LOAD, Device_StopAtLoad, "1 - deadloop on device during startup, wait for debugger" );
        MICDeviceConfigPrintKey( CL_CONFIG_MIC_DEVICE_SAFE_KERNEL_EXECUTION, Device_safeKernelExecution, "1 - wrap kernels with try/catch" );

        std::cout << std::endl;

        MICDeviceConfigPrintKey( CL_CONFIG_MIC_DEVICE_USE_AFFINITY, Device_UseAffinity, "1 - do not allow TBB workers to switch between HW threads" );
        MICDeviceConfigPrintKey( CL_CONFIG_MIC_DEVICE_THREADS_PER_CORE, Device_ThreadsPerCore, "0 - create 1 TBB worker per each HW thread per core, !=0 - limit TBB workers to the given number per core" );
        MICDeviceConfigPrintKey( CL_CONFIG_MIC_DEVICE_NUM_CORES, Device_NumCores, "0 - use all MIC cores, !=0 - use specified number of cores" );
        MICDeviceConfigPrintKey( CL_CONFIG_MIC_DEVICE_IGNORE_CORE_0, Device_IgnoreCore0, "1 - do not use 0 (system) MIC core" );
        MICDeviceConfigPrintKey( CL_CONFIG_MIC_DEVICE_IGNORE_LAST_CORE, Device_IgnoreLastCore, "1 - do not use last MIC core" );
        MICDeviceConfigPrintKey( CL_CONFIG_MIC_DEVICE_2MB_BUF_MINSIZE_KB, Device_2MB_BufferMinSizeInKB, "0 - disabled, !=0 - minimum size of buffer in kilobytes to use 2MB pages"  );
        MICDeviceConfigPrintKey( CL_CONFIG_MIC_DEVICE_TBB_GRAIN_SIZE, Device_TbbGrainSize, "must not be 0, recommended number of WGs to schedule for same thread" );
        MICDeviceConfigPrintKey( CL_CONFIG_MIC_DEVICE_TBB_SCHEDULER, Device_TbbScheduler, "affinity - tbb:affinity_partitioner, other - tbb::auto_partitioner" );
        MICDeviceConfigPrintKey( CL_CONFIG_MIC_DEVICE_TBB_BLOCK_OPTIMIZATION, Device_TbbBlockOptimization, "default_TBB_tile - optimize by square tiles using TBB default implementation, columns - optimize columns, rows - optimize rows, tiles - optimize square tiles" );
        MICDeviceConfigPrintKey( CL_CONFIG_MIC_DEVICE_TBB_TRAP_WORKERS, Device_TbbTrapWorkers, "1 - do not allow TBB workers to leave arena. Deadlocks if more than a single queue." );
        MICDeviceConfigPrintKey( CL_CONFIG_MIC_DEVICE_LAZY_TRANSFER, Device_LazyTransfer, "1 - perform host->device transfer only when really required");

        std::cout << std::endl;

        MICDeviceConfigPrintKey( OFFLOAD_DEVICES, Device_offloadDevices, "Restricts the process to use only the MIC cards specified as the value of the variable");

        std::cout << std::endl;

#ifdef __MIC_DA_OMP__
		MICDeviceConfigPrintKey( CL_CONFIG_MIC_DEVICE_OMP_SCHEDULE, Device_OmpSchedule, "Define the OpenMP scheduler to use, the default is \"static\"");
		MICDeviceConfigPrintKey( CL_CONFIG_MIC_DEVICE_OMP_KMP_AFFINITY, Device_OmpKmpAffinity, "Define the OpenMP affinity type, the default is \"granularity=fine,balanced\"");
		MICDeviceConfigPrintKey( CL_CONFIG_MIC_DEVICE_OMP_KMP_BLOCKTIME, Device_OmpKmpBlockTime, "Define the OpenMP blocktime of the threads, the default is \"infinite\"");
		
		std::cout << std::endl;
#endif

		MICDeviceConfigPrintKey( CL_CONFIG_MIC_DEVICE_PARALLEL_FILL_BUFFER_FROM_SIZE_BYTE, Device_ParallelFillBufferFromSize, "Minimum size of buffer in kilobytes to use parallel fill for clEnqueueFillBuffer"  );
		MICDeviceConfigPrintKey( CL_CONFIG_MIC_DEVICE_PARALLEL_FILL_MAX_TASK_PER_WORKER, Device_ParallelFillMaxTaskPerWorker, "Max tasks per worker when calling to clEnqueueFillBuffer"  );
		MICDeviceConfigPrintKey( CL_CONFIG_MIC_DEVICE_PARALLEL_FILL_MAX_WORKERS, Device_ParallelFillMaxWorkers, "Max workers to use when calling to clEnqueueFillBuffer"  );

        std::cout << "--------------------------------------------------------" << std::endl << std::endl;
    }

    ConfigFile * m_pConfigFile;
    static bool config_already_printed;

};

}}}
