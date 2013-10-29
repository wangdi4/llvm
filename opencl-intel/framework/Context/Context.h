// Copyright (c) 2006-2012 Intel Corporation
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

#pragma once
///////////////////////////////////////////////////////////////////////////////////////////////////
//  Context.h
//  Implementation of the Context class
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "cl_framework.h"

#include "program_service.h"
#include "cl_object.h"
#include "cl_objects_map.h"
#include "MemoryAllocator/MemoryObjectFactory.h"
#include "MemoryAllocator/MemoryObject.h"

#include <Logger.h>
#include <cl_synch_objects.h>
#include <list>
#include <map>
#include <set>
#include "ocl_itt.h"
#include "cl_heap.h"
#include "cl_shared_ptr.h"
#include "sampler.h"

namespace Intel { namespace OpenCL { namespace Framework {

    typedef void (CL_CALLBACK *pfnNotifyBuildDone)(cl_program, void *);

    class Device;
	class FissionableDevice;
	class Program;
	class MemoryObject;
    class ContextModule;
	class SVMBuffer;

    typedef std::set<SharedPtr<Device> > tSetOfDevices;

	/**********************************************************************************************
	* Class name:	Context
	*
	* Inherit:		OCLObject
	* Description:	represents a context
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/		
	class Context : public OCLObject<_cl_context_int>
	{
	public:

        PREPARE_SHARED_PTR(Context)

        /******************************************************************************************
		* Function: 	Allocate
		* Description:	Allocate a new Context
		* Arguments:	clProperties [in] -	context's properties
		*				uiNumDevices [in] -	number of devices associated to the context
		*				ppDevice [in] -		list of devices
		*				pfnNotify [in] -	error notification function's pointer
		*				pUserData [in] -	user date
        * Return:       a SharedPtr<Context> holding the new Context
		* Author:		Aharon Abramson
		* Date:			March 2012
		******************************************************************************************/		
        static SharedPtr<Context> Allocate(const cl_context_properties * clProperties, cl_uint uiNumDevices, cl_uint uiNumRootDevices,
            SharedPtr<FissionableDevice>*ppDevice, logging_fn pfnNotify, void *pUserData, cl_err_code * pclErr, ocl_entry_points * pOclEntryPoints,
            ocl_gpa_data * pGPAData, ContextModule& contextModule)
        {
            return SharedPtr<Context>(new Context(clProperties, uiNumDevices, uiNumRootDevices, ppDevice, pfnNotify, pUserData, pclErr, pOclEntryPoints, pGPAData, contextModule));
        }

		/******************************************************************************************
		* Function: 	Context
		* Description:	The Context class constructor
		* Arguments:	clProperties [in] -	context's properties
		*				uiNumDevices [in] -	number of devices associated to the context
		*				ppDevice [in] -		list of devices
		*				pfnNotify [in] -	error notification function's pointer
		*				pUserData [in] -	user date
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/		
		Context(const cl_context_properties * clProperties, cl_uint uiNumDevices, cl_uint uiNumRootDevices, SharedPtr<FissionableDevice>*ppDevice, logging_fn pfnNotify,
            void *pUserData, cl_err_code * pclErr, ocl_entry_points * pOclEntryPoints, ocl_gpa_data * pGPAData, ContextModule& contextModule);

		/******************************************************************************************
        * Function: 	Cleanup    
        * Description:	Cleanup devices in the context if terminate is false
        * Arguments:	
        * Author:		Arnon Peleg
        *
        ******************************************************************************************/
        void Cleanup( bool bTerminate = false );

