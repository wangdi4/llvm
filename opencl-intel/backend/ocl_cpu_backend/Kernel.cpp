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
            // On the last use optimal size
            // Fill first dimension with WG size
            outputWorkSizes.localWorkSize[0] = GCD(pInputWorkSizes->globalWorkSize[0], m_pProps->GetOptWGSize());
            
            for(unsigned int i=1; i<outputWorkSizes.workDimension; ++i)
            {
                outputWorkSizes.localWorkSize[i] = 1;
            }
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