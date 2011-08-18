// Copyright (c) 2006-2008 Intel Corporation
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

#include "command.h"

#include <vector>

using namespace std;

namespace Intel { namespace OpenCL { namespace MICDevice {

class BufferCommands : public Command
{

protected:

	// struct that define the copy info in order to calculate the offsets
	struct mem_copy_info_struct
	{
		cl_uint			uiDimCount;
		cl_char*		pHostPtr;
		size_t			vHostPitch[MAX_WORK_DIM-1];
		uint64_t		pCoiBuffOffset;
		size_t			vCoiBuffPitch[MAX_WORK_DIM-1];
		size_t			vRegion[MAX_WORK_DIM];
	}; 

	BufferCommands(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd);

	virtual ~BufferCommands();

	/* Get mem_copy_info_struct as input (The other arguments are out parameters) and calculate the following:
	       * The host pointers to read / write, from / to - will store in vHostPtr.
		   * The coiBuffer offsets to read / write, from / to - will store in vCoiBuffOffset.
		   * The size of each read / write operation - will store in vSize.
	   If the size of the output vectors is 1 than it is regular read / write, otherwise it is rectangular read / write. */
	void calculateCopyRegion(mem_copy_info_struct* pMemCopyInfo, vector<void*>* vHostPtr, vector<uint64_t>* vCoiBuffOffset, vector<uint64_t>* vSize);

	/* Get the estimated amount of copy operation to perform, in order to initialize the input vectors with this size.
	   In order to improve performance (avoid frequent memory allocations). */
	unsigned int getEstimatedCopyOperationsAmount(mem_copy_info_struct& memCopyInfo) 
	{ return memCopyInfo.uiDimCount == 1 ? 1 : ( memCopyInfo.uiDimCount == 2 ? memCopyInfo.vRegion[1] : memCopyInfo.vRegion[1] * memCopyInfo.vRegion[2] ); };

};

class ReadWriteMemObject : public BufferCommands
{

public:

	/* static function for ReadWriteMemObject Command creation */
    static cl_dev_err_code Create(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd, Command** pOutCommand);

	cl_dev_err_code execute();

private:

	/* Private constructor because We like to create Commands only by the factory method */
    ReadWriteMemObject(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd);

};

class CopyMemObject : public BufferCommands
{

public:

	/* static function for CopyMemObject Command creation */
    static cl_dev_err_code Create(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd, Command** pOutCommand);

	cl_dev_err_code execute();

private:

	/* Private constructor because We like to create Commands only by the factory method */
    CopyMemObject(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd);

};

class MapMemObject : public BufferCommands
{

public:

	/* static function for MapMemObject Command creation */
    static cl_dev_err_code Create(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd, Command** pOutCommand);

	cl_dev_err_code execute();

private:

	/* Private constructor because We like to create Commands only by the factory method */
    MapMemObject(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd);

};

class UnmapMemObject : public BufferCommands
{

public:

	/* static function for UnmapMemObject Command creation */
    static cl_dev_err_code Create(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd, Command** pOutCommand);

	cl_dev_err_code execute();

private:

	/* Private constructor because We like to create Commands only by the factory method */
    UnmapMemObject(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd);

};

}}}