		/******************************************************************************************
		* Function: 	GetInfo    
		* Description:	get object specific information (inherited from OCLObject) the function 
		*				query the desirable parameter value from the device
		* Arguments:	param_name [in]				parameter's name
		*				param_value_size [inout]	parameter's value size (in bytes)
		*				param_value [out]			parameter's value
		*				param_value_size_ret [out]	parameter's value return size
		* Return value:	CL_SUCCESS - operation succeeded
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		virtual cl_err_code	GetInfo(cl_int param_name, size_t param_value_size, void * param_value, size_t * param_value_size_ret) const;

		/******************************************************************************************
		* Function: 	CreateProgramWithSource    
		* Description:	creates new program object with source code attached
		* Arguments:	
		* Return value:	
		* Author:		Uri Levy
		* Date:			January 2009
		******************************************************************************************/
		cl_err_code CreateProgramWithSource(cl_uint        IN  uiCount, 
											const char **  IN  ppcStrings, 
											const size_t * IN  szLengths, 
											SharedPtr<Program>*     OUT ppProgram);

		/******************************************************************************************
		* Function: 	CreateProgramWithBinary    
		* Description:	creates new program object with binaries
		* Arguments:	
		* Return value:	
		* Author:		Uri Levy
		* Date:			January 2009
		******************************************************************************************/
		cl_err_code CreateProgramWithBinary(cl_uint					IN  uiNumDevices, 
											const cl_device_id *	IN  pclDeviceList, 
											const size_t *			IN  pszLengths, 
											const unsigned char **	IN  ppBinaries, 
											cl_int *				OUT piBinaryStatus, 
											SharedPtr<Program>*		OUT ppProgram);

		/******************************************************************************************
		* Function: 	CreateProgramWithBuiltInKernels    
		* Description:	creates new program object with built-in kernels
		* Arguments:	
		* Return value:	
		* Author:		Evgeny Fiksman
		* Date:			June 2012
		******************************************************************************************/
		cl_err_code CreateProgramWithBuiltInKernels(cl_uint IN uiNumDevices,
													const cl_device_id * IN pclDeviceList,
													const char IN *szKernelNames,
													SharedPtr<Program>* OUT ppProgram);

        /******************************************************************************************
		* Function: 	CreateProgramForLink    
		* Description:	creates an empty program
		* Arguments:	
		* Return value:	
		* Author:		Sagi Shahar
		* Date:			January 2012
		******************************************************************************************/
		cl_err_code CreateProgramForLink(cl_uint				IN  uiNumDevices, 
										  const cl_device_id *	IN  pclDeviceList, 
										  SharedPtr<Program>*	OUT ppProgram);

        /******************************************************************************************
		* Function: 	CompileProgram  
		* Description:	Compile program from a set of source and headers
		* Arguments:	
		* Return value:	
		* Author:		Sagi Shahar
		* Date:			January 2012
		******************************************************************************************/
		cl_err_code CompileProgram(cl_program			IN  clProgram, 
                                   cl_uint				IN  uiNumDevices,
								   const cl_device_id*	IN  pclDeviceList, 
                                   cl_uint				IN  uiNumHeaders,
                                   const cl_program*	IN  pclHeaders, 
                                   const char**         IN  pszHeadersNames, 
                                   const char*          IN  szOptions, 
                                   pfnNotifyBuildDone   IN  pfn_notify,
                                   void*                IN  user_data);

        /******************************************************************************************
		* Function: 	LinkProgram  
		* Description:	Link program from a set of binaries
		* Arguments:	
		* Return value:	
		* Author:		Sagi Shahar
		* Date:			January 2012
		******************************************************************************************/
		cl_err_code LinkProgram(cl_program				IN  clProgram, 
                                cl_uint					IN  uiNumDevices,
								const cl_device_id*	    IN  pclDeviceList, 
                                cl_uint					IN  uiNumBinaries,
                                const cl_program*		IN  pclBinaries, 
                                const char*             IN  szOptions, 
                                pfnNotifyBuildDone      IN  pfn_notify,
                                void*                   IN  user_data);

        /******************************************************************************************
		* Function: 	BuildProgram  
		* Description:	Build program from source or executable binary
		* Arguments:	
		* Return value:	
		* Author:		Sagi Shahar
		* Date:			January 2012
		******************************************************************************************/
		cl_err_code BuildProgram(cl_program				IN  clProgram, 
                                 cl_uint				IN  uiNumDevices,
								 const cl_device_id*	IN  pclDeviceList, 
                                 const char*            IN  szOptions, 
                                 pfnNotifyBuildDone     IN  pfn_notify,
                                 void*                  IN  user_data);


