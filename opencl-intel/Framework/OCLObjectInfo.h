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
///////////////////////////////////////////////////////////
//  OCLObjectInfo.h
//  Implementation of the Class OCLObjectInfo
//  Created on:      10-Dec-2008 4:45:30 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////

#if !defined(OCL_OBJECT_INFO_H_)
#define OCL_OBJECT_INFO_H_

#include "cl_framework.h"
#include <map>
using namespace std;

namespace Intel { namespace OpenCL { namespace Framework {

	/**
	* OCLObjectInfoParam class
	* represents the information prameter value
	*/
	class OCLObjectInfoParam
	{
	public:
		OCLObjectInfoParam(cl_int param_name, size_t param_value_size, void * param_value);
		OCLObjectInfoParam();
		virtual ~OCLObjectInfoParam();

		/**
		* GetName
		* Returns the name of the current parameter
		* Return values:
		* [cl_int] - parameter's name 
		* -1 - in case of invalid object
		*/
		const cl_int	GetName( void );
		
		/**
		* GetSize
		* Returns the size (in bytes) of the current parameter
		* Return values:
		* [size_t] - parameter's name 
		* 0 - in case of invalid object
		*/
		const size_t	GetSize( void );

		/**
		* GetValue
		* Returns the value of the current parameter
		* Return values:
		* [void*] - parameter's value
		*/
		const void *	GetValue( void );

	private:
		cl_int	m_iParamName;	// parameter's name
		void *	m_pParamValue;	// parameter's value
		size_t	m_szParamSize;	// parameter's value size (in bytes)
	};

	/**
	* OCLObjectInfo class
	* represent the data structure that contains all the required information of the
	* OCLObject
	*/
	class OCLObjectInfo
	{
	
	public:
		OCLObjectInfo();
		virtual ~OCLObjectInfo();

		/**
		* GetParam
		* The functions returns the parameter object that holds the information data
		* Return values:
		* CL_SUCCESS - if the information parameters returned successfully
		* CL_INVALID_VALUE - if the value doesn't exist in the information object
		*/
		cl_err_code GetParam(cl_int param_name, OCLObjectInfoParam ** ppParam);
		
		/**
		* SetParam
		* The functions set the parameter object in the information object
		* Return values:
		* CL_SUCCESS - if the information parameters was set successfully in the information object
		*/
		cl_err_code SetParam(cl_int param_name, OCLObjectInfoParam * pParam);

		/**
		* SetString
		* set an array of chars in the information object
		* Return values:
		* CL_SUCCESS - if the information parameters was set successfully in the information object
		*/
		cl_err_code SetString(cl_int param_name, const size_t length, const char str[]);

	private:
		map<cl_int, OCLObjectInfoParam*>	m_mapInfoParams;
	};

}}};
#endif // !defined(OCL_OBJECT_INFO_H_)
