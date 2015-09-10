// Copyright (c) 2006-2014 Intel Corporation
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
#include <cstring>
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
class CrtGLBuffer;
class CrtImage;
class CrtGLImage;
class CrtObject;
struct CrtSampler;
class SyncManager;
typedef std::map<cl_device_id, cl_context>          DEV_CTX_MAP;
typedef std::map<cl_context, KHRicdVendorDispatch> SHARED_CTX_DISPATCH;
typedef std::map<cl_context, cl_mem>                CTX_MEM_MAP;
typedef std::map<cl_context, cl_program>            CTX_PGM_MAP;
typedef std::map<cl_context, cl_kernel>             CTX_KRN_MAP;
typedef std::map<cl_context, cl_sampler>            CTX_SMP_MAP;
typedef std::vector<void*>                          MEMMAP_PTR_VEC;

bool operator==(const cl_image_format &rhs1, const cl_image_format &rhs2);
bool operator<(const cl_image_format &rhs1, const cl_image_format &rhs2);

#ifdef _WIN32
#define ALIGNED_MALLOC( size, alignment ) _aligned_malloc( size, (alignment) < sizeof(void*) ? sizeof(void*) : (alignment))
#define ALIGNED_FREE _aligned_free
#elif defined(__ANDROID__)
inline void* ALIGNED_MALLOC( size_t size, size_t alignment )
{
    return memalign( alignment < sizeof(void*) ? sizeof(void*) : alignment, size);
}
#define ALIGNED_FREE free
#else
inline void* ALIGNED_MALLOC( size_t size, size_t alignment )
{
    void* t = NULL;
    if (0 != posix_memalign(&t, alignment < sizeof(void*) ? sizeof(void*) : alignment, size))
    {
        t = NULL;
    }
    return t;
}
#define ALIGNED_FREE free
#endif
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
        CL_PIPE         = 0x10,
        CL_INVALID      = 0x20
    };
    virtual CrtObjectType getObjectType() const { return CL_INVALID; };

    long IncRefCnt();
    long DecRefCnt();
    long IncPendencyCnt();
    long DecPendencyCnt();

    long m_refCount;
    long m_pendencyCount;
};

// CRT handles definition
struct CrtPlatform
{
    CrtPlatform();
    cl_platform_id          m_platformIdDEV;
    char*                   m_supportedExtensionsStr;
    char*                   m_icdSuffix;
    cl_int                  m_supportedExtensions;
    OclDynamicLib           m_lib;
};

struct CrtDeviceInfo
{
    // platform matching this device
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

    CrtDeviceInfo*                  GetChildObject();
};

struct CrtContextInfo
{
    // Specifies the type of context, whether its
    enum ContextType
    {
        SinglePlatformCPUContext,
        SinglePlatformGPUContext,
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
    CTX_PGM_MAP                 m_ContextToProgram;
    // Tracks which contexts a build request has been submitted to for this program.
    // We need this since the spec demands the clCreateKernel only for devices
    // the build context has been submitted to.
    std::vector<cl_context>     m_buildContexts;
    std::vector<cl_device_id>   m_assocDevices;
    CrtContext*                 m_contextCRT;
    cl_program                  m_program_handle;
    std::string                 m_options;
    cl_int                      Release();
};

struct CrtKernel: public CrtObject
{
    CrtKernel(CrtProgram* program);
    virtual ~CrtKernel();
    CTX_KRN_MAP                 m_ContextToKernel;
    CrtProgram*                 m_programCRT;
    cl_int                      Release();
};
struct CrtQueue: public CrtObject
{
    CrtQueue(CrtContext* ctx);
    virtual ~CrtQueue();
    cl_command_queue    m_cmdQueueDEV;
    cl_device_id        m_device;
    CrtContext*         m_contextCRT;
	
	// CRT opaque handle for the command_queue
	cl_command_queue	m_queue_handle;

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


inline cl_int ValidateMapFlags( cl_map_flags map_flags )
{
    cl_int validFlags = ( CL_MAP_READ | CL_MAP_WRITE | CL_MAP_WRITE_INVALIDATE_REGION );
    if( map_flags & ~validFlags )
    {
        return CL_INVALID_VALUE;
    }
    if( ( ( map_flags & ( CL_MAP_READ | CL_MAP_WRITE ) ) != 0 )
        && ( ( map_flags & CL_MAP_WRITE_INVALIDATE_REGION ) != 0 ) )
    {
        return CL_INVALID_VALUE;
    }
    return CL_SUCCESS;
}

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

