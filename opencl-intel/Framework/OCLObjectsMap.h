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
//  OCLObjectsMap.h
//  Implementation of the Class OCLObjectsMap
//  Created on:      10-Dec-2008 4:45:30 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(OCL_OBJECTS_MAP_H_)
#define OCL_OBJECTS_MAP_H_

#include "cl_framework.h"
#include "OCLObject.h"
#include <map>
using namespace std;

namespace Intel { namespace OpenCL { namespace Framework {

	/**********************************************************************************************
	* Class name:	OCLObjectsMap
	*
	* Description:	represnts an OpneCL objects map
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/
	class OCLObjectsMap
	{
	protected:
		// object's map
		map<cl_int, OCLObject*>		m_mapObjects;
		cl_int						m_iMaxKey;

	public:

		/******************************************************************************************
		* Function: 	OCLObjectsMap
		* Description:	The OCLObjectsMap class constructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/		
		OCLObjectsMap();
		
		/******************************************************************************************
		* Function: 	~OCLObjectsMap
		* Description:	The OCLObjectsMap class destructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/			
		virtual ~OCLObjectsMap();

		/******************************************************************************************
		* Function: 	AddObject    
		* Description:	This function adds an object to a map and returns its handle. It is 
		*				important to save this handle to query and remove the object.
		*				it's on the caller responsibility to allocates and destroy the object's 
		*				resource. once an object was added to the map list, a new id was assign to 
		*				it.
		* Arguments:	pObject - pointer to the OpenCL object. must be a valid OpenCL object.
		* Return value:	[cl_int] - the handle of the object in the map list
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/	
		cl_int AddObject(OCLObject * pObject);

		/******************************************************************************************
		* Function: 	GetObject    
		* Description:	returns the OpenCL object which assign to the object id
		* Arguments:	iObjectId [in]	the id of the OpenCL object
		*				pObject	[out]	pointer to the OpenCL object's opinter. must be a valid 
		*								pointer
		* Return value:	CL_SUCCESS -			the object was found and returned
		*				CL_ERR_KEY_NOT_FOUND -	the current object id wasn't found in the map
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/	
		cl_err_code GetObject(cl_int iObjectId, OCLObject ** ppObject);


		/******************************************************************************************
		* Function: 	RemoveObject    
		* Description:	remove the OpenCL object which assign to the object id from the map
		* Arguments:	iObjectId [in]	the id of the OpenCL object
		* Return value:	CL_SUCCESS -			the object was removed from the map
		*				CL_ERR_KEY_NOT_FOUND -	the current object id wasn't found in the map
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/	
		cl_err_code RemoveObject(cl_int iObjectId);

	};

}}};

#endif //OCL_OBJECTS_MAP_H_