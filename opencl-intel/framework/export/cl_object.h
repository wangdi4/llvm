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

#pragma once
///////////////////////////////////////////////////////////////////////////////////////////////////
//  OCLObject.h
//  Implementation of the Class OCLObject
//  Created on:      10-Dec-2008 4:45:30 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(OCL_OBJECT_H_)
#define OCL_OBJECT_H_

#include "cl_types.h"
#include "logger.h"

using namespace Intel::OpenCL::Utils;

namespace Intel { namespace OpenCL { namespace Framework {

	/**********************************************************************************************
	* Class name:	OCLObject
	*
	* Description:	represnts an OpneCL object
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/	
	class OCLObject
	{
	public:

		/******************************************************************************************
		* Function: 	OCLObject
		* Description:	The OCLObject class constructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/		
		OCLObject();
		
		/******************************************************************************************
		* Function: 	~OCLObject
		* Description:	The OCLObject class destructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/			
		virtual ~OCLObject();
		
        /******************************************************************************************
        * Function: 	Cleanup    
        * Description:	Used to cleanup data before deletion. But can be done differentially if
        *               application is terminated or not.
        * Arguments:	
        * Author:		Arnon Peleg
        * 
        ******************************************************************************************/	
        virtual void Cleanup( bool bIsTerminate = false )   { return; }

        /******************************************************************************************
		* Function: 	Release    
		* Description:	release the OCLObject - decrease referace count in 1, inharited 
		*				objects might want to add functionality to this metod
		* Arguments:	
		* Return value:	CL_SUCCESS - release operation succeded
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/	
		cl_err_code Release();
		
		/******************************************************************************************
		* Function: 	Retain    
		* Description:	retain the OCLObject - increase referace count in 1, inharited objects 
		*				might want to add functionality to this metod
		* Arguments:	
		* Return value:	CL_SUCCESS - retain operation succeded
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/		
		cl_err_code Retain();
		
		/******************************************************************************************
		* Function: 	GetInfo    
		* Description:	get object specific information
		* Arguments:	param_name [in]				parameter's name
		*				param_value_size [inout]	parameter's value size (in bytes)
		*				param_value [out]			parameter's value
		*				param_value_size_ret [out]	parameter's value return size
		* Return value:	CL_SUCCESS - operation succeded
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		cl_err_code	GetInfo(cl_int iParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet);

		/******************************************************************************************
		* Function: 	AddPendency    
		* Description:	increase the pendency counter by 1
		* Arguments:	
		* Return value:	
		* Author:		Uri Levy
		* Date:			February 2008
		******************************************************************************************/
		void AddPendency();

		/******************************************************************************************
		* Function: 	RemovePendency    
		* Description:	decrease the pendency counter by 1
		* Arguments:	
		* Return value:	
		* Author:		Uri Levy
		* Date:			February 2008
		******************************************************************************************/
		void RemovePendency();

		/******************************************************************************************
		* Function: 	ReadyForDeletion    
		* Description:	inform whether the OpenCL object is ready for deletion
		*				one who's overide this function might want to know if there are
		*				resources that attached to this object that need to be deleted
		* Arguments:	
		* Return value:	True	- the object is ready for deletion
		*				False	- the object is not ready for deletion
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		virtual bool ReadyForDeletion();

        cl_err_code SetId(cl_int obj_id) { m_iId = obj_id; return CL_SUCCESS; }		
		cl_int      GetId() const { return m_iId; }
		cl_uint     GetReferenceCount() const { return m_uiRefCount; }

		void *		GetHandle() const { return m_pHandle; }

		/******************************************************************************************
		* Function: 	SetLoggerClient    
		* Description:	set the logger client to the object
		* Arguments:	pLoggerClient - pointer to the logger client
		* Return value:	void
		* Author:		Uri Levy
		* Date:			July 2009
		******************************************************************************************/
		void SetLoggerClient(LoggerClient * pLoggerClient){ SET_LOGGER_CLIENT(pLoggerClient); }

	protected:
		
		cl_int			m_iId;				// object id
		cl_uint			m_uiRefCount;		// reference count
		
		long			m_uiPendency;		// recall the number of dependant resources - will be 
											// used in order to ensure that current object is ready 
											// for deletion

		_cl_object *	m_pHandle;			// the OpenCL handle of the object

		DECLARE_LOGGER_CLIENT;



	};

}}}
#endif // !defined(OCL_OBJECT_H_)
