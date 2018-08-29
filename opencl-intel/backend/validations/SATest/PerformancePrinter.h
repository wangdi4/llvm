// INTEL CONFIDENTIAL
//
// Copyright 2011-2018 Intel Corporation.
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

#ifndef PERFORMANCEPRINTER_H
#define PERFORMANCEPRINTER_H

#include <string>
#include <iostream>
#include <fstream>
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
    std::fstream m_fOutStream;
    std::ostream* m_pOutStream;
};

}
#endif // PERFORMANCE_H
