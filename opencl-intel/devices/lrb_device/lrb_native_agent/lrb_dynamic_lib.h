/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2009 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related 
// to the source code ("Material") are owned by Intel Corporation or its 
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and 
// proprietary and confidential information of Intel Corporation and its 
// suppliers and licensors, and is protected by worldwide copyright and trade 
// secret laws and treaty provisions. No part of the Material may be used, copied, 
// reproduced, modified, published, uploaded, posted, transmitted, distributed, 
// or disclosed in any way without Intel’s prior express written permission. 
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery 
// of the Materials, either expressly, by implication, inducement, estoppel or 
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice 
// or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors 
// in any way.
/////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
//  lrb_dynamic_lib.h
//  Created on:      Aug-2009
//  Original author: Peleg, Arnon
//  Description:
//          This header file expose API to manage load of dynamic lib on LRB native
//          The user is expected to use the header whether the code runs on NetSim/XenSim or
//          the HW itself. it is this header implementation to deal with that.
//          TODO: Add support to FreeBSD format (dlopen)
///////////////////////////////////////////////////////////
#if !defined(__LRB_DYNAMIC_LIB_H__)
#define __LRB_DYNAMIC_LIB_H__

/************************************************************************
 * Loads a dynamically link library into process address space
 * Input
 *	 pLibName - A pointer to null terminated string that describes library file name.
 *              The file name includes the full path, but do not include the extension.
 *              The extension defined by the platform that this call is running on
 * Returns handle to the loaded library. On error a NULL is returned
 ************************************************************************/ 
void* lrb_dynmic_load_library(const char* pLibName);

/************************************************************************
 * close and free all library resources
 ************************************************************************/
void lrb_dynmic_close_library( void* hLib);

/************************************************************************
 * Returns the address of the exported function pFuncName in the library hLib.
 * The return values should be casted to the function pointer.
 * If function do not exist a NULL is returned.
 ************************************************************************/
void* lrb_dynmic_get_function( void* hLib, char* pFuncName);

/************************************************************************
 * Returns the address of the exported variable pVarName in the library hLib.
 * The return values should be casted to exported variable type.
 * If variable do not exist a NULL is returned.
 ************************************************************************/
void* lrb_dynmic_get_variable( void* hLib, char* pVarName);

/************************************************************************
 * Returns pointer to the file name without the leading path
 * the
 ************************************************************************/
const char* lrb_dynmic_trim_file_path( const char* pFileName );


#endif  // !defined(__LRB_DYNAMIC_LIB_H__)
