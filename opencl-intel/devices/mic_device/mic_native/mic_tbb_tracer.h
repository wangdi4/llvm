// Copyright (c) 2006-2013 Intel Corporation
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

/////////////////////////////////////////////////////////////
//  mic_tbb_tracer.h
//  Implementation of the Class ExecutionTask
//  Class Object is responsible on execution of NDRange task
/////////////////////////////////////////////////////////////

#pragma once
#include "native_globals.h"
#include <pthread.h>

namespace Intel { namespace OpenCL { namespace MICDeviceNative {

class NDRangeTask;

#ifdef ENABLE_MIC_TBB_TRACER

class PerfData 
{
public:
    void work_group_start();
    void work_group_end();
    
    void append_data_item( unsigned int n_coords, unsigned int col, unsigned int row = 0, unsigned int page = 0 );
    
    static void global_init();
private:
    unsigned long long start_time;
    unsigned long long end_time;
    unsigned long long search_time;
    unsigned int*      processed_indices;
    unsigned int       processed_indices_limit;
    unsigned int       processed_indices_current;
    
    unsigned int       m_worker_id;
    
    static const unsigned int INDICES_DELTA  = 64;

    static pthread_key_t g_phys_processor_id_tls_key;

    void resize( unsigned int n_coords ); 
    void getHwInfoForPhysProcessor( unsigned int physical_processor_id, unsigned int& core_id, unsigned int& thread_id_on_core );
    bool is_thread_recorded();
    void dump_thread_attach();

    void construct(unsigned int worker_id);
    void destruct();

    void dump_data_item( char* buffer, unsigned int n_coords, unsigned int index );
    friend class NDRangePerfData;
};

class NDRangePerfData
{
public:
    NDRangePerfData( const NDRangeTask& task );

    void work_group_start() { get_thread_data().work_group_start(); };
    void work_group_end()   { get_thread_data().work_group_end(); };
    void append_data_item( unsigned int n_coords, unsigned int col, unsigned int row = 0, unsigned int page = 0 )
    {
        get_thread_data().append_data_item( n_coords, col, row, page );
    }

    void PerfDataFini();

private:
    void PerfDataFiniInternal( unsigned int command_id, unsigned int dims, size_t dim_0, size_t dim_1, size_t dim_2 );
    PerfData& get_thread_data();

    PerfData m_perf_data[MIC_NATIVE_MAX_WORKER_THREADS];
    const NDRangeTask& m_task;
};


#else // no ENABLE_MIC_TBB_TRACER

class NDRangePerfData
{
public:
    NDRangePerfData( const NDRangeTask& task ) {}
    void work_group_start() {};
    void work_group_end() {};
    void append_data_item( unsigned int n_coords, unsigned int col, unsigned int row = 0, unsigned int page = 0 ) {};
    void PerfDataFini() {};
};

#endif // ENABLE_MIC_TBB_TRACER

}}}

