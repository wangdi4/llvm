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
//  OCLObjectsMap.h
//  Implementation of the Class OCLObjectsMap
//  Created on:      10-Dec-2008 4:45:30 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _DEBUG
//#define _SECURE_SCL 0
#endif

#include "cl_types.h"
#include "cl_object.h"
#include "cl_synch_objects.h"
#include "cl_sys_defines.h"
#include <map>
#include "ocl_object_base.h"
#include "cl_shared_ptr.h"

namespace Intel { namespace OpenCL { namespace Framework {

    /**********************************************************************************************
    * Class name:    OCLObjectsMap
    *
    * Description:    represents an OpneCL objects map
    * Author:        Uri Levy
    * Date:            December 2008
    **********************************************************************************************/
    template <class HandleType, class ParentHandleType = _cl_context_int>
    class OCLObjectsMap : public OCLObjectBase
    {
    protected:

        typedef typename std::map<HandleType*, SharedPtr<OCLObject<HandleType, ParentHandleType> > >                 HandleTypeMap;
        typedef typename std::map<HandleType*, SharedPtr<OCLObject<HandleType, ParentHandleType> > >::iterator       HandleTypeMapIterator;
        typedef typename std::map<HandleType*, SharedPtr<OCLObject<HandleType, ParentHandleType> > >::const_iterator HandleTypeMapConstIterator;

        // object's map
        HandleTypeMap                                          m_mapObjects;
        static Intel::OpenCL::Utils::AtomicCounter             m_iNextGenKey;
        mutable Intel::OpenCL::Utils::OclNonReentrantSpinMutex m_muMapMutex;
        bool                                                   m_bDisableAdding;
        bool                                                   m_bPreserveUserHandles;

    public:

        /******************************************************************************************
        * Function:     OCLObjectsMap
        * Description:    The OCLObjectsMap class constructor
        * Arguments:        
        * Author:        Uri Levy
        * Date:            December 2008
        ******************************************************************************************/        
        OCLObjectsMap() : OCLObjectBase("OCLObjectsMap"), m_bDisableAdding(false), m_bPreserveUserHandles(false) {}
        
        /******************************************************************************************
        * Function:     ~OCLObjectsMap
        * Description:    The OCLObjectsMap class destructor
        * Arguments:        
        * Author:        Uri Levy
        * Date:            December 2008
        ******************************************************************************************/            
        virtual ~OCLObjectsMap();

        /******************************************************************************************
        * Function:     AddObject    
        * Description:    This function adds an object to a map and returns its handle. It is 
        *                important to save this handle to query and remove the object.
        *                it's on the caller responsibility to allocates and destroy the object's 
        *                resource. once an object was added to the map list, a new id was assign to 
        *                it.
        * Arguments:    pObject - pointer to the OpenCL object. must be a valid OpenCL object.
        * Return value:    [cl_int] - the handle of the object in the map list
        * Author:        Uri Levy
        * Date:            December 2008
        ******************************************************************************************/    
        HandleType* AddObject(const SharedPtr<OCLObject<HandleType, ParentHandleType> >& pObject);

        /******************************************************************************************
        * Function:     AddObject    
        * Description:    This function adds an object to a map with assigned handle id. It is 
        *                important to save this handle to query and remove the object.
        *                it's on the caller responsibility to allocates and destroy the object's 
        *                resource. 
        * Arguments:    pObject        - pointer to the OpenCL object. must be a valid OpenCL object.
        *                bAssignId    - if True the function assign the id to the object
        * Return value:    
        * Author:        Uri Levy
        * Date:            January 2008
        ******************************************************************************************/    
        cl_err_code AddObject(const SharedPtr<OCLObject<HandleType, ParentHandleType> >& pObject, bool bAssignId);

        /******************************************************************************************
        * Function:     GetOCLObject    
        * Description:    returns the OpenCL object which assign to the object id
        * Arguments:    hObjectHandle [in]    the handle of the OpenCL object
        * Return value:    the OpenCL object or NULL if it is found
        * Author:        Uri Levy
        * Date:            December 2008
        ******************************************************************************************/    
        SharedPtr<OCLObject<HandleType, ParentHandleType> > GetOCLObject(HandleType* hObjectHandle);

        /******************************************************************************************
        * Function:     GetOCLObjectPtr
        * Description:  returns normal pointer to the OpenCL object which assign to the object id
        * Arguments:    hObjectHandle [in]    the handle of the OpenCL object
        * Return value: the OpenCL object or NULL if it is not found
        * Author:       Nael Meraey
        * Date:         October 2013
        ******************************************************************************************/    
        OCLObject<HandleType, ParentHandleType>* GetOCLObjectPtr(HandleType* hObjectHandle);

