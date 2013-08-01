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

    for(unsigned int i=1; i<MAX_WORK_DIM; ++i) {
      outputWorkSizes.localWorkSize[i] = 1;
    }

    bool bLocalZeros = true;
    for(unsigned int i=0; i<outputWorkSizes.workDimension; ++i) {
      bLocalZeros &= ( (outputWorkSizes.localWorkSize[i]=pInputWorkSizes->localWorkSize[i]) == 0);
    }
    
    if ( bLocalZeros ) {
      // Try hint size, find GDC for each dimension
      if ( m_pProps->GetHintWGSize()[0] != 0 ) {
          for(unsigned int i=0; i<outputWorkSizes.workDimension; ++i) {
            outputWorkSizes.localWorkSize[i] = GCD(pInputWorkSizes->globalWorkSize[i], m_pProps->GetHintWGSize()[i]);
          }
      }
      else {
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
#define HEURISTIC_MAX_WG_FACTORIAL_EXP (4)
#define HEURISTIC_FACTOR_THRESHOLD (0.06)
#define HEURISTIC_INVOCATION_OVERHEAD (2)
        outputWorkSizes.minWorkGroupNum = pInputWorkSizes->minWorkGroupNum;

        unsigned int globalWorkSizeYZ = 1;
        for(unsigned int i=1; i<outputWorkSizes.workDimension; ++i) {
          // Calculate global group size on dimensions Y & Z
          globalWorkSizeYZ *= pInputWorkSizes->globalWorkSize[i];
          // Set local group size on dimensions Y & Z to 1
          outputWorkSizes.localWorkSize[i] = 1;
        }

        unsigned int kernelPrivateMemSize = (unsigned int)m_pProps->GetPrivateMemorySize();
        unsigned int globalWorkSizeX = pInputWorkSizes->globalWorkSize[0];
        unsigned int localSizeMaxLimit =
          min ( min(CPU_MAX_WORK_GROUP_SIZE, globalWorkSizeX),                                        // localSizeMaxLimit_1
                CPU_DEV_MAX_WG_PRIVATE_SIZE / (kernelPrivateMemSize > 0 ? kernelPrivateMemSize : 1));  // localSizeMaxLimit_2

        unsigned int minMultiplyFactor = m_pProps->GetMinGroupSizeFactorial();
        assert( minMultiplyFactor && (minMultiplyFactor & (minMultiplyFactor-1)) == 0 &&
          "minMultiplyFactor assumed to be power of 2 that is not zero!" );
        assert((!m_pProps->GetJitCreateWIids() ||
          GetKernelJIT(0)->GetProps()->GetVectorSize() == minMultiplyFactor) &&
          "GetMinGroupSizeFactorial is not equal to VectorSize!");

        //These two variables hold the max utility of SIMD and work threads
        //Initialized to 1 by default, assuming utility is satisfied.
        //If it will not be sutisfied (see below) then we will update these variables.
        unsigned int workThreadUtils = outputWorkSizes.minWorkGroupNum;
        unsigned int simdUtils = 1;
        //Try to assure (if possible) the local-size is a multiply of vector width.
        if (((globalWorkSizeX & (minMultiplyFactor-1)) == 0) && (localSizeMaxLimit > minMultiplyFactor)) {
          globalWorkSizeX /= minMultiplyFactor;
          localSizeMaxLimit /= minMultiplyFactor;
        } else {
          //SIMD utility was not satisfied
          simdUtils = minMultiplyFactor;
          minMultiplyFactor = 1;
        }
        const unsigned int globalWorkSize = globalWorkSizeX * globalWorkSizeYZ;
        //This number can be decreased in case there is globalWorkSizeYZ larger than 1!
        const unsigned int workGroupNumMinLimitFactor = (outputWorkSizes.minWorkGroupNum * outputWorkSizes.minWorkGroupNum);
        for(int i=HEURISTIC_MAX_WG_FACTORIAL_EXP; i>=0; --i) {
          if ( globalWorkSize > (workGroupNumMinLimitFactor * (1<<(2*i))) ) {
            localSizeMaxLimit = min(localSizeMaxLimit, globalWorkSize / (outputWorkSizes.minWorkGroupNum * (1<<i)));
            break;
          }
        }
        //Search for max local size that satisfies the constraints
        unsigned int bestHeuristic = 0;
        float bestFactor = 0.0;
        for (; localSizeMaxLimit>0; localSizeMaxLimit--) {
          if ( globalWorkSizeX % localSizeMaxLimit == 0 ) {
            //Check cost function
            const unsigned int numIterVectorizedLoop = (localSizeMaxLimit / simdUtils);
            const unsigned int numIterScalarizerLoop = (localSizeMaxLimit % simdUtils);
            const unsigned int numWorkGroups = (globalWorkSize / localSizeMaxLimit);
            const unsigned int numIterWorkGroupLoop = (numWorkGroups + (workThreadUtils - 1)) / workThreadUtils;
            //Cost function: This is the number of work items that could be executed using the given utils
            const unsigned int heuristicUtils =
              ((numIterVectorizedLoop + numIterScalarizerLoop + HEURISTIC_INVOCATION_OVERHEAD) * simdUtils) *
              (numIterWorkGroupLoop * workThreadUtils);

            assert(heuristicUtils >= globalWorkSize && "global size must be buttom bound for cost function!");
            float wasteFactor = (float)(heuristicUtils-globalWorkSize)/(float)(globalWorkSize);
            if (bestHeuristic == 0) {
              //initialize bestHeuristic
              bestHeuristic = localSizeMaxLimit;
              bestFactor = wasteFactor;
            } else if(wasteFactor < bestFactor) {
              //update bestHeuristic
              bestHeuristic = localSizeMaxLimit;
              bestFactor = wasteFactor;
            }
            if (wasteFactor < HEURISTIC_FACTOR_THRESHOLD) {
              //Reached good enough waste-factor, stop searching
              break;
            }
            //not good enough waste-factor keep searching
          }
        }
        unsigned int newHeuristic = max(1, minMultiplyFactor * bestHeuristic);

        outputWorkSizes.localWorkSize[0] = newHeuristic;
        //printf("heuristic = %d, numOfWG=%d, global_size=%d = (global_size_to_satisfy=%d) * (tsize=%d), minGroupNum=%d, bestHeuristic=%d, bestFactor=%f\n",
        // outputWorkSizes.localWorkSize[0], (globalWorkSize * minMultiplyFactor)/newHeuristic, (globalWorkSize * minMultiplyFactor),
        //  globalWorkSize, minMultiplyFactor, outputWorkSizes.minWorkGroupNum, bestHeuristic, bestFactor);
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