		// get the number of devices
		cl_uint GetDevicesCount() { return m_mapDevices.Count(); }

		// get the device object pointers that associated to the context
		SharedPtr<FissionableDevice>* GetDevices(cl_uint* puiNumDevices);

		// get the device ids that associated to the context
		cl_device_id * GetDeviceIds(cl_uint* puiNumDevices);

        // Get the list of root-level devices explicitly associated with this context
        SharedPtr<Device>* GetExplicitlyAssociatedRootDevices(cl_uint* puiNumDevices);

        // Get a set of all root devices associated with the context - implicitly, and explicitly.
        const tSetOfDevices *GetAllRootDevices() const;


        cl_dev_subdevice_id GetSubdeviceId(cl_device_id id); 

        /******************************************************************************************
		* Function: 	GetDeviceByIndex
		* Description:	Get a device associated with the context according to the device index
		* Arguments:	uiDeviceIndex [in]	- Device's index
		* Return value:	a SharedPtr<Device> pointing to the device or NULL if the device index is
        *               not associated with the context
		* Author:		Arnon Peleg
		* Date:			January 2009
		******************************************************************************************/
        SharedPtr<FissionableDevice> GetDeviceByIndex(cl_uint uiDeviceIndex);

		// get device by device id
		SharedPtr<FissionableDevice> GetDevice(cl_device_id clDeviceId)
		{
            return m_mapDevices.GetOCLObject((_cl_device_id_int*)clDeviceId).DynamicCast<FissionableDevice>();
		}

        // remove the program from the context
		cl_err_code RemoveProgram(cl_program clProgramId);

		 // remove the memory object from the context
		cl_err_code RemoveMemObject(cl_mem clMem);

		// remove the memory object from the context
		cl_err_code RemoveSampler(cl_sampler clSampler);

		// create new buffer object
		cl_err_code CreateBuffer(cl_mem_flags clFlags, size_t szSize, void * pHostPtr, SharedPtr<MemoryObject>* ppBuffer);

		// create new sub buffer object
		cl_err_code CreateSubBuffer(SharedPtr<MemoryObject> buffer, cl_mem_flags clFlags, cl_buffer_create_type szSize, const void * buffer_create_info, SharedPtr<MemoryObject>* ppBuffer);

		// add an existing SVMBuffer as a cl_mem buffer
		void AddSvmBufferAsMemBuffer(const SharedPtr<SVMBuffer>& pSvmBuffer);

		// create new image object
        template<size_t DIM, cl_mem_object_type OBJ_TYPE>
		cl_err_code CreateImage(	cl_mem_flags	        clFlags,
									const cl_image_format * pclImageFormat,
									void *                  pHostPtr,
                                    const size_t*           szImageDims,
									const size_t*           szImagePitches,
									SharedPtr<MemoryObject>*         ppImage,
                                    bool                    bIsImageBuffer);

        // create new array of image objects
        cl_err_code CreateImageArray(cl_mem_flags		    clflags,
                                     const cl_image_format*	pclImageFormat,
                                     void*					pHostPtr,
                                     const cl_image_desc*   pClImageDesc,
                                     SharedPtr<MemoryObject>*         ppImageArr);

		// get the supported image formats for this context
		cl_err_code GetSupportedImageFormats(	cl_mem_flags		clFlags,
												cl_mem_object_type	clType,
												cl_uint				uiNumEntries,
												cl_image_format*	pclImageFormats,
												cl_uint *			puiNumImageFormats);

		// get memory object according the mem id
        SharedPtr<MemoryObject> GetMemObject(cl_mem clMemId)
                { return m_mapMemObjects.GetOCLObject((_cl_mem_int*)clMemId).StaticCast<MemoryObject>(); }

