#ifndef __INSTTOFUNCCALL_H__
#define __INSTTOFUNCCALL_H__

namespace llvm {
    class ModulePass;
}

namespace Intel { namespace OpenCL { namespace DeviceBackend {

/// Returns an instance of the Inst2Func pass,
/// which will be added to a PassManager and run on a Module.
    llvm::ModulePass *createInstToFuncCallPass();

}}}

#endif 