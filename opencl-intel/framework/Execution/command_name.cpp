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

#include "command_name.h"
#include "llvm/Support/Compiler.h"
#include <CL/cl_usm_ext.h>

namespace Intel {
namespace OpenCL {
namespace Framework {

const std::map<cl_command_type, CommandNameDevType> CommandTypeToNames = {
    {CL_COMMAND_NDRANGE_KERNEL,
     {"CL_COMMAND_NDRANGE_KERNEL", nullptr, CL_DEV_CMD_EXEC_KERNEL}},

    {CL_COMMAND_TASK, {"CL_COMMAND_TASK", nullptr, CL_DEV_CMD_EXEC_KERNEL}},

    {CL_COMMAND_NATIVE_KERNEL,
     {"CL_COMMAND_NATIVE_KERNEL", "Native Kernel", CL_DEV_CMD_EXEC_NATIVE}},

    {CL_COMMAND_READ_BUFFER,
     {"CL_COMMAND_READ_BUFFER", "Read Buffer", CL_DEV_CMD_READ}},

    {CL_COMMAND_WRITE_BUFFER,
     {"CL_COMMAND_WRITE_BUFFER", "Write Buffer", CL_DEV_CMD_WRITE}},

    {CL_COMMAND_COPY_BUFFER,
     {"CL_COMMAND_COPY_BUFFER", "Copy Buffer", CL_DEV_CMD_COPY}},

    {CL_COMMAND_READ_IMAGE,
     {"CL_COMMAND_READ_IMAGE", "Read Image", CL_DEV_CMD_READ}},

    {CL_COMMAND_WRITE_IMAGE,
     {"CL_COMMAND_WRITE_IMAGE", "Write Image", CL_DEV_CMD_WRITE}},

    {CL_COMMAND_COPY_IMAGE,
     {"CL_COMMAND_COPY_IMAGE", "Copy Image", CL_DEV_CMD_COPY}},

    {CL_COMMAND_COPY_IMAGE_TO_BUFFER,
     {"CL_COMMAND_COPY_IMAGE_TO_BUFFER", "Copy Image To Buffer",
      CL_DEV_CMD_COPY}},

    {CL_COMMAND_COPY_BUFFER_TO_IMAGE,
     {"CL_COMMAND_COPY_BUFFER_TO_IMAGE", "Copy Buffer To Image",
      CL_DEV_CMD_COPY}},

    {CL_COMMAND_MAP_BUFFER,
     {"CL_COMMAND_MAP_BUFFER", "Map Buffer", CL_DEV_CMD_MAP}},

    {CL_COMMAND_MAP_IMAGE,
     {"CL_COMMAND_MAP_IMAGE", "Map Image", CL_DEV_CMD_MAP}},

    {CL_COMMAND_UNMAP_MEM_OBJECT,
     {"CL_COMMAND_UNMAP_MEM_OBJECT", "Unmap", CL_DEV_CMD_UNMAP}},

    {CL_COMMAND_MARKER, {"CL_COMMAND_MARKER", "Marker", CL_DEV_CMD_INVALID}},

    {CL_COMMAND_READ_BUFFER_RECT,
     {"CL_COMMAND_READ_BUFFER_RECT", "Read Buffer Rect", CL_DEV_CMD_READ}},

    {CL_COMMAND_WRITE_BUFFER_RECT,
     {"CL_COMMAND_WRITE_BUFFER_RECT", "Write Buffer Rect", CL_DEV_CMD_WRITE}},

    {CL_COMMAND_COPY_BUFFER_RECT,
     {"CL_COMMAND_COPY_BUFFER_RECT", "Copy Buffer Rect", CL_DEV_CMD_COPY}},

    {CL_COMMAND_BARRIER, {"CL_COMMAND_BARRIER", "Barrier", CL_DEV_CMD_INVALID}},

    {CL_COMMAND_MIGRATE_MEM_OBJECTS,
     {"CL_COMMAND_MIGRATE_MEM_OBJECTS", "Migrate Memory Object",
      CL_DEV_CMD_MIGRATE}},

    {CL_COMMAND_FILL_BUFFER,
     {"CL_COMMAND_FILL_BUFFER", "Fill Buffer", CL_DEV_CMD_FILL_BUFFER}},

    {CL_COMMAND_FILL_IMAGE,
     {"CL_COMMAND_FILL_IMAGE", "Fill Image", CL_DEV_CMD_FILL_IMAGE}},

    {CL_COMMAND_SVM_FREE,
     {"CL_COMMAND_SVM_FREE", "SVM Free", CL_DEV_CMD_INVALID}},

    {CL_COMMAND_SVM_MEMCPY,
     {"CL_COMMAND_SVM_MEMCPY", "SVM Memcpy", CL_DEV_CMD_COPY}},

    {CL_COMMAND_SVM_MEMFILL,
     {"CL_COMMAND_SVM_MEMFILL", "Fill SVM Buffer", CL_DEV_CMD_FILL_BUFFER}},

    {CL_COMMAND_SVM_MAP,
     {"CL_COMMAND_SVM_MAP", "Map SVM Buffer", CL_DEV_CMD_MAP}},

    {CL_COMMAND_SVM_UNMAP,
     {"CL_COMMAND_SVM_UNMAP", "Unmap SVM Buffer", CL_DEV_CMD_UNMAP}},

    {CL_COMMAND_SVM_MIGRATE_MEM,
     {"CL_COMMAND_SVM_MIGRATE_MEM", "Migrate SVM", CL_DEV_CMD_SVM_MIGRATE}},

    {CL_COMMAND_MEMFILL_INTEL,
     {"CL_COMMAND_MEMFILL_INTEL", "Fill USM", CL_DEV_CMD_FILL_BUFFER}},

    {CL_COMMAND_MEMCPY_INTEL,
     {"CL_COMMAND_MEMCPY_INTEL", "USM Memcpy", CL_DEV_CMD_COPY}},

    {CL_COMMAND_MIGRATEMEM_INTEL,
     {"CL_COMMAND_MIGRATEMEM_INTEL", "Migrate USM", CL_DEV_CMD_USM_MIGRATE}},

    {CL_COMMAND_MEMADVISE_INTEL,
     {"CL_COMMAND_MEMADVISE_INTEL", "Advise USM", CL_DEV_CMD_USM_ADVISE}},

    // Custom types
    {CL_COMMAND_RUNTIME,
     {"CL_COMMAND_RUNTIME", "Runtime Command", CL_DEV_CMD_INVALID}},

    {CL_COMMAND_WAIT_FOR_EVENTS,
     {"CL_COMMAND_WAIT_FOR_EVENTS", "Wait For Events", CL_DEV_CMD_INVALID}},

    {CL_COMMAND_READ_MEM_OBJECT,
     {"CL_COMMAND_READ_MEM_OBJECT", "Read Mem Object", CL_DEV_CMD_READ}},

    {CL_COMMAND_WRITE_MEM_OBJECT,
     {"CL_COMMAND_WRITE_MEM_OBJECT", "Write Mem Object", CL_DEV_CMD_WRITE}},

    {CL_COMMAND_FILL_MEM_OBJECT,
     {"CL_COMMAND_FILL_MEM_OBJECT", "Fill Mem Object", CL_DEV_CMD_FILL_BUFFER}},
};

} // namespace Framework
} // namespace OpenCL
} // namespace Intel
