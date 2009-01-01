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
//  OCLObjectInfo.h
//  Implementation of the information object (Class OCLObjectInfo and OCLObjectInfoParam)
//  Created on:      10-Dec-2008 4:45:30 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(OCL_OBJECT_INFO_H_)
#define OCL_OBJECT_INFO_H_

#include "cl_types.h"
#include <map>
using namespace std;

namespace Intel { namespace OpenCL { namespace Framework {

	/**********************************************************************************************
	* Class name:	OCLObjectInfoParam
	*
	* Description:	represents the information prameter value within the information object.
	*				each param includes the parameter's name, the value and the size of the value
	*				(in bytes)
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/	
	class OCLObjectInfoParam
	{
	public:
		/******************************************************************************************
		* Function: 	OCLObjectInfoParam
		* Description:	The OCLObjectInfoParam class constructor
		* Arguments:	param_name [in]			parameter's name
		*				param_value_size [in]	parameter's value size (in bytes)
		*				param_value [in]		parameter's value
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/	
		OCLObjectInfoParam(cl_int param_name, size_t param_value_size, void * param_value);
		
		/******************************************************************************************
		* Function: 	OCLObjectInfoParam
		* Description:	The OCLObjectInfoParam class empty constructor
		* Arguments:	
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/	
		OCLObjectInfoParam();
		
		/******************************************************************************************
		* Function: 	~OCLObjectInfoParam
		* Description:	The OCLObjectInfoParam class destructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/		
		virtual ~OCLObjectInfoParam();

		/******************************************************************************************
		* Function: 	GetName    
		* Description:	Returns the name of the current parameter
		* Arguments:	
		* Return value:	[cl_int]	parameter's name 
		*				(-1)		in case of invalid object
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/	
		const cl_int	GetName( void );
		
		/******************************************************************************************
		* Function: 	GetSize    
		* Description:	Returns the size (in bytes) of the current parameter
		* Arguments:	
		* Return value:	[size_t]	parameter's value size 
		*				0		in case of invalid object
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/	
		const size_t	GetSize( void );

		/******************************************************************************************
		* Function: 	GetValue    
		* Description:	Returns the value of the current parameter
		* Arguments:	
		* Return value:	parameter's value 
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/	
		const void *	GetValue( void );

	private:
		cl_int	m_iParamName;	// parameter's name
		void *	m_pParamValue;	// parameter's value
		size_t	m_szParamSize;	// parameter's value size (in bytes)
	};

	/**********************************************************************************************
	* Class name:	OCLObjectInfo
	*
	* Description:	represent the data structure that contains all the required information of the
	*				OCLObject
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/	
	class OCLObjectInfo
	{
	
	public:
		/******************************************************************************************
		* Function: 	OCLObjectInfo
		* Description:	The OCLObjectInfo class constructor
		* Arguments:
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/	
		OCLObjectInfo();

		/******************************************************************************************
		* Function: 	~OCLObjectInfo
		* Description:	The OCLObjectInfo class destructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/	
		virtual ~OCLObjectInfo();

		/******************************************************************************************
		* Function: 	GetValue    
		* Description:	The functions returns the parameter object that holds the information data
		* Arguments:	param_name [in]		the name of the parameter
		*				ppParam [out]		pointer to the parameter object
		* Return value:	CL_SUCCESS			if the information parameters returned successfully
		*				CL_INVALID_VALUE	if the value doesn't exist in the information object
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/	
		cl_err_code GetParam(cl_int param_name, OCLObjectInfoParam ** ppParam);
		
		/******************************************************************************************
		* Function: 	SetParam    
		* Description:	The functions set the parameter object in the information object
		* Arguments:	param_name [in]		the name of the parameter
		*				ppParam [in]		pointer to the parameter object
		* Return value:	CL_SUCCESS		if the information parameters was set successfully in 
		*								the information object
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		cl_err_code SetParam(cl_int param_name, OCLObjectInfoParam * pParam);

		/******************************************************************************************
		* Function: 	SetString    
		* Description:	set an array of chars in the information object
		* Arguments:	param_name [in]		the name of the parameter
		*				length [in]			length of the string
		*				str[in]				char's array
		* Return value:	CL_SUCCESS		if the information parameters was set successfully in the 
		*								information object
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		cl_err_code SetString(cl_int param_name, const size_t length, const char str[]);

	private:
		// map of the information parameters
		map<cl_int, OCLObjectInfoParam*>	m_mapInfoParams;
	};

}}};
#endif // !defined(OCL_OBJECT_INFO_H_)
