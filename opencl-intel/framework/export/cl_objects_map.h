// INTEL CONFIDENTIAL
//
// Copyright 2006 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#pragma once

#ifndef _DEBUG
// #define _SECURE_SCL 0
#endif

#include "cl_object.h"
#include "cl_shared_ptr.h"
#include "cl_sys_defines.h"
#include "cl_types.h"
#include "ocl_object_base.h"
#include <atomic>
#include <map>
#include <mutex>

namespace Intel {
namespace OpenCL {
namespace Framework {

/*******************************************************************************
 * Class name:    OCLObjectsMap
 *
 * Description:    represents an OpneCL objects map
 ******************************************************************************/
template <class HandleType, class ParentHandleType = _cl_context_int>
class OCLObjectsMap : public OCLObjectBase {
protected:
  typedef typename std::map<HandleType *,
                            SharedPtr<OCLObject<HandleType, ParentHandleType>>>
      HandleTypeMap;
  typedef
      typename std::map<HandleType *,
                        SharedPtr<OCLObject<HandleType, ParentHandleType>>>::
          iterator HandleTypeMapIterator;
  typedef
      typename std::map<HandleType *,
                        SharedPtr<OCLObject<HandleType, ParentHandleType>>>::
          const_iterator HandleTypeMapConstIterator;

  // object's map
  HandleTypeMap m_mapObjects;
  static std::atomic<long> m_iNextGenKey;
  mutable std::mutex m_muMapMutex;
  bool m_bDisableAdding;
  bool m_bPreserveUserHandles;

public:
  /*****************************************************************************
   * Function:     OCLObjectsMap
   * Description:    The OCLObjectsMap class constructor
   * Arguments:
   ****************************************************************************/
  OCLObjectsMap()
      : OCLObjectBase("OCLObjectsMap"), m_bDisableAdding(false),
        m_bPreserveUserHandles(false) {}

  /*****************************************************************************
   * Function:     ~OCLObjectsMap
   * Description:    The OCLObjectsMap class destructor
   * Arguments:
   ****************************************************************************/
  virtual ~OCLObjectsMap();

  /*****************************************************************************
   * Function:     AddObject
   * Description: This function adds an object to a map and returns its handle.
   *              It is important to save this handle to query and remove the
   *              object. it's on the caller responsibility to allocates and
   *              destroy the object's resource. once an object was added to the
   *              map list, a new id was assign to it.
   * Arguments: pObject - pointer to the OpenCL object. must be a valid OpenCL
   *                      object.
   * Return value: [cl_int] - the handle of the object in the map list
   ****************************************************************************/
  HandleType *
  AddObject(const SharedPtr<OCLObject<HandleType, ParentHandleType>> &pObject);

  /*****************************************************************************
   * Function: AddObject
   * Description: This function adds an object to a map with assigned handle id.
   *              It is important to save this handle to query and remove the
   *              object. it's on the caller responsibility to allocates and
   *              destroy the object's resource.
   * Arguments: pObject - pointer to the OpenCL object. must be a valid OpenCL
   *                      object.
   *            bAssignId - if True the function assign the id to the object
   * Return value:
   ****************************************************************************/
  cl_err_code
  AddObject(const SharedPtr<OCLObject<HandleType, ParentHandleType>> &pObject,
            bool bAssignId);

  /*****************************************************************************
   * Function:     GetOCLObject
   * Description: returns the OpenCL object which assign to the object id
   * Arguments: hObjectHandle [in]    the handle of the OpenCL object
   * Return value:    the OpenCL object or NULL if it is found
   ****************************************************************************/
  SharedPtr<OCLObject<HandleType, ParentHandleType>>
  GetOCLObject(HandleType *hObjectHandle);