    virtual void*  GetMapPointer(const size_t* origin, const size_t* region) = 0;
    virtual cl_int CheckParamsAndBounds(const size_t* origin, const size_t* region) = 0;

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

    virtual cl_bool isInteropObject(){ return CL_FALSE; };

    cl_bool HasPrivateCopy();

    inline cl_bool IsValidMemObjSize( cl_mem ptr )
    {
        return ( ( ptr == ( cl_mem )INVALID_MEMOBJ_SIZE ) ? CL_FALSE : CL_TRUE );
    }

    inline cl_bool IsValidImageFormat( cl_mem ptr )
    {
        return ( ( ptr == ( cl_mem )INVALID_IMG_FORMAT ) ? CL_FALSE : CL_TRUE );
    }

    void SetMemHandle( cl_mem memHandle )
    {
        m_memHandle = memHandle;
    }

    cl_mem  GetMemHandle() const
    {
        return m_memHandle;
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

    long                m_mapCount;

    MEMMAP_PTR_VEC      m_mappedPointers;

    // some underlying devices doesn't support same image formats
    // other devices might support; this counter remembers
    // how many underlying contexts are valid at m_ContextToMemObj
    long                m_numValidContextObjs;

    cl_mem              m_memHandle;
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
        CrtMemObject*    parent_buffer,
        cl_mem_flags    flags,
        CrtContext*     ctx);

    virtual ~CrtBuffer();

    CrtObjectType getObjectType() const {  return CrtMemObject::CL_BUFFER; }

    virtual void*  GetMapPointer(const size_t* origin, const size_t* region);
    virtual cl_int CheckParamsAndBounds(const size_t* origin, const size_t* region);

    // overriding CrtMemOBject::Create for creating buffers (not sub-buffers)
    virtual cl_int Create(CrtMemObject** memObj);

    // Used for creating sub-buffers
    cl_int Create(
        CrtMemObject**          memObj,
        cl_buffer_create_type   buffer_create_type,
        const void *            buffer_create_info);

    // Used by sub-buffers
    CrtMemObject*       m_parentBuffer;
};

class CrtGLBuffer: public CrtBuffer
{
public:
    // Buffer Ctor
    CrtGLBuffer(
        cl_bool         isRenderBuffer,
        cl_mem_flags    flags,
        GLuint          bufobj,
        CrtContext*     ctx);

    CrtObjectType getObjectType() const {  return CrtMemObject::CL_BUFFER; }

    cl_bool isInteropObject(){ return CL_TRUE; };

    cl_int Create(CrtMemObject** memObj);

    void*  GetMapPointer(const size_t* origin, const size_t* region){ return NULL; };
    cl_int CheckParamsAndBounds(const size_t* origin, const size_t* region){ return CL_SUCCESS; }

    cl_bool             m_isRenderBuffer;
    GLuint              m_glBufObj;
};


#ifdef _WIN32
class CrtDX9MediaSurface: public CrtMemObject
{
public:
    // Buffer Ctor
    CrtDX9MediaSurface(
        cl_mem_flags            flags,
        IDirect3DSurface9*      resource,
        HANDLE                  sharedhandle,
        UINT                    plane,
        CrtContext*             ctx);

    CrtObjectType getObjectType() const {  return CrtMemObject::CL_BUFFER; }

    cl_bool isInteropObject(){ return CL_TRUE; };

    cl_int Create(CrtMemObject** memObj);

    void*  GetMapPointer(const size_t* origin, const size_t* region){ return NULL; };
    cl_int CheckParamsAndBounds(const size_t* origin, const size_t* region){ return CL_SUCCESS; }

    IDirect3DSurface9*      m_resource;
    HANDLE                  m_sharedHandle;
    UINT                    m_plane;
};
#endif

size_t  GetImageElementSize(const cl_image_format * format);

class CrtImage: public CrtMemObject
{
public:
    // Image Descriptor; augumenting all image params
    // I liked the idea of image descriptor so i adopted that
    // for internal implementation too;
    // Since this isn't gonna be available for OCL 1.1 headers;
    // i duplicated this structure definition internally.
    struct CrtImageDesc {
        cl_image_desc           desc;
        CrtBuffer*              crtBuffer;
    };

