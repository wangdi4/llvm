// Copyright (c) 2006-2007 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly
//
//  Original author: rjiossy
///////////////////////////////////////////////////////////
#pragma once

#include "crt_interface.h"

#include <crt_dynamic_lib.h>
#include <cl_synch_objects.h>
#include <list>
#include <vector>
#include <string>

using namespace OCLCRT::Utils;

// Forward declarations and typedefs
class CrtContext;
class CrtMemObject;
class CrtBuffer;
class CrtImage;
class CrtObject;
struct CrtSampler;
class SyncManager;
typedef std::map<cl_device_id, cl_context>          DEV_CTX_MAP;
typedef std::map<cl_context, KHRicdVendorDispatch*> SHARED_CTX_DISPATCH;
typedef std::map<cl_context, cl_mem>                CTX_MEM_MAP;
typedef std::map<cl_context, cl_program>            CTX_PGM_MAP;
typedef std::map<cl_context, cl_kernel>             CTX_KRN_MAP;
typedef std::map<cl_context, cl_sampler>            CTX_SMP_MAP;

bool operator==(const cl_image_format &rhs1, const cl_image_format &rhs2);
bool operator<(const cl_image_format &rhs1, const cl_image_format &rhs2);

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
// All objects inheret from this, like Buffer/Image/Sampler
class CrtObject
{
public:

    CrtObject();
    virtual ~CrtObject(){};

    enum CrtObjectType
    {
        CL_BUFFER       = 0x1,
        CL_SUB_BUFFER   = 0x2,
        CL_IMAGE        = 0x4,
        CL_SAMPLER      = 0x8,
        CL_INVALID      = 0x10
    };
    virtual CrtObjectType getObjectType() const { return CL_INVALID; };

    long IncRefCnt();
    long DecRefCnt();
    long IncPendencyCnt();
    long DecPendencyCnt();

    long         m_refCount;
    long         m_pendencyCount;
};

// CRT handles definition
struct CrtPlatform
{
    cl_platform_id          m_platformIdDEV;
    char*                   m_supportedExtensionsStr;
    char*                   m_icdSuffix;
    cl_int                  m_supportedExtensions;
    OclDynamicLib           m_lib;
};

struct CrtDeviceInfo
{
    // platform id of the device
    CrtPlatform*                    m_crtPlatform;

    // device Id changed; some functions replaced with CRT functions
    KHRicdVendorDispatch            m_origDispatchTable;

    // Signals if this an original device
    // or created using device fission
    bool                            m_isRootDevice;

    cl_device_type                  m_devType;

    // Device capabilties as table 4.3 in spec 1.1
    cl_device_exec_capabilities     m_deviceCapabilities;

    // refCount valid for Sub-Devices only
    long                            m_refCount;

    // Vector defining if we need to sync and what and which direction
    unsigned int                    m_syncAttribs;
};

struct CrtContextInfo
{
    // Specifies the type of context, whether its
    enum ContextType
    {
        SinglePlatformContext,
        SharedPlatformContext
    };
    // Context Type
    ContextType             m_contextType;

    // Used for single device context
    // Allows better vendor extensions
    // handling.
    CrtPlatform*            m_crtPlatform;

    // For single device context: pointer to original dispatch table
    // For shared device context: poiter to CrtContext
    void*                   m_object;
};

struct CrtProgram: public CrtObject
{
    CrtProgram(CrtContext* ctx);
    virtual ~CrtProgram();
    CTX_PGM_MAP              m_ContextToProgram;
    // Tracks which contexts a build request has been submitted to for this program.
    // We need this since the spec demands the clCreateKernel only for devices 
    // the build context has been submitted to.
    std::vector<cl_context>  m_buildContexts;
    CrtContext*              m_contextCRT;
    cl_program               m_program_handle;

    cl_int                   Release();
};

struct CrtKernel: public CrtObject
{
    CrtKernel(CrtProgram* program);
    virtual ~CrtKernel();
    CTX_KRN_MAP             m_ContextToKernel;
    CrtProgram*             m_programCRT;

    cl_int                  Release();
};
struct CrtQueue: public CrtObject
{
    CrtQueue(CrtContext* ctx);
    virtual ~CrtQueue();
    cl_command_queue    m_cmdQueueDEV;
    cl_device_id        m_device;
    CrtContext*         m_contextCRT;

    cl_int              Release();
};



struct CrtEvent: public CrtObject
{
    CrtEvent(CrtQueue* queue, bool isUserEvent = false);
    virtual     ~CrtEvent();

    CrtQueue*   m_queueCRT;
    cl_event    m_eventDEV;
    bool        m_isUserEvent;