        MemoryObject*           GetMemObjectPtr(cl_mem clMemId)
                { return (MemoryObject*)(m_mapMemObjects.GetOCLObjectPtr((_cl_mem_int*)clMemId)); }


		// create new sampler object
		cl_err_code CreateSampler(cl_bool bNormalizedCoords, cl_addressing_mode clAddressingMode, cl_filter_mode clFilterMode, SharedPtr<Sampler>* ppSampler);

		// get sampler object according to the sampler id
		SharedPtr<Sampler> GetSampler(cl_sampler clSamplerId);

		// check that all devices belong to this context
		bool CheckDevices(cl_uint uiNumDevices, const cl_device_id * pclDevices);

		// Get devices from the device list
		bool GetDevicesFromList(cl_uint uiNumDevices, const cl_device_id * pclDevices, SharedPtr<FissionableDevice>* ppDevices);
		
		ocl_gpa_data * GetGPAData() const;
        /******************************************************************************************
		* Function: 	NotifyError
		* Description:	Report information on errors that occur in this context using the callback
		*				function registered by the application
		* Arguments:	pcErrInfo [in]		- pointer to an error string
		*				pPrivateInfo [in]	- represent a pointer to binary data that is returned
		*									that can be used to log additional information helpful 
		*									in debugging the error
		*				szCb [in]			- length of binary data
		* Return value:	February
		* Author:		Uri Levy
		* Date:			January 2009
		******************************************************************************************/
		void NotifyError(const char * pcErrInfo, const void * pPrivateInfo, size_t szCb);

		// return context-specific memory objects heap handle
		Intel::OpenCL::Utils::ClHeap	GetMemoryObjectsHeap( void ) const { return m_MemObjectsHeap; };

        // return context module
        ContextModule&                  GetContextModule( void ) { return m_contextModule; };

		bool DoesSupportSvmSystem() const { return m_bSupportsSvmSystem; }

		/**
		 * @param ptr some pointer
		 * @return an SVMBuffer that contains the address pointed to by ptr or NULL if no such SVMBuffer exists
		 */
		ConstSharedPtr<SVMBuffer> GetSVMBufferContainingAddr(const void* ptr) const;
		SharedPtr<SVMBuffer> GetSVMBufferContainingAddr(void* ptr);

		cl_int SetKernelArgSVMPointer(const SharedPtr<Kernel>& pKernel, cl_uint uiArgIndex, const void* pArgValue);

		cl_int SetKernelExecInfo(const SharedPtr<Kernel>& pKernel, cl_kernel_exec_info paramName, size_t szParamValueSize, const void* pParamValue);

		void* SVMAlloc(cl_svm_mem_flags flags, size_t size, unsigned int uiAlignment);

		void SVMFree(void* pSvmPtr);

		cl_err_code CreatePipe(cl_uint uiPipePacketSize, cl_uint uiPipeMaxPackets, SharedPtr<MemoryObject>& pPipe, void* pHostPtr);

		// return access to context-specific OS event pool
#if OCL_EVENT_WAIT_STRATEGY == OCL_EVENT_WAIT_OS_DEPENDENT
		Intel::OpenCL::Utils::OclOsDependentEvent* GetOSEvent();
		void	RecycleOSEvent(Intel::OpenCL::Utils::OclOsDependentEvent* pEvent);		
#endif

	protected:
		/******************************************************************************************
		* Function: 	~Device
		* Description:	The Context class destructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/			
		virtual ~Context();

		cl_ulong	GetMaxMemAllocSize();
		cl_err_code GetMaxImageDimensions(	size_t * psz2dWidth, 
											size_t * psz2dHeight, 
											size_t * psz3dWidth, 
											size_t * psz3dHeight, 
											size_t * psz3dDepth,
                                            size_t * pszArraySize,
                                            size_t * psz1dImgBufSize);

		/**
		 * Compate against UNION of all device capabilities in the context (see clGetSupportedImageFormats).
		 * @param pclImageFormat
		 * @param clMemFlags
		 * @param clObjType
		 * @return 0 if format is supported by any device in the context.
		 */
		cl_err_code	CheckSupportedImageFormat(const cl_image_format *pclImageFormat, cl_mem_flags clMemFlags, cl_mem_object_type clObjType);