  /*****************************************************************************
   * Function:     GetOCLObjectPtr
   * Description: returns normal pointer to the OpenCL object which assign to
   *              the object id
   * Arguments: hObjectHandle [in] the handle of the OpenCL object
   * Return value: the OpenCL object or NULL if it is not found
   ****************************************************************************/
  OCLObject<HandleType, ParentHandleType> *
  GetOCLObjectPtr(HandleType *hObjectHandle);

  /*****************************************************************************
   * Function:     GetObjectByIndex
   * Description: returns the OpenCL object which assign to the index
   * Arguments:  uiIndex [in]    object's index
   * Return value: a SharedPtr pointing to the OpenCL object or NULL if the
   *               index number is too high
   ****************************************************************************/
  SharedPtr<OCLObject<HandleType, ParentHandleType>>
  GetObjectByIndex(cl_uint uiIndex);

  /*****************************************************************************
   * Function:     GetObjects
   * Description:    returns an array with all objects
   ****************************************************************************/
  void GetObjects(
      std::vector<SharedPtr<OCLObject<HandleType, ParentHandleType>>> &Objects);

  /*****************************************************************************
   * Function:     GetIDs
   * Description:    returns an array with all ids
   * Arguments:    uiIdsCount [in]
   *                ppIds [out]
   *                puiIdsCountRet [out]
   * Return value:    CL_SUCCESS -
   ****************************************************************************/
  cl_err_code GetIDs(cl_uint uiIdsCount, HandleType **pIds,
                     cl_uint *puiIdsCountRet);

  /*****************************************************************************
   * Function:     RemoveObject
   * Description: remove the OpenCL object which assign to the object id from
   *              the map
   * Arguments: hObjectHandle [in] the handle of the OpenCL object
   * Return value: CL_SUCCESS - the object was removed from the map
   *               CL_ERR_KEY_NOT_FOUND - the current object id wasn't found in
   *                                      the map
   ****************************************************************************/
  cl_err_code RemoveObject(HandleType *hObjectHandle);

  /*****************************************************************************
   * Function:     Count
   * Description:    get the number of items
   * Arguments:
   * Return value:    number of items
   ****************************************************************************/
  cl_uint Count() const;

  /*****************************************************************************
   * Function:     ForEach
   * Description:    Call a functor for each object in the map
   * Arguments: F - type of functor, which must have a method bool operator()
   *                (const SharedPtr<OCLObject<HandleType, ParentHandleType> >&
   *                obj). obj - the object on which to perform the operation
   *                returns whether to continue traversing the map
   * Return value:    whether all objects have been traversed
   ****************************************************************************/
  template <class F> bool ForEach(F &functor);

  /*****************************************************************************
   * Function:     ReleaseObject
   * Description: calls ->Release() on the given object and removes it from the
   *              map if applicable
   * Return Value: Whatever ->Release() returned or CL_ERR_KEY_NOT_FOUND
   ****************************************************************************/
  cl_err_code ReleaseObject(HandleType *hObject);

  /*****************************************************************************
   * Function:     ReleaseAllObjects
   * Description: calls ->Release() on all contained objects, then clears the
   *              map. If bPreserveHandles==true set PreserveHandles flag in
   *              objects before deletion
   ****************************************************************************/
  void ReleaseAllObjects(bool bTerminate);

  /*****************************************************************************
   * Function:     Clear
   * Description: clear map list from all objects - this function remove the
   *              items from the objects map list only! it's not deleting the
   *              OpenCL objects the function call to the Garbage Collector as
   *              well.
   * Arguments:
   * Return value:
   ****************************************************************************/
  void Clear();

  /*****************************************************************************
   * Function:     DisableAdding
   * Description:    disable AddObject method
   * Arguments:
   * Return value:
   ****************************************************************************/
  void DisableAdding();
  void EnableAdding();
  void SetPreserveUserHandles() { m_bPreserveUserHandles = true; }

  // check if current object id exists in map list
  bool IsExists(HandleType *hObjectHandle);
};
#include "cl_objects_map.hpp"
} // namespace Framework
} // namespace OpenCL
} // namespace Intel
