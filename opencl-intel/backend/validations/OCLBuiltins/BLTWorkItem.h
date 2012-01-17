/*****************************************************************************\

Copyright (c) Intel Corporation (2011-2012).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  BLTWorkItem.h

\*****************************************************************************/
#ifndef BLT_WORK_ITEM_H
#define BLT_WORK_ITEM_H

#include <map>
#include <string>
#include "IBLTMapFiller.h"

namespace Validation {
namespace OCLBuiltins {

// This class adds references to the implementations of OpenCL built-in functions from 6.11.4 section.
class WorkItemMapFiller : public IBLTMapFiller
{
public:
    void addOpenCLBuiltins(std::map<std::string, PBLTFunc>& funcNames);
};



} // namespace OCLBuiltins
} // namespace Validation
#endif // BLT_WORK_ITEM_H