    virtual cl_int      Release();
    virtual CrtContext* getContext() { return m_queueCRT->m_contextCRT; }
};

struct CrtUserEvent: public CrtEvent
{
    CrtUserEvent(CrtContext* ctx);
    virtual     ~CrtUserEvent();

    CrtContext* m_pContext;
    std::map<cl_context, cl_event> m_ContextToEvent;

    virtual cl_int      Release();
    CrtContext* getContext() { return m_pContext; }
};


class CrtMemObject: public CrtObject
{
public:
    CrtMemObject(
        cl_mem_flags        flags,
        void*               hostPtr,
        CrtContext*         ctx);

    virtual ~CrtMemObject();
    // Get the memory object belong to the device in the input
    virtual cl_mem getDeviceMemObj(cl_device_id deviceId);

    // Some entries might be non-valid since the devices don't
    // support the image format param
    virtual cl_mem getAnyValidDeviceMemObj();

    // Get Type of memory object (CL_BUFFER, CL_IMAGE)
    virtual CrtObjectType getObjectType() const  {  return CrtObject::CL_INVALID;  }

    virtual cl_int RegisterDestructorCallback(mem_dtor_fn memDtorFunc, void* user_data);

    // Used whenever the device cannot share memory with host
    virtual cl_event SynchronizeFromDeviceToHost(
        CrtDeviceInfo*  sourceDevice,
        CrtContext*     context) {  return NULL; }

    virtual cl_event SynchronizeToDeviceFromHost(
        CrtDeviceInfo*  targetDevice,
        CrtContext*     context,
        cl_event* eventWaitList) {  return NULL; }

    virtual cl_int Create(CrtMemObject** memObj) { return CL_SUCCESS; };

    virtual cl_int Release();

    bool HasPrivateCopy();

    inline cl_bool IsValidMemObjSize( cl_mem ptr )
    {
        return ( ( ptr == ( cl_mem )INVALID_MEMOBJ_SIZE ) ? CL_FALSE : CL_TRUE );
    }

    inline cl_bool IsValidImageFormat( cl_mem ptr )
    {
        return ( ( ptr == ( cl_mem )INVALID_IMG_FORMAT ) ? CL_FALSE : CL_TRUE );
    }

    // Map between underlying contexts and memory objects
    CTX_MEM_MAP         m_ContextToMemObj;

    // Pointer to shared context
    CrtContext*         m_pContext;

    // backing store memory size
    size_t              m_size;

    // memory creation flags as stated by the user
    cl_mem_flags        m_flags;

    // User provided pointer at creation time
    void*               m_pUsrPtr;

    // Backing store pointer
    void*               m_pBstPtr;

    // some underlying devices doesn't support same image formats
    // other devices might support; this counter remembers
    // how many underlying contexts are valid at m_ContextToMemObj
    long                m_numValidContextObjs;
};


class CrtBuffer: public CrtMemObject
{
public:
    // Buffer Ctor
    CrtBuffer(
        const size_t    size,
        cl_mem_flags    flags,
        void*           host_ptr,
        CrtContext*     ctx);

    // Sub-buffer Ctor
    CrtBuffer(
        _cl_mem_crt*    parent_buffer,
        cl_mem_flags    flags,
        CrtContext*     ctx);

    virtual ~CrtBuffer();

    CrtObjectType getObjectType() const {  return CrtMemObject::CL_BUFFER; }

    // overriding CrtMemOBject::Create for creating buffers (not sub-buffers)
    cl_int Create(CrtMemObject**            memObj);

    // Used for creating sub-buffers
    cl_int Create(
        CrtMemObject**          memObj,
        cl_buffer_create_type   buffer_create_type,
        const void *            buffer_create_info);

    // Used by sub-buffers
    _cl_mem_crt*    m_parentBuffer;
};

size_t  GetImageElementSize(const cl_image_format * format);

class CrtImage: public CrtMemObject
{
public:
    CrtImage(
        cl_mem_object_type      image_type,
        const cl_image_format * image_format,
        size_t                  image_width,
        size_t                  image_height,
        size_t                  image_depth,
        cl_mem_flags            flags,
        void*                   host_ptr,
        CrtContext*             ctx);


    virtual ~CrtImage();

    CrtObjectType getObjectType() const {  return CrtMemObject::CL_IMAGE; }

    // overriding CrtMemOBject::Create for creating buffers (not sub-buffers)
    cl_int Create(size_t rowPitch, size_t slicePitch, CrtMemObject** memObj);

    const cl_image_format *     m_imageFormat;
    size_t                      m_imageWidth;
    size_t                      m_imageHeight;
    size_t                      m_imageDepth;

