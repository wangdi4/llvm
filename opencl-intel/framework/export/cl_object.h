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
#include "cl_shared_ptr.h"
#include "cl_framework.h"

namespace Intel { namespace OpenCL { namespace Framework {

    using Intel::OpenCL::Utils::SmartPtr;
    using Intel::OpenCL::Utils::SharedPtr;
    using Intel::OpenCL::Utils::ConstSharedPtr;

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
	template <class HandleType, class ParentHandleType = _cl_context_int> 
    class OCLObject : virtual public Intel::OpenCL::Utils::ReferenceCountedObject
	{
	public:

        friend class Intel::OpenCL::Utils::SharedPtr<OCLObject<HandleType, ParentHandleType> >;
        friend class Intel::OpenCL::Utils::SharedPtr<const OCLObject<HandleType, ParentHandleType> >;
        SharedPtr<OCLObject<HandleType, ParentHandleType> > operator&() { return SharedPtr<OCLObject<HandleType, ParentHandleType> >(this); }
        ConstSharedPtr<OCLObject<HandleType, ParentHandleType> > operator&() const { return ConstSharedPtr<OCLObject<HandleType, ParentHandleType> >(this); }

		/******************************************************************************************
		* Function: 	OCLObject
		* Description:	The OCLObject class constructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/		
        OCLObject(ParentHandleType* context, const std::string& typeName);

        /******************************************************************************************
        * Function: 	NotifyInvisible    
        * Description:	Called when the reference count for the object is decremented to zero,
        *               indicating the object is no longer user-visible
        * Arguments:	
        * Author:		Doron Singer
        * 
        ******************************************************************************************/	
        virtual void NotifyInvisible() {}

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
		virtual cl_err_code	GetInfo(cl_int iParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet) const = 0;

        cl_err_code SetId(cl_int obj_id) { m_iId = obj_id; return CL_SUCCESS; }		
		cl_int      GetId() const { return m_iId; }
        HandleType* GetHandle() { return &m_handle; }
		const HandleType* GetHandle() const  { return &m_handle; }        

		// Get the context to which object belongs
		ParentHandleType* GetParentHandle() const {return m_pParentHandle;}

		
		/******************************************************************************************
		* Function: 	SetLoggerClient    
		* Description:	set the logger client to the object
		* Arguments:	pLoggerClient - pointer to the logger client
		* Return value:	void
		* Author:		Uri Levy
		* Date:			July 2009
		******************************************************************************************/
		void SetLoggerClient(Intel::OpenCL::Utils::LoggerClient * pLoggerClient){ SET_LOGGER_CLIENT(pLoggerClient); }        

		/******************************************************************************************
		* Function: 	SetTerminate    
		* Description:	set the terminate flag (process shutdown)
		* Arguments:	bTerminate - is true if process terminate sequence is running
		* Return value:	void
		* Author:		Uri Levy
		* Date:			July 2009
		******************************************************************************************/
		void SetTerminate(bool bTerminate) { m_bTerminate = bTerminate;}

        std::string GetTypeName() const { return m_typename; }

    protected:        

        /******************************************************************************************
		* Function: 	~OCLObject
		* Description:	The OCLObject class destructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/			
		virtual ~OCLObject() {}
		OCLObject(const OCLObject& O){}


		HandleType	m_handle;									// the OpenCL handle of the object        
		cl_int								m_iId;				// object id
        Intel::OpenCL::Utils::AtomicCounter	m_uiRefCount;		// reference count

		ParentHandleType*					m_pParentHandle;	// the OpenCL handle of the parent the object belongs to

		bool		m_bTerminate;        

        const std::string m_typename;

        DECLARE_LOGGER_CLIENT;

	};
#include "cl_object.hpp"

}}}
