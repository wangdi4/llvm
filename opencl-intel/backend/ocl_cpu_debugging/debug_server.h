#ifndef DEBUG_SERVER_H
#define DEBUG_SERVER_H

#ifndef OclCpuDebugging_EXPORTS
#define OclCpuDebugging_EXPORTS
#endif // OclCpuDebugging_EXPORTS

#include "export/icldebuggingservice.h"
#include <memory>


#pragma warning (disable : 4985 ) /* disable ceil warnings */ 

namespace llvm {
    class MDNode;
    class NamedMDNode;
}


// Initialize the debug server if debugging is enabled with an env var.
// Note: this blocks further execution until a client connects to the server
// and initiates a debugging session.
// Return false iff debugging is enabled and initialization failed.
//
bool InitDebugServer();


// Interface of the debug server.
//
class DebugServer : public ICLDebuggingService
{
public:
    // The debug server is a singleton. Access it via this method
    //
    static DebugServer& GetInstance() {return instance;}

    DebugServer();
    ~DebugServer();

    // Initialize the server. Note that this blocks until a client is 
    // connected.
    //
    bool Init();

    // Explicitly terminate the server connection
    //
    void TerminateConnection();

    // These methods are called by the corresponding debug builtins
    //
    void Stoppoint(const llvm::MDNode* line_metadata);
    void EnterFunction(const llvm::MDNode* subprogram_mdn);
    void ExitFunction(const llvm::MDNode* subprogram_mdn);
    void DeclareLocal(void* addr, const llvm::MDNode* description);
    void DeclareGlobal(void* addr, const llvm::MDNode* description);

    // Check if the global id passed in as a triple of numbers matches the
    // debugged global id
    //
    bool DebuggedGlobalIdMatch(unsigned x, unsigned y, unsigned z);

    // Wait until the client asks to start debugging, initializing the 
    // debugging session according to the client's request.
    //
    void WaitForStartCommand();

private:
    static DebugServer instance;
    struct DebugServerImpl;
    std::auto_ptr<DebugServerImpl> d;

    // No copying
    DebugServer(const DebugServer&);
    DebugServer& operator=(const DebugServer&);
};


#endif // DEBUG_SERVER_H
