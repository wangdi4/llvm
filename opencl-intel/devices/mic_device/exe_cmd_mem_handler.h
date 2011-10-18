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

#include "cl_device_api.h"
#include "mic_device_interface.h"

#include <source/COIBuffer_source.h>
#include <source/COIPipeline_source.h>

#include <vector>

using namespace std;

namespace Intel { namespace OpenCL { namespace MICDevice {

template <class T>
class RunFuncData;

/* DispatcherDataHandler is handler class which manage the use of dispatcher_data structure on different situations.
(When the dispatcher_data size is smaller / larger than COI_PIPELINE_MAX_IN_MISC_DATA_LEN) */
class DispatcherDataHandler
{

public:

	struct data
	{
		// reperesent the COIBUFFER that store the data (If needed).
		COIBUFFER coiBuffer;
		// host pointer that store the data.
		char* pDataBuffer;
	};

	DispatcherDataHandler();

	virtual ~DispatcherDataHandler();

	/* Initialize the Dispatcher data. */
	cl_dev_err_code init(dispatcher_data& dispatcherData, const directive_pack* preExeDirectives, const directive_pack* postExeDirectives, const char* pKernelParams, COIPROCESS* pCoiProcess);

	/* Register the buffer in "coiBuffsArr" and the access flag in "coiBuffsAccessFlag". (If needed - in case of COIBUFFER only, when dispatcher data size is greater than COI_PIPELINE_MAX_IN_MISC_DATA_LEN) */
	void registerDispatcherData(vector<COIBUFFER>& coiBuffsArr, vector<COI_ACCESS_FLAGS>& coiBuffsAccessFlag);

	/* return pointer to the memory that We like to dispatch to MIC device (a pointer which will be send by in_misc COIPipelineRunFunction). (if needed) */
	void* getDispatcherDataPtrForCoiRunFunc();
	
	/* return the size of dispatcher_data (size of return value for COIPipelineRunFunction). (if needed) */
	uint16_t getDispatcherDataSizeForCoiRunFunc();

	/* Release the memory used by this class. */
	void release();

private:

	data m_data;

	// m_size is only for optimization.
	size_t m_size;

	// Hold the appropriate DispatcherData object (RunFuncDataUsingMisc or RunFuncDataUsingBuffer).
	RunFuncData<data>* m_pDispatcherData;

	// Static array which contains at the first location pointer to RunFuncDataUsingBuffer singleton object and in the second location pointer to RunFuncDataUsingMisc singleton object.
	// This array is just for optimization in order to avoid "if" statement.
	static RunFuncData<data>* m_dispatcherDataOptionsArr[2];
};



class MiscDataHandler
{

public:

	struct data
	{
		// reperesent the COIBUFFER that store the data (If needed).
		COIBUFFER coiBuffer;
		// host pointer that store the data.
		misc_data* pDataBuffer;
	};

	MiscDataHandler();

	virtual ~MiscDataHandler();

	/* Initialize the MiscData object. (If needed) */
	cl_dev_err_code init(bool useCoiBuffer, COIPROCESS* pCoiProcess);

	/* Register the buffer in "coiBuffsArr" and the access flag in "coiBuffsAccessFlag". (If needed - in case of COIBUFFER only) */
	void registerMiscData(vector<COIBUFFER>& coiBuffsArr, vector<COI_ACCESS_FLAGS>& coiBuffsAccessFlag);

	/* return pointer to misc_data (to be a pointer for return value of COIPipelineRunFunction). (if needed) */
	void* getMiscDataPtrForCoiRunFunc();
	
	/* return the size of misc_data (size of return value for COIPipelineRunFunction). (if needed) */
	uint16_t getMiscDataSizeForCoiRunFunc();

	/* Read the data and store it in 'pOutMiscData'. */
	cl_dev_err_code readMiscData(misc_data* pOutMiscData);

	/* Release the COIBUFFER object. (if needed) */
	void release();

private:

    data m_data;

	// The misc_data.
	misc_data m_miscData;

	// Hold the appropriate MiscData object (RunFuncDataUsingMisc or RunFuncDataUsingBuffer).
	RunFuncData<data>* m_pMiscData;

	// Static array which contains at the first location pointer to RunFuncDataUsingMisc singleton object and in the second location pointer to RunFuncDataUsingBuffer singleton object.
	// This array is just for optimization in order to avoid "if" statement.
	static RunFuncData<data>* m_miscDataOptionsArr[2];
};


template <class T>
class RunFuncData
{
public:

