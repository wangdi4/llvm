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

	/// Forward declarations and typedefs
class CrtContext;
class CrtMemObject;
class CrtBuffer;
class CrtImage;
class CrtObject;
typedef std::map<cl_device_id, cl_context>	DEV_CTX_MAP;	
typedef std::map<cl_context, KHRicdVendorDispatch*>	SHARED_CTX_DISPATCH;
typedef std::map<cl_context, cl_mem>		CTX_MEM_MAP;

	/// CRT handles definition
struct CrtPlatform
{		
	cl_platform_id 	m_platformIdDEV;
	OclDynamicLib	m_lib;
};

struct CrtDeviceInfo
{	
		/// platform id of the device	
	cl_platform_id					m_platformIdDEV;	

		///	device Id changed; some functions replaced with CRT functions	
	KHRicdVendorDispatch			m_origDispatchTable;
		
		/// Singlas if this an original device
		/// or created using device fission
	bool							m_isRootDevice;
		
		/// Device capabilties as table 4.3 in spec 1.1	
	cl_device_exec_capabilities		m_deviceCapabilities;

		/// refCount valid for Sub-Devices only
	cl_uint							m_refCount;

		///	Vector defining if we need to sync and what and which direction
	unsigned int					m_syncAttribs;
};

struct CrtContextInfo
{
		/// Specifies the type of context, whether its
	enum ContextType
	{
		SinglePlatformContext,
		SharedPlatformContext
	};
	ContextType				m_contextType;
	cl_context_properties	m_gLHGLRCHandle;
    cl_context_properties	m_gLHDCHandle;
	void*					m_object;
};

struct CrtProgram
{
    typedef std::map<cl_context, cl_program> CTX_PGM_MAP;
    CTX_PGM_MAP             m_ContextToProgram;
	CrtContext*             m_contextCRT;
		/// refCount for handling retain/release
	cl_uint				    m_refCount;
};

struct CrtKernel
{
    typedef std::map<cl_context, cl_kernel> CTX_KER_MAP;
    CTX_KER_MAP             m_ContextToKernel;
	CrtProgram*             m_programCRT;
		/// refCount for handling retain/release
	cl_uint				    m_refCount;
};
struct CrtQueue
{	
	CrtQueue(): m_refCount(1){}
	cl_command_queue 	m_cmdQueueDEV;
	cl_device_id 		m_device; 
	CrtContext* 		m_contextCRT;		
		/// refCount for handling retain/release
	cl_uint				m_refCount;
};



struct CrtEvent
{	
	CrtEvent(bool isUserEvent = false): m_refCount(1), m_isUserEvent(isUserEvent){}
	CrtContext* getContext() { return m_queueCRT->m_contextCRT; }
	virtual ~CrtEvent();
	CrtQueue* 	m_queueCRT;
	cl_event 	m_eventDEV;	
	/// Underlying context identifer
	cl_uint		m_refCount;
	bool		m_isUserEvent;
};

struct CrtUserEvent: public CrtEvent
{
	CrtUserEvent(): CrtEvent(true){}
	CrtContext* getContext() { return m_pContext; }
	virtual ~CrtUserEvent();
	CrtContext*	m_pContext;
	std::map<cl_context, cl_event> m_ContextToEvent;
};



/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
	/// All objects inheret from this, like Buffer/Image/Sampler
class CrtObject
{
public:
	enum MemObjType
	{
		CL_BUFFER 		= 0x1,
		CL_SUB_BUFFER 	= 0x2,
		CL_IMAGE  		= 0x4,
		CL_SAMPLER		= 0x8,
		CL_INVALID		= 0x10
	};
	virtual MemObjType getObjectType() const = 0;			
};

class CrtMemObject: public CrtObject
{
public:
		/// Ctor
	CrtMemObject(
		cl_mem_flags		flags, 
		void*				hostPtr, 
		CrtContext*			ctx);

	virtual ~CrtMemObject();
		// Get the memory object belong to the device in the input	
	virtual cl_mem getDeviceMemObj(cl_device_id deviceId);

		/// Get Type of memory object (CL_BUFFER, CL_IMAGE)
	virtual MemObjType getObjectType() const  {  return CrtObject::CL_INVALID;  }

	virtual cl_int RegisterDestructorCallback(mem_dtor_fn memDtorFunc, void* user_data);

		/// Used whenever the device cannot share memory with host
	virtual cl_event SynchronizeFromDeviceToHost( 
		CrtDeviceInfo*	sourceDevice, 
		CrtContext*		context) {	return NULL; }

	virtual cl_event SynchronizeToDeviceFromHost( 
		CrtDeviceInfo*	targetDevice, 
		CrtContext*		context,  
		cl_event* eventWaitList) {	return NULL; }

	virtual cl_int Create(CrtMemObject** memObj) { return CL_SUCCESS; };

	virtual cl_int Release();	

		/// Map between underlying contexts and memory objects	
	CTX_MEM_MAP 	m_ContextToMemObj;	

