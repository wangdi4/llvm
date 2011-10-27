/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  IRunResultComparator.h

\*****************************************************************************/
#ifndef I_RUN_RESULT_COMPARATOR_H
#define I_RUN_RESULT_COMPARATOR_H

namespace Validation
{
    class IRunResult;
    class IRunResultComparison;

    /// @brief Contains the result of comparison for entire run result
    class IRunResultComparator
    {
    public:
        /// @brief Compares the result of two runs
        virtual IRunResultComparison* Compare( IRunResult* lhs, IRunResult* rhs ) const = 0;
    };
}

#endif // I_RUN_RESULT_H
