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

#ifdef ENABLE_MIC_TBB_TRACER

#include "mic_tbb_tracer.h"
#include "native_ndrange_task.h"
#include "native_thread_pool.h"

using namespace Intel::OpenCL::MICDeviceNative;

//
//
//  TBB thacer
//
//

pthread_key_t                               PerfData::g_phys_processor_id_tls_key;

class MicTbbTracerInitializer
{
public:
    MicTbbTracerInitializer() { PerfData::global_init(); }
};

MicTbbTracerInitializer                     mic_tbb_tracer_initializer;


#define MIC_TBB_TRACER_PREFIX               "MIC_TBB_TRACER: "
#define MIC_TBB_TRACER_STREAM               stderr

void PerfData::construct(unsigned int worker_id)
{
    start_time = 0;
    end_time = 0;
    search_time = 0;
    processed_indices = NULL;
    processed_indices_limit = 0;
    processed_indices_current = 0;

    m_worker_id = worker_id;
}

void PerfData::destruct()
{
    if (NULL != processed_indices)
    {
        free (processed_indices);
        construct(m_worker_id);
    }
}

void PerfData::append_data_item( unsigned int n_coords, unsigned int col, unsigned int raw, unsigned int page )
{
    if (processed_indices_current >= processed_indices_limit)
    {
        resize(n_coords);
    }
    switch (n_coords)
    {
        default:
            break;
        case 3:
            processed_indices[processed_indices_current*n_coords+2] = page;
        case 2:
            processed_indices[processed_indices_current*n_coords+1] = raw;
        case 1:
            processed_indices[processed_indices_current*n_coords+0] = col;
            break;
         
    }
    ++processed_indices_current;
}

void PerfData::dump_data_item( char* buffer, unsigned int n_coords, unsigned int index )
{
    switch (n_coords)
    {
        default:
            break;

        case 1:
            sprintf(buffer, " %d", processed_indices[index*n_coords+0]);
            break;
        case 2:
            sprintf(buffer, " %d:%d", processed_indices[index*n_coords+0], processed_indices[index*n_coords+1]);
            break;
        case 3:
            sprintf(buffer, " %d:%d:%d", processed_indices[index*n_coords+0], processed_indices[index*n_coords+1], processed_indices[index*n_coords+2]);
            break;                 
    }
}

void PerfData::resize( unsigned int n_coords ) 
{
    processed_indices_limit += INDICES_DELTA;
    processed_indices = (unsigned int*)realloc(processed_indices, n_coords*sizeof(unsigned int)*processed_indices_limit);
    assert( NULL != processed_indices );
}

void PerfData::work_group_start()
{
    if (!is_thread_recorded())
    {
        dump_thread_attach();
    }
    
    if (0 == start_time)
    {
        start_time = Intel::OpenCL::Utils::RDTSC();
    }
    else
    {
        search_time += (Intel::OpenCL::Utils::RDTSC() - end_time);
    }
}

void PerfData::work_group_end()
{
    end_time = Intel::OpenCL::Utils::RDTSC();
}

void PerfData::global_init()
{
    pthread_key_create( &g_phys_processor_id_tls_key, NULL );
}

void PerfData::getHwInfoForPhysProcessor( unsigned int processor, 
                                          unsigned int& core_id, 
                                          unsigned int& thread_id_on_core ) 
{
    core_id = processor / MIC_NATIVE_MAX_THREADS_PER_CORE;
    thread_id_on_core = processor % MIC_NATIVE_MAX_THREADS_PER_CORE;
}

void PerfData::dump_thread_attach()
{
    unsigned int core_id;
    unsigned int thread_on_core_id;
    unsigned int hwThreadId = hw_cpu_idx();

    getHwInfoForPhysProcessor( hwThreadId, core_id, thread_on_core_id );

    fprintf(MIC_TBB_TRACER_STREAM, MIC_TBB_TRACER_PREFIX "THREAD %03d ATTACH_TO HW_CORE=%03d HW_THREAD_ON_CORE=%d\n", m_worker_id, core_id, thread_on_core_id );
    fflush(MIC_TBB_TRACER_STREAM);

    pthread_setspecific( g_phys_processor_id_tls_key, (void*)1 );
}

bool PerfData::is_thread_recorded()
{
    return (bool)(size_t)pthread_getspecific( g_phys_processor_id_tls_key );
}

NDRangePerfData::NDRangePerfData( const NDRangeTask& task ) : m_task(task)
{
    for (unsigned int i = 0; i < MIC_NATIVE_MAX_WORKER_THREADS; ++i)
    {
        m_perf_data[i].construct(i);
    }
}

void NDRangePerfData::PerfDataFiniInternal( unsigned int command_id, unsigned int dims, size_t dim_0, size_t dim_1, size_t dim_2 )
{
    
    vector<char> buffer;

    size_t buffer_capacity = 10240;
    buffer.resize( buffer_capacity );

    size_t cols  = 1;
    size_t raws  = 1;
    size_t pages = 1;

    if (1 == dims)
    {
        cols = dim_0;
    }
    else if (2 == dims)
    {
        cols = dim_1;
        raws = dim_0;
    }
    else
    {
        cols  = dim_2;
        raws  = dim_1;
        pages = dim_0;
    }

    fprintf(MIC_TBB_TRACER_STREAM, MIC_TBB_TRACER_PREFIX "NDRANGE %05d COORDINATES %d: COLS=%ld RAWS=%ld PAGES=%ld\n", 
                                    command_id, dims, cols, raws, pages);
    fflush(MIC_TBB_TRACER_STREAM);

    for (int i = 0; i < MIC_NATIVE_MAX_WORKER_THREADS; ++i)
    {
        PerfData& data = m_perf_data[i];
        char* start = &(buffer[0]); 
        char* last = start;
        last[0] = '\0';
        
        for (unsigned int idx=0; idx<data.processed_indices_current; ++idx)
        {
            if ((last - start + 32) > buffer_capacity)
            {
                buffer_capacity *= 2;
                buffer.resize( buffer_capacity );

                char* old_start = start;
                start = &(buffer[0]); 
                last = start + (last-old_start);
            }

            data.dump_data_item( last, dims, idx );
            last += strlen(last);
        }
        fprintf(MIC_TBB_TRACER_STREAM, MIC_TBB_TRACER_PREFIX "NDRANGE %05d THREAD %03d: attach=%ld detach=%ld search=%ld indices: %s\n", 
                 command_id, i, data.start_time, data.end_time, data.search_time, start);
        fflush(MIC_TBB_TRACER_STREAM);

        data.destruct();
    }
}

void NDRangePerfData::PerfDataFini()
{
    PerfDataFiniInternal((unsigned int)(size_t)(m_task.m_commandIdentifier), 
                         (unsigned int)(m_task.m_dim), 
                         m_task.m_region[0], m_task.m_region[1], m_task.m_region[2]);
}

PerfData& NDRangePerfData::get_thread_data() 
{
	unsigned int uiWorkerId = ThreadPool::getInstance()->getWorkerID();
    if (ThreadPool::INVALID_WORKER_ID == uiWorkerId)
    {
        uiWorkerId = 0;
    }
    return m_perf_data[uiWorkerId];
}

#endif // ENABLE_MIC_TBB_TRACER