    // Parameteres forwarded by the CRT to the underlying platforms
    size_t                      m_imageRowPitch;
    size_t                      m_imageSlicePitch;

    // Parameters provided by the user on image create
    size_t                      m_hostPtrRowPitch;
    size_t                      m_hostPtrSlicePitch;

    // Specifies Image type (Image2D/Image3D)
    cl_mem_object_type          m_imageType;
    // For Image2D this will be =2
    // For Image3D this will be =3
    cl_uint                     m_dimCount;
};

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------

// Shared Platform Context
// Manages a number of underlying contexts
// created on the different platforms
class CrtContext: public CrtObject
{
public:
    CrtContext(
        cl_context                      context_handle,
        const cl_context_properties *   properties,
        cl_uint                         num_devices,
        const cl_device_id *            devices,
        ctxt_logging_fn                 pfn_notify,
        void *                          user_data,
        cl_int *                        errcode_ret);

    virtual ~CrtContext();
    // Memory APIs
    cl_int CreateBuffer(
        cl_mem_flags            flags,
        size_t                  size,
        void *                  host_ptr,
        CrtMemObject**          memObj);

    cl_int CreateSubBuffer(
        _cl_mem_crt*            parent_buffer,
        cl_mem_flags            flags,
        cl_buffer_create_type   buffer_create_type,
        const void *            buffer_create_info,
        CrtMemObject**          memObj);

    cl_int CreateImage(
        cl_mem_object_type      mem_obj_type,
        cl_mem_flags            flags,
        const cl_image_format * image_format,
        size_t                  image_width,
        size_t                  image_height,
        size_t                  image_depth,
        size_t                  image_row_pitch,
        size_t                  image_slice_pitch,
        void *                  host_ptr,
        CrtMemObject**          memObj);

    cl_int CreateSampler(
        cl_bool                 normalized_coords,
        cl_addressing_mode      addressing_mode,
        cl_filter_mode          filter_mode,
        CrtSampler**            sampler);

    // Command Queue and Build
    cl_int  CreateCommandQueue(
        cl_device_id                    device,
        cl_command_queue_properties     properties,
        CrtQueue**                      crtQueue);

    cl_int CreateProgramWithSource(
        cl_uint            count ,
        const char **      strings ,
        const size_t *     lengths ,
        CrtProgram **      crtProgram );


    cl_int CreateProgramWithBinary(
        cl_uint                 num_devices,
        const cl_device_id *    device_list,
        const size_t *          lengths,
        const unsigned char **  binaries,
        cl_int *                binary_status,
        CrtProgram **           crtProgram );

    cl_device_id GetDeviceByType( cl_device_type device_type );

    // Flush all command queues on all devices
    cl_int FlushQueues();

    // Used when some devices cannot share memory with Host
    bool memObjectAlwaysInSync(CrtObject::CrtObjectType objType) const {    return true;  }
    bool memObjectAlwaysNotInSync(CrtObject::CrtObjectType objType) const { return false; }

    // Get the alignment agreed by all moinitored devices
    // Returns value in bytes
    cl_uint getAlignment(CrtObjectType objType) const;

    // Returns the Context to which the device belongs
    inline cl_context GetContextByDeviceID( cl_device_id devID ) { return m_DeviceToContext[devID]; }

    // Returns in (outDevices, outNumDevices) the devices which belong
    // to the input platform id (pId)
    void GetDevicesByPlatformId(
        const cl_uint           inNumDevices,
        const cl_device_id*     inDevices,
        const cl_platform_id&   pId,
        cl_uint*                outNumDevices,
        cl_device_id*           outDevices);

    void GetDevsIndicesByPlatformId(
        const cl_uint           inNumDevices,
        const cl_device_id*     inDevices,
        const cl_platform_id&   pId,
        cl_uint*                outNumIndices,
        cl_uint*                outIndices);

    // Get Context reference count
    cl_int GetReferenceCount(cl_uint* refCountParam);

    // Store for all underlying contexts
    // for each it stores the original dispatch table
    // pointer too.
    SHARED_CTX_DISPATCH     m_contexts;

    // Map from device id to matching underlying context couplying that device id
    DEV_CTX_MAP             m_DeviceToContext;

    // Release all underlying contexts
    cl_int Release();

    // We need to keep track of all command queue for flush/finish/WaitForEvents
    std::list<cl_command_queue> m_commandQueues;

    cl_context              m_context_handle;

private:

    // Calculate the alignment agreed by all devices
    // In this context
    cl_int GetDevicesPreferredAlignment(
        const cl_uint           numDevices,
        const cl_device_id*     devices,
        cl_uint*                alignment);