		/// Pointer to shared context
	CrtContext*			m_pContext;	

		/// Internal reference counting
	cl_uint				m_refCount;

protected:
		/// memory creation flags as stated by the user
	cl_mem_flags		m_flags;				
		/// User provided pointer at creation time		
	void*				m_pUsrPtr;				
		/// Backing store pointer
	void*				m_pBstPtr;					
		
};


class CrtBuffer: public CrtMemObject
{
public:		
		/// Buffer Ctor
	CrtBuffer(
		const size_t	size, 
		cl_mem_flags	flags, 
		void*			host_ptr, 
		CrtContext*		ctx);	
	
		/// Sub-buffer Ctor
	CrtBuffer(
		CrtBuffer*		parent_buffer,
		cl_mem_flags	flags,		
		CrtContext*		ctx);	

	virtual ~CrtBuffer();

	MemObjType getObjectType() const {  return CrtMemObject::CL_BUFFER; }

		/// overriding CrtMemOBject::Create for creating buffers (not sub-buffers)
	cl_int Create(CrtMemObject**			memObj);	

		/// Used for creating sub-buffers
	cl_int Create(
		CrtMemObject**			memObj,
		cl_buffer_create_type	buffer_create_type,
		const void *            buffer_create_info);


	
private:
	CrtBuffer*	m_parentBuffer;
	size_t		m_size; // buffer size
};


class CrtImage: public CrtMemObject
{
public:		
		/// Buffer Ctor
	CrtImage(
		cl_mem_object_type		image_type,
		const cl_image_format * image_format,
		size_t                  image_width,
		size_t                  image_height,
		size_t                  image_depth,
		size_t                  image_row_pitch,
		size_t                  image_slice_pitch, 
		cl_mem_flags			flags, 
		void*					host_ptr, 
		CrtContext*				ctx);	
		

	virtual ~CrtImage();

	MemObjType getObjectType() const {  return CrtMemObject::CL_IMAGE; }

		/// overriding CrtMemOBject::Create for creating buffers (not sub-buffers)
	cl_int Create(CrtMemObject**	memObj);	
		
private:
		/// Inspects the image format and returns the corresponding
		/// element size
	size_t	GetElementSize() const;

	cl_mem_object_type			m_imageType;
	const cl_image_format *		m_imageFormat;
	size_t						m_imageWidth;
	size_t						m_imageHeight;
	size_t						m_imageDepth;
	size_t						m_imageRowPitch;
	size_t						m_imageSlicePitch;

	size_t						m_size;
};

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------

	/// Shared Platform Context
	/// Manages a number of underlying contexts 
	/// created on the different platforms
class CrtContext
{
public:
	CrtContext(
		cl_context 						context_handle,
		const cl_context_properties *	properties,
		cl_uint							num_devices,
		const cl_device_id *			devices,
		logging_fn						pfn_notify,
		void *							user_data,
		cl_int *						errcode_ret);

	virtual ~CrtContext();
		/// Memory APIs
	cl_int CreateBuffer( 
		cl_mem_flags			flags, 
		size_t					size, 
		void *					host_ptr,		
		CrtMemObject**			memObj);

	cl_int CreateSubBuffer( 
		CrtBuffer*				parent_buffer,
		cl_mem_flags			flags,		
		cl_buffer_create_type	buffer_create_type,
		const void *            buffer_create_info,
		CrtMemObject**			memObj);
		
	cl_int CreateImage(
		cl_mem_object_type		mem_obj_type,
		cl_mem_flags            flags,
		const cl_image_format * image_format,
		size_t                  image_width,
		size_t                  image_height,
		size_t                  image_depth,
		size_t                  image_row_pitch,
		size_t                  image_slice_pitch,
		void *                  host_ptr,
		CrtMemObject**			memObj);
	
		/// Command Queue and Build
	cl_int	CreateCommandQueue(	
		cl_device_id					device, 
		cl_command_queue_properties		properties, 
		CrtQueue**						crtQueue);
	
    cl_int CreateProgramWithSource( cl_uint            count ,
                                    const char **      strings ,
                                    const size_t *     lengths ,
                                    CrtProgram **      crtProgram );

		/// Flush all command queues on all devices
	cl_int FlushQueues();

		/// Used when some devices cannot share memory with Host
	bool memObjectAlwaysInSync(CrtObject::MemObjType objType) const {	return true;  }
	bool memObjectAlwaysNotInSync(CrtObject::MemObjType objType) const {	return false; }	

		/// Get the alignment agreed by all moinitored devices
		/// Returns value in bytes
	cl_uint getAlignment() const { return (m_alignment >> 3); }

        /// Returns the Context to which the device belongs
    inline cl_context GetContextByDeviceID( cl_device_id devID ) { return m_DeviceToContext[devID]; }
		
		/// Returns in (outDevices, outNumDevices) the devices which belong
		/// to the input platform id (pId)
	void GetDevicesByPlatformId(
		const cl_uint			inNumDevices, 
		const cl_device_id*		inDevices, 
		const cl_platform_id&	pId, 
		cl_uint*				outNumDevices, 
		cl_device_id*			outDevices);	

