/*****************************************************************************\

Copyright (c) Intel Corporation (2013).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.
\*****************************************************************************/
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
