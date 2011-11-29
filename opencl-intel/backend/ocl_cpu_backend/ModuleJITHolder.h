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

File Name:  ModuleJITHolder.h

\*****************************************************************************/
#ifndef __MODULE_JIT_HOLDER
#define __MODULE_JIT_HOLDER

#include <string>
#include <map>
#include "Serializer.h"

#include "assert.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class ICLDevBackendJITAllocator;
class SerializationStatus;
typedef unsigned long long int KernelID;
typedef struct
{
    int kernelOffset;
    int kernelSize;
} KernelInfo;

/**
 * Represent JIT Code Holder for Module, which contains the main proporties about
 * the jitted code, used in case the JIT is for the whole module at once
 */
class ModuleJITHolder
{
public:
    ModuleJITHolder();
    virtual ~ModuleJITHolder();

    /**
     * @effects sets the JIT code buffer size
     */
    virtual void SetJITCodeSize(int jitSize);

    /**
     * @return the whole JIT Buffer (Code\Data) size
     */
    virtual int GetJITCodeSize() const;
    
    /**
     * @effects sets the required alignment for the executable code
     */
    virtual void SetJITAlignment(size_t alignment);
    
    /**
     * @return the required alignment for the executable code
     */
    virtual size_t GetJITAlignment() const;

    /**
     * @effects sets the JIT code buffer startpoint for execution
     */
    virtual void SetJITCodeStartPoint(const void* pJITCodeStartpoint);

    /**
     * @return the JIT Buffer (Code/Data) start point for execution
     */
    virtual const void* GetJITCodeStartPoint() const;
    
    /**
     * @effects sets the JIT code buffer startpoint for freeing the JIT
     *      NOTE: gets ownership of the JIT code
     */
    virtual void SetJITBufferPointer(void* pJITBuffer);

    /**
     * @return the JIT Buffer (Code/Data) start point, assuming that all
     *    the JIT code in continunios memory
     */
    virtual const void* GetJITBufferPointer() const;

    /**
     * @effects registers a new kernel, for each kernel need to specifiy 
     *      it's id and info, the info should be related to the given JIT
     */
    virtual void RegisterKernel(KernelID kernelId, KernelInfo kernelinfo);

    /**
     * @param kernel identifier
     * @return the entry point of the specified function (relative to the startpoint
     *    of the JIT buffer); Exception will be raised if errors occurs
     */
    virtual int GetKernelEntryPoint(KernelID kernelId) const;
    
    /**
     * @param kernel identifier
     * @returns the size (in bytes) of the given kernel JIT code
     */
    virtual int GetKernelJITSize( KernelID kernelId ) const;

    /**
     * @returns the count of kernels in the JIT code
     */
    virtual int GetKernelCount() const;

    /**
     * Serialization methods for the class (used by the serialization service)
     */
    virtual void Serialize(IOutputStream& ost, SerializationStatus* stats);
    virtual void Deserialize(IInputStream& ist, SerializationStatus* stats);
private:
    char*  m_pJITBuffer;
    const char*  m_pJITCode;
    size_t m_JITCodeSize;
    size_t m_alignment;
    std::map<KernelID, KernelInfo> m_KernelsMap;
    
    ICLDevBackendJITAllocator* m_pJITAllocator; 

    // Klockwork Issue
    ModuleJITHolder ( const ModuleJITHolder& x );

    // Klockwork Issue
    ModuleJITHolder& operator= ( const ModuleJITHolder& x );
};

}}} // namespace

#endif // __MODULE_JIT_HOLDER
