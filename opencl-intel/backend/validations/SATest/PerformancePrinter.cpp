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

File Name:  PerformancePrinter.cpp

\*****************************************************************************/

#include <iostream>
#include "PerformancePrinter.h"
#include "OpenCLRunConfiguration.h"

using namespace Validation;

PerformancePrinter::PerformancePrinter(const IProgramConfiguration * pProgramConfiguration,
                                       const IRunComponentConfiguration* pRunConfiguration )
{
    const BERunOptions *pBERunConfig = static_cast<const BERunOptions *>(pRunConfiguration);

    m_programName= pProgramConfiguration->GetProgramName();
    m_IRFilename = pBERunConfig->GetValue<std::string>(RC_BR_DUMP_OPTIMIZED_LLVM_IR, "");
    m_JITFilename= pBERunConfig->GetValue<std::string>(RC_BR_DUMP_JIT, "");
}

void PerformancePrinter::OnKernelSample(
                    const std::string& kernel,  
                    cl_long buildTicks, 
                    double buildSDMean,
#ifdef MIC_ENABLE
                    cl_long serializationTicks,
                    double serializationSDMean,
                    cl_long deserializationTicks,
                    double deserializationSDMean,
#endif //MIC_ENABLE
                    cl_long executionTicks,
                    double executionSDMean
                    )
{    
    std::cout << m_programName << "."
              << kernel << ","
              << buildTicks << ","
              << buildSDMean << ","
#ifdef MIC_ENABLE
              << serializationTicks << ","
              << serializationSDMean << ","
              << deserializationTicks << ","
              << deserializationSDMean << ","
#endif //MIC_ENABLE
              << executionTicks << ","
              << executionSDMean << ","
              << "0" << "," // Actualy vector size
              << m_IRFilename << ","
              << m_JITFilename 
              << std::endl;        
}                                
