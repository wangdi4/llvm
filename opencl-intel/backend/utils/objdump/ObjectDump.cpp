// INTEL CONFIDENTIAL
//
// Copyright 2020 Intel Corporation.
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

#include "ObjectDump.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/TargetSelect.h"

using namespace llvm;

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {
namespace Utils {

raw_fd_ostream *Out;

ObjectDump::ObjectDump() {
  // Initialize targets and assembly printers/parsers.
  InitializeAllTargetInfos();
  InitializeAllTargetMCs();
  InitializeAllDisassemblers();

  std::vector<const char *> argv = {"ObjectDump", "--x86-asm-syntax=intel"};
  cl::ParseCommandLineOptions((int)argv.size(), &argv[0]);
}

Error ObjectDump::dumpObject(const MemoryBuffer *ObjBuffer,
                             raw_fd_ostream &OS) {
  Expected<std::unique_ptr<object::ObjectFile>> OFOrErr =
      object::ObjectFile::createObjectFile(*ObjBuffer);
  if (!OFOrErr)
    return OFOrErr.takeError();
  object::ObjectFile *OF = (*OFOrErr).get();

  Out = &OS;
  dumpObjectToFile(OF);
  return Error::success();
}

} // namespace Utils
} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
