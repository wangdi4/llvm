/*****************************************************************************\

Copyright (c) Intel Corporation (2010).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  Kernel.cpp

\*****************************************************************************/

#include "Kernel.h"
#include "KernelProperties.h"
#include "exceptions.h"
#include "cpu_dev_limits.h"
#include <string.h>


size_t GCD(size_t a, size_t b)
{
    while( 1 )
    {
        a = a % b;
        if( a == 0 )
            return b;
        b = b % a;
        if( b == 0 )
            return a;
    }
}

unsigned int min(unsigned int a, unsigned int b) {
  return a < b ? a : b;
}

unsigned int max(unsigned int a, unsigned int b) {
  return a > b ? a : b;
}

namespace Intel { namespace OpenCL { namespace DeviceBackend {


Kernel::Kernel(const std::string& name, 
               const std::vector<cl_kernel_argument>& args,
               KernelProperties* pProps):
    m_name(name),
    m_args(args),
    m_pProps(pProps)
{
}

Kernel::~Kernel()
{
    delete m_pProps;
    for( std::vector<IKernelJITContainer*>::iterator i = m_JITs.begin(), e = m_JITs.end(); i != e; ++i)
    {
        delete *i;
    }
}

void Kernel::AddKernelJIT( IKernelJITContainer* pJIT)
{
    m_JITs.push_back( pJIT );
}

void Kernel::FreeAllJITs()
{
    for( unsigned i = 0; i < m_JITs.size(); ++i)
    {
        m_JITs[i]->FreeJITCode();
    }
}

void Kernel::CreateWorkDescription( const cl_work_description_type* pInputWorkSizes, 
                                    cl_work_description_type&       outputWorkSizes) const
{
#if defined(_WIN32)
    memcpy_s(&outputWorkSizes, sizeof(cl_work_description_type), pInputWorkSizes, sizeof(cl_work_description_type));
#else
    memcpy(&outputWorkSizes, /*sizeof(cl_work_description_type),*/ pInputWorkSizes, sizeof(cl_work_description_type));
#endif

    for(unsigned int i=1; i<MAX_WORK_DIM; ++i)
    {
        outputWorkSizes.localWorkSize[i] = 1;
    }

    bool bLocalZeros = true;
    for(unsigned int i=0; i<outputWorkSizes.workDimension; ++i)
    {
        bLocalZeros &= ( (outputWorkSizes.localWorkSize[i]=pInputWorkSizes->localWorkSize[i]) == 0);
    }
    
    if ( bLocalZeros )
    {
        // Try hint size, find GDC for each dimension
        if ( m_pProps->GetHintWGSize()[0] != 0 )
        {
            for(unsigned int i=0; i<outputWorkSizes.workDimension; ++i)
            {
                outputWorkSizes.localWorkSize[i] = GCD(pInputWorkSizes->globalWorkSize[i], m_pProps->GetHintWGSize()[i]);
            }
        }
        else
        {
            // Old Heuristic
            // On the last use optimal size
            // Fill first dimension with WG size
            //outputWorkSizes.localWorkSize[0] = GCD(pInputWorkSizes->globalWorkSize[0], m_pProps->GetOptWGSize());
            //
            //for(unsigned int i=1; i<outputWorkSizes.workDimension; ++i)
            //{
            //    outputWorkSizes.localWorkSize[i] = 1;
            //}

            // New Heuristic
            outputWorkSizes.minWorkGroupNum = pInputWorkSizes->minWorkGroupNum;

            unsigned int globalGroupSizeYZ = 1;
            for(unsigned int i=1; i<outputWorkSizes.workDimension; ++i)
            {
                // Calculate global group size on dimensions Y & Z
                globalGroupSizeYZ *= pInputWorkSizes->globalWorkSize[i];
                // Set local group size on dimensions Y & Z to 1
                outputWorkSizes.localWorkSize[i] = 1;
            }
            //This number can be decreased in case there is globalGroupSizeYZ larger than 1!
            unsigned int workGroupNumMinLimit =
                (outputWorkSizes.minWorkGroupNum + (globalGroupSizeYZ-1)) / globalGroupSizeYZ;

            unsigned int globalGroupSizeX = pInputWorkSizes->globalWorkSize[0];
            unsigned int localSizeMaxLimit =
                min ( min(CPU_MAX_WORK_GROUP_SIZE, globalGroupSizeX),                // localSizeMaxLimit_1
                min ( CPU_DEV_MAX_WG_PRIVATE_SIZE / m_pProps->GetPrivateMemorySize(),// localSizeMaxLimit_2
                      max(1, globalGroupSizeX / workGroupNumMinLimit) ));            // localSizeMaxLimit_3

            unsigned int minMultiplyFactor = m_pProps->GetMinGroupSizeFactorial();
            assert( minMultiplyFactor && (minMultiplyFactor & (minMultiplyFactor-1)) == 0 &&
                "minMultiplyFactor assumed to be power of 2 that is not zero!" );
            if ( (globalGroupSizeX & (minMultiplyFactor-1)) != 0 ) {
                // globalGroupSize in diminsion x is not multiply of minMultiplyFactor
                // Cannot satisfy the requist that local group size be multiply of minMultiplyFactor
                // Thus, need set minMultiplyFactor to 1, and let local size be any high number.
                minMultiplyFactor = 1;
            }
            if (m_pProps->GetJitCreateWIids()) {
              // In case of one JIT minMultiplyFactor should be equal to 1 for the new heuristic.
              minMultiplyFactor = 1;
            }

            globalGroupSizeX /= minMultiplyFactor;
            localSizeMaxLimit /= minMultiplyFactor;
            for (; localSizeMaxLimit>0; localSizeMaxLimit--) {
                if ( globalGroupSizeX % localSizeMaxLimit == 0 ) break;
            }
            unsigned int newHeuristic = max(1, minMultiplyFactor * localSizeMaxLimit);

            unsigned int oldHeuristic = GCD(pInputWorkSizes->globalWorkSize[0], m_pProps->GetOptWGSize());
            globalGroupSizeX = pInputWorkSizes->globalWorkSize[0];
            workGroupNumMinLimit = max(1, (outputWorkSizes.minWorkGroupNum/2 + (globalGroupSizeYZ-1)) / globalGroupSizeYZ);
            if (m_pProps->GetJitCreateWIids()) {
              // In case of one JIT we want to use the vector size used in the vector loop.
              minMultiplyFactor = GetKernelJIT(0)->GetProps()->GetVectorSize();
            }

#if 0 // 0 to use second heuristic factor - 1 to use first hueristic factors.
            // One Option for creating the Heuristic factor: (A + B) where,
            // A = #WorkGroups * [unutilized lanes in tail scalar loop]
            // B = #LocalSize * [unutilized cores in last work-group iteration]
            unsigned int newHeuristicFactor = workGroupNumMinLimit * ((minMultiplyFactor - (newHeuristic % minMultiplyFactor)) % minMultiplyFactor) + 
                                 minMultiplyFactor * ((workGroupNumMinLimit-((globalGroupSizeX / newHeuristic) % workGroupNumMinLimit)) % workGroupNumMinLimit);

            unsigned int oldHeuristicFactor = workGroupNumMinLimit * ((minMultiplyFactor - (oldHeuristic % minMultiplyFactor)) % minMultiplyFactor) + 
                                 minMultiplyFactor * ((workGroupNumMinLimit-((globalGroupSizeX / oldHeuristic) % workGroupNumMinLimit)) % workGroupNumMinLimit);
#else
            // Second option for creating the Heuristic factor
            // A = number of iterations in vectorized loop + number of iterations in tail scalar loop.
            // B = number of iterations in the work-group loop.
            unsigned int newHeuristicFactor = ((newHeuristic / minMultiplyFactor) + (newHeuristic % minMultiplyFactor)) *
              (((globalGroupSizeX / newHeuristic) + workGroupNumMinLimit - 1) / workGroupNumMinLimit);

            unsigned int oldHeuristicFactor = ((oldHeuristic / minMultiplyFactor) + (oldHeuristic % minMultiplyFactor)) *
              (((globalGroupSizeX / oldHeuristic) + workGroupNumMinLimit - 1) / workGroupNumMinLimit);
#endif
            // lower factor is better
            outputWorkSizes.localWorkSize[0] = oldHeuristicFactor < newHeuristicFactor ? oldHeuristic : newHeuristic;

            if( 1 == minMultiplyFactor ) {
              // No vectorized loop, only scalar -> choose new heuristic
              outputWorkSizes.localWorkSize[0] = newHeuristic;
            }

            // This line is for debugging
            //printf("heuristic = %d, newF=%d, oldF=%d, new=%d, old=%d, tsize=%d, minGroupNum=%d, global_size=%d\n",
            //  outputWorkSizes.localWorkSize[0],newHeuristicFactor, oldHeuristicFactor, newHeuristic, oldHeuristic,
            //  minMultiplyFactor, workGroupNumMinLimit, globalGroupSizeX);
        }
    }
}

unsigned long long int Kernel::GetKernelID() const
{
    //TODO: MIC specific impl needed
    return 0;
}

const char* Kernel::GetKernelName() const
{
    return m_name.c_str();
}

int Kernel::GetKernelParamsCount() const
{
    return m_args.size();
}

const cl_kernel_argument* Kernel::GetKernelParams() const
{
    return m_args.empty() ? NULL : &*m_args.begin();
}

const cl_kernel_argument_info* Kernel::GetKernelArgInfo() const
{
	return NULL;
}

const ICLDevBackendKernelProporties* Kernel::GetKernelProporties() const
{
    return m_pProps;
}

const std::vector<cl_kernel_argument>* Kernel::GetKernelParamsVector() const
{
    return &m_args;
}

const IKernelJITContainer* Kernel::GetKernelJIT( unsigned int index) const 
{
    assert( index < m_JITs.size() );
    return m_JITs[index];
}

unsigned int Kernel::GetKernelJITCount() const
{
    return m_JITs.size();
}

KernelSet::~KernelSet()
{
    for(std::vector<Kernel*>::const_iterator i = m_kernels.begin(), e = m_kernels.end(); i != e; ++i)
    {
        delete *i;
    }
}

Kernel* KernelSet::GetKernel(int index) const 
{ 
    if( index < 0 || index > (int)m_kernels.size() )
    {
        throw Exceptions::DeviceBackendExceptionBase("Index OOB while accessing the kernel set");
    }
    return m_kernels[index]; 
} 


Kernel* KernelSet::GetKernel(const char* name) const
{ 
    for(std::vector<Kernel*>::const_iterator i = m_kernels.begin(), e = m_kernels.end(); i != e; ++i)
    {
        if( !std::string((*i)->GetKernelName()).compare(name) )
            return *i;
    }
    throw Exceptions::DeviceBackendExceptionBase("No kernel found for given name");
}

}}}