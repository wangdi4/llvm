// INTEL CONFIDENTIAL
//
// Copyright 2019 Intel Corporation.
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
#ifndef CHANNELPIPEUTILS_H
#define CHANNELPIPEUTILS_H

#include <CompilationUtils.h>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

extern cl::opt<int> ChannelDepthEmulationMode;
extern std::string ChannelPipesErrorLog;

GlobalVariable *
createPipeBackingStore(GlobalVariable *GV,
                       const ChannelPipeMetadata::ChannelPipeMD &MD);

void
setPipeMetadata(GlobalVariable *GV,
                const ChannelPipeMetadata::ChannelPipeMD &MD);

Function *createGlobalPipeCtor(Module &M);

void initializeGlobalPipeScalar(GlobalVariable *PipeGV,
                                const ChannelPipeMetadata::ChannelPipeMD &MD,
                                Function *GlobalCtor,
                                Function *PipeInit);

}}} // namespace Intel::OpenCL::DeiceBackend

#endif
