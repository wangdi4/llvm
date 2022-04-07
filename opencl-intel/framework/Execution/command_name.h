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

#ifndef FRAMEWORK_EXECUTION_COMMAND_NAME_H
#define FRAMEWORK_EXECUTION_COMMAND_NAME_H

#include "cl_device_api.h"
#include "cl_types.h"
#include <map>

namespace Intel {
namespace OpenCL {
namespace Framework {

struct CommandNameDevType {
  const char *Name;
  const char *NameGPA;
  cl_dev_cmd_type DevType;
};

extern const std::map<cl_command_type, CommandNameDevType> CommandTypeToNames;

inline const char *getCommandName(cl_command_type T) {
  return CommandTypeToNames.count(T) ? CommandTypeToNames.at(T).Name : nullptr;
}

inline const char *getCommandNameGPA(cl_command_type T) {
  return CommandTypeToNames.count(T) ? CommandTypeToNames.at(T).NameGPA
                                     : nullptr;
}

inline cl_dev_cmd_type getCommandDevType(cl_command_type T) {
  return CommandTypeToNames.count(T) ? CommandTypeToNames.at(T).DevType
                                     : CL_DEV_CMD_INVALID;
}

} // namespace Framework
} // namespace OpenCL
} // namespace Intel

#endif // FRAMEWORK_EXECUTION_COMMAND_NAME_H