		/// Get Context reference count
	cl_int GetReferenceCount(cl_uint* refCountParam);

		/// Store for all underlying contexts	
		/// for each it stores the original dispatch table
		/// pointer too.
	SHARED_CTX_DISPATCH		m_contexts;
	
		/// Map from device id to matching underlying context couplying that device id
	DEV_CTX_MAP				m_DeviceToContext;

		/// Release all underlying contexts
	cl_int Release();
				
		/// Context reference counting
	cl_uint					m_refCount;
		/// We need to keep track of all command queue for flush/finish/WaitForEvents
	std::list<cl_command_queue>	m_commandQueues;

	cl_context				m_context_handle;
private:	
		

		/// Calculate the alignment agreed by all devices
		/// In this context
	cl_int GetDevicesPreferredAlignment(
		const cl_uint			numDevices, 
		const cl_device_id*		devices, 
		cl_uint*				alignment);

		
		/// Common alignment agreed by all devices in the context
	cl_uint m_alignment; 
	
};

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------

class CrtSampler: public CrtObject
{
	MemObjType getObjectType() const {  return CrtObject::CL_SAMPLER; }
};
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
	/// Mem destructor callback and user data
struct CrtMemDtorCallBackData
{
	cl_uint			m_count;

	CrtMemObject**  m_clMemHandle;

} ;

void CL_CALLBACK CrtMemDestructorCallBack(cl_mem m, void* userData);


typedef struct _CrtEventCallBackData
{
	cl_event	m_eventDEV;
	cl_uint		numReqCalls;
}CrtEventCallBackData;

typedef struct _CrtBuildCallBackData
{
    prog_logging_fn     pfn_notify;
    void *              user_data;
    cl_uint             numBuild;
}CrtBuildCallBackData;
void CL_CALLBACK CrtEventCallBack(cl_event e, cl_int status, void* user_data);


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------


	/// Class SyncManager
	///
	/// 1. Responsible for triggering memory synchronization process
	///		in case some devices aren't synched with host memory.
	///
	/// 2. Responsible for creating the common runtime user event which
	///		connects between events belonging to different queues on different
	///		underlying contexts.
	///
	/// This class needs to be used in the following manner:
	///		1. Create EventsPartitioner object
	///		2. Init() the created object
	///		3. Destroy the created object

class SyncManager
{
public:
	SyncManager();
	~SyncManager();
	cl_int PrepareToExecute(
		CrtQueue*		queue,
		cl_uint			NumEventsInWaitList,
		const cl_event*	inEventWaitList,
		cl_uint*		numOutEvents,
		cl_event**		OutEvents,
		const cl_uint	numMemObjects,
		CrtMemObject*	memObjects);
private:
	CrtEventCallBackData*	m_callBackData;
	cl_event*				m_outEventArray;
	bool					m_eventRetained;
	cl_event				m_userEvent;
};

	

class CrtBuildMgr 
{
public:
	CrtBuildMgr()
	{
		m_buildData     = NULL;
        m_CRTProgram    = NULL;
        m_numBuild      = 0;
    }

    cl_uint Init(cl_program pgm)
    {
		cl_int errCode = CL_SUCCESS;
        m_CRTProgram   = pgm;

		if (!m_buildData)
		{
				/// Initialize crt build callback data
			m_buildData = new CrtBuildCallBackData;
			if (!m_buildData)						
			{
				errCode = CL_OUT_OF_HOST_MEMORY;
				return errCode;
			}
            m_buildData->pfn_notify = NULL;
            m_buildData->user_data = NULL;
        }
        return errCode;
    }

    void Cleanup()
    {
        if(m_buildData)
            delete m_buildData;
    }

    inline void incNumBuild()
    {
         atomic_increment(&(m_numBuild));
    }

    inline void decNumBuild()
    {
         atomic_decrement(&(m_numBuild));
    }

    inline void setNumBuild(int build)
    {
        m_numBuild = build;
    }

    inline cl_uint getNumBuild()
    {
        return m_numBuild;
    }

    inline CrtBuildCallBackData* setBuildCallBackData(prog_logging_fn pfn, void * udata)
    {
        m_buildData->pfn_notify = pfn;
        m_buildData->user_data = udata;
        return m_buildData;
    }

    prog_logging_fn buildCompleteFn( cl_program program, void *userData )
    {
        if (m_numBuild > 0)
        {
            decNumBuild();
        }
		    /// Check if this is the last build routine to finish
        if (0 == m_numBuild)
        {
            CrtBuildCallBackData *data = ( CrtBuildCallBackData* ) userData;

            if( data->pfn_notify )
            {
                data->pfn_notify( m_CRTProgram, data->user_data );
            }
	    }
        return 0;
    }

private:
    CrtBuildCallBackData            *m_buildData;
    cl_program                       m_CRTProgram;
    cl_uint                          m_numBuild;

};
