// INTEL CONFIDENTIAL
//
// Copyright 2013-2018 Intel Corporation.
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

#ifndef KERNELINFO_H
#define KERNELINFO_H

#include <QString>
#include "CL/cl.h"
#include "OpenCLProgramConfiguration.h"

namespace Validation
{
namespace GUI
{

#define MAX_WORK_DIM 3
/**
 * @brief The KernelInfo struct
 * @detailed contains all information about kernel whitch can read from *.cfg file
 */
struct KernelInfo
{
    QString name;
    QString infile;
    QString reffile;
    QString neatfile;
    cl_uint workDimension;
    const size_t* arrGlobalWorkOffset;
    const size_t* arrGlobalWorkSize;
    const size_t* arrLocalWorkSize;
    DataFileType infileType;
    DataFileType neatfileType;
    DataFileType reffileType;


};

}
}

#endif // KERNELINFO_H
