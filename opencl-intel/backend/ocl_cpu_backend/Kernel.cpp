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
#include <math.h>

static size_t GCD(size_t a, size_t b) {
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

#define BITS_IN_BYTE (8)
static unsigned int LOG(unsigned int a) {
  assert((a!=0) && ((a & (a-1))==0) && "assume a is a power of 2");
  for(unsigned int i=0; i<(unsigned int)sizeof(a)*BITS_IN_BYTE; ++i) {
    if (a & 0x1) return i;
    a = a >> 1;
  }
  return 0;
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
               const std::vector<unsigned int>& memArgs,
               KernelProperties* pProps):
    m_name(name),
    m_args(args),
    m_memArgs(memArgs),
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
        unsigned int globalWorkSizeYZ = 1;
        for(unsigned int i=1; i<outputWorkSizes.workDimension; ++i) {
          // Calculate global group size on dimensions Y & Z
          globalWorkSizeYZ *= pInputWorkSizes->globalWorkSize[i];
          // Set local group size on dimensions Y & Z to 1
          outputWorkSizes.localWorkSize[i] = 1;
        }

        //const unsigned int kernelExecutionLength = (unsigned int)m_pProps->GetKernelExecutionLength();
        const unsigned int kernelPrivateMemSize = (unsigned int)m_pProps->GetPrivateMemorySize();
        unsigned int globalWorkSizeX = pInputWorkSizes->globalWorkSize[0];
        unsigned int localSizeUpperLimit =
          min ( min(CPU_MAX_WORK_GROUP_SIZE, globalWorkSizeX),                                        // localSizeMaxLimit_1
                CPU_DEV_MAX_WG_PRIVATE_SIZE / (kernelPrivateMemSize > 0 ? kernelPrivateMemSize : 1)); // localSizeMaxLimit_2

        unsigned int minMultiplyFactor = m_pProps->GetMinGroupSizeFactorial();
        assert( minMultiplyFactor && (minMultiplyFactor & (minMultiplyFactor-1)) == 0 &&
          "minMultiplyFactor assumed to be power of 2 that is not zero!" );
        assert((!m_pProps->GetJitCreateWIids() ||
          GetKernelJIT(0)->GetProps()->GetVectorSize() == minMultiplyFactor) &&
          "GetMinGroupSizeFactorial is not equal to VectorSize!");
        unsigned int minMultiplyFactorLog = LOG(minMultiplyFactor);

        const unsigned int globalWorkSize = globalWorkSizeX * globalWorkSizeYZ;
        //These variables hold the max utility of SIMD and work threads
        const unsigned int workThreadUtils = outputWorkSizes.minWorkGroupNum;
        const unsigned int simdUtilsLog = minMultiplyFactorLog;
        //Try to assure (if possible) the local-size is a multiply of vector width.
        if (((globalWorkSizeX & (minMultiplyFactor-1)) == 0) && (localSizeUpperLimit >= minMultiplyFactor)) {
          globalWorkSizeX = globalWorkSizeX >> minMultiplyFactorLog;
          localSizeUpperLimit = localSizeUpperLimit >> minMultiplyFactorLog;
        }
        else {
          //SIMD utility was not satisfied
          minMultiplyFactor = 1;
          minMultiplyFactorLog = 0;
        }
        unsigned int localSizeMaxLimit = localSizeUpperLimit;
        const bool isLargeGlobalWGsize = ((workThreadUtils << simdUtilsLog) < globalWorkSize);
        if (isLargeGlobalWGsize) {
          localSizeMaxLimit = min(localSizeMaxLimit,
            // Calculating lower bound for [sqrt(global/(WT*SIMD))*SIMD]
            // Starting the search from this number improves the chances to
            // find a local size that satisfies the "balance" factor.
            // Optimal balanced local size applies the following:
            // Let,
            //   X - local size
            //   Y - number of work groups = (global size / local size)
            // Then: (X * workThreadUtils) == (Y << simdUtilsLog)
            ((unsigned int)sqrt((float)(globalWorkSize/(workThreadUtils << simdUtilsLog)))) << (simdUtilsLog-minMultiplyFactorLog) );
        } else {
          //In this case we have few work-items, try satisfy as much as possible of work threads.
          const unsigned int workGroupNumMinLimit = (workThreadUtils + (globalWorkSizeYZ-1)) / globalWorkSizeYZ;
          localSizeMaxLimit = max(1, localSizeMaxLimit / workGroupNumMinLimit);
        }
        assert(localSizeMaxLimit <= globalWorkSizeX && "global size in dim X must be upper bound for local size");
        //Search for max local size that satisfies the constraints
        unsigned int newHeuristic = localSizeMaxLimit;
        for (; newHeuristic>1; newHeuristic--) {
          if ( globalWorkSizeX % newHeuristic == 0 ) {
            break;
          }
        }
        newHeuristic = newHeuristic << minMultiplyFactorLog;
        //For small global WG size no need for checking the balance cost function
        if(isLargeGlobalWGsize) {
          //Cost function: check if we found a balanced local size compared to number of work groups
          #define BALANCE_FACTOR (2)
          const unsigned int workGroups = globalWorkSize / newHeuristic;
          assert((workGroups << simdUtilsLog) >= (newHeuristic * workThreadUtils) && "Wrong balance factor calculation!");
          const int balanceFactor = (workGroups << simdUtilsLog) - BALANCE_FACTOR * (newHeuristic * workThreadUtils);
          if ( balanceFactor > 0 ) {
            localSizeUpperLimit = min(localSizeUpperLimit, (unsigned int)sqrt((float)globalWorkSize));
            assert(localSizeUpperLimit <= globalWorkSizeX && "global size in dim X must be upper bound for local size");
            //Try to search better local size
            unsigned int newHeuristicUp = localSizeMaxLimit+1;
            for (; newHeuristicUp<=localSizeUpperLimit; newHeuristicUp++) {
              if ( globalWorkSizeX % newHeuristicUp == 0 ) {
                newHeuristicUp = newHeuristicUp << minMultiplyFactorLog;
                //Check cost function and update heuristic if needed
                const unsigned int workGroupsUp = globalWorkSize / newHeuristicUp;
                const int balanceFactorUp = (newHeuristicUp * workThreadUtils) >= (workGroupsUp << simdUtilsLog) ?
                  (newHeuristicUp * workThreadUtils) - BALANCE_FACTOR * (workGroupsUp << simdUtilsLog) :
                  (workGroupsUp << simdUtilsLog) - BALANCE_FACTOR * (newHeuristicUp * workThreadUtils);
                if (balanceFactorUp < balanceFactor) {
                  //Found better local size
                  newHeuristic = newHeuristicUp;
                }
                break;
              }
            }
          }
        }
        outputWorkSizes.localWorkSize[0] = newHeuristic;
        assert(outputWorkSizes.localWorkSize[0] > 0 && "local size must be positive number");
        //printf(
        //  "heuristic = %d, numOfWG = %d, max_local_size = %d, #WorkThreads = %d, "
        //  "(global_size = %d)=(global_size_to_satisfy = %d)*(tsize = %d), balanceFactor = %d\n",
        //  newHeuristic, globalWorkSize/newHeuristic, localSizeMaxLimit, workThreadUtils,
        //  (globalWorkSizeX << minMultiplyFactorLog), globalWorkSizeX, minMultiplyFactor, balanceFactor);
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
    return m_args.empty() ? NULL : &m_args[0];
}