    CrtImage(
        const cl_image_format * image_format,
        const cl_image_desc *   image_desc,
        cl_mem_flags            flags,
        void*                   host_ptr,
        CrtContext*             ctx);

    CrtImage(
        cl_mem_flags            flags,
        CrtContext*             ctx);

    void StoreMappedRegion(const void* ptr, const size_t* region);
    std::vector<size_t> GetMappedRegion( const void* ptr);

    void StoreMappedOrigin(const void* ptr, const size_t* origin);
    std::vector<size_t> GetMappedOrigin( const void* ptr);

    virtual ~CrtImage();

    void*  GetMapPointer(const size_t* origin, const size_t* region);
    cl_int CheckParamsAndBounds(const size_t* origin, const size_t* region);

    CrtObjectType getObjectType() const {  return CrtMemObject::CL_IMAGE; }

    // overriding CrtMemOBject::Create for creating buffers (not sub-buffers)
    cl_int Create(CrtMemObject** memObj);

    cl_image_format             m_imageFormat;
    CrtImageDesc                m_imageDesc;

    // Parameters provided by the user on image create
    size_t                      m_hostPtrRowPitch;
    size_t                      m_hostPtrSlicePitch;

    // For Image2D this will be =2
    // For Image3D this will be =3
    cl_uint                     m_dimCount;
    std::map< const void*, std::vector<size_t> > m_mappedOrigins;
    std::map< const void*, std::vector<size_t> > m_mappedRegions;
};

class CrtGLImage: public CrtImage
{
public:
    CrtGLImage(
        cl_uint                 dim_count,
        cl_mem_flags            flags,
        GLenum                  texture_target,
        GLint                   mipleve,
        GLuint                  texture,
        CrtContext*             ctx);

    virtual ~CrtGLImage();

    cl_bool isInteropObject(){ return CL_TRUE; };
    void*  GetMapPointer(const size_t* origin, const size_t* region){ return NULL; }
    cl_int CheckParamsAndBounds(const size_t* origin, const size_t* region){ return CL_SUCCESS; }

    CrtObjectType getObjectType() const {  return CrtMemObject::CL_IMAGE; }

    cl_int Create(CrtMemObject** memObj);

    cl_uint             m_dimCount;
    GLenum              m_textureTarget;
    GLint               m_mipLevel;
    GLuint              m_texture;
};

class CrtPipe: public CrtMemObject
{
public:
    CrtPipe(
        const cl_uint               packetSize,
        const cl_uint               maxPackets,
        const cl_pipe_properties*   properties,
        cl_mem_flags                flags,
        CrtContext*                 ctx);

    CrtObjectType getObjectType() const {  return CrtMemObject::CL_PIPE; }

    void*  GetMapPointer(const size_t* origin, const size_t* region)
    {
        assert( 0 && "GetMapPointer is not supported for Pipes!" );
        return NULL;
    }
    cl_int CheckParamsAndBounds(const size_t* origin, const size_t* region){ return CL_SUCCESS; }

    // overriding CrtMemOBject::Create for creating pipes
    virtual cl_int Create(CrtMemObject** memObj);

    cl_uint             m_packetSize;
    cl_uint             m_maxPackets;
    cl_pipe_properties* m_properties;
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

    cl_int CreateGLBuffer(
        bool                    isRender,
        cl_mem_flags            flags,
        GLuint                  bufobj,
        CrtMemObject**          memObj);

    cl_int CreateSubBuffer(
        _cl_mem_crt*            parent_buffer,
        cl_mem_flags            flags,
        cl_buffer_create_type   buffer_create_type,
        const void *            buffer_create_info,
        CrtMemObject**          memObj);

    cl_int CreateImage(
        cl_mem_flags                    flags,
        const cl_image_format *         image_format,
        const cl_image_desc *           image_desc,
        void *                          host_ptr,
        CrtMemObject**                  memObj);

