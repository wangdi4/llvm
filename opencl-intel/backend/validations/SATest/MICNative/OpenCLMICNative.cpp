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

File Name:  OpenCLMICNative.cpp

\*****************************************************************************/

#include <assert.h>
#include <sink/COIPipeline_sink.h>
#include <sink/COIProcess_sink.h>
#include <common/COIMacros_common.h>
#include <stdio.h>
#include <string.h>
#include "cl_dev_backend_api.h"
#include "mic_dev_limits.h"
#include "WGContext.h"
#include "common.h"
#include "Performance.h"

//#define DEVICE_DEBUG
#ifdef DEVICE_DEBUG
#define DEBUG_PRINT(...)   { printf( "MIC: " __VA_ARGS__); fflush(0); }
#else //DEVICE_DEBUG
#define DEBUG_PRINT(...)   {}
#endif // DEVICE_DEBUG

#define CHECK_RESULT(_FUNC)                                     \
{                                                               \
    cl_dev_err_code result = _FUNC;                             \
    if (CL_DEV_SUCCESS != result)                               \
    {                                                           \
        /* TODO: print error code using logger. */              \
        printf("%s failed\n", #_FUNC); fflush(0);               \
    }                                                           \
}

ICLDevBackendSerializationService *serializer;
ICLDevBackendExecutionService *executor;

// execution memory allocator required by Device Backend
class MICNativeBackendExecMemoryAllocator : public ICLDevBackendJITAllocator
{
public:
    void* AllocateExecutable(size_t size, size_t alignment){return NULL;}
    void FreeExecutable(void* ptr){}
};

// kernel printf filler required by Device Backend
class MICNativeBackendPrintfFiller //: public ICLDevBackendPrintf
{
public:
    void  print( const char* buf ) {}
};

// class required by Device Backend to specify options
class MICNativeBackendOptions : public ICLDevBackendOptions
{
public:

    // ICLDevBackendOptions interface
    bool GetBooleanValue( int optionId, bool defaultValue) const
    { return defaultValue;}
    int GetIntValue( int optionId, int defaultValue) const
    { return defaultValue;}
    const char* GetStringValue( int optionId, const char* defaultValue)const
    { return defaultValue;}
    bool GetValue( int optionId, void* Value, size_t* pSize) const
{
    if (NULL == Value || NULL == pSize || sizeof(void*) != *pSize)
    {
        return false;
    }

    switch (optionId)
    {
        case CL_DEV_BACKEND_OPTION_JIT_ALLOCATOR:
            *(void**)Value = (void*)(&m_allocator);
            return false;

        default:
            return false;
    }
}

private:
    MICNativeBackendExecMemoryAllocator m_allocator;
    MICNativeBackendPrintfFiller        m_printf;
};

// main is automatically called whenever the source creates a process.
// However, once main exits, the process that was created exits.
int main(int argc, char** argv)
{
    UNREFERENCED_PARAM (argc);
    UNREFERENCED_PARAM (argv);

    MICNativeBackendOptions options;

    cl_dev_err_code err = CL_DEV_SUCCESS;
    err = InitDeviceBackend( NULL );
    if (CL_DEV_FAILED( err ))
    {
        printf("InitDeviceBackend failed\n"); fflush(0);
        return -1;
    }

    // load Backend Compiler
    ICLDevBackendServiceFactory* be_factory = GetDeviceBackendFactory();

    if (NULL == be_factory)
    {
        printf("GetDeviceBackendFactory failed\n"); fflush(0);
        return -1;
    }

    err = be_factory->GetExecutionService( &options, &executor );
    if (CL_DEV_FAILED( err ))
    {
        printf("GetExecutionService failed\n"); fflush(0);
        return -1;
    }

    err = be_factory->GetSerializationService( &options, &serializer );

    if (CL_DEV_FAILED( err ))
    {
        printf("GetSerializationService failed\n"); fflush(0);
        executor->Release();
        executor = NULL;
        return -1;
    }

    // Functions enqueued on the sink side will not start executing until
    // you call COIPipelineStartExecutingRunFunctions(). This call is to 
    // synchronize any initialization required on the sink side.

    COIRESULT result = COIPipelineStartExecutingRunFunctions();
    if ( COI_SUCCESS != result )
    {
        printf("COIPipelineStartExecutingRunFunctions failed\n"); fflush(0);
        executor->Release();
        executor = NULL;
        serializer->Release();
        serializer = NULL;
        return -1;
    }

    // This call will wait until COIProcessDestroy() gets called on the source 
    // side If COIProcessDestroy is called without force flag set, this call 
    // will make sure all the functions enqueued are executed and does all 
    // clean up required to
    // exit gracefully.
    COIProcessWaitForShutdown();

    executor->Release();
    executor = NULL;
    serializer->Release();
    serializer = NULL;

    return 0;
}

namespace Validation {
extern void GenINT3();
}

void ExecuteWorkGroup( size_t x, size_t y, size_t z, WGContext& context, Validation::Sample& timer, bool useTraceMarks)
{
    size_t groupId[MAX_WORK_DIM] = {x, y, z};

    // In production sequence the Runtime calls Executable::PrepareThread()
    // and Executable::RestoreThreadState() respectively before and 
    // after executing a work group. 
    // These routines setup and restore the MXCSR register and zero the upper parts of YMMs. 
    CHECK_RESULT(context.GetExecutable()->PrepareThread());

    if( useTraceMarks )
    {
        Validation::GenINT3();
    }

    timer.Start();
    context.GetExecutable()->Execute(groupId, NULL, NULL);
    timer.Stop();

    if( useTraceMarks )
    {
        Validation::GenINT3();
    }

    CHECK_RESULT(context.GetExecutable()->RestoreThreadState());
}


COINATIVELIBEXPORT
void executeKernels(uint32_t         in_BufferCount,
          void**           in_ppBufferPointers,
          uint64_t*        in_pBufferLengths,
          void*            in_pMiscData,
          uint16_t         in_MiscDataLength,
          void*            in_pReturnValue,
          uint16_t         in_ReturnValueLength)
{

    UNREFERENCED_PARAM(in_ReturnValueLength);

    DEBUG_PRINT("Program execution has started on the MIC side!\n");

    Validation::Sample deserializationTimer; 
    deserializationTimer.Start();
    // 0 buffer contains test program.
    ICLDevBackendProgram_ *pProgram;
    DEBUG_PRINT("Program deserialization has started ...\n");
    CHECK_RESULT(serializer->DeSerializeProgram(&pProgram, in_ppBufferPointers[0], size_t(in_pBufferLengths[0])));
    DEBUG_PRINT("done.\n");
    deserializationTimer.Stop();
    // Save deserialization time to the return value.
    *(Validation::Sample*)(in_pReturnValue) = deserializationTimer;

    uint64_t numOfKernels = *(uint64_t*)in_pMiscData;
    // Last buffer contains dispatcher data.    
    DispatcherData *dispatchers = (DispatcherData *)(in_ppBufferPointers[in_BufferCount-1]);
    ExecutionOptions *exeOptions = (ExecutionOptions*)(dispatchers + numOfKernels);
    uint64_t kernelsArgIndex = 0;
    DEBUG_PRINT("Number of kernels to execute = %d\n", (int)numOfKernels);
    WGContext context;
    for (uint64_t i = 0; i < numOfKernels; ++i)
    {
        kernelsArgIndex += dispatchers[i].preExeDirectivesCount + 1;
        DEBUG_PRINT("Running \"%s\" kernel\n", (const char *)(in_ppBufferPointers[kernelsArgIndex]));
        DEBUG_PRINT("Kernel arguments index = %d\n", (int)kernelsArgIndex);
        // First we have the name of kernel followed by '\0' character.
        const ICLDevBackendKernel_* pKernel;
        CHECK_RESULT(pProgram->GetKernelByName((const char *)(in_ppBufferPointers[kernelsArgIndex]), &pKernel));

        DEBUG_PRINT("Creating binary ... ");
        ICLDevBackendBinary_* pBinary;
        cl_work_description_type workDesc;
        dispatchers[i].workDesc.convertToClWorkDescriptionType(&workDesc);
        DEBUG_PRINT("\nWork space params:\nDimension:\t%d\nGlobal work size:\t[%d, %d, %d]\nGlobal work offset:\t[%d, %d, %d]\nLocal work size:\t[%d, %d, %d]\n", workDesc.workDimension, (int)workDesc.globalWorkSize[0], (int)workDesc.globalWorkSize[1], (int)workDesc.globalWorkSize[2], (int)workDesc.globalWorkOffset[0], (int)workDesc.globalWorkOffset[1], (int)workDesc.globalWorkOffset[2], (int)workDesc.localWorkSize[0], (int)workDesc.localWorkSize[1], (int)workDesc.localWorkSize[2]);
        DEBUG_PRINT("Kernel arguments size = %d\n", (int)dispatchers[i].kernelArgSize);
        char *kernelsArgs = new char[dispatchers[i].kernelArgSize];
        memcpy(kernelsArgs, (void*)((char*)(in_ppBufferPointers[kernelsArgIndex]) + dispatchers[i].kernelDirective.kernelNameSize), dispatchers[i].kernelArgSize);
        DEBUG_PRINT("Float value = %f\n", *(float*)(kernelsArgs + 24));
        DirectivePack* directives = (DirectivePack*)((char*)(in_ppBufferPointers[kernelsArgIndex]) + dispatchers[i].kernelDirective.kernelNameSize + dispatchers[i].kernelArgSize);
        DEBUG_PRINT("Number of directives: %d\n", (int)dispatchers[i].preExeDirectivesCount);
        DEBUG_PRINT("Possible directive values: %d, %d, %d\n", (int)BUFFER, (int)PRINTF, (int)KERNEL);
        // copy pointer to buffers
        for (uint32_t j = 0; j < dispatchers[i].preExeDirectivesCount; ++j)
        {
            DEBUG_PRINT("Directive[%d] id: %d\n", j, (int)directives[j].id);
            if (directives[j].id == BUFFER)
            {
                *((void**)(kernelsArgs + directives[j].bufferDirective.offset_in_blob)) = &(directives[j].bufferDirective.mem_obj_desc);
                DEBUG_PRINT("Pointer value[%d] = %p\n", j, &(directives[j].bufferDirective.mem_obj_desc));
                directives[j].bufferDirective.mem_obj_desc.pData = in_ppBufferPointers[directives[j].bufferDirective.bufferIndex];
                DEBUG_PRINT("Pointer value = %p\n", directives[j].bufferDirective.mem_obj_desc.pData);
                DEBUG_PRINT("First three values = %f, %f, %f\n", *(float*)directives[j].bufferDirective.mem_obj_desc.pData, *((float*)(directives[j].bufferDirective.mem_obj_desc.pData)+1), *((float*)(directives[j].bufferDirective.mem_obj_desc.pData)+2));
            }
            if (directives[j].id == PRINTF)
            {
                // TODO: implement
            }
        }

        CHECK_RESULT(executor->CreateBinary(pKernel, (void*)(kernelsArgs), dispatchers[i].kernelArgSize, &workDesc, &pBinary));
        DEBUG_PRINT("done.\n");

        DEBUG_PRINT("Preparing executable ... ");
        size_t memBuffCount;
        CHECK_RESULT(pBinary->GetMemoryBuffersDescriptions(NULL, &memBuffCount));
        size_t *pMemBuffSizes = new size_t[memBuffCount];
        CHECK_RESULT(pBinary->GetMemoryBuffersDescriptions(pMemBuffSizes, &memBuffCount));
        context.CreateContext(pBinary, pMemBuffSizes, memBuffCount);
        DEBUG_PRINT("done.\n");

        DEBUG_PRINT("Executing the kernel ... ");
        for (uint32_t j = 0; j < exeOptions->executeIterationsCount; ++j)
        {
            // Do we need boost priority on MIC???
            Validation::PriorityBooster booster(!exeOptions->measurePerformance);
            Validation::Sample timer;
            if (exeOptions->runSingleWG)
            {
                ExecuteWorkGroup(0, 0, 0, context, timer, exeOptions->useTraceMarks);
            }
            else
            {
                size_t dims[3];
                // init the work group regions
                for (size_t j=0; j < workDesc.workDimension; ++j)
                {
                    dims[j] = (size_t)( workDesc.globalWorkSize[j]/workDesc.localWorkSize[j]);
                }
                DEBUG_PRINT("Number of work dimensions: %d\n", uint32_t(workDesc.workDimension));
                DEBUG_PRINT("work dimensions: [%d, %d, %d]\n", uint32_t(dims[0]), int32_t(dims[1]), uint32_t(dims[2]));
                switch( workDesc.workDimension)
                {
                case 1:
                    for( size_t x = 0; x < dims[0]; ++x)
                    {
                        ExecuteWorkGroup(x, 0, 0, context, timer, exeOptions->useTraceMarks);
                    }
                    break;
                case 2:
                    for(size_t y = 0; y < dims[1]; ++y)
                    {
                        for( size_t x = 0; x < dims[0]; ++x)
                        {
                            ExecuteWorkGroup(x, y, 0, context, timer, exeOptions->useTraceMarks);
                        }
                    }
                    break;
                case 3:
                    for(size_t z = 0; z < dims[2]; ++z)
                    {
                        for(size_t y = 0; y < dims[1]; ++y)
                        {
                            for( size_t x = 0; x < dims[0]; ++x)
                            {
                                ExecuteWorkGroup(x, y, z, context, timer, exeOptions->useTraceMarks);
                            }
                        }
                    }
                    break;
                default:
                    DEBUG_PRINT("Wrong number of dimensions while running the kernel ... ");
                    //throw Validation::Exception::TestRunnerException("Wrong number of dimensions while running the kernel");
                }
            }
            DEBUG_PRINT("Timer data: total time: %lu, total ticks: %lu, samples count: %lu", timer.TotalTime(), timer.TotalTicks(), timer.SamplesCount());
            ((Validation::Sample*)in_pReturnValue)[i*exeOptions->executeIterationsCount+j+1] = timer;
        }
        DEBUG_PRINT("done.\n");

        delete [] kernelsArgs;
        delete [] pMemBuffSizes;
        ++kernelsArgIndex;
    }
}

COINATIVELIBEXPORT
void getBackendTargetDescriptionSize(
    uint32_t         in_BufferCount,
    void**           in_ppBufferPointers,
    uint64_t*        in_pBufferLengths,
    void*            in_pMiscData,
    uint16_t         in_MiscDataLength,
    void*            in_pReturnValue,
    uint16_t         in_ReturnValueLength)
{
    assert( 0 == in_BufferCount && "SINK: get_backend_target_description_size() should receive 0 buffers" );
    assert( sizeof(uint64_t) == in_ReturnValueLength && "SINK: get_backend_target_description_size() should receive return-value as uint64_t" );

    size_t filled_size = executor->GetTargetMachineDescriptionSize();

    *((uint64_t*)in_pReturnValue) = (uint64_t)filled_size;
}

COINATIVELIBEXPORT
void getBackendTargetDescription(
    uint32_t         in_BufferCount,
    void**           in_ppBufferPointers,
    uint64_t*        in_pBufferLengths,
    void*            in_pMiscData,
    uint16_t         in_MiscDataLength,
    void*            in_pReturnValue,
    uint16_t         in_ReturnValueLength)
{
    assert( 1 == in_BufferCount && "SINK: get_backend_target_description() should receive 1 buffer" );
    assert( sizeof(uint64_t) == in_ReturnValueLength && "SINK: get_backend_target_description() should receive return-value as uint64_t" );

    size_t filled_size = 0;

    filled_size = executor->GetTargetMachineDescription( in_ppBufferPointers[0], (size_t)in_pBufferLengths[0] );

    *((uint64_t*)in_pReturnValue) = (uint64_t)filled_size;
}

