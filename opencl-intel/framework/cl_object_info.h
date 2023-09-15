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

#include "cl_types.h"
#include <map>

namespace Intel {
namespace OpenCL {
namespace Framework {

/*******************************************************************************
 * Class name: OCLObjectInfoParam
 *
 * Description: represents the information prameter value within the
 *              information object. each param includes the parameter's name,
 *              the value and the size of the value (in bytes)
 ******************************************************************************/
class OCLObjectInfoParam {
public:
  /*****************************************************************************
   * Function: OCLObjectInfoParam
   * Description: The OCLObjectInfoParam class constructor
   * Arguments: param_name [in]       parameter's name
   *            param_value_size [in] parameter's value size (in bytes)
   *            param_value [in]      parameter's value
   ****************************************************************************/
  OCLObjectInfoParam(cl_int param_name, size_t param_value_size,
                     void *param_value);

  /*****************************************************************************
   * Function: OCLObjectInfoParam
   * Description: The OCLObjectInfoParam class empty constructor
   * Arguments:
   ****************************************************************************/
  OCLObjectInfoParam();

  /*****************************************************************************
   * Function: ~OCLObjectInfoParam
   * Description: The OCLObjectInfoParam class destructor
   * Arguments:
   ****************************************************************************/
  virtual ~OCLObjectInfoParam();

  /*****************************************************************************
   * Function: GetName
   * Description: Returns the name of the current parameter
   * Arguments:
   * Return value: [cl_int]  parameter's name
   *               (-1)    in case of invalid object
   ****************************************************************************/
  cl_int GetName(void);

  /*****************************************************************************
   * Function:   GetSize
   * Description:  Returns the size (in bytes) of the current parameter
   * Arguments:
   * Return value:  [size_t]  parameter's value size
   *                0    in case of invalid object
   ****************************************************************************/
  size_t GetSize(void);

  /*****************************************************************************
   * Function:   GetValue
   * Description:  Returns the value of the current parameter
   * Arguments:
   * Return value:  parameter's value
   ****************************************************************************/
  const void *GetValue(void);

private:
  cl_int m_iParamName;  // parameter's name
  void *m_pParamValue;  // parameter's value
  size_t m_szParamSize; // parameter's value size (in bytes)
};

/*******************************************************************************
 * Class name:  OCLObjectInfo
 *
 * Description: represent the data structure that contains all the required
 *              information of the OCLObject
 ******************************************************************************/
class OCLObjectInfo {

public:
  /*****************************************************************************
   * Function:   OCLObjectInfo
   * Description: The OCLObjectInfo class constructor
   * Arguments:
   ****************************************************************************/
  OCLObjectInfo();

  /*****************************************************************************
   * Function:   ~OCLObjectInfo
   * Description:  The OCLObjectInfo class destructor
   * Arguments:
   ****************************************************************************/
  virtual ~OCLObjectInfo();

  /*****************************************************************************
   * Function:   GetValue
   * Description: The functions returns the parameter object that holds the
   *              information data
   * Arguments: param_name [in]  the name of the parameter
   *            ppParam [out]    pointer to the parameter object
   * Return value: CL_SUCCESS if the information parameters returned
   *                          successfully
   *               CL_INVALID_VALUE  if the value doesn't exist in the
   *                                 information object
   ****************************************************************************/
  cl_err_code GetParam(cl_int param_name, OCLObjectInfoParam **ppParam);

  /*****************************************************************************
   * Function:   SetParam
   * Description:  The functions set the parameter object in the information
   *               object
   * Arguments: param_name [in] the name of the parameter
   *            ppParam [in] pointer to the parameter object
   * Return value: CL_SUCCESS if the information parameters was set successfully
   *                          in the information object
   ****************************************************************************/
  cl_err_code SetParam(cl_int param_name, OCLObjectInfoParam *pParam);

  /*****************************************************************************
   * Function:   SetString
   * Description: set an array of chars in the information object
   * Arguments: param_name [in] the name of the parameter
   *            length [in] length of the string
   *            str[in] char's array
   * Return value: CL_SUCCESS if the information parameters was set successfully
   *                          in the information object
   ****************************************************************************/
  cl_err_code SetString(cl_int param_name, const size_t length,
                        const char str[]);

private:
  // map of the information parameters
  std::map<cl_int, OCLObjectInfoParam *> m_mapInfoParams;
};

} // namespace Framework
} // namespace OpenCL
} // namespace Intel
