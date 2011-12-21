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

File Name:  MICSerializationService.h

\*****************************************************************************/
#ifndef __SERIALIZATION_SERVICE
#define __SERIALIZATION_SERVICE

#include "cl_dev_backend_api.h"
#include "IAbstractBackendFactory.h"
#include <map>
#include <string>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class TargetDescription;

class SerializationStatus
{
public:
    SerializationStatus();
    
    void  SetPointerMark(const std::string& mark, void* pointer);
    void* GetPointerMark(const std::string& mark);
    
    void SetJITAllocator(ICLDevBackendJITAllocator* pJITAllocator);
    ICLDevBackendJITAllocator* GetJITAllocator();

    void SetBackendFactory(IAbstractBackendFactory* pBackendFactory);
    IAbstractBackendFactory* GetBackendFactory();
    
private:
    ICLDevBackendJITAllocator* m_pJITAllocator;
    IAbstractBackendFactory* m_pBackendFactory;
    
    std::map<std::string, void*> m_marksMap;
};

class DefaultJITMemoryManager : public ICLDevBackendJITAllocator
{
private:
    DefaultJITMemoryManager() { };
    ~DefaultJITMemoryManager() { };

public:
    static void Init();
    static void Terminate();
    static DefaultJITMemoryManager* GetInstance();

    void* AllocateExecutable(size_t size, size_t alignment);
    void FreeExecutable(void* ptr);
private:
    static DefaultJITMemoryManager* s_pInstance;
};


class MICSerializationService : public ICLDevBackendSerializationService
{
public:
    
    MICSerializationService(const ICLDevBackendOptions* pBackendOptions);

    // Program Functions
    virtual cl_dev_err_code GetSerializationBlobSize(
        cl_serialization_type serializationType,
        const ICLDevBackendProgram_* pProgram, size_t* pSize) const;

    virtual cl_dev_err_code SerializeProgram(
        cl_serialization_type serializationType, 
        const ICLDevBackendProgram_* pProgram, 
        void* pBlob, size_t blobSize) const;

    virtual cl_dev_err_code DeSerializeProgram(
        ICLDevBackendProgram_** ppProgram, 
        const void* pBlob,
        size_t blobSize) const;

    // Target Description Functions
    virtual cl_dev_err_code GetTargetDescriptionBlobSize(
        const TargetDescription* pTargetDescription, 
        size_t* pSize) const;

    virtual cl_dev_err_code SerializeTargetDescription(
        const TargetDescription* pTargetDescription, 
        void* pBlob, 
        size_t blobSize) const;

    virtual cl_dev_err_code DeSerializeTargetDescription(
        TargetDescription** pTargetDescription, 
        const void* pBlob,
        size_t blobSize) const;

    virtual void Release();
private:
    // pointer to the JIT memory allocator\cleaner, not owned by this class
    ICLDevBackendJITAllocator* m_pJITAllocator;
    IAbstractBackendFactory* m_pBackendFactory;
};

}}} // namespace

#endif // __SERIALIZATION_SERVICE