const cl_kernel_argument_info* Kernel::GetKernelArgInfo() const
{
    return NULL;
}

const ICLDevBackendKernelProporties* Kernel::GetKernelProporties() const
{
    return m_pProps;
}

int Kernel::GetLineNumber(void* pointer) const
{
    const unsigned int tNumJits = GetKernelJITCount();
    int lineNum = -1;
    for (unsigned int i = 0; (i < tNumJits) && (-1 == lineNum); i++)
    {
        lineNum = GetKernelJIT(i)->GetLineNumber(pointer);
    }
    return lineNum;
}

size_t Kernel::GetArgumentBufferSize() const
{
    if ( m_args.empty() )
        return 0;

    const cl_kernel_argument& arg = m_args[m_args.size()-1];
    unsigned int mswSize = arg.size_in_bytes >> 16;
    unsigned int size = mswSize > 0 ? (mswSize * arg.size_in_bytes & 0xFFFF) : arg.size_in_bytes;
#ifdef __USE_NEW_BACKEND_API__
    assert( 0 && "This code never tested");
    // TODO: put here size of the uniform args. To be allocate in one call and minimize memcpy
    size_t uniformArgSize = 100;
#else
    size_t uniformArgSize = 0;
#endif
    return arg.offset_in_bytes + size + uniformArgSize;
}

unsigned int Kernel::GetMemoryObjectArgumentCount() const
{
    return m_memArgs.size();
}

const unsigned int* Kernel::GetMemoryObjectArgumentIndexes() const
{
    return m_memArgs.empty() ? NULL : &m_memArgs[0];
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
