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

#ifndef _DEBUG
//#define _SECURE_SCL 0
#endif

#include "cl_types.h"
#include "cl_object.h"
#include "cl_synch_objects.h"
#include <map>

namespace Intel { namespace OpenCL { namespace Framework {

	/**********************************************************************************************
	* Class name:	OCLObjectsMap
	*
	* Description:	represents an OpneCL objects map
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/
	template <class HandleType>
	class OCLObjectsMap
	{
	protected:
		// object's map
		std::map<HandleType*, OCLObject<HandleType>*>		m_mapObjects;
		// dirty object's map
		std::map<HandleType*, OCLObject<HandleType>*>		m_mapDirtyObjects;
		static Intel::OpenCL::Utils::AtomicCounter		    m_iNextGenKey;
		Intel::OpenCL::Utils::OclSpinMutex				    m_muMapMutex;

	public:

		/******************************************************************************************
		* Function: 	OCLObjectsMap
		* Description:	The OCLObjectsMap class constructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/		
		OCLObjectsMap() {}
		
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
		HandleType* AddObject(OCLObject<HandleType> * pObject);

		/******************************************************************************************
		* Function: 	AddObject    
		* Description:	This function adds an object to a map with assigned handle id. It is 
		*				important to save this handle to query and remove the object.
		*				it's on the caller responsibility to allocates and destroy the object's 
		*				resource. 
		* Arguments:	pObject		- pointer to the OpenCL object. must be a valid OpenCL object.
		*				bAssignId	- if True the function assign the id to the object
		* Return value:	
		* Author:		Uri Levy
		* Date:			January 2008
		******************************************************************************************/	
		cl_err_code AddObject(OCLObject<HandleType> * pObject, bool bAssignId);

		/******************************************************************************************
		* Function: 	GetOCLObject    
		* Description:	returns the OpenCL object which assign to the object id
		* Arguments:	hObjectHandle [in]	the handle of the OpenCL object
		*				pObject	[out]	pointer to the OpenCL object's pointer. must be a valid 
		*								pointer
		* Return value:	CL_SUCCESS -			the object was found and returned
		*				CL_ERR_KEY_NOT_FOUND -	the current object id wasn't found in the map
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/	
		cl_err_code GetOCLObject(HandleType* hObjectHandle, OCLObject<HandleType> ** ppObject);

		/******************************************************************************************
		* Function: 	GetObjectByIndex    
		* Description:	returns the OpenCL object which assign to the index
		* Arguments:	uiIndex [in]	object's index
		*				pObject	[out]	pointer to the OpenCL object's pointer. must be a valid 
		*								pointer
		* Return value:	CL_SUCCESS -			the object was found and returned
		*				CL_ERR_LIST_EMPTY -		there are no objects left in the map
		*				CL_ERR_KEY_NOT_FOUND -	the index number is too high
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/	
		cl_err_code GetObjectByIndex(cl_uint uiIndex, OCLObject<HandleType> ** ppObject);

		/******************************************************************************************
		* Function: 	GetObjects    
		* Description:	returns an array with all objects
		* Arguments:	uiObjectCount [in]	
		*				ppObjects [out]				
		*				puiObjectCountRet [out]	
		* Return value:	CL_SUCCESS -
		* Author:		Uri Levy
		* Date:			January 2008
		******************************************************************************************/	
		cl_err_code GetObjects(cl_uint uiObjectCount, OCLObject<HandleType> ** ppObjects, cl_uint * puiObjectCountRet);

		/******************************************************************************************
		* Function: 	GetIDs    
		* Description:	returns an array with all ids
		* Arguments:	uiIdsCount [in]	
		*				ppIds [out]				
		*				puiIdsCountRet [out]	
		* Return value:	CL_SUCCESS -
		* Author:		Uri Levy
		* Date:			January 2008
		******************************************************************************************/	
		cl_err_code GetIDs(cl_uint uiIdsCount, HandleType** pIds, cl_uint * puiIdsCountRet);

		/******************************************************************************************
		* Function: 	RemoveObject    
		* Description:	remove the OpenCL object which assign to the object id from the map and
		*				returns the object if necessary
		*				if bSetDirty flag is true, the object is not entirely removed from the
		*				objects map but set to dirty object. Once the garbage collector is
		*				activated, the object will be removed if it ready for deletion
		* Arguments:	hObjectHandle [in]		the handle of the OpenCL object
		*				ppObjectRet [out]	return the object being removed. if ppObjectRet is
		*									NULL it is ignored
		* Return value:	CL_SUCCESS -			the object was removed from the map
		*				CL_ERR_KEY_NOT_FOUND -	the current object id wasn't found in the map
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/	
		cl_err_code RemoveObject(HandleType* hObjectHandle, OCLObject<HandleType> ** ppObjectRet, bool bSetDirty = false);
		
		/******************************************************************************************
		* Function: 	Count    
		* Description:	get the number of items
		* Arguments:	
		* Return value:	number of items
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/	
		cl_uint	Count();

		/******************************************************************************************
		* Function: 	Clear    
		* Description:	clear map list from all objects - this function remove the items from the
		*				objects map list only! it's not deleting the OpenCL objects
		*				the function call to the Garbage Collector as well.
		* Arguments:	
		* Return value:	
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/	
		void Clear(bool bSetDirty = false);

		// check if current object id exists in map list
		bool IsExists(HandleType* hObjectHandle);

		void GarbageCollector( bool bIsTerminate = false);

	};
#include "cl_objects_map.hpp"
}}};

#endif //OCL_OBJECTS_MAP_H_