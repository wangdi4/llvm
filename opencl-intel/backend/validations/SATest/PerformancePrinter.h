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

File Name:  PerformancePrinter.h

\*****************************************************************************/
#ifndef PERFORMANCEPRINTER_H
#define PERFORMANCEPRINTER_H

#include <string>
#include "IPerformance.h"
#include "IRunConfiguration.h"
#include "IProgramConfiguration.h"

namespace Validation
{

class PerformancePrinter: public IPerformanceVisitor
{
public:
    PerformancePrinter( const IProgramConfiguration * pProgramConfiguration,
                        const IRunComponentConfiguration* pRunConfiguration );

private:
    void OnKernelSample(const std::string& kernel,  
                        unsigned int vectorSize,
                        cl_long buildTicks, 
                        double buildSDMean,
                        cl_long executionTicks,
                        double executionSDMean,
                        cl_long serializationTicks,
                        double serializationSDMean,
                        cl_long deserializationTicks,
                        double deserializationSDMean
                        );
private:
    std::string m_IRFilename;
    std::string m_JITFilename;
    std::string m_programName;
};

}
#endif // PERFORMANCE_H