		/**
		 * Calculate the supported file formats for context. UNION of all device capabilities (see clGetSupportedImageFormats).
		 * @param clMemFlags
		 * @param clObjType
		 * @return size of supported image formats list.
		 */
		size_t		CalculateSupportedImageFormats( const cl_mem_flags clMemFlags, cl_mem_object_type clObjType );

        cl_err_code CheckSupportedImageFormatByMemFlags(cl_mem_flags clFlags, const cl_image_format& clImageFormat, cl_mem_object_type objType);

		bool									m_bTEActivated;

		// -------------- DEVICES -------------- 
		
		OCLObjectsMap<_cl_device_id_int, _cl_platform_id_int> m_mapDevices;			  // holds the devices that associated to the program
        SharedPtr<Device>*                                m_ppExplicitRootDevices;  // list of all explicit root devices in the context
		SharedPtr<FissionableDevice>*					m_ppAllDevices;
		tSetOfDevices                           m_allRootDevices;         // set of all root devices implicitly/explicitly defined in the context.
		cl_device_id *							m_pDeviceIds;
        cl_device_id *                          m_pOriginalDeviceIds;
        cl_uint                                 m_pOriginalNumDevices;
        cl_uint                                 m_uiNumRootDevices;

		cl_bitfield								m_devTypeMask;			// Mask of device types involved by the context
		
        OCLObjectsMap<_cl_program_int>			m_mapPrograms;			// holds the programs that related to this context
		OCLObjectsMap<_cl_mem_int>		        m_mapMemObjects;		// holds the memory objects that belongs to the context
		OCLObjectsMap<_cl_sampler_int>			m_mapSamplers;			// holds the sampler objects that belongs to the context

        Intel::OpenCL::Framework::ProgramService                          m_programService;

		cl_context_properties *					m_pclContextProperties; // context properties
//		std::map<cl_context_properties, cl_context_properties>
//												m_mapPropertyMap;		// map to search context properties

		cl_uint									m_uiContextPropCount;

		logging_fn								m_pfnNotify; // notify function's pointer

		void *									m_pUserData; // user data

		ocl_gpa_data *							m_pGPAData;
		cl_ulong								m_ulMaxMemAllocSize;
        size_t                                  m_sz1dImgBufSize;
		size_t									m_sz2dWidth;
		size_t									m_sz2dHeight;
		size_t									m_sz3dWidth;
		size_t									m_sz3dHeight;
		size_t									m_sz3dDepth;
		size_t									m_szArraySize;

		typedef std::list<cl_image_format>		tImageFormatList;
		typedef std::map<cl_mem_flags, tImageFormatList>	tImageFormatMap;
		Intel::OpenCL::Utils::OclSpinMutex		m_muFormatsMap;
		tImageFormatMap							m_mapSupportedFormats;

		Intel::OpenCL::Utils::ClHeap			m_MemObjectsHeap;
        ContextModule&                          m_contextModule;
        bool									m_bSupportsSvmSystem;	// if there is at least one device that supports this
		std::map<void*, SharedPtr<SVMBuffer> >  m_svmBuffers;
		mutable Intel::OpenCL::Utils::OclReaderWriterLock m_svmBuffersRwlock;

#if OCL_EVENT_WAIT_STRATEGY == OCL_EVENT_WAIT_OS_DEPENDENT
		typedef Intel::OpenCL::Utils::OclNaiveConcurrentQueue<Intel::OpenCL::Utils::OclOsDependentEvent*> t_OsEventPool;

		t_OsEventPool	m_OsEventPool;
#endif
	
	private:
		Context(const Context&);
		Context& operator=(const Context&);
	};


}}}
