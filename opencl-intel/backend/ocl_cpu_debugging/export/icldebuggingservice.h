#ifndef ICLDEBUGGINGSERVICE_H
#define ICLDEBUGGINGSERVICE_H


namespace llvm {
    class MDNode;
    class NamedMDNode;
}


namespace Intel { namespace OpenCL { namespace DeviceBackend {

#ifndef DEBUG_SERVICE_API
    #if defined(_WIN32)
        #ifdef OclCpuDebugging_EXPORTS
            #define DEBUG_SERVICE_API __declspec(dllexport)
        #else
            #define DEBUG_SERVICE_API __declspec(dllimport)
        #endif
    #else
        #define DEBUG_SERVICE_API 
    #endif
#endif


class ICLDebuggingService {
public:
    virtual void Stoppoint(const llvm::MDNode* line_metadata) = 0;
    virtual void EnterFunction(const llvm::MDNode* subprogram_mdn) = 0;
    virtual void ExitFunction(const llvm::MDNode* subprogram_mdn) = 0;
    virtual void DeclareLocal(void* addr, const llvm::MDNode* description) = 0;
    virtual void DeclareGlobal(void* addr, const llvm::MDNode* description) = 0;
    virtual bool DebuggedGlobalIdMatch(unsigned x, unsigned y, unsigned z) = 0;
};


}}}


#ifdef __cplusplus
extern "C"
{
#endif
    using namespace Intel::OpenCL::DeviceBackend;

    // Initialize the debugging service. Return 'true' if initialization 
    // succeeded, false otherwise. If the debugging service is disabled, true
    // is returned.
    //
    DEBUG_SERVICE_API bool InitDebuggingService();

    // Get a pointer to the debugging service instance
    //
    DEBUG_SERVICE_API ICLDebuggingService* DebuggingServiceInstance();

    // Terminates the debugging service
    //
    DEBUG_SERVICE_API void TerminateDebuggingService();

    // Function pointer types
    //
    typedef bool (*DEBUGGING_SERVICE_INIT_FUNC)();
    typedef ICLDebuggingService* (*DEBUGGING_SERVICE_INSTANCE_FUNC)();
    typedef void (*DEBUGGING_SERVICE_TERMINATE_FUNC)();

#ifdef __cplusplus
}
#endif


#endif // ICLDEBUGGINGSERVICE_H