    // Common alignment agreed by all devices in the context (in Bytes)
    cl_uint m_alignment;

};

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------

struct CrtSampler: public CrtObject
{
    CTX_SMP_MAP             m_ContextToSampler;
    CrtContext*             m_contextCRT;
    cl_int Release();
    CrtObjectType getObjectType() const {  return CrtObject::CL_SAMPLER; }
};
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
// Mem destructor callback and user data
struct CrtMemDtorCallBackData
{
    long            m_count;
    CrtMemObject*   m_clMemHandle;
};

void CL_CALLBACK CrtMemDestructorCallBack(cl_mem m, void* userData);


struct CrtEventCallBackData
{
    cl_event    m_eventDEV;
    long        numReqCalls;
};

struct CrtBuildCallBackData
{
    prog_logging_fn                     m_pfnNotify;
    void *                              m_userData;
    long                                m_numBuild;
    cl_program                          m_clProgramHandle;
    OCLCRT::Utils::OclBinarySemaphore   m_lock;
};

void CL_CALLBACK buildCompleteFn( cl_program program, void *userData );

void CL_CALLBACK CrtEventCallBack(cl_event e, cl_int status, void* user_data);

struct CrtMapUnmapCallBackData
{
    CrtEvent*               m_userEvent;    // UserEvent to fire
    cl_int                  m_syncWay;      // True -> Map ;  False -> Unmap
    CrtMemObject*           m_memObj;
    long                    m_numDepEvents;
};
void CL_CALLBACK CrtMapUnmapCallBack(cl_event e, cl_int status, void* user_data);


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------


// Class SyncManager
//
// 1. Responsible for triggering memory synchronization process
//     in case some devices aren't synched with host memory.
//
// 2. Responsible for creating the common runtime user event which
//     connects between events belonging to different queues on different
//     underlying contexts.
//
// This class needs to be used in the following manner:
//     1. Create EventsPartitioner object
//     2. Init() the created object
//     3. Destroy the created object

class SyncManager
{
public:

    enum SYNC_WAY
    {
        SYNC_FROM_BACKING_STORE,
        SYNC_TO_BACKING_STORE
    };

    SyncManager();
    ~SyncManager();
    cl_int PrepareToExecute(
        CrtQueue*       queue,
        cl_uint         NumEventsInWaitList,
        const cl_event* inEventWaitList,
        cl_uint*        numOutEvents,
        cl_event**      OutEvents,
        const cl_uint   numMemObjects,
        CrtMemObject**  memObjects);

    cl_int SyncManager::PreExecuteMemSync(
        SYNC_WAY        syncWay,
        CrtQueue*       queue,
        CrtMemObject*   memObj,
        cl_uint         numDepEvents,
        cl_event*       depEvents,
        CrtEvent**      outEvent);

    cl_int PostExecuteMemSync(
        SYNC_WAY        syncWay,
        CrtQueue*       queue,
        CrtMemObject*   memObj,
        CrtEvent*       depEvent,
        CrtEvent**      outEvent);

    void Release(cl_int errCode);

private:


    CrtEventCallBackData*       m_callBackData;
    cl_event*                   m_outEventArray;
    bool                        m_eventRetained;
    cl_event                    m_userEvent;
    CrtMapUnmapCallBackData*    m_postCallBackData;
};

// This class is used to protect access to shared STL Map resource
// We need this since, on STL, its not thread-safe to call multiple functions
// on the same resource where one of them is write operation.
template <class TKEY, class TVAL>
class GuardedMap
{
public:

    GuardedMap(std::map<TKEY, TVAL>& map):m_map(map){};

    std::map<TKEY, TVAL>& get()
    {
        return m_map;
    }
    void Lock()
    {
        m_mutex.Lock();
    }
    void Release()
    {
        m_mutex.Unlock();
    }
    void Add(TKEY key, TVAL value)
    {
        m_mutex.Lock();
        m_map[key] = value;
        m_mutex.Unlock();
    }
    void Remove(TKEY key)
    {
        m_mutex.Lock();
        std::map<TKEY,TVAL>::iterator itr = m_map.find(key);
        m_map.erase(itr);
        m_mutex.Unlock();
    }
    TVAL GetValue(TKEY key)
    {
        m_mutex.Lock();
        TVAL val =  m_map[key];
        m_mutex.Unlock();
        return val;
    }
    size_t size()
    {
        m_mutex.Lock();
        size_t retSize = m_map.size();
        m_mutex.Unlock();
        return retSize;
    }
private:
    std::map<TKEY, TVAL>& m_map;
    OclMutex m_mutex;
};
