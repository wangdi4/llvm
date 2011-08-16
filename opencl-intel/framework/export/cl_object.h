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

///////////////////////////////////////////////////////////////////////////////////////////////////
//  OCLObject.h
//  Implementation of the Class OCLObject
//  Created on:      10-Dec-2008 4:45:30 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "cl_types.h"
#include "Logger.h"
#include "cl_synch_objects.h"

namespace Intel { namespace OpenCL { namespace Framework {

	/**********************************************************************************************
	* Class name:	OCLObject
	*
	* Description:	represents an OpneCL object
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/	

	/**********************************************************************************************
	* A note about reference count and pendencies: an OCLObject is created with one of each.
	* The floating reference is standard (implicitly owned by the object's creator)
	* The floating pendency will be removed when the ref count goes to zero, 
	* so it represents that the object is still "alive" and visible to the user.
	**********************************************************************************************/
	template <class HandleType> 
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
		* Description:	release the OCLObject - decrease reference count in 1, inherited 
		*				objects might want to add functionality to this method
		* Arguments:	
		* Return value:	The new ref count or -1 in case of error
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/	
		virtual long Release();
		
		/******************************************************************************************
		* Function: 	Retain    
		* Description:	retain the OCLObject - increase reference count in 1, inherited objects 
		*				might want to add functionality to this method
		* Arguments:	
		* Return value:	CL_SUCCESS - retain operation succeeded
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/		
		virtual cl_err_code Retain();
		
		/******************************************************************************************
		* Function: 	GetInfo    
		* Description:	get object specific information
		* Arguments:	param_name [in]				parameter's name
		*				param_value_size [inout]	parameter's value size (in bytes)
		*				param_value [out]			parameter's value
		*				param_value_size_ret [out]	parameter's value return size
		* Return value:	CL_SUCCESS - operation succeeded
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		virtual cl_err_code	GetInfo(cl_int iParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet) = 0;

		/******************************************************************************************
		* Function: 	AddPendency    
		* Description:	increase the pendency counter by 1
		* Arguments:	
		* Return value:	
		* Author:		Uri Levy
		* Date:			February 2008
		******************************************************************************************/
		virtual long AddPendency();

		/******************************************************************************************
		* Function: 	RemovePendency    
		* Description:	decrease the pendency counter by 1
		* Arguments:	
		* Return value:	
		* Author:		Uri Levy
		* Date:			February 2008
		******************************************************************************************/
		virtual long RemovePendency();

        cl_err_code SetId(cl_int obj_id) { m_iId = obj_id; return CL_SUCCESS; }		
		cl_int      GetId() const { return m_iId; }
		HandleType* GetHandle()   { return &m_handle; }

		/******************************************************************************************
		* Function: 	SetLoggerClient    
		* Description:	set the logger client to the object
		* Arguments:	pLoggerClient - pointer to the logger client
		* Return value:	void
		* Author:		Uri Levy
		* Date:			July 2009
		******************************************************************************************/
		void SetLoggerClient(Intel::OpenCL::Utils::LoggerClient * pLoggerClient){ SET_LOGGER_CLIENT(pLoggerClient); }

	protected:
		
		/******************************************************************************************
		* Function: 	~OCLObject
		* Description:	The OCLObject class destructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/			
		virtual ~OCLObject();

		OCLObject(const OCLObject& O){}
		cl_int								m_iId;				// object id
		Intel::OpenCL::Utils::AtomicCounter	m_uiRefCount;		// reference count
		
		Intel::OpenCL::Utils::AtomicCounter	m_uiPendency;		// recall the number of dependent resources - will be 
											                    // used in order to ensure that current object is ready 
											                    // for deletion

		HandleType	m_handle;			// the OpenCL handle of the object

		DECLARE_LOGGER_CLIENT;



	};
#include "cl_object.hpp"

}}}