	/* Initialize the data object. (If needed) */
	virtual  cl_dev_err_code init(T& data, const size_t size, COIPROCESS* pCoiProcess) = 0;

	/* Register the buffer in "coiBuffsArr" and the access flag in "coiBuffsAccessFlag". (If needed - in case of COIBUFFER only) */
	virtual void registerData(T& data, const COI_ACCESS_FLAGS& accessFlag, vector<COIBUFFER>& coiBuffsArr, vector<COI_ACCESS_FLAGS>& coiBuffsAccessFlag) = 0;

	/* return pointer to host ptr which store the data. (if needed) */
	virtual void* getDataPtrForCoiRunFunc(T& data) = 0;
	
	/* return the size of the data. (if needed) */
	virtual uint16_t getDataSizeForCoiRunFunc(const size_t& size) = 0;

	/* Read the data and store it in 'pOutMiscData'. */
	virtual cl_dev_err_code readData(T& data, void* pOutMiscData, const size_t size) = 0;

	/* Release the Data object. (if needed) */
	virtual void release(T& data) = 0;
};

template <class T>
class RunFuncDataUsingMisc : public RunFuncData<T>
{
public:

	/* In case of RunFuncDataUsingMisc there is nothing to do. */
	cl_dev_err_code init(T& data, const size_t size, COIPROCESS* pCoiProcess) { return CL_DEV_SUCCESS; };

	/* In case of RunFuncDataUsingMisc there is nothing to do. Because We send the data on Misc area and not as COIBuffer. */
	void registerData(T& data, const COI_ACCESS_FLAGS& accessFlag, vector<COIBUFFER>& coiBuffsArr, vector<COI_ACCESS_FLAGS>& coiBuffsAccessFlag) {};

	/* return pointer to host data (to be a pointer for return value of COIPipelineRunFunction)*/
	void* getDataPtrForCoiRunFunc(T& data) { return data.pDataBuffer; };
	
	/* return the size of the data (size of return value for COIPipelineRunFunction)*/
	uint16_t getDataSizeForCoiRunFunc(const size_t& size) { return (uint16_t)size; };

	/* Read the data and store it in 'pOutMiscData'. */
	cl_dev_err_code readData(T& data, void* pOutMiscData, const size_t size);

	/* In case of RunFuncDataUsingMisc there is nothing to do. */
	void release(T& data) {};

	/* Return singleton instance of this object. */
	static RunFuncData<T>& getInstance() { return m_singletonRunFuncData; };

private:

	RunFuncDataUsingMisc() {};

	static RunFuncDataUsingMisc<T> m_singletonRunFuncData;
};

template <class T>
class RunFuncDataUsingBuffer : public RunFuncData<T>
{
public:

	/* Create a new COIBUFFER. TODO - consider using pool of COIBUFFERs instead of creating each every time. */
	cl_dev_err_code init(T& data, const size_t size, COIPROCESS* pCoiProcess);

	/* Add the created COIBUFFER to "coiBuffsArr" and set the access flag to "COI_SINK_WRITE_ENTIRE", All the arguments are out parameters - reference. */
	void registerData(T& data, const COI_ACCESS_FLAGS& accessFlag, vector<COIBUFFER>& coiBuffsArr, vector<COI_ACCESS_FLAGS>& coiBuffsAccessFlag);

	/* In case of RunFuncDataUsingBuffer there is nothing to do. */
	void* getDataPtrForCoiRunFunc(T& data) { return NULL; };
	
	/* In case of RunFuncDataUsingBuffer there is nothing to do. */
	uint16_t getDataSizeForCoiRunFunc(const size_t& size) { return 0; };

	/* Read the data and store it in 'pOutMiscData'. */
	cl_dev_err_code readData(T& data, void* pOutMiscData, const size_t size);

	/* Release the created COIBUFFER. */
	void release(T& data);

	/* Return singleton instance of this object. */
	static RunFuncData<T>& getInstance() { return m_singletonRunFuncData; };

private:

	RunFuncDataUsingBuffer() {};

	static RunFuncDataUsingBuffer<T> m_singletonRunFuncData;
};


}}}

#include "exe_cmd_mem_handler.hpp"