    cl_int CreateGLImage(
        cl_uint                 dim_count,
        cl_mem_flags            flags,
        cl_GLenum               target,
        cl_GLint                miplevel,
        cl_GLuint               texture,
        CrtMemObject**          memObj);

#ifdef _WIN32
    cl_int CreateFromDX9MediaSurface(
        cl_mem_flags            flags,
        IDirect3DSurface9 *     resource,
        HANDLE                  sharehandle,
        UINT                    plane,
        CrtMemObject**          memObj);
#endif
    cl_int CreateSampler(
        cl_bool                 normalized_coords,
        cl_addressing_mode      addressing_mode,
        cl_filter_mode          filter_mode,
        CrtSampler**            sampler);

    cl_int clCreateSamplerWithProperties(
        const cl_sampler_properties *sampler_properties,
        CrtSampler                  **sampler );

    cl_int CreatePipe(
        cl_mem_flags                flags,
        cl_uint                     pipe_packet_size,
        cl_uint                     pipe_max_packets,
        const cl_pipe_properties *  properties,
        CrtMemObject**              memObj);

    // Command Queue and Build
    cl_int  CreateCommandQueue(
		cl_command_queue				queue_crt_handle,
        cl_device_id                    device,
        cl_command_queue_properties     properties,
        CrtQueue**                      crtQueue);

    // Command Queue wit Properties and Build
    cl_int  CreateCommandQueueWithProperties(
        cl_command_queue                queue_crt_handle,
        cl_device_id                    device,
        const cl_queue_properties *     properties,
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

    cl_int CreateProgramWithBuiltInKernels(
        cl_uint                 num_devices,
        const cl_device_id *    device_list,
        const char *            kernel_names,
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

    // We need to keep track of all host command queue for flush/finish/WaitForEvents
    std::list<cl_command_queue> m_HostcommandQueues;

    cl_context              m_context_handle;

    // mutex gaurding context from concurrent accesses
    OCLCRT::Utils::OclMutex          m_mutex;

    // cache of SVM pointers created using clSVMAlloc()
    std::list<void *>       m_svmPointers;
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
    CrtBuildCallBackData( cl_program prog, prog_logging_fn pfn_notify, void* user_data ):
        m_pfnNotify( pfn_notify ),
        m_userData( user_data ),
        m_clProgramHandle( prog ) {};

    prog_logging_fn                     m_pfnNotify;
    void *                              m_userData;
    long                                m_numBuild;
    cl_program                          m_clProgramHandle;
    OCLCRT::Utils::OclBinarySemaphore   m_lock;
};

void CL_CALLBACK buildCompleteFn( cl_program program, void *userData );

void CL_CALLBACK CrtEventCallBack( cl_event e, cl_int status, void* user_data );


// callback data use db
struct CrtSetEventCallBackData
{
	// crt event 
    cl_event    m_crtEvent;
	// user provided data at the call to clSetEventCallback
    void*       m_userData;
	// user provided notify callback
	pfn_notify  m_userPfnNotify;
};

void CL_CALLBACK CrtSetEventCallBack( cl_event e, cl_int status, void* user_data );


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
        cl_event**      OutEvents);

    cl_int  EnqueueNopCommand(
        CrtMemObject*   memObj,
        CrtQueue*       queue,
        cl_uint         NumEventsInWaitList,
        const cl_event* inEventWaitList,
        cl_event*      outEvent);

    void Release(cl_int errCode);

private:

    CrtEventCallBackData*       m_callBackData;
    cl_event*                   m_outEventArray;
    bool                        m_eventRetained;
    cl_event                    m_userEvent;
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
        typename std::map<TKEY,TVAL>::iterator itr = m_map.find(key);
        m_map.erase(itr);
        m_mutex.Unlock();
    }
    TVAL GetValue(TKEY key)
    {
        m_mutex.Lock();
        typename std::map<TKEY,TVAL>::iterator itr = m_map.find(key);
        TVAL val = (itr != m_map.end()) ? itr->second : NULL;
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

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
// EnqueueSVMFree callback:
struct SVMFreeCallbackData
{
    ~SVMFreeCallbackData();

    bool                CopySVMPointers(void** SVMPointers, cl_uint numSVMPointers);

    bool                m_isGpuQueue;
    bool                m_shouldReleaseEvent;
    cl_command_queue    m_queue;
    void **             m_SVMPointers;
    cl_uint             m_numSVMPointers;
    CrtEvent *          m_svmFreeUserEvent;
    void *              m_originalUserData;
    pfn_free            m_originalCallback;
};
void CL_CALLBACK SVMFreeCallbackFunction(cl_event event, cl_int status, void *user_data);
