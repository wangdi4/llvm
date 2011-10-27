/*****************************************************************************\

Copyright (c) Intel Corporation (2010).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  DynamicLib.h

\*****************************************************************************/

#pragma once

namespace Intel { namespace OpenCL { namespace DeviceBackend { namespace Utils {


class DynamicLib
{
public:
	DynamicLib(void);
	~DynamicLib(void);

	// Loads a dynamically link library into process address space
	// Input
	//		pLibName	- A pointer to null tirminated string that describes library file name
	// Returns
	//		true - if succesully loaded
	//		false - if file doesn't exists or other error has occured
	bool Load(const char* pLibName);

	// Release all allocated resourses and unloads the library
	void Close();

	// Returns the pointer to exported function within a loaded module
	void* GetFuncPtr(const char* funcName);

private:
	void* m_hLibrary;		// A handle to loaded library

};

}}}}