        /******************************************************************************************
        * Function:     GetObjectByIndex    
        * Description:    returns the OpenCL object which assign to the index
        * Arguments:    uiIndex [in]    object's index
        * Return value:    a SharedPtr pointing to the OpenCL object or NULL if the index number is
        *               too high
        * Author:        Uri Levy
        * Date:            December 2008
        ******************************************************************************************/    
        SharedPtr<OCLObject<HandleType, ParentHandleType> > GetObjectByIndex(cl_uint uiIndex);

        /******************************************************************************************
        * Function:     GetObjects    
        * Description:    returns an array with all objects
        * Arguments:    uiObjectCount [in]    
        *                ppObjects [out]                
        *                puiObjectCountRet [out]    
        * Return value:    CL_SUCCESS -
        * Author:        Uri Levy
        * Date:            January 2008
        ******************************************************************************************/    
        cl_err_code GetObjects(cl_uint uiObjectCount, SharedPtr<OCLObject<HandleType, ParentHandleType> >* ppObjects, cl_uint * puiObjectCountRet);

        /******************************************************************************************
        * Function:     GetIDs    
        * Description:    returns an array with all ids
        * Arguments:    uiIdsCount [in]    
        *                ppIds [out]                
        *                puiIdsCountRet [out]    
        * Return value:    CL_SUCCESS -
        * Author:        Uri Levy
        * Date:            January 2008
        ******************************************************************************************/    
        cl_err_code GetIDs(cl_uint uiIdsCount, HandleType** pIds, cl_uint * puiIdsCountRet);

        /******************************************************************************************
        * Function:     RemoveObject    
        * Description:    remove the OpenCL object which assign to the object id from the map 
        * Arguments:    hObjectHandle [in]        the handle of the OpenCL object
        * Return value:    CL_SUCCESS -            the object was removed from the map
        *                CL_ERR_KEY_NOT_FOUND -    the current object id wasn't found in the map
        * Author:        Doron Singer
        * Date:            July 2010
        ******************************************************************************************/    
        cl_err_code RemoveObject(HandleType* hObjectHandle);
        
        /******************************************************************************************
        * Function:     Count    
        * Description:    get the number of items
        * Arguments:    
        * Return value:    number of items
        * Author:        Uri Levy
        * Date:            December 2008
        ******************************************************************************************/    
        cl_uint    Count() const;

        /******************************************************************************************
        * Function:     ForEach
        * Description:    Call a functor for each object in the map
        * Arguments:    F - type of functor, which must have a method bool operator()(const SharedPtr<OCLObject<HandleType, ParentHandleType> >& obj).
        *                     obj - the object on which to perform the operation
        *                     returns whether to continue traversing the map
        * Return value:    whether all objects have been traversed
        * Author:        Aharon Abramson
        * Date:            May 2013
        ******************************************************************************************/
        template<class F>
        bool ForEach(F& functor);

        /******************************************************************************************
        * Function:     ReleaseObject    
        * Description:    calls ->Release() on the given object and removes it from the map if applicable
        * Return Value: Whatever ->Release() returned or CL_ERR_KEY_NOT_FOUND
        * Author:        Doron Singer
        * Date:            July 2010
        ******************************************************************************************/    
        cl_err_code ReleaseObject(HandleType* hObject);

        /******************************************************************************************
        * Function:     ReleaseAllObjects    
        * Description:    calls ->Release() on all contained objects, then clears the map.
        *               If bPreserveHandles==true set PreserveHandles flag in objects before deletion
        * Author:        Doron Singer
        * Date:            July 2010
        ******************************************************************************************/    
        void ReleaseAllObjects(bool bTerminate);

        /******************************************************************************************
        * Function:     Clear    
        * Description:    clear map list from all objects - this function remove the items from the
        *                objects map list only! it's not deleting the OpenCL objects
        *                the function call to the Garbage Collector as well.
        * Arguments:    
        * Return value:    
        * Author:        Uri Levy
        * Date:            December 2008
        ******************************************************************************************/    
        void Clear();

        /******************************************************************************************
        * Function:     DisableAdding    
        * Description:    disable AddObject method
        * Arguments:    
        * Return value:    
        * Author:        Dmitry Kaptsenel
        * Date:            May 2013
        ******************************************************************************************/    
        void DisableAdding();
        void EnableAdding();
        void SetPreserveUserHandles() { m_bPreserveUserHandles = true; }

        // check if current object id exists in map list
        bool IsExists(HandleType* hObjectHandle);

    };
#include "cl_objects_map.hpp"
}}}